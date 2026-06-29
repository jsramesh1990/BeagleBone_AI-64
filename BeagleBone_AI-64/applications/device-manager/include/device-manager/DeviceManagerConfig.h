#ifndef DEVICE_MANAGER_CONFIG_H
#define DEVICE_MANAGER_CONFIG_H

#include <string>
#include <vector>
#include <map>
#include <json/json.h>
#include <chrono>

namespace device_manager {

/**
 * @brief Device Manager Configuration
 * 
 * Configuration structure for the Device Manager with
 * comprehensive settings for discovery, monitoring,
 * and device management
 */
struct DeviceManagerConfig {
    /**
     * @brief Discovery configuration
     */
    struct DiscoveryConfig {
        bool enabled = true;
        int intervalSeconds = 30;
        int timeoutMs = 5000;
        bool scanOnStart = true;
        std::vector<std::string> scanPaths;
        std::vector<std::string> scanBuses;
        bool scanI2C = true;
        bool scanSPI = true;
        bool scanUSB = true;
        bool scanPCI = true;
        bool scanNetwork = true;
        std::map<std::string, std::string> scanParams;
        int maxDevices = 100;
        bool filterDuplicates = true;
        bool autoAddDiscovered = true;
        std::vector<std::string> deviceFilters;
        std::vector<std::string> deviceExcludes;
        
        Json::Value toJson() const {
            Json::Value json;
            json["enabled"] = enabled;
            json["intervalSeconds"] = intervalSeconds;
            json["timeoutMs"] = timeoutMs;
            json["scanOnStart"] = scanOnStart;
            
            Json::Value paths(Json::arrayValue);
            for (const auto& path : scanPaths) {
                paths.append(path);
            }
            json["scanPaths"] = paths;
            
            Json::Value buses(Json::arrayValue);
            for (const auto& bus : scanBuses) {
                buses.append(bus);
            }
            json["scanBuses"] = buses;
            
            json["scanI2C"] = scanI2C;
            json["scanSPI"] = scanSPI;
            json["scanUSB"] = scanUSB;
            json["scanPCI"] = scanPCI;
            json["scanNetwork"] = scanNetwork;
            
            for (const auto& [key, value] : scanParams) {
                json["scanParams"][key] = value;
            }
            
            json["maxDevices"] = maxDevices;
            json["filterDuplicates"] = filterDuplicates;
            json["autoAddDiscovered"] = autoAddDiscovered;
            
            Json::Value filters(Json::arrayValue);
            for (const auto& filter : deviceFilters) {
                filters.append(filter);
            }
            json["deviceFilters"] = filters;
            
            Json::Value excludes(Json::arrayValue);
            for (const auto& exclude : deviceExcludes) {
                excludes.append(exclude);
            }
            json["deviceExcludes"] = excludes;
            
            return json;
        }
        
        static DiscoveryConfig fromJson(const Json::Value& json) {
            DiscoveryConfig config;
            if (json.isMember("enabled")) config.enabled = json["enabled"].asBool();
            if (json.isMember("intervalSeconds")) config.intervalSeconds = json["intervalSeconds"].asInt();
            if (json.isMember("timeoutMs")) config.timeoutMs = json["timeoutMs"].asInt();
            if (json.isMember("scanOnStart")) config.scanOnStart = json["scanOnStart"].asBool();
            
            if (json.isMember("scanPaths") && json["scanPaths"].isArray()) {
                for (const auto& path : json["scanPaths"]) {
                    config.scanPaths.push_back(path.asString());
                }
            }
            
            if (json.isMember("scanBuses") && json["scanBuses"].isArray()) {
                for (const auto& bus : json["scanBuses"]) {
                    config.scanBuses.push_back(bus.asString());
                }
            }
            
            if (json.isMember("scanI2C")) config.scanI2C = json["scanI2C"].asBool();
            if (json.isMember("scanSPI")) config.scanSPI = json["scanSPI"].asBool();
            if (json.isMember("scanUSB")) config.scanUSB = json["scanUSB"].asBool();
            if (json.isMember("scanPCI")) config.scanPCI = json["scanPCI"].asBool();
            if (json.isMember("scanNetwork")) config.scanNetwork = json["scanNetwork"].asBool();
            
            if (json.isMember("scanParams")) {
                for (const auto& key : json["scanParams"].getMemberNames()) {
                    config.scanParams[key] = json["scanParams"][key].asString();
                }
            }
            
            if (json.isMember("maxDevices")) config.maxDevices = json["maxDevices"].asInt();
            if (json.isMember("filterDuplicates")) config.filterDuplicates = json["filterDuplicates"].asBool();
            if (json.isMember("autoAddDiscovered")) config.autoAddDiscovered = json["autoAddDiscovered"].asBool();
            
            if (json.isMember("deviceFilters") && json["deviceFilters"].isArray()) {
                for (const auto& filter : json["deviceFilters"]) {
                    config.deviceFilters.push_back(filter.asString());
                }
            }
            
            if (json.isMember("deviceExcludes") && json["deviceExcludes"].isArray()) {
                for (const auto& exclude : json["deviceExcludes"]) {
                    config.deviceExcludes.push_back(exclude.asString());
                }
            }
            
            return config;
        }
    };
    
