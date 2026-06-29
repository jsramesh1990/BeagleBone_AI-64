// ipc_server.cpp
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
    
    // Initialize IPC as server
    ipc.initialize(IpcManager::TransportType::UNIX_SOCKET, "/tmp/app.sock");
    ipc.start();
    
    // Handle all messages
    ipc.registerHandler("#", [](const IpcMessage& msg) {
        std::cout << "Received: " << msg.getTopic() << std::endl;
        
        // Respond to requests
        if (msg.getType() == IpcMessage::REQUEST) {
            IpcMessage response;
            response.setTopic(msg.getTopic());
            response.setType(IpcMessage::RESPONSE);
            response.setCorrelationId(msg.getRequestId());
            response.getBody()["status"] = "ok";
            IpcManager::getInstance().sendMessage(response);
        }
    });
    
    // Keep running
    std::cout << "IPC Server running..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    ipc.stop();
    return 0;
}
