#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <string>
#include "Event.h"
using namespace std;

// Scheduler HAS-A list of Events (composition, same pattern as Event has-a DateTime).
class Scheduler {
private:
    vector<Event> events;

public:
    // Returns true if added, false if it clashed with an existing event
    // and was rejected (caller decides what "addEvent" should do on conflict).
    bool addEvent(const Event &e);

    bool hasConflict(const Event &e) const;
    const Event *findConflict(const Event &e) const; // returns the clashing event, or nullptr
    vector<Event> getEventsOn(const DateTime &date) const;
    void listAll() const;
    int count() const;

    const vector<Event> &getAll() const;  // for indexed display / deletion
    bool deleteAt(size_t index);          // removes event at that index

    // Removes any event whose end time is already in the past.
    // Returns how many were removed (0 if none).
    int purgeExpired();

    void saveToFile(const string &filename) const;
    void loadFromFile(const string &filename);
};

#endif
