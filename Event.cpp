#include "Event.h"
#include <sstream>
#include <stdexcept>

Event::Event(string t, DateTime s, DateTime e) : title(t), start(s), end(e) {
    if (!(s < e)) {
        throw invalid_argument("Event '" + t + "': start time must be before end time.");
    }
}

string Event::getTitle() const { return title; }
DateTime Event::getStart() const { return start; }
DateTime Event::getEnd() const { return end; }

bool Event::overlaps(const Event &other) const {
    // Standard interval overlap check: [s1,e1) intersects [s2,e2)
    // when s1 < e2 AND s2 < e1.
    return (start < other.end) && (other.start < end);
}

bool Event::isOnDate(const DateTime &date) const {
    // Compare using day-precision only (ignore hour/minute of 'date').
    DateTime dayStart(date.getDay(), date.getMonth(), date.getYear(), 0, 0);
    DateTime dayEnd(date.getDay(), date.getMonth(), date.getYear(), 23, 59);
    return (start < dayEnd || start == dayEnd) && (dayStart < end || dayStart == end);
}

ostream &operator<<(ostream &os, const Event &e) {
    os << "[" << e.title << "]  " << e.start << "  ->  " << e.end;
    return os;
}

// Format: title|d,m,y,h,mi|d,m,y,h,mi
// We use '|' between fields because the title itself may contain commas.
string Event::toCSV() const {
    return title + "|" + start.toFileFormat() + "|" + end.toFileFormat();
}

Event Event::fromCSV(const string &line) {
    stringstream ss(line);
    string t, s, e;
    if (!getline(ss, t, '|')) throw invalid_argument("Bad event line: " + line);
    if (!getline(ss, s, '|')) throw invalid_argument("Bad event line: " + line);
    if (!getline(ss, e, '|')) throw invalid_argument("Bad event line: " + line);
    return Event(t, DateTime::fromFileFormat(s), DateTime::fromFileFormat(e));
}
