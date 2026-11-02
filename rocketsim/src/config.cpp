// config.cpp -- WEEK 7 (Hard). Until this is written, rocketsim ignores the
// --launch option and flies the default rocket from rocket.h.

#include "config.h"

#include <iostream>

bool loadLaunchFile(const std::string& path, Rocket& rocket, SimSettings& settings)
{
    // TODO (Week 7, Hard):
    //   1. open the file with std::ifstream; return false if it is not open
    //   2. read it a line at a time with std::getline
    //   3. strip anything after '#', skip blank lines
    //   4. split each line at '=' into key and value (trim spaces)
    //   5. convert the value with std::stod and assign it to the matching
    //      field of `rocket` or `settings`; `name` is the only string value
    (void)path;
    (void)rocket;
    (void)settings;
    std::cerr << "loadLaunchFile is not implemented yet; using the default rocket\n";
    return false;
}
