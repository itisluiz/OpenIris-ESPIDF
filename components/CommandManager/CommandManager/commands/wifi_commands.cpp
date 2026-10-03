#include "wifi_commands.hpp"
#include <cctype>
#include <string_view>
#include "esp_netif.h"
#include "sdkconfig.h"

// format is XX:XX:XX:XX:XX:XX, an empty string means we don't care about the BSSID
static bool isValidBSSID(std::string_view bssid)
{
    if (bssid.empty())
    {
        return true;
    }

    if (bssid.length() != 17)
    {
        return false;
    }

    for (size_t i = 0; i < bssid.length(); i++)
    {
        const bool is_separator_position = i % 3 == 2;
        if (is_separator_position ? bssid[i] != ':' : !std::isxdigit(static_cast<unsigned char>(bssid[i])))
        {
            return false;
        }
    }

    return true;
}

CommandResult setWiFiCommand(std::shared_ptr<DependencyRegistry> registry, const nlohmann::json& json)
{
#if !CONFIG_GENERAL_ENABLE_WIRELESS
    return CommandResult::getErrorResult("Not supported by current firmware");
#endif

    auto payload = json.get<WifiPayload>();
    if (!payload.is_valid)
    {
        return CommandResult::getErrorResult("Invalid payload");
    }

    if (payload.name.empty())
    {
        payload.name = std::string("main");
    }

    if (payload.ssid.empty())
    {
        return CommandResult::getErrorResult("Invalid payload: missing SSID");
    }

    const std::string bssid = payload.bssid.has_value() ? payload.bssid.value() : "";
    if (!isValidBSSID(bssid))
    {
        return CommandResult::getErrorResult("BSSID is malformed");
    }

    std::shared_ptr<ProjectConfig> projectConfig = registry->resolve<ProjectConfig>(DependencyType::project_config);
    projectConfig->setWifiConfig(payload.name, payload.ssid, bssid, payload.password, payload.channel, payload.power);

    return CommandResult::getSuccessResult("Config updated");
}

CommandResult deleteWiFiCommand(std::shared_ptr<DependencyRegistry> registry, const nlohmann::json& json)
{
#if !CONFIG_GENERAL_ENABLE_WIRELESS
    return CommandResult::getErrorResult("Not supported by current firmware");
#endif

    const auto payload = json.get<deleteNetworkPayload>();
    if (!payload.is_valid || payload.name.empty())
        return CommandResult::getErrorResult("Invalid payload");

    auto projectConfig = registry->resolve<ProjectConfig>(DependencyType::project_config);

    projectConfig->deleteWifiConfig(payload.name);
    return CommandResult::getSuccessResult("Config updated");
}

CommandResult updateWiFiCommand(std::shared_ptr<DependencyRegistry> registry, const nlohmann::json& json)
{
#if !CONFIG_GENERAL_ENABLE_WIRELESS
    return CommandResult::getErrorResult("Not supported by current firmware");
#endif

    auto payload = json.get<UpdateWifiPayload>();
    if (!payload.is_valid)
    {
        return CommandResult::getErrorResult("Invalid payload");
    }

    if (payload.name.empty())
    {
        return CommandResult::getErrorResult("Invalid payload - missing network name");
    }

    if (payload.bssid.has_value() && !isValidBSSID(payload.bssid.value()))
    {
        return CommandResult::getErrorResult("BSSID is malformed");
    }

    auto projectConfig = registry->resolve<ProjectConfig>(DependencyType::project_config);
    auto storedNetworks = projectConfig->getWifiConfigs();
    if (const auto networkToUpdate = std::ranges::find_if(storedNetworks, [&](auto& network) { return network.name == payload.name; });
        networkToUpdate != storedNetworks.end())
    {
        projectConfig->setWifiConfig(payload.name, payload.ssid.has_value() ? payload.ssid.value() : networkToUpdate->ssid,
                                     payload.bssid.has_value() ? payload.bssid.value() : networkToUpdate->bssid,
                                     payload.password.has_value() ? payload.password.value() : networkToUpdate->password,
                                     payload.channel.has_value() ? payload.channel.value() : networkToUpdate->channel,
                                     payload.power.has_value() ? payload.power.value() : networkToUpdate->power);

        return CommandResult::getSuccessResult("Config updated");
    }
    else
        return CommandResult::getErrorResult("Requested network does not exist");
}

