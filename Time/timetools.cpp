#include "timetools.h"

bool ConvertISO8601TimeToUnix(std::string ISOTime, std::time_t& UnixTime)
{
    std::tm inputTimeInfo = {};
    std::istringstream iss(ISOTime);
    iss >> std::get_time(&inputTimeInfo, "%Y-%m-%dT%H:%M:%SZ");
    if (iss.fail())
        return false;

    UnixTime = timegm(&inputTimeInfo);
    return true;
}

bool ConvertISO8601TimeToLocal(std::string ISOTime, std::tm& LocalTime)
{
    std::time_t utcTime;
    if(!ConvertISO8601TimeToUnix(ISOTime, utcTime))
        return false;
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

std::string convertGregorianToJalali(int gy, int gm, int gd)
{
    int g_d_m[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int jy;
    if(gy > 1600)
    {
        jy = 979;
        gy -= 1600;
    }
    else
    {
        jy = 0;
        gy -= 621;
    }
    int gy2 = (gm > 2) ? (gy + 1) : gy;
    int days = (365 * gy) + ((int)((gy2 + 3) / 4)) - ((int)((gy2 + 99) / 100)) + ((int)((gy2 + 399) / 400)) - 80 + gd + g_d_m[gm - 1];
    jy += 33 * ((int)(days / 12053));
    days %= 12053;
    jy += 4 * ((int)(days / 1461));
    days %= 1461;
    if(days > 365)
    {
        jy += (int)((days - 1) / 365);
        days = (days - 1) % 365;
    }
    int jm = (days < 186) ? 1 + (int)(days / 31) : 7 + (int)((days - 186) / 30);
    int jd = 1 + ((days < 186) ? (days % 31) : ((days - 186) % 30));
    std::string resultY = std::to_string(jy);
    std::string resultM = jm < 10 ? "0" + std::to_string(jm) : std::to_string(jm);
    std::string resultD = jd < 10 ? "0" + std::to_string(jd) : std::to_string(jd);
    std::string today = resultY + '/' + resultM + '/' + resultD;
    return today;
}