    /**
     * @brief Monitoring configuration
     */
    struct MonitoringConfig {
        bool enabled = true;
        int intervalMs = 1000;
        bool monitorStatus = true;
        bool monitorData = true;
        bool monitorErrors = true;
        int maxSamples = 1000;
        bool aggregateStats = true;
        int statsWindowSeconds = 300;
        std::map<std::string, double> thresholds;
        std::vector<std::string> monitoredDevices;
        bool autoRecover = true;
        int recoveryAttempts = 3;
        int recoveryDelayMs = 5000;
        bool logHealth = true;
        int healthCheckIntervalMs = 5000;
        double healthThreshold = 0.8;
        
        Json::Value toJson() const {
            Json::Value json;
            json["enabled"] = enabled;
            json["intervalMs"] = intervalMs;
            json["monitorStatus"] = monitorStatus;
            json["monitorData"] = monitorData;
            json["monitorErrors"] = monitorErrors;
            json["maxSamples"] = maxSamples;
            json["aggregateStats"] = aggregateStats;
            json["statsWindowSeconds"] = statsWindowSeconds;
            
            for (const auto& [key, value] : thresholds) {
                json["thresholds"][key] = value;
            }
            
            Json::Value devices(Json::arrayValue);
            for (const auto& device : monitoredDevices) {
                devices.append(device);
            }
            json["monitoredDevices"] = devices;
            
            json["autoRecover"] = autoRecover;
            json["recoveryAttempts"] = recoveryAttempts;
            json["recoveryDelayMs"] = recoveryDelayMs;
            json["logHealth"] = logHealth;
            json["healthCheckIntervalMs"] = healthCheckIntervalMs;
            json["healthThreshold"] = healthThreshold;
            
            return json;
        }
        
        static MonitoringConfig fromJson(const Json::Value& json) {
            MonitoringConfig config;
            if (json.isMember("enabled")) config.enabled = json["enabled"].asBool();
            if (json.isMember("intervalMs")) config.intervalMs = json["intervalMs"].asInt();
            if (json.isMember("monitorStatus")) config.monitorStatus = json["monitorStatus"].asBool();
            if (json.isMember("monitorData")) config.monitorData = json["monitorData"].asBool();
            if (json.isMember("monitorErrors")) config.monitorErrors = json["monitorErrors"].asBool();
            if (json.isMember("maxSamples")) config.maxSamples = json["maxSamples"].asInt();
            if (json.isMember("aggregateStats")) config.aggregateStats = json["aggregateStats"].asBool();
            if (json.isMember("statsWindowSeconds")) config.statsWindowSeconds = json["statsWindowSeconds"].asInt();
            
            if (json.isMember("thresholds")) {
                for (const auto& key : json["thresholds"].getMemberNames()) {
                    config.thresholds[key] = json["thresholds"][key].asDouble();
                }
            }
            
            if (json.isMember("monitoredDevices") && json["monitoredDevices"].isArray()) {
                for (const auto& device : json["monitoredDevices"]) {
                    config.monitoredDevices.push_back(device.asString());
                }
            }
            
            if (json.isMember("autoRecover")) config.autoRecover = json["autoRecover"].asBool();
            if (json.isMember("recoveryAttempts")) config.recoveryAttempts = json["recoveryAttempts"].asInt();
            if (json.isMember("recoveryDelayMs")) config.recoveryDelayMs = json["recoveryDelayMs"].asInt();
            if (json.isMember("logHealth")) config.logHealth = json["logHealth"].asBool();
            if (json.isMember("healthCheckIntervalMs")) config.healthCheckIntervalMs = json["healthCheckIntervalMs"].asInt();
            if (json.isMember("healthThreshold")) config.healthThreshold = json["healthThreshold"].asDouble();
            
            return config;
        }
    };
    
