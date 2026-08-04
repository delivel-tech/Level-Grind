#pragma once

namespace levelgrind {

struct EventInfo {
    bool exists;
    int levelId;
    int id;
    double secondsLeft;
};

struct Events {
    bool ok;
    EventInfo classicEvent;
    EventInfo platEvent;
};

enum EventType {
    Daily = 0,
    Weekly = 1,
    Monthly = 2
};

struct SetEventsResponse {
    bool ok;
};

}
