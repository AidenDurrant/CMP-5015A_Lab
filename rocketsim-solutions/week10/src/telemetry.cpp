#include "telemetry.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>

bool writeCsv(const TelemetryLog& log, const std::string& path)
{
    std::ofstream out(path);
    if (!out)
    {
        return false;
    }
    out << "time,x,y,speed,fuel\n";
    for (const Sample& s : log)
    {
        out << s.time << ',' << s.x << ',' << s.y << ',' << s.speed << ',' << s.fuel << '\n';
    }
    return true;
}

void printLogSummary(const TelemetryLog& log, double dt)
{
    if (log.empty())
    {
        return;
    }
    std::cout << std::fixed << std::setprecision(1);

    // Easy 3: time above 5 km and mean speed.
    const auto above = std::count_if(log.begin(), log.end(), [](const Sample& s) { return s.y > 5000.0; });
    const double meanSpeed =
        std::accumulate(log.begin(), log.end(), 0.0, [](double sum, const Sample& s) { return sum + s.speed; }) /
        static_cast<double>(log.size());
    std::cout << "  time above 5 km: " << above * dt << " s, mean speed " << meanSpeed << " m/s\n";

    // Easy 4: the last ten samples, newest first. The naive version
    //   for (std::size_t i = log.size() - 1; i >= log.size() - 10; --i)
    // never terminates when size() < 10 because size_t cannot go negative.
    std::cout << "  last samples (newest first):";
    int shown = 0;
    for (auto it = log.rbegin(); it != log.rend() && shown < 10; ++it, ++shown)
    {
        std::cout << ' ' << it->y;
    }
    std::cout << " m\n";

    // Hard 4: altitudes via transform, five fastest via partial_sort.
    std::vector<double> altitudes(log.size());
    std::transform(log.begin(), log.end(), altitudes.begin(), [](const Sample& s) { return s.y; });
    std::vector<double> speeds(log.size());
    std::transform(log.begin(), log.end(), speeds.begin(), [](const Sample& s) { return s.speed; });
    std::partial_sort(speeds.begin(), speeds.begin() + 5, speeds.end(), std::greater<double>());
    std::cout << "  five fastest samples:";
    for (int i = 0; i < 5 && i < static_cast<int>(speeds.size()); ++i)
    {
        std::cout << ' ' << speeds[i];
    }
    std::cout << " m/s (highest altitude in that list: "
              << *std::max_element(altitudes.begin(), altitudes.end()) << " m)\n";
}
