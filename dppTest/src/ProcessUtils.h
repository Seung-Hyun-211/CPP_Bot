#pragma once
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

class ProcessUtils {
public:
    static std::string ExecCmd(const char* cmd) {
        std::cout << "[ProcessUtils] Executing command..." << std::endl;
        std::string result;
        FILE* pipe = _popen(cmd, "r");
        if (!pipe) {
            std::cerr << "[ProcessUtils] Failed to open pipe" << std::endl;
            return "";
        }
        
        std::cout << "[ProcessUtils] Reading output..." << std::endl;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        
        std::cout << "[ProcessUtils] Closing pipe..." << std::endl;
        _pclose(pipe);
        std::cout << "[ProcessUtils] Command completed. Output length: " << result.length() << std::endl;
        return result;
    }
};
