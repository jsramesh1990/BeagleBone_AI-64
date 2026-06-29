#include "device-manager/DeviceManagerConfig.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <random>

using namespace device_manager;

/**
 * @brief Generate a unique instance ID
 */
static std::string generateInstanceId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9999);
    
    return "dm_" + std::to_string(timestamp) + "_" + std::to_string(dis(gen));
}

/**
 * @brief Load configuration from file
 */
DeviceManagerConfig DeviceManagerConfig::fromJson(const Json::Value& json) {
    DeviceManagerConfig config;
    
    // Discovery
    if (json.isMember("discovery")) {
        config.discovery = DiscoveryConfig::fromJson(json["discovery"]);
    } else {
        // Set defaults
        config.discovery.enabled = true;
        config.discovery.intervalSeconds = 30;
        config.discovery.timeoutMs = 5000;
        config.discovery.scanOnStart = true;
        config.discovery.scanI2C = true;
        config.discovery.scanSPI = true;
        config.discovery.scanUSB = true;
        config.discovery.maxDevices = 100;
        config.discovery.autoAddDiscovered = true;
    }
    
    // Monitoring
    if (json.isMember("monitoring")) {
        config.monitoring = MonitoringConfig::fromJson(json["monitoring"]);
    } else {
        config.monitoring.enabled = true;
        config.monitoring.intervalMs = 1000;
        config.monitoring.monitorStatus = true;
        config.monitoring.monitorData = true;
        config.monitoring.maxSamples = 1000;
        config.monitoring.autoRecover = true;
        config.monitoring.recoveryAttempts = 3;
        config.monitoring.healthThreshold = 0.8;
    }
    
    // Defaults
    if (json.isMember("defaults")) {
        config.defaults = DeviceDefaults::fromJson(json["defaults"]);
    } else {
        config.defaults.autoStart = true;
        config.defaults.timeoutMs = 5000;
        config.defaults.retryCount = 3;
        config.defaults.retryDelayMs = 1000;
        config.defaults.maxDataAgeMs = 60000;
    }
    
    // Storage
    if (json.isMember("storage")) {
        config.storage = StorageConfig::fromJson(json["storage"]);
    } else {
        config.storage.enabled = true;
        config.storage.dataPath = "/var/lib/devices/";
        config.storage.configPath = "/etc/devices/";
        config.storage.logPath = "/var/log/devices/";
        config.storage.autoSave = true;
        config.storage.saveIntervalMs = 60000;
        config.storage.maxHistoryDays = 30;
        config.storage.compressData = true;
        config.storage.maxFileSizeMB = 100;
    }
    
    // Security
    if (json.isMember("security")) {
        config.security = SecurityConfig::fromJson(json["security"]);
    } else {
        config.security.enabled = true;
        config.security.requireAuthentication = false;
        config.security.encryptCommunication = true;
        config.security.enableAuditLog = true;
        config.security.auditLogPath = "/var/log/device-audit.log";
    }
    
    // Logging
    if (json.isMember("logging")) {
        config.logging = LoggingConfig::fromJson(json["logging"]);
    } else {
        config.logging.enabled = true;
        config.logging.level = "INFO";
        config.logging.logToFile = true;
        config.logging.logFile = "/var/log/device-manager.log";
        config.logging.logToSyslog = true;
        config.logging.logToConsole = true;
        config.logging.maxLogSizeMB = 100;
        config.logging.maxLogFiles = 5;
    }
    
    // General
    if (json.isMember("version")) {
        config.version = json["version"].asString();
    }
    
    if (json.isMember("instanceId")) {
        config.instanceId = json["instanceId"].asString();
    } else {
        config.instanceId = generateInstanceId();
    }
    
    if (json.isMember("environment")) {
        config.environment = json["environment"].asString();
    }
    
    if (json.isMember("debugMode")) {
        config.debugMode = json["debugMode"].asBool();
    }
    
    if (json.isMember("shutdownTimeoutMs")) {
        config.shutdownTimeoutMs = json["shutdownTimeoutMs"].asInt();
    }
    
    if (json.isMember("custom")) {
        for (const auto& key : json["custom"].getMemberNames()) {
            config.customSettings[key] = json["custom"][key];
        }
    }
    
    return config;
}

/**
 * @brief Convert configuration to JSON
 */
Json::Value DeviceManagerConfig::toJson() const {
    Json::Value json;
    
    json["discovery"] = discovery.toJson();
    json["monitoring"] = monitoring.toJson();
    json["defaults"] = defaults.toJson();
    json["storage"] = storage.toJson();
    json["security"] = security.toJson();
    json["logging"] = logging.toJson();
    
    json["version"] = version;
    json["instanceId"] = instanceId;
    json["environment"] = environment;
    json["debugMode"] = debugMode;
    json["shutdownTimeoutMs"] = shutdownTimeoutMs;
    
    for (const auto& [key, value] : customSettings) {
        json["custom"][key] = value;
    }
    
    return json;
}

/**
 * @brief Validate configuration
 */
