// events.h -- Week 9 (Hard): things scheduled to happen at a given time.
// The action is a std::function so any lambda will do; capture by value so
// the event is safe to store and run later.

#pragma once

#include <functional>
#include <string>
#include <vector>

class Rocket;

struct Event
{
    double time;
    std::string label;
    std::function<void(Rocket&)> action;
};

using EventList = std::vector<Event>;
