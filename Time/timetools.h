#ifndef TIMETOOLS_H
#define TIMETOOLS_H

#include "../Logs/log.h"

bool ConvertISO8601TimeToLocal(std::string ISOTime, std::tm& LocalTime);
int compareDate(const std::tm& tm1, const std::tm& tm2);
std::tm getDaysAgo(int daysAgo);

#endif //TIMETOOLS_H
