#include "DiscordBotClient.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "YoutubeVideoInfo.h"
#include "YoutubeVideoInfo.h"
#include "FileLoader.h"
#include "DurationParser.h"
#include <filesystem>

namespace fs = std::filesystem;



// Define a safe chunk size for reading/sending audio
// 48000Hz * 2 channels * 2 bytes (16bit) = 192000 bytes per second
// 20ms of audio = 3840 bytes. DPP usually wants smaller chunks or to handle timing itself.
constexpr size_t AUDIO_BUFFER_SIZE = 11520; // Example buffer size


DiscordBotClient::DiscordBotClient(const std::string& token)
    : bot(token, dpp::i_all_intents)
{
    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([this](const dpp::ready_t& event) {
        std::cout << "Bot logged in successfully!\n";
    });

    bot.on_message_create([this](const dpp::message_create_t& event) {
        if (event.msg.content.rfind("!", 0) == 0) { // Starts with !
            std::string command = event.msg.content.substr(1);
            HandleCommand(command, event);
        }
    });
};

void DiscordBotClient::run() {
    bot.start(dpp::st_wait);
}

void DiscordBotClient::HandleCommand(const std::string& command, const dpp::message_create_t& event) {
    if (command == "join") {
        dpp::guild* g = dpp::find_guild(event.msg.guild_id);
        if (!g) {
            event.reply("서버를 찾을 수 없음");
            return;
        }

        auto voice_member = g->voice_members.find(event.msg.author.id);
        if (voice_member == g->voice_members.end()) {
            event.reply("음성채널에 접속 필요");
            return;
        }

        g->connect_member_voice(bot, event.msg.author.id);
        bot.message_create(dpp::message(event.msg.channel_id, "ㅎㅇ"));
        bot.message_delete(event.msg.id, event.msg.channel_id);
    }
     else if (command.rfind("play ", 0) == 0 || command.rfind("p ", 0) == 0) {
        // Extract arguments
        size_t spacePos = command.find(' ');
        std::string args = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";
        HandlePlayCommand(args, event);
    }
    // Add more commands here
};

void DiscordBotClient::HandlePlayCommand(const std::string& args, const dpp::message_create_t& event) {
    if (args.empty()) {
        event.reply("Usage: !play <url or search query>");
        return;
    }

    std::cout << "[HandlePlayCommand] Starting with args: " << args << std::endl;
    event.reply("Processing request...");

    // Call the server API - it now returns the info directly!
    VideoDbInfo dbInfo = FileLoader::RunGoLocal(args, bot);

    std::cout << "[HandlePlayCommand] RunGoLocal returned. Success: " << dbInfo.success 
              << ", FilePath empty: " << dbInfo.filePath.empty() << std::endl;

    if (!dbInfo.success || dbInfo.filePath.empty()) {
        std::cout << "[HandlePlayCommand] Failed - no valid file path" << std::endl;
        event.reply("Failed to process or find audio");
        return;
    }

    std::cout << "[HandlePlayCommand] Calling DownloadAndPlay..." << std::endl;
    event.reply("Found! Starting playback...");
    
    DownloadAndPlay(dbInfo.filePath, event);
    std::cout << "[HandlePlayCommand] DownloadAndPlay completed" << std::endl;
};

void DiscordBotClient::DownloadAndPlay(const std::string& videoIdOrPath, const dpp::message_create_t& event) {
    dpp::guild* g = dpp::find_guild(event.msg.guild_id);
    if (!g) {
        event.reply("Guild not found.");
        return;
    }

    // Connect logic: if bot not in voice, connect to user's voice
    dpp::voiceconn* vconn = event.from()->get_voice(event.msg.guild_id);
    dpp::discord_voice_client* v = (vconn) ? vconn->voiceclient : nullptr;

    if (!v || !v->is_ready()) {
        // Try to join author's channel
        if (!g->connect_member_voice(bot, event.msg.author.id)) {
            event.reply("Could not connect to voice channel. Please ensure you are in a voice channel.");
            return;
        }
    }

    vconn = event.from()->get_voice(event.msg.guild_id);
    v = (vconn) ? vconn->voiceclient : nullptr;
    if (!v || !v->is_ready()) {
         event.reply("Voice connection establishing... please try again shortly.");
         return;
    }
    
    // 1. Get Audio Stream (from PCM file or FFmpeg pipe)
    FILE* pipe = FileLoader::GetAudioStream(videoIdOrPath);
    if (!pipe) {
        event.reply("Failed to start audio stream.");
        return;
    }

    printf("open %s", videoIdOrPath.c_str());
    std::cout << "[DiscordBotClient] Starting audio playback..." << std::endl;

    constexpr size_t FRAME_SIZE = 11520; // 20ms frame
    std::vector<uint8_t> buffer(FRAME_SIZE);
    size_t totalBytesRead = 0;
    size_t framesStreamed = 0;
    
    while (true) {
        size_t bytesRead = fread(buffer.data(), 1, buffer.size(), pipe);
        if (bytesRead == 0) break;

        if (bytesRead % 2 != 0) bytesRead--;

        uint16_t* pcm = reinterpret_cast<uint16_t*>(buffer.data());
        size_t samples = bytesRead / 2;

        if (!v || !v->is_ready()) break;

        v->send_audio_raw(pcm, bytesRead);
    }

    
    std::cout << "[DiscordBotClient] Playback finished. Total: " << framesStreamed 
              << " frames, " << totalBytesRead << " bytes" << std::endl;
    
    // 3. Close Stream
    FileLoader::CloseAudioStream(pipe, videoIdOrPath);

    // 4. Cleanup (optional for DB files)
    FileLoader::Cleanup(videoIdOrPath);
};

void DiscordBotClient::SendBotMessage(const dpp::message_create_t& event)
{
    //bot.message_create(dpp::message("ㅎㅇ"));
}