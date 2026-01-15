#include "FileLoader.h"
#include <iostream>
#include <cstdlib>
#include <fstream>

// For Windows _popen/_pclose
#include <cstdio>
#include <mysql.h>
#include <filesystem>
#include <dpp/dpp.h> // For JSON parsing and potential future HTTP use
#include <dpp/json.h>

namespace fs = std::filesystem;

// Helper to get consistent filename
static std::string GetFilename(const std::string& idOrUrl) {
    if (idOrUrl.find("http") == 0) {
        // Simple hash for URL
        return "url_" + std::to_string(std::hash<std::string>{}(idOrUrl)) + ".opus";
    }
    return idOrUrl + ".opus";
}

VideoDbInfo FileLoader::RunGoLocal(const std::string& query, dpp::cluster& bot) {
    VideoDbInfo info;
    
    // URL encode the query manually to avoid encoding issues
    std::string encoded_query;
    for (unsigned char c : query) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded_query += c;
        } else if (c == ' ') {
            encoded_query += '+';
        } else {
            char hex[4];
            snprintf(hex, sizeof(hex), "%%%02X", c);
            encoded_query += hex;
        }
    }
    
    std::string url = "http://localhost:8080/process?q=" + encoded_query;
    
    std::cout << "[YoutubeDownloader] API Request: " << url << std::endl;

    try {
        // Use curl.exe with proper output handling
        std::string command = "curl.exe -s \"" + url + "\"";
        
        std::string result;
        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "[YoutubeDownloader] Failed to execute curl" << std::endl;
            return info;
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        _pclose(pipe);
        
        std::cout << "[YoutubeDownloader] Response length: " << result.length() << std::endl;
        
        if (result.empty()) {
            std::cerr << "[YoutubeDownloader] API Response is empty." << std::endl;
            return info;
        }

        auto json = nlohmann::json::parse(result);
        if (json.contains("success") && json["success"].get<bool>()) {
            if (json.contains("metadata")) {
                auto& meta = json["metadata"];
                info.channelId = meta.contains("channelId") ? meta["channelId"].get<std::string>() : "";
                info.title = meta.contains("title") ? meta["title"].get<std::string>() : "";
            }
            
            if (json.contains("local_path")) {
                info.filePath = json["local_path"].get<std::string>();
            } else if (json.contains("filepath")) {
                info.filePath = json["filepath"].get<std::string>();
            }
            
            info.success = true;
            // Avoid printing Unicode characters that may cause encoding errors
            std::cout << "[YoutubeDownloader] API Success - File path received" << std::endl;
        } else {
            std::string errMsg = json.contains("error") ? json["error"].get<std::string>() : "Unknown error";
            std::cerr << "[YoutubeDownloader] API Error: " << errMsg << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[YoutubeDownloader] Exception in RunGoLocal: " << e.what() << std::endl;
    }

    return info;
}

FILE* FileLoader::GetAudioStream(const std::string& videoIdOrPath) {
    printf("%s",videoIdOrPath.c_str());
    fs::path p = fs::u8path(videoIdOrPath);

    if (!fs::exists(p)) {
        std::cerr << "[YoutubeDownloader] File not found" << std::endl;
        return nullptr;
    }

    std::cout << "[YoutubeDownloader] Opening PCM file..." << std::endl;
    
    // Use wide character version to handle Unicode paths
    std::wstring wpath = p.wstring();
    return _wfopen(wpath.c_str(), L"rb");
}

void FileLoader::CloseAudioStream(FILE* stream, const std::string& videoIdOrPath) {
    if (stream) {
        // Robust check: only close if it's a valid open pointer
        // In some environments, multiple threads might attempt double-close if not careful
        fclose(stream);
    }
}

void FileLoader::Cleanup(const std::string& videoIdOrPath) {
    // Files are managed in the persistent DB directory, no automatic cleanup of PCM files.
}

