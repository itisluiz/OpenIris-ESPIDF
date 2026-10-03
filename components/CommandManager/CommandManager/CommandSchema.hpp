#ifndef COMMAND_SCHEMA_HPP
#define COMMAND_SCHEMA_HPP
#include <nlohmann-json.hpp>
#include <optional>
#include <string>
#include <type_traits>

struct BasePayload
{
    // set to false by from_json when a required field is missing or any field has the wrong type
    bool is_valid = true;
};

// Reads j[key] into out, returns false if the key is missing or holds an incompatible type.
// We have to check this ourselves, exceptions are disabled so nlohmann-json would abort() instead of throwing.
template <typename T>
bool readField(const nlohmann::json& j, const char* key, T& out)
{
    if (!j.is_object() || !j.contains(key))
    {
        return false;
    }

    const auto& value = j.at(key);
    if constexpr (std::is_same_v<T, std::string>)
    {
        if (!value.is_string())
            return false;
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        if (!value.is_boolean())
            return false;
    }
    else if constexpr (std::is_arithmetic_v<T>)
    {
        if (!value.is_number())
            return false;
    }
    else
    {
        static_assert(sizeof(T) == 0, "readField: unsupported field type");
    }

    out = value.get<T>();
    return true;
}

// Optional fields may be missing, but if present they still have to be of a compatible type
template <typename T>
bool readField(const nlohmann::json& j, const char* key, std::optional<T>& out)
{
    if (!j.is_object() || !j.contains(key))
    {
        return true;
    }

    T value;
    if (!readField(j, key, value))
    {
        return false;
    }

    out = value;
    return true;
}

struct WifiPayload : BasePayload
{
    std::string name;
    std::string ssid;
    std::optional<std::string> bssid;
    std::string password;
    uint8_t channel;
    uint8_t power;
};

void to_json(nlohmann::json& j, const WifiPayload& payload);
void from_json(const nlohmann::json& j, WifiPayload& payload);

struct UpdateWifiPayload : BasePayload
{
    std::string name;
    std::optional<std::string> ssid;
    std::optional<std::string> bssid;
    std::optional<std::string> password;
    std::optional<uint8_t> channel;
    std::optional<uint8_t> power;
};

void to_json(nlohmann::json& j, const UpdateWifiPayload& payload);
void from_json(const nlohmann::json& j, UpdateWifiPayload& payload);
struct deleteNetworkPayload : BasePayload
{
    std::string name;
};

void to_json(nlohmann::json& j, const deleteNetworkPayload& payload);
void from_json(const nlohmann::json& j, deleteNetworkPayload& payload);

struct UpdateAPWiFiPayload : BasePayload
{
    std::optional<std::string> ssid;
    std::optional<std::string> password;
    std::optional<uint8_t> channel;
};

void to_json(nlohmann::json& j, const UpdateAPWiFiPayload& payload);
void from_json(const nlohmann::json& j, UpdateAPWiFiPayload& payload);
struct MDNSPayload : BasePayload
{
    std::string hostname;
};

void to_json(nlohmann::json& j, const MDNSPayload& payload);
void from_json(const nlohmann::json& j, MDNSPayload& payload);

struct UpdateCameraConfigPayload : BasePayload
{
    std::optional<uint8_t> vflip;
    std::optional<uint8_t> href;
    std::optional<uint8_t> framesize;
    std::optional<uint8_t> quality;
    std::optional<uint8_t> brightness;
    // TODO add more options here
};

void to_json(nlohmann::json& j, const UpdateCameraConfigPayload& payload);
void from_json(const nlohmann::json& j, UpdateCameraConfigPayload& payload);
#endif