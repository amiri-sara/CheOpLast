#include "timetools.h"

bool ConvertISO8601TimeToLocal(std::string ISOTime, std::tm& LocalTime)
{
    std::tm inputTimeInfo = {};
    std::istringstream iss(ISOTime);
    iss >> std::get_time(&inputTimeInfo, "%Y-%m-%dT%H:%M:%SZ");
    if (iss.fail())
        return false;

    std::time_t utcTime = timegm(&inputTimeInfo);
    LocalTime = *std::localtime(&utcTime);
    return true;
}

int compareDate(const std::tm& tm1, const std::tm& tm2) {
    if (tm1.tm_year < tm2.tm_year) return -1;
    if (tm1.tm_year > tm2.tm_year) return 1;

    if (tm1.tm_mon < tm2.tm_mon) return -1;
    if (tm1.tm_mon > tm2.tm_mon) return 1;

    if (tm1.tm_mday < tm2.tm_mday) return -1;
    if (tm1.tm_mday > tm2.tm_mday) return 1;

    return 0;  // Structures are equal
}

std::tm getDaysAgo(int daysAgo) {

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::duration<int> durationAgo(daysAgo * 24 * 60 * 60);
    std::chrono::system_clock::time_point timeAgo = now - durationAgo;

    std::time_t timeAgoT = std::chrono::system_clock::to_time_t(timeAgo);
    std::tm* tmStruct = std::localtime(&timeAgoT);

    return *tmStruct;
}