    /**
     * @brief Device configuration
     */
    struct DeviceDefaults {
        bool autoStart = true;
        bool autoConfigure = true;
        bool enableHealthCheck = true;
        int timeoutMs = 5000;
        int retryCount = 3;
        int retryDelayMs = 1000;
        int maxDataAgeMs = 60000;
        std::map<std::string, std::string> defaultParams;
        std::map<DeviceType, std::map<std::string, std::string>> typeParams;
        
        Json::Value toJson() const {
            Json::Value json;
            json["autoStart"] = autoStart;
            json["autoConfigure"] = autoConfigure;
            json["enableHealthCheck"] = enableHealthCheck;
            json["timeoutMs"] = timeoutMs;
            json["retryCount"] = retryCount;
            json["retryDelayMs"] = retryDelayMs;
            json["maxDataAgeMs"] = maxDataAgeMs;
            
            for (const auto& [key, value] : defaultParams) {
                json["defaultParams"][key] = value;
            }
            
            for (const auto& [type, params] : typeParams) {
                for (const auto& [key, value] : params) {
                    json["typeParams"][std::to_string(static_cast<int>(type))][key] = value;
                }
            }
            
            return json;
        }
        
        static DeviceDefaults fromJson(const Json::Value& json) {
            DeviceDefaults config;
            if (json.isMember("autoStart")) config.autoStart = json["autoStart"].asBool();
            if (json.isMember("autoConfigure")) config.autoConfigure = json["autoConfigure"].asBool();
            if (json.isMember("enableHealthCheck")) config.enableHealthCheck = json["enableHealthCheck"].asBool();
            if (json.isMember("timeoutMs")) config.timeoutMs = json["timeoutMs"].asInt();
            if (json.isMember("retryCount")) config.retryCount = json["retryCount"].asInt();
            if (json.isMember("retryDelayMs")) config.retryDelayMs = json["retryDelayMs"].asInt();
            if (json.isMember("maxDataAgeMs")) config.maxDataAgeMs = json["maxDataAgeMs"].asInt();
            
            if (json.isMember("defaultParams")) {
                for (const auto& key : json["defaultParams"].getMemberNames()) {
                    config.defaultParams[key] = json["defaultParams"][key].asString();
                }
            }
            
            if (json.isMember("typeParams")) {
                for (const auto& typeKey : json["typeParams"].getMemberNames()) {
                    DeviceType type = static_cast<DeviceType>(std::stoi(typeKey));
                    for (const auto& paramKey : json["typeParams"][typeKey].getMemberNames()) {
                        config.typeParams[type][paramKey] = 
                            json["typeParams"][typeKey][paramKey].asString();
                    }
                }
            }
            
            return config;
        }
    };
    
    /**
     * @brief Storage configuration
     */
    struct StorageConfig {
        bool enabled = true;
        std::string dataPath = "/var/lib/devices/";
        std::string configPath = "/etc/devices/";
        std::string logPath = "/var/log/devices/";
        bool autoSave = true;
        int saveIntervalMs = 60000;
        int maxHistoryDays = 30;
        bool compressData = true;
        bool encryptData = false;
        std::string encryptionKey;
        int maxFileSizeMB = 100;
        bool enableBackup = true;
        std::string backupPath = "/backup/devices/";
        int backupIntervalHours = 24;
        int maxBackups = 7;
        
        Json::Value toJson() const {
            Json::Value json;
            json["enabled"] = enabled;
            json["dataPath"] = dataPath;
            json["configPath"] = configPath;
            json["logPath"] = logPath;
            json["autoSave"] = autoSave;
            json["saveIntervalMs"] = saveIntervalMs;
            json["maxHistoryDays"] = maxHistoryDays;
            json["compressData"] = compressData;
            json["encryptData"] = encryptData;
            json["encryptionKey"] = encryptionKey;
            json["maxFileSizeMB"] = maxFileSizeMB;
            json["enableBackup"] = enableBackup;
            json["backupPath"] = backupPath;
            json["backupIntervalHours"] = backupIntervalHours;
            json["maxBackups"] = maxBackups;
            return json;
        }
        
