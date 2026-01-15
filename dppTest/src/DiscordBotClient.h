#pragma once
#include <dpp/dpp.h>
#include <string>

class DiscordBotClient {
public:
    DiscordBotClient(const std::string& token);
    void run();

private:
    void HandleCommand(const std::string& command, const dpp::message_create_t& event);
    void HandlePlayCommand(const std::string& args, const dpp::message_create_t& event);
    void DownloadAndPlay(const std::string& videoId, const dpp::message_create_t& event);
    void SendBotMessage(const dpp::message_create_t& event);
    
    
    dpp::cluster bot;
};