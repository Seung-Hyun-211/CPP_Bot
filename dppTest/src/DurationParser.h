#pragma once
#include <string>
#include <regex>
#include <iostream>

class DurationParser {
public:
    // Parses ISO 8601 Duration string (PT#H#M#S) to seconds
    static int Parse(const std::string& durationStr) {
        if (durationStr.empty()) return 0;

        int hours = 0;
        int minutes = 0;
        int seconds = 0;

        try {
            std::regex re("PT(?:(\\d+)H)?(?:(\\d+)M)?(?:(\\d+)S)?");
            std::smatch match;
            if (std::regex_match(durationStr, match, re)) {
                if (match[1].matched) hours = std::stoi(match[1].str());
                if (match[2].matched) minutes = std::stoi(match[2].str());
                if (match[3].matched) seconds = std::stoi(match[3].str());
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing duration: " << durationStr << " - " << e.what() << std::endl;
            return 0; // return 0 on error
        }

        return hours * 3600 + minutes * 60 + seconds;
    }
};
