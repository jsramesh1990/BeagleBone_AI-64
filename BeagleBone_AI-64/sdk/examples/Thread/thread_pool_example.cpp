// thread_pool_example.cpp
#include "utils/ThreadPool.h"
#include "logger/Logger.h"
#include <iostream>
#include <vector>

using namespace common;

int main() {
    auto& logger = Logger::getInstance();
    logger.initialize();
    
    // Create thread pool with 4 threads
    utils::ThreadPool pool(4);
    
    // Submit multiple tasks
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 10; i++) {
        futures.push_back(pool.submit([i]() {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        }));
    }
    
    // Collect results
    for (auto& future : futures) {
        try {
            int result = future.get();
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
