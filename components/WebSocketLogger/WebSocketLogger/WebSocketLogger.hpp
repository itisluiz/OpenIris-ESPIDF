#pragma once
#ifndef WEBSOCKETLOGGER_HPP
#define WEBSOCKETLOGGER_HPP

#include "esp_http_server.h"
#include "esp_log.h"

#define WS_LOG_BUFFER_LEN 1024

struct async_resp_arg
{
    httpd_handle_t hd;
    int fd;
};

// a single formatted log line, allocated per message and freed by the httpd work item that sends it
struct ws_log_message
{
    async_resp_arg client;
    size_t len;
    char* text;
};

namespace LoggerHelpers
{
void ws_async_send(void* arg);
}

class WebSocketLogger
{
    async_resp_arg connected_socket_client{};

   public:
    WebSocketLogger();

    esp_err_t log_message(const char* format, va_list args);
    esp_err_t register_socket_client(httpd_req_t* req);
    void unregister_socket_client();
    bool is_client_connected();
};

extern WebSocketLogger webSocketLogger;

#endif