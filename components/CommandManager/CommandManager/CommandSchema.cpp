#include "CommandSchema.hpp"

void to_json(nlohmann::json& j, const WifiPayload& payload)
{
    j = nlohmann::json{
        {"name", payload.name},         {"ssid", payload.ssid},       {"bssid", payload.bssid},
        {"password", payload.password}, {"channel", payload.channel}, {"power", payload.power},
    };
}

void from_json(const nlohmann::json& j, WifiPayload& payload)
{
    payload.is_valid = readField(j, "name", payload.name) && readField(j, "ssid", payload.ssid) && readField(j, "password", payload.password) &&
                       readField(j, "channel", payload.channel) && readField(j, "power", payload.power) && readField(j, "bssid", payload.bssid);
}

void to_json(nlohmann::json& j, const UpdateWifiPayload& payload)
{
    j = nlohmann::json{
        {"name", payload.name},         {"ssid", payload.ssid},       {"bssid", payload.bssid},
        {"password", payload.password}, {"channel", payload.channel}, {"power", payload.power},
    };
}

void from_json(const nlohmann::json& j, UpdateWifiPayload& payload)
{
    payload.is_valid = readField(j, "name", payload.name) && readField(j, "ssid", payload.ssid) && readField(j, "bssid", payload.bssid) &&
                       readField(j, "password", payload.password) && readField(j, "channel", payload.channel) && readField(j, "power", payload.power);
}

void to_json(nlohmann::json& j, const deleteNetworkPayload& payload)
{
    j = nlohmann::json{{"name", payload.name}};
}

void from_json(const nlohmann::json& j, deleteNetworkPayload& payload)
{
    payload.is_valid = readField(j, "name", payload.name);
}

void to_json(nlohmann::json& j, const UpdateAPWiFiPayload& payload)
{
    j = nlohmann::json{{"ssid", payload.ssid}, {"password", payload.password}, {"channel", payload.channel}};
}

void from_json(const nlohmann::json& j, UpdateAPWiFiPayload& payload)
{
    payload.is_valid = readField(j, "ssid", payload.ssid) && readField(j, "password", payload.password) && readField(j, "channel", payload.channel);
}

void to_json(nlohmann::json& j, const UpdateCameraConfigPayload& payload)
{
    j = nlohmann::json{
        {"vflip", payload.vflip}, {"href", payload.href}, {"framesize", payload.framesize}, {"quality", payload.quality}, {"brightness", payload.brightness},
    };
}

void from_json(const nlohmann::json& j, UpdateCameraConfigPayload& payload)
{
    payload.is_valid = readField(j, "vflip", payload.vflip) && readField(j, "href", payload.href) && readField(j, "framesize", payload.framesize) &&
                       readField(j, "quality", payload.quality) && readField(j, "brightness", payload.brightness);
}

void to_json(nlohmann::json& j, const MDNSPayload& payload)
{
    j = nlohmann::json{{"hostname", payload.hostname}};
}

void from_json(const nlohmann::json& j, MDNSPayload& payload)
{
    payload.is_valid = readField(j, "hostname", payload.hostname);
}