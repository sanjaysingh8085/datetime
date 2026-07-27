#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <iostream>
#include "DateTime.h"
using namespace std;

// An Event HAS-A start DateTime and an end DateTime (composition).
class Event {
private:
    string title;
    DateTime start, end;

public:
    Event(string t, DateTime s, DateTime e);

    string getTitle() const;
    DateTime getStart() const;
    DateTime getEnd() const;

    // Two events "clash" if their [start, end) time ranges overlap.
    bool overlaps(const Event &other) const;

    // True if the given date falls within [start, end] (date part only).
    bool isOnDate(const DateTime &date) const;

    friend ostream &operator<<(ostream &os, const Event &e);

    string toCSV() const;                     // title,startFields,endFields
    static Event fromCSV(const string &line); // reverse of above
};

#endif
