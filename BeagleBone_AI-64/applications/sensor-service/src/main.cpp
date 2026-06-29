#include "sensor-service/SensorService.h"
#include "sensor-service/SensorManager.h"
#include "logger/Logger.h"
#include "config/ConfigManager.h"
#include "ipc/IpcManager.h"
#include <signal.h>
#include <iostream>
#include <thread>

using namespace common;
using namespace sensor_service;

// Global shutdown flag
std::atomic<bool> g_shutdown(false);

/**
 * @brief Signal handler for graceful shutdown
 */
void signalHandler(int signal) {
    LOG_INFO("Received signal " + std::to_string(signal) + ", shutting down...");
    g_shutdown = true;
}

/**
 * @brief Setup signal handlers
 */
void setupSignalHandlers() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    // Initialize logger
    auto& logger = Logger::getInstance();
    logger.initialize("/etc/sensor-service/logging.json");
    LOG_INFO("Sensor Service starting...");
    
    // Load configuration
    auto& config = ConfigManager::getInstance();
    std::string configPath = "/etc/sensor-service/config.json";
    if (argc > 1) {
        configPath = argv[1];
    }
    config.loadConfig(configPath);
    
    // Get service configuration
    Json::Value serviceConfig;
    bool discovery = true;
    int collectionInterval = 1000;
    
    if (config.get("sensor-service").isObject()) {
        serviceConfig = config.get("sensor-service");
        if (serviceConfig.isMember("discovery")) {
            discovery = serviceConfig["discovery"].asBool();
        }
        if (serviceConfig.isMember("collection_interval")) {
            collectionInterval = serviceConfig["collection_interval"].asInt();
        }
    }
    
    // Initialize IPC
    auto& ipc = IpcManager::getInstance();
    ipc.initialize(IpcManager::TransportType::UNIX_SOCKET, "/tmp/sensor-service.sock");
    ipc.start();
    
    // Initialize sensor manager
    auto& sensorManager = SensorManager::getInstance();
    
    // Set data callback
    sensorManager.setDataCallback([](const SensorReading& reading) {
        // Send data over IPC
        IpcMessage msg("sensor/data", reading.toJson());
        IpcManager::getInstance().sendMessage(msg);
        
        // Log data
        LOG_DEBUG("Sensor reading: " + reading.sensorId + " - " + 
                  std::to_string(reading.values.size()) + " values");
    });
    
    // Set event callback
    sensorManager.setEventCallback([](const SensorEvent& event) {
        // Send event over IPC
        Json::Value json = event.toJson();
        IpcMessage msg("sensor/event", json);
        IpcManager::getInstance().sendMessage(msg);
        
        // Log event
        LOG_INFO("Sensor event: " + SensorEvent::eventTypeToString(event.type) + 
                 " - " + event.message);
    });
    
    // Initialize sensor manager
    if (!sensorManager.initialize(serviceConfig)) {
        LOG_ERROR("Failed to initialize sensor manager");
        return 1;
    }
    
    // Discover sensors
    if (discovery) {
        LOG_INFO("Discovering sensors...");
        int count = sensorManager.discoverSensors(10000);
        LOG_INFO("Discovered " + std::to_string(count) + " sensors");
    }
    
    // Start sensors
    if (!sensorManager.start()) {
        LOG_ERROR("Failed to start sensors");
        return 1;
    }
    
    LOG_INFO("Sensor Service started successfully");
    LOG_INFO("Press Ctrl+C to stop");
    
    // Setup signal handlers
    setupSignalHandlers();
    
    // Main loop
    while (!g_shutdown) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Cleanup
    LOG_INFO("Shutting down Sensor Service...");
    
    sensorManager.stop();
    ipc.stop();
    
    LOG_INFO("Sensor Service stopped");
    return 0;
}
