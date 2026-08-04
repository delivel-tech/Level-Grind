#pragma once
#include <string>
#include <vector>

namespace levelgrind {

struct AnnouncementInfo {
    int id;
    std::string title;
    std::string content;
    std::string addedBy;
    std::string createdAt;
};

struct AnnouncementsResponse {
    std::vector<AnnouncementInfo> announcements;
    bool ok;
};

struct AddAnnouncementResponse {
    bool ok;
};

struct DeleteAnnouncementResponse {
    bool ok;
};

}
