#include "WebSocketLogger.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

WebSocketLogger::WebSocketLogger()
{
    this->connected_socket_client = async_resp_arg{
        .hd = nullptr,
        .fd = -1,
    };
}

void LoggerHelpers::ws_async_send(void* arg)
{
    auto* message = static_cast<ws_log_message*>(arg);

    auto websocket_packet = httpd_ws_frame_t{};

    websocket_packet.payload = reinterpret_cast<uint8_t*>(message->text);
    websocket_packet.len = message->len;
    websocket_packet.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(message->client.hd, message->client.fd, &websocket_packet);
    free(message);
}

esp_err_t WebSocketLogger::log_message(const char* format, va_list args)
{
    // take a snapshot, the client may get unregistered while we're formatting
    const auto client = connected_socket_client;
    if (client.fd == -1 || client.hd == nullptr)
    {
        return ESP_FAIL;
    }

    // we can be called from any task, some with small stacks, so measure first and format straight into the heap.
    // Work on copies of args, the caller may still want to use them (e.g. to also vprintf the message)
    va_list measure_args;
    va_copy(measure_args, args);
    const int needed = vsnprintf(nullptr, 0, format, measure_args);
    va_end(measure_args);
    if (needed <= 0)
    {
        return ESP_FAIL;
    }

    const size_t len = std::min(static_cast<size_t>(needed), static_cast<size_t>(WS_LOG_BUFFER_LEN - 1));
    // the text lives right after the struct in the same allocation, so a single free() releases both
    auto* message = static_cast<ws_log_message*>(malloc(sizeof(ws_log_message) + len + 1));
    if (message == nullptr)
    {
        return ESP_ERR_NO_MEM;
    }

    message->client = client;
    message->len = len;
    message->text = reinterpret_cast<char*>(message + 1);

    va_list format_args;
    va_copy(format_args, args);
    vsnprintf(message->text, len + 1, format, format_args);
    va_end(format_args);

    // the message is owned by the work item from now on, ws_async_send frees it
    esp_err_t ret = httpd_queue_work(client.hd, LoggerHelpers::ws_async_send, message);
    if (ret != ESP_OK)
    {
        free(message);
        connected_socket_client.fd = -1;
        connected_socket_client.hd = nullptr;
    }

    return ret;
}

esp_err_t WebSocketLogger::register_socket_client(httpd_req_t* req)
{
    if (connected_socket_client.fd != -1 && connected_socket_client.hd != nullptr)
    {
        // we're already connected
        return ESP_OK;
    }

    connected_socket_client.hd = req->handle;
    connected_socket_client.fd = httpd_req_to_sockfd(req);
    return ESP_OK;
}

void WebSocketLogger::unregister_socket_client()
{
    connected_socket_client.fd = -1;
    connected_socket_client.hd = nullptr;
}