        static StorageConfig fromJson(const Json::Value& json) {
            StorageConfig config;
            if (json.isMember("enabled")) config.enabled = json["enabled"].asBool();
            if (json.isMember("dataPath")) config.dataPath = json["dataPath"].asString();
            if (json.isMember("configPath")) config.configPath = json["configPath"].asString();
            if (json.isMember("logPath")) config.logPath = json["logPath"].asString();
            if (json.isMember("autoSave")) config.autoSave = json["autoSave"].asBool();
            if (json.isMember("saveIntervalMs")) config.saveIntervalMs = json["saveIntervalMs"].asInt();
            if (json.isMember("maxHistoryDays")) config.maxHistoryDays = json["maxHistoryDays"].asInt();
            if (json.isMember("compressData")) config.compressData = json["compressData"].asBool();
            if (json.isMember("encryptData")) config.encryptData = json["encryptData"].asBool();
            if (json.isMember("encryptionKey")) config.encryptionKey = json["encryptionKey"].asString();
            if (json.isMember("maxFileSizeMB")) config.maxFileSizeMB = json["maxFileSizeMB"].asInt();
            if (json.isMember("enableBackup")) config.enableBackup = json["enableBackup"].asBool();
            if (json.isMember("backupPath")) config.backupPath = json["backupPath"].asString();
            if (json.isMember("backupIntervalHours")) config.backupIntervalHours = json["backupIntervalHours"].asInt();
            if (json.isMember("maxBackups")) config.maxBackups = json["maxBackups"].asInt();
            return config;
        }
    };
    
    /**
     * @brief Security configuration
     */
    struct SecurityConfig {
        bool enabled = true;
        bool requireAuthentication = false;
        std::string authMethod = "token";
        std::map<std::string, std::string> authTokens;
        bool encryptCommunication = true;
        bool verifyCertificates = true;
        std::string caCertificate;
        std::string clientCertificate;
        std::string clientKey;
        bool enableAuditLog = true;
        std::string auditLogPath = "/var/log/device-audit.log";
        bool restrictDeviceAccess = false;
        std::map<std::string, std::vector<std::string>> devicePermissions;
        std::vector<std::string> adminUsers;
        bool enableFirewall = false;
        std::vector<std::string> firewallRules;
        
        Json::Value toJson() const {
            Json::Value json;
            json["enabled"] = enabled;
            json["requireAuthentication"] = requireAuthentication;
            json["authMethod"] = authMethod;
            
            for (const auto& [key, value] : authTokens) {
                json["authTokens"][key] = value;
            }
            
            json["encryptCommunication"] = encryptCommunication;
            json["verifyCertificates"] = verifyCertificates;
            json["caCertificate"] = caCertificate;
            json["clientCertificate"] = clientCertificate;
            json["clientKey"] = clientKey;
            json["enableAuditLog"] = enableAuditLog;
            json["auditLogPath"] = auditLogPath;
            json["restrictDeviceAccess"] = restrictDeviceAccess;
            
            for (const auto& [device, permissions] : devicePermissions) {
                Json::Value perms(Json::arrayValue);
                for (const auto& perm : permissions) {
                    perms.append(perm);
                }
                json["devicePermissions"][device] = perms;
            }
            
            Json::Value admins(Json::arrayValue);
            for (const auto& admin : adminUsers) {
                admins.append(admin);
            }
            json["adminUsers"] = admins;
            
            json["enableFirewall"] = enableFirewall;
            Json::Value rules(Json::arrayValue);
            for (const auto& rule : firewallRules) {
                rules.append(rule);
            }
            json["firewallRules"] = rules;
            
            return json;
        }
        
