#include "DateTime.h"
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <ctime>

// ---------- leap year / days in month ----------

bool DateTime::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int DateTime::daysInMonth(int month, int year) {
    static const int base[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) return 29;
    return base[month - 1];
}

// ---------- validation ----------

bool DateTime::isValid() const {
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > daysInMonth(month, year)) return false;
    if (hour < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;
    return true;
}

// ---------- civil <-> days conversion (Howard Hinnant algorithm) ----------
// This correctly handles leap years and month boundaries without any
// lookup-table hacks. Reference: http://howardhinnant.github.io/date_algorithms.html

long DateTime::daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = static_cast<unsigned>(y - era * 400);
    unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long>(doe) - 719468; // days since 1970-01-01
}

void DateTime::civilFromDays(long z, int &y, int &m, int &d) {
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = static_cast<unsigned>(z - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long yy = static_cast<long>(yoe) + era * 400;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp + (mp < 10 ? 3 : -9);
    y = static_cast<int>(yy + (m <= 2));
}

DateTime DateTime::now() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    return DateTime(lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900, lt->tm_hour, lt->tm_min);
}

// ---------- construction ----------

DateTime::DateTime(int d, int m, int y, int h, int mi)
    : day(d), month(m), year(y), hour(h), minute(mi) {
    if (!isValid()) {
        throw invalid_argument("Invalid date/time: " + to_string(d) + "-" +
                                to_string(m) + "-" + to_string(y) + " " +
                                to_string(h) + ":" + to_string(mi));
    }
}

// ---------- getters ----------

int DateTime::getDay() const { return day; }
int DateTime::getMonth() const { return month; }
int DateTime::getYear() const { return year; }
int DateTime::getHour() const { return hour; }
int DateTime::getMinute() const { return minute; }

// ---------- core arithmetic ----------

long DateTime::toMinutes() const {
    long days = daysFromCivil(year, month, day);
    return days * 24 * 60 + hour * 60 + minute;
}

DateTime DateTime::addDays(int n) const {
    long days = daysFromCivil(year, month, day) + n;
    int ny, nm, nd;
    civilFromDays(days, ny, nm, nd);
    return DateTime(nd, nm, ny, hour, minute);
}

long DateTime::diffInDays(const DateTime &other) const {
    long a = daysFromCivil(year, month, day);
    long b = daysFromCivil(other.year, other.month, other.day);
    return a - b;
}

// ---------- operators ----------

bool DateTime::operator<(const DateTime &other) const {
    return toMinutes() < other.toMinutes();
}

bool DateTime::operator>(const DateTime &other) const {
    return toMinutes() > other.toMinutes();
}

bool DateTime::operator==(const DateTime &other) const {
    return toMinutes() == other.toMinutes();
}

DateTime &DateTime::operator+=(int days) {
    *this = addDays(days);
    return *this;
}

ostream &operator<<(ostream &os, const DateTime &dt) {
    os << setfill('0') << setw(2) << dt.day << "-"
       << setw(2) << dt.month << "-" << dt.year << " "
       << setw(2) << dt.hour << ":" << setw(2) << dt.minute
       << setfill(' ');
    return os;
}

// ---------- string / file conversion ----------

string DateTime::toString() const {
    ostringstream oss;
    oss << *this;
    return oss.str();
}

string DateTime::toFileFormat() const {
    ostringstream oss;
    oss << day << "," << month << "," << year << "," << hour << "," << minute;
    return oss.str();
}

DateTime DateTime::fromFileFormat(const string &token) {
    // token looks like "12,7,2026,14,30"
    stringstream ss(token);
    string part;
    int vals[5];
    for (int i = 0; i < 5; ++i) {
        if (!getline(ss, part, ',')) throw invalid_argument("Bad DateTime file token: " + token);
        vals[i] = stoi(part);
    }
    return DateTime(vals[0], vals[1], vals[2], vals[3], vals[4]);
}
