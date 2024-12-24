#include "../lib/time_utils.h"
#include <string>
#include <sstream>
#include <iomanip>

std::string timeFormat(int seconds) {
    int hours = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = seconds % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << mins << ":"
        << std::setw(2) << std::setfill('0') << secs;

    return oss.str();
}