CommandResult updateAPWiFiCommand(std::shared_ptr<DependencyRegistry> registry, const nlohmann::json& json)
{
#if !CONFIG_GENERAL_ENABLE_WIRELESS
    return CommandResult::getErrorResult("Not supported by current firmware");
#endif

    const auto payload = json.get<UpdateAPWiFiPayload>();
    if (!payload.is_valid)
    {
        return CommandResult::getErrorResult("Invalid payload");
    }

    auto projectConfig = registry->resolve<ProjectConfig>(DependencyType::project_config);
    const auto previousAPConfig = projectConfig->getAPWifiConfig();

    const auto ssid = payload.ssid.has_value() ? payload.ssid.value() : previousAPConfig.ssid;
    const auto password = payload.password.has_value() ? payload.password.value() : previousAPConfig.password;
    const auto channel = payload.channel.has_value() ? payload.channel.value() : previousAPConfig.channel;

    // these are the limits of the ESP soft-AP config, the AP is started from these values on the next boot
    if (ssid.empty() || ssid.length() > 32)
    {
        return CommandResult::getErrorResult("Invalid payload - AP SSID must be between 1 and 32 characters");
    }

    if (!password.empty() && (password.length() < 8 || password.length() > 63))
    {
        return CommandResult::getErrorResult("Invalid payload - AP password must be empty or between 8 and 63 characters");
    }

    if (channel > 13)
    {
        return CommandResult::getErrorResult("Invalid payload - AP channel must be between 0 (auto) and 13");
    }

    projectConfig->setAPWifiConfig(ssid, password, channel);

    return CommandResult::getSuccessResult("Config updated");
}

CommandResult getWiFiStatusCommand(std::shared_ptr<DependencyRegistry> registry)
{
#if !CONFIG_GENERAL_ENABLE_WIRELESS
    return CommandResult::getErrorResult("Not supported by current firmware");
#endif

    auto wifiManager = registry->resolve<WiFiManager>(DependencyType::wifi_manager);
    auto projectConfig = registry->resolve<ProjectConfig>(DependencyType::project_config);

    auto wifiState = wifiManager->GetCurrentWiFiState();
    auto networks = projectConfig->getWifiConfigs();
    nlohmann::json result;

    switch (wifiState)
    {
    case WiFiState_e::WiFiState_NotInitialized:
        result["status"] = "not_initialized";
        break;
    case WiFiState_e::WiFiState_Initialized:
        result["status"] = "initialized";
        break;
    case WiFiState_e::WiFiState_ReadyToConnect:
        result["status"] = "ready";
        break;
    case WiFiState_e::WiFiState_Connecting:
        result["status"] = "connecting";
        break;
    case WiFiState_e::WiFiState_WaitingForIp:
        result["status"] = "waiting_for_ip";
        break;
    case WiFiState_e::WiFiState_Connected:
        result["status"] = "connected";
        break;
    case WiFiState_e::WiFiState_Disconnected:
        result["status"] = "disconnected";
        break;
    case WiFiState_e::WiFiState_Error:
        result["status"] = "error";
        break;
    }

    result["networks_configured"] = networks.size();

    if (wifiState == WiFiState_e::WiFiState_Connected)
    {
        // Get IP address from ESP32
        esp_netif_ip_info_t ip_info;
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
        {
            char ip_str[16];
            sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));
            result["ip_address"] = ip_str;
        }
    }

    return CommandResult::getSuccessResult(result);
}

CommandResult connectWiFiCommand(std::shared_ptr<DependencyRegistry> registry)
{
#if !CONFIG_GENERAL_ENABLE_WIRELESS
    return CommandResult::getErrorResult("Not supported by current firmware");
#endif

    auto wifiManager = registry->resolve<WiFiManager>(DependencyType::wifi_manager);
    auto projectConfig = registry->resolve<ProjectConfig>(DependencyType::project_config);

    auto networks = projectConfig->getWifiConfigs();
    if (networks.empty())
    {
        return CommandResult::getErrorResult("No WiFi networks configured");
    }

    // Trigger WiFi connection attempt
    wifiManager->TryConnectToStoredNetworks();

    return CommandResult::getSuccessResult("WiFi connection attempt started");
}