        static SecurityConfig fromJson(const Json::Value& json) {
            SecurityConfig config;
            if (json.isMember("enabled")) config.enabled = json["enabled"].asBool();
            if (json.isMember("requireAuthentication")) config.requireAuthentication = json["requireAuthentication"].asBool();
            if (json.isMember("authMethod")) config.authMethod = json["authMethod"].asString();
            
            if (json.isMember("authTokens")) {
                for (const auto& key : json["authTokens"].getMemberNames()) {
                    config.authTokens[key] = json["authTokens"][key].asString();
                }
            }
            
            if (json.isMember("encryptCommunication")) config.encryptCommunication = json["encryptCommunication"].asBool();
            if (json.isMember("verifyCertificates")) config.verifyCertificates = json["verifyCertificates"].asBool();
            if (json.isMember("caCertificate")) config.caCertificate = json["caCertificate"].asString();
            if (json.isMember("clientCertificate")) config.clientCertificate = json["clientCertificate"].asString();
            if (json.isMember("clientKey")) config.clientKey = json["clientKey"].asString();
            if (json.isMember("enableAuditLog")) config.enableAuditLog = json["enableAuditLog"].asBool();
            if (json.isMember("auditLogPath")) config.auditLogPath = json["auditLogPath"].asString();
            if (json.isMember("restrictDeviceAccess")) config.restrictDeviceAccess = json["restrictDeviceAccess"].asBool();
            
            if (json.isMember("devicePermissions")) {
                for (const auto& device : json["devicePermissions"].getMemberNames()) {
                    if (json["devicePermissions"][device].isArray()) {
                        for (const auto& perm : json["devicePermissions"][device]) {
                            config.devicePermissions[device].push_back(perm.asString());
                        }
                    }
                }
            }
            
            if (json.isMember("adminUsers") && json["adminUsers"].isArray()) {
                for (const auto& admin : json["adminUsers"]) {
                    config.adminUsers.push_back(admin.asString());
                }
            }
            
            if (json.isMember("enableFirewall")) config.enableFirewall = json["enableFirewall"].asBool();
            if (json.isMember("firewallRules") && json["firewallRules"].isArray()) {
                for (const auto& rule : json["firewallRules"]) {
                    config.firewallRules.push_back(rule.asString());
                }
            }
            
            return config;
        }
    };
    
    /**
     * @brief Logging configuration
     */
    struct LoggingConfig {
        bool enabled = true;
        std::string level = "INFO";
        bool logToFile = true;
        std::string logFile = "/var/log/device-manager.log";
        bool logToSyslog = true;
        std::string syslogFacility = "local0";
        bool logToConsole = true;
        bool logDeviceEvents = true;
        bool logDeviceData = false;
        bool logErrors = true;
        bool logWarnings = true;
        bool logInfo = true;
        bool logDebug = false;
        int maxLogSizeMB = 100;
        int maxLogFiles = 5;
        bool compressLogs = true;
        bool includeTimestamp = true;
        bool includeLevel = true;
        bool includeSource = true;
        
        Json::Value toJson() const {
            Json::Value json;
            json["enabled"] = enabled;
            json["level"] = level;
            json["logToFile"] = logToFile;
            json["logFile"] = logFile;
            json["logToSyslog"] = logToSyslog;
            json["syslogFacility"] = syslogFacility;
            json["logToConsole"] = logToConsole;
            json["logDeviceEvents"] = logDeviceEvents;
            json["logDeviceData"] = logDeviceData;
            json["logErrors"] = logErrors;
            json["logWarnings"] = logWarnings;
            json["logInfo"] = logInfo;
            json["logDebug"] = logDebug;
            json["maxLogSizeMB"] = maxLogSizeMB;
            json["maxLogFiles"] = maxLogFiles;
            json["compressLogs"] = compressLogs;
            json["includeTimestamp"] = includeTimestamp;
            json["includeLevel"] = includeLevel;
            json["includeSource"] = includeSource;
            return json;
        }
        