bool DeviceManagerConfig::validate() const {
    bool valid = true;
    
    // Validate discovery
    if (discovery.maxDevices <= 0) {
        std::cerr << "Invalid discovery.maxDevices: " << discovery.maxDevices << std::endl;
        valid = false;
    }
    if (discovery.intervalSeconds <= 0) {
        std::cerr << "Invalid discovery.intervalSeconds: " << discovery.intervalSeconds << std::endl;
        valid = false;
    }
    if (discovery.timeoutMs <= 0) {
        std::cerr << "Invalid discovery.timeoutMs: " << discovery.timeoutMs << std::endl;
        valid = false;
    }
    
    // Validate monitoring
    if (monitoring.intervalMs <= 0) {
        std::cerr << "Invalid monitoring.intervalMs: " << monitoring.intervalMs << std::endl;
        valid = false;
    }
    if (monitoring.maxSamples <= 0) {
        std::cerr << "Invalid monitoring.maxSamples: " << monitoring.maxSamples << std::endl;
        valid = false;
    }
    if (monitoring.statsWindowSeconds <= 0) {
        std::cerr << "Invalid monitoring.statsWindowSeconds: " << monitoring.statsWindowSeconds << std::endl;
        valid = false;
    }
    if (monitoring.healthCheckIntervalMs <= 0) {
        std::cerr << "Invalid monitoring.healthCheckIntervalMs: " << monitoring.healthCheckIntervalMs << std::endl;
        valid = false;
    }
    if (monitoring.healthThreshold < 0 || monitoring.healthThreshold > 1) {
        std::cerr << "Invalid monitoring.healthThreshold: " << monitoring.healthThreshold << std::endl;
        valid = false;
    }
    if (monitoring.recoveryAttempts < 0) {
        std::cerr << "Invalid monitoring.recoveryAttempts: " << monitoring.recoveryAttempts << std::endl;
        valid = false;
    }
    
    // Validate defaults
    if (defaults.timeoutMs <= 0) {
        std::cerr << "Invalid defaults.timeoutMs: " << defaults.timeoutMs << std::endl;
        valid = false;
    }
    if (defaults.retryCount < 0) {
        std::cerr << "Invalid defaults.retryCount: " << defaults.retryCount << std::endl;
        valid = false;
    }
    if (defaults.retryDelayMs <= 0) {
        std::cerr << "Invalid defaults.retryDelayMs: " << defaults.retryDelayMs << std::endl;
        valid = false;
    }
    if (defaults.maxDataAgeMs <= 0) {
        std::cerr << "Invalid defaults.maxDataAgeMs: " << defaults.maxDataAgeMs << std::endl;
        valid = false;
    }
    
    // Validate storage
    if (storage.saveIntervalMs <= 0) {
        std::cerr << "Invalid storage.saveIntervalMs: " << storage.saveIntervalMs << std::endl;
        valid = false;
    }
    if (storage.maxHistoryDays <= 0) {
        std::cerr << "Invalid storage.maxHistoryDays: " << storage.maxHistoryDays << std::endl;
        valid = false;
    }
    if (storage.maxFileSizeMB <= 0) {
        std::cerr << "Invalid storage.maxFileSizeMB: " << storage.maxFileSizeMB << std::endl;
        valid = false;
    }
    if (storage.backupIntervalHours <= 0) {
        std::cerr << "Invalid storage.backupIntervalHours: " << storage.backupIntervalHours << std::endl;
        valid = false;
    }
    if (storage.maxBackups <= 0) {
        std::cerr << "Invalid storage.maxBackups: " << storage.maxBackups << std::endl;
        valid = false;
    }
    
    // Validate security
    if (security.enabled && security.requireAuthentication && security.authTokens.empty()) {
        std::cerr << "Invalid security: authentication required but no tokens defined" << std::endl;
        valid = false;
    }
    
    // Validate logging
    if (logging.maxLogSizeMB <= 0) {
        std::cerr << "Invalid logging.maxLogSizeMB: " << logging.maxLogSizeMB << std::endl;
        valid = false;
    }
    if (logging.maxLogFiles <= 0) {
        std::cerr << "Invalid logging.maxLogFiles: " << logging.maxLogFiles << std::endl;
        valid = false;
    }
    
    // Validate custom settings
    // No specific validation for custom settings - they're application specific
    
    return valid;
}

/**
 * @brief Load configuration from file
 */
bool DeviceManagerConfig::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << path << std::endl;
        return false;
    }
    
    Json::Value json;
    Json::CharReaderBuilder builder;
    std::string errs;
    
    if (!Json::parseFromStream(builder, file, &json, &errs)) {
        std::cerr << "Failed to parse config file: " << errs << std::endl;
        file.close();
        return false;
    }
    file.close();
    
    *this = fromJson(json);
    return validate();
}

/**
 * @brief Save configuration to file
 */
bool DeviceManagerConfig::saveToFile(const std::string& path) const {
    Json::Value json = toJson();
    
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file for writing: " << path << std::endl;
        return false;
    }
    
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(json, &file);
    file.close();
    
    return true;
}

/**
 * @brief Merge with another configuration
 */
DeviceManagerConfig DeviceManagerConfig::merge(const DeviceManagerConfig& other) const {
    DeviceManagerConfig merged = *this;
    
    // Merge discovery
    if (other.discovery.enabled) {
        merged.discovery = other.discovery;
    }
    
    // Merge monitoring
    if (other.monitoring.enabled) {
        merged.monitoring = other.monitoring;
    }
    
    // Merge defaults
    merged.defaults = other.defaults;
    
    // Merge storage
    if (other.storage.enabled) {
        merged.storage = other.storage;
    }
    
    // Merge security
    if (other.security.enabled) {
        merged.security = other.security;
    }
    
    // Merge logging
    if (other.logging.enabled) {
        merged.logging = other.logging;
    }
    
    // Merge custom settings
    for (const auto& [key, value] : other.customSettings) {
        merged.customSettings[key] = value;
    }
    
    // Use other's general settings if they're not empty
    if (!other.version.empty()) {
        merged.version = other.version;
    }
    if (!other.instanceId.empty()) {
        merged.instanceId = other.instanceId;
    }
    if (!other.environment.empty()) {
        merged.environment = other.environment;
    }
    
    return merged;
}
