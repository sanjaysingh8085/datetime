#ifndef DATETIME_H
#define DATETIME_H

#include <string>
#include <iostream>
using namespace std;

// Represents a calendar date + time (day, month, year, hour, minute).
// Internally converts to "minutes since epoch" for correct, leap-year-safe
// comparisons and arithmetic (uses the Howard Hinnant civil_from_days /
// days_from_civil algorithm — the standard trick for this kind of problem).
class DateTime {
private:
    int day, month, year, hour, minute;

    bool isValid() const;
    static long daysFromCivil(int y, int m, int d);
    static void civilFromDays(long z, int &y, int &m, int &d);

public:
    DateTime(int d = 1, int m = 1, int y = 2000, int h = 0, int mi = 0);

    static bool isLeapYear(int year);
    static int daysInMonth(int month, int year);
    static DateTime now(); // current system date/time, minute precision

    int getDay() const;
    int getMonth() const;
    int getYear() const;
    int getHour() const;
    int getMinute() const;

    long toMinutes() const;          // minutes since epoch, used for all comparisons
    DateTime addDays(int n) const;   // returns a NEW DateTime, doesn't mutate this one
    long diffInDays(const DateTime &other) const;

    bool operator<(const DateTime &other) const;
    bool operator>(const DateTime &other) const;
    bool operator==(const DateTime &other) const;
    DateTime &operator+=(int days);   // mutates this object, adds n days

    friend ostream &operator<<(ostream &os, const DateTime &dt);

    string toString() const;                 // "DD-MM-YYYY HH:MM"
    string toFileFormat() const;              // "DD,MM,YYYY,HH,MI" for CSV
    static DateTime fromFileFormat(const string &token); // reverse of above
};

#endif
