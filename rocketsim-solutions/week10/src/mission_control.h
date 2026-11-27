// mission_control.h -- Week 10 (Hard 3): watches rockets without owning them.
// A weak_ptr does not keep the rocket alive; expired() tells us whether
// anyone else still does. Holding shared_ptr here instead would keep every rocket
// alive for as long as mission control existed, and if a rocket ever pointed
// back at mission control the two would keep each other alive forever.

#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "rocket.h"

class MissionControl
{
public:
    void track(const std::shared_ptr<Rocket>& rocket) { tracked_.push_back(rocket); }

    int alive() const
    {
        return static_cast<int>(std::count_if(tracked_.begin(), tracked_.end(),
                                              [](const std::weak_ptr<Rocket>& w) { return !w.expired(); }));
    }

    std::size_t tracked() const { return tracked_.size(); }

private:
    std::vector<std::weak_ptr<Rocket>> tracked_;
};
