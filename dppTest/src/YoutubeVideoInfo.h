#pragma once
#include <string>
#include <vector>
#include <dpp/dpp.h>
#include <dpp/json.h>

struct Thumbnail {
    std::string url;
    int width;
    int height;
};

struct Thumbnails {
    Thumbnail default_tn;
    Thumbnail high;
    Thumbnail medium;
    // Add others if needed
};

struct Snippet {
    std::string title;
    std::string description;
    std::string channelTitle;
    Thumbnails thumbnails;
};

struct Statistics {
    std::string viewCount;
    int likeCount;
};

struct ContentDetails {
    std::string duration; // ISO 8601 format e.g., "PT10M5S"
};

struct YoutubeVideoInfo {
    std::string id;
    Snippet snippet;
    Statistics statistics;
    ContentDetails contentDetails;
};

// JSON parsing macros/functions
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Thumbnail, url, width, height)


/* 
   Thumbnails object in JSON has "default", "high", "medium", etc. 
   We need custom from_json to map them to our struct fields if their names are reserved or we want mapping.
   However, "default" is a C++ keyword (if we used it as member name), but we used default_tn.
*/
inline void from_json(const nlohmann::json& j, Thumbnails& t) {
    if (j.contains("default")) j.at("default").get_to(t.default_tn);
    if (j.contains("high")) j.at("high").get_to(t.high);
    if (j.contains("medium")) j.at("medium").get_to(t.medium);
}

inline void to_json(nlohmann::json& j, const Thumbnails& t) {
    j = nlohmann::json{{"default", t.default_tn}, {"high", t.high}, {"medium", t.medium}};
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Snippet, title, description, channelTitle, thumbnails)

inline void from_json(const nlohmann::json& j, Statistics& s) {
    // viewCount comes as string in the example
    if (j.contains("viewCount") && !j["viewCount"].is_null()) {
        if (j["viewCount"].is_string())
            s.viewCount = j["viewCount"].get<std::string>();
        else 
            s.viewCount = std::to_string(j["viewCount"].get<long long>());
    }
    if (j.contains("likeCount") && !j["likeCount"].is_null()) {
         if (j["likeCount"].is_number())
            s.likeCount = j["likeCount"].get<int>();
         else if(j["likeCount"].is_string())
             s.likeCount = std::stoi(j["likeCount"].get<std::string>());
    }
}

inline void to_json(nlohmann::json& j, const Statistics& s) {
    j = nlohmann::json{{"viewCount", s.viewCount}, {"likeCount", s.likeCount}};
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ContentDetails, duration)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(YoutubeVideoInfo, id, snippet, statistics, contentDetails)