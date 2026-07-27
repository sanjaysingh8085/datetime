#include "Scheduler.h"
#include <fstream>
#include <iostream>
#include <algorithm>

const Event *Scheduler::findConflict(const Event &e) const {
    for (const auto &existing : events) {
        if (existing.overlaps(e)) return &existing;
    }
    return nullptr;
}

bool Scheduler::hasConflict(const Event &e) const {
    return findConflict(e) != nullptr;
}

bool Scheduler::addEvent(const Event &e) {
    if (hasConflict(e)) return false;
    events.push_back(e);
    return true;
}

const vector<Event> &Scheduler::getAll() const { return events; }

bool Scheduler::deleteAt(size_t index) {
    if (index >= events.size()) return false;
    events.erase(events.begin() + index);
    return true;
}

int Scheduler::purgeExpired() {
    DateTime now = DateTime::now();
    size_t before = events.size();
    events.erase(
        remove_if(events.begin(), events.end(),
                  [&](const Event &e) { return e.getEnd() < now; }),
        events.end());
    return static_cast<int>(before - events.size());
}

vector<Event> Scheduler::getEventsOn(const DateTime &date) const {
    vector<Event> result;
    for (const auto &e : events) {
        if (e.isOnDate(date)) result.push_back(e);
    }
    return result;
}

void Scheduler::listAll() const {
    if (events.empty()) {
        cout << "No events scheduled.\n";
        return;
    }
    for (const auto &e : events) cout << e << "\n";
}

int Scheduler::count() const { return static_cast<int>(events.size()); }

void Scheduler::saveToFile(const string &filename) const {
    ofstream out(filename);
    if (!out) {
        cout << "Could not open file for writing: " << filename << "\n";
        return;
    }
    for (const auto &e : events) out << e.toCSV() << "\n";
    out.close();
    cout << "Saved " << events.size() << " event(s) to " << filename << "\n";
}

void Scheduler::loadFromFile(const string &filename) {
    ifstream in(filename);
    if (!in) {
        cout << "Could not open file for reading: " << filename << "\n";
        return;
    }
    events.clear();
    string line;
    int loaded = 0, skipped = 0;
    while (getline(in, line)) {
        if (line.empty()) continue;
        try {
            events.push_back(Event::fromCSV(line));
            loaded++;
        } catch (const exception &ex) {
            skipped++;
        }
    }
    in.close();
    cout << "Loaded " << loaded << " event(s)";
    if (skipped) cout << " (" << skipped << " bad line(s) skipped)";
    cout << " from " << filename << "\n";
}
