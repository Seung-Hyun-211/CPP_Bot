#include <string>
#include <cstdio>
#include <vector>
#include <filesystem>
#include <dpp/dpp.h>

struct VideoDbInfo {
    std::string channelId;
    std::string title;
    std::string filePath;
    bool success = false;
};

class FileLoader {
public:
    static VideoDbInfo RunGoLocal(const std::string& query, dpp::cluster& bot) ;
    // Opens an existing .pcm file directly.
    static FILE* GetAudioStream(const std::string& videoIdOrPath);

    // Closes the stream (handles both pipe and file).
    static void CloseAudioStream(FILE* stream, const std::string& videoIdOrPath);

    // Cleans up the temporary file associated with the videoId.
    static void Cleanup(const std::string& videoIdOrPath);

};
