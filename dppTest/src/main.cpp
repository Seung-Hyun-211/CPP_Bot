#ifdef _WIN32
#pragma warning(disable: 4819)
#endif
#include "JsonReader.h"
#include "DiscordBotClient.h"
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::string token = JsonReader::GetDiscordToken();
    if (token.empty()) {
        std::cerr << "Discord token not found." << std::endl;
        return 1;
    }
    
    DiscordBotClient client(token);
    client.run();

    return 0;
}