        static LoggingConfig fromJson(const Json::Value& json) {
            LoggingConfig config;
            if (json.isMember("enabled")) config.enabled = json["enabled"].asBool();
            if (json.isMember("level")) config.level = json["level"].asString();
            if (json.isMember("logToFile")) config.logToFile = json["logToFile"].asBool();
            if (json.isMember("logFile")) config.logFile = json["logFile"].asString();
            if (json.isMember("logToSyslog")) config.logToSyslog = json["logToSyslog"].asBool();
            if (json.isMember("syslogFacility")) config.syslogFacility = json["syslogFacility"].asString();
            if (json.isMember("logToConsole")) config.logToConsole = json["logToConsole"].asBool();
            if (json.isMember("logDeviceEvents")) config.logDeviceEvents = json["logDeviceEvents"].asBool();
            if (json.isMember("logDeviceData")) config.logDeviceData = json["logDeviceData"].asBool();
            if (json.isMember("logErrors")) config.logErrors = json["logErrors"].asBool();
            if (json.isMember("logWarnings")) config.logWarnings = json["logWarnings"].asBool();
            if (json.isMember("logInfo")) config.logInfo = json["logInfo"].asBool();
            if (json.isMember("logDebug")) config.logDebug = json["logDebug"].asBool();
            if (json.isMember("maxLogSizeMB")) config.maxLogSizeMB = json["maxLogSizeMB"].asInt();
            if (json.isMember("maxLogFiles")) config.maxLogFiles = json["maxLogFiles"].asInt();
            if (json.isMember("compressLogs")) config.compressLogs = json["compressLogs"].asBool();
            if (json.isMember("includeTimestamp")) config.includeTimestamp = json["includeTimestamp"].asBool();
            if (json.isMember("includeLevel")) config.includeLevel = json["includeLevel"].asBool();
            if (json.isMember("includeSource")) config.includeSource = json["includeSource"].asBool();
            return config;
        }
    };
    
    // Configuration sections
    DiscoveryConfig discovery;
    MonitoringConfig monitoring;
    DeviceDefaults defaults;
    StorageConfig storage;
    SecurityConfig security;
    LoggingConfig logging;
    
    // Additional settings
    std::string version = "1.0";
    std::string instanceId;
    std::string environment = "production";
    bool debugMode = false;
    int shutdownTimeoutMs = 10000;
    std::map<std::string, Json::Value> customSettings;
    
    /**
     * @brief Load configuration from JSON
     */
    static DeviceManagerConfig fromJson(const Json::Value& json) {
        DeviceManagerConfig config;
        
        if (json.isMember("discovery")) {
            config.discovery = DiscoveryConfig::fromJson(json["discovery"]);
        }
        
        if (json.isMember("monitoring")) {
            config.monitoring = MonitoringConfig::fromJson(json["monitoring"]);
        }
        
        if (json.isMember("defaults")) {
            config.defaults = DeviceDefaults::fromJson(json["defaults"]);
        }
        
        if (json.isMember("storage")) {
            config.storage = StorageConfig::fromJson(json["storage"]);
        }
        
        if (json.isMember("security")) {
            config.security = SecurityConfig::fromJson(json["security"]);
        }
        
        if (json.isMember("logging")) {
            config.logging = LoggingConfig::fromJson(json["logging"]);
        }
        
        if (json.isMember("version")) {
            config.version = json["version"].asString();
        }
        
        if (json.isMember("instanceId")) {
            config.instanceId = json["instanceId"].asString();
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
    Json::Value toJson() const {
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
    bool validate() const {
        // Validate discovery settings
        if (discovery.maxDevices <= 0) return false;
        if (discovery.intervalSeconds <= 0) return false;
        if (discovery.timeoutMs <= 0) return false;
        
        // Validate monitoring settings
        if (monitoring.intervalMs <= 0) return false;
        if (monitoring.maxSamples <= 0) return false;
        if (monitoring.statsWindowSeconds <= 0) return false;
        if (monitoring.healthCheckIntervalMs <= 0) return false;
        if (monitoring.healthThreshold < 0 || monitoring.healthThreshold > 1) return false;
        
        // Validate defaults
        if (defaults.timeoutMs <= 0) return false;
        if (defaults.retryCount < 0) return false;
        if (defaults.retryDelayMs <= 0) return false;
        if (defaults.maxDataAgeMs <= 0) return false;
        
        // Validate storage
        if (storage.saveIntervalMs <= 0) return false;
        if (storage.maxHistoryDays <= 0) return false;
        if (storage.maxFileSizeMB <= 0) return false;
        if (storage.backupIntervalHours <= 0) return false;
        if (storage.maxBackups <= 0) return false;
        
        // Validate security
        if (security.enabled && security.requireAuthentication) {
            if (security.authTokens.empty()) return false;
        }
        
        // Validate logging
        if (logging.maxLogSizeMB <= 0) return false;
        if (logging.maxLogFiles <= 0) return false;
        
        return true;
    }
};

} // namespace device_manager

#endif // DEVICE_MANAGER_CONFIG_H
