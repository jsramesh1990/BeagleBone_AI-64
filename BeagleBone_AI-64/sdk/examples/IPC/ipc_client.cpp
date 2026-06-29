// ipc_client.cpp
#include "ipc/IpcManager.h"
#include "ipc/IpcMessage.h"
#include "logger/Logger.h"
#include <iostream>
#include <thread>

using namespace common;

int main() {
    auto& ipc = IpcManager::getInstance();
    auto& logger = Logger::getInstance();
    
    logger.initialize();
    
    // Initialize IPC
    ipc.initialize(IpcManager::TransportType::UNIX_SOCKET, "/tmp/app.sock");
    ipc.start();
    
    // Register handler
    ipc.registerHandler("sensor/data", [](const IpcMessage& msg) {
        std::cout << "Received sensor data: " << msg.getBody().toStyledString() << std::endl;
    });
    
    // Send a message
    Json::Value body;
    body["temperature"] = 25.5;
    body["humidity"] = 60.0;
    
    IpcMessage msg("sensor/data", body);
    ipc.sendMessage(msg);
    
    // Send a request
    IpcMessage request("system/status", Json::objectValue);
    auto response = ipc.sendRequest(request, 5000);
    
    if (response.isValid()) {
        std::cout << "Response: " << response.getBody().toStyledString() << std::endl;
    }
    
    // Keep running
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    ipc.stop();
    return 0;
}
