#include <iostream>
#include <limits>
#include <algorithm>
#include <cctype>
#include "DateTime.h"
#include "Event.h"
#include "Scheduler.h"
using namespace std;

const string DATA_FILE = "schedule.csv";

// Thrown when the user types "cancel" at any prompt during a multi-step
// input (adding an event, deleting one, etc). Caught once at the top of
// that menu action, so the user drops straight back to the main menu.
struct OperationCancelled {};

// ---------- small string helpers ----------

string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string toLower(const string &s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return tolower(c); });
    return r;
}

// ---------- cancel-aware input ----------

// Reads one line, trims it, and throws OperationCancelled if the user
// typed "cancel" (case-insensitive). Otherwise returns the trimmed text.
string readLineOrCancel(const string &prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    line = trim(line);
    if (toLower(line) == "cancel") throw OperationCancelled{};
    return line;
}

int readIntOrCancel(const string &prompt) {
    while (true) {
        string line = readLineOrCancel(prompt);
        try {
            size_t pos;
            int val = stoi(line, &pos);
            if (pos != line.size()) throw invalid_argument("trailing junk");
            return val;
        } catch (...) {
            cout << "  Not a valid number. Type a number, or 'cancel' to abort.\n";
        }
    }
}

// Accepts either a month number (1-12) or a month name / 3-letter
// abbreviation (Jan, january, DEC, ...). Case-insensitive.
int readMonthOrCancel(const string &prompt) {
    static const string abbrev[12] = {"jan", "feb", "mar", "apr", "may", "jun",
                                       "jul", "aug", "sep", "oct", "nov", "dec"};
    while (true) {
        string line = readLineOrCancel(prompt);
        string low = toLower(line);

        // Try as a plain number first.
        bool allDigits = !low.empty() && all_of(low.begin(), low.end(), ::isdigit);
        if (allDigits) {
            int m = stoi(low);
            if (m >= 1 && m <= 12) return m;
            cout << "  Month must be 1-12. Try again, or 'cancel'.\n";
            continue;
        }

        // Otherwise match against the first 3 letters of the name.
        if (low.size() >= 3) {
            string prefix = low.substr(0, 3);
            for (int i = 0; i < 12; ++i) {
                if (prefix == abbrev[i]) return i + 1;
            }
        }
        cout << "  Didn't recognise that month. Use a number (1-12) or a name "
                "(e.g. Jan / January). Or type 'cancel'.\n";
    }
}

DateTime readDateTimeOrCancel(const string &label) {
    while (true) {
        cout << "-- " << label << " (type 'cancel' to abort) --\n";
        int d = readIntOrCancel("  Day: ");
        int m = readMonthOrCancel("  Month (name/number): ");
        int y = readIntOrCancel("  Year: ");
        int h = readIntOrCancel("  Hour (0-23): ");
        int mi = readIntOrCancel("  Minute (0-59): ");
        try {
            return DateTime(d, m, y, h, mi);
        } catch (const exception &e) {
            cout << "  Invalid: " << e.what() << " Try again.\n";
        }
    }
}

void clearInputError() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readMenuChoice(const string &prompt) {
    cout << prompt;
    int val;
    while (!(cin >> val)) {
        cout << "Please enter a valid number.\n" << prompt;
        clearInputError();
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // eat trailing newline
    return val;
}

// ---------- self tests (unchanged from before) ----------

void runSelfTests() {
    cout << "\n===== Self-test: DateTime utilities =====\n";
    cout << "isLeapYear(2024) = " << boolalpha << DateTime::isLeapYear(2024) << " (expect true)\n";
    cout << "isLeapYear(2023) = " << DateTime::isLeapYear(2023) << " (expect false)\n";
    cout << "isLeapYear(1900) = " << DateTime::isLeapYear(1900) << " (expect false, div by 100 not 400)\n";
    cout << "isLeapYear(2000) = " << DateTime::isLeapYear(2000) << " (expect true, div by 400)\n";

    DateTime feb28(28, 2, 2024, 10, 0);
    cout << "\n28-Feb-2024 + 1 day = " << feb28.addDays(1) << " (expect 29-02-2024, leap year)\n";

    DateTime feb28_2023(28, 2, 2023, 10, 0);
    cout << "28-Feb-2023 + 1 day = " << feb28_2023.addDays(1) << " (expect 01-03-2023)\n";

    DateTime dec31(31, 12, 2025, 23, 59);
    cout << "31-Dec-2025 23:59 + 1 day = " << dec31.addDays(1) << " (expect 01-01-2026, year rollover)\n";

    DateTime a(1, 1, 2026, 0, 0), b(10, 1, 2026, 0, 0);
    cout << "\n1-Jan-2026 < 10-Jan-2026 ? " << (a < b) << " (expect true)\n";
    cout << "Days between them: " << b.diffInDays(a) << " (expect 9)\n";

    cout << "Current system time (DateTime::now()) = " << DateTime::now() << "\n";
    cout << "===== End self-test =====\n\n";
}

// ---------- menu actions ----------

void actionAddEvent(Scheduler &scheduler) {
    try {
        string title = readLineOrCancel("Event title (or 'cancel'): ");
        DateTime start = readDateTimeOrCancel("Start time");
        DateTime end = readDateTimeOrCancel("End time");

        // Rule: no events in the past.
        if (start < DateTime::now()) {
            cout << "REJECTED: start time is in the past. Events must start now or later.\n";
            return;
        }

        Event candidate(title, start, end); // throws if start >= end

        // Rule: reject immediately if it clashes, and say with what.
        const Event *clash = scheduler.findConflict(candidate);
        if (clash) {
            cout << "REJECTED: clashes with existing event " << *clash << "\n";
            return;
        }

        scheduler.addEvent(candidate);
        cout << "Event added.\n";

    } catch (const OperationCancelled &) {
        cout << "Cancelled. Back to menu.\n";
    } catch (const exception &ex) {
        cout << "Could not create event: " << ex.what() << "\n";
    }
}

void actionViewOnDate(Scheduler &scheduler) {
    try {
        cout << "Enter the date to check (type 'cancel' anytime to abort):\n";
        int d = readIntOrCancel("  Day: ");
        int m = readMonthOrCancel("  Month (number or name): ");
        int y = readIntOrCancel("  Year: ");
        DateTime date(d, m, y, 0, 0);
        auto results = scheduler.getEventsOn(date);
        if (results.empty()) {
            cout << "No events on that date.\n";
        } else {
            cout << "Events on " << d << "-" << m << "-" << y << ":\n";
            for (auto &e : results) cout << "  " << e << "\n";
        }
    } catch (const OperationCancelled &) {
        cout << "Cancelled. Back to menu.\n";
    } catch (const exception &ex) {
        cout << "Invalid date: " << ex.what() << "\n";
    }
}

void actionDeleteEvent(Scheduler &scheduler) {
    const auto &all = scheduler.getAll();
    if (all.empty()) {
        cout << "No events to delete.\n";
        return;
    }
    cout << "Current events:\n";
    for (size_t i = 0; i < all.size(); ++i) {
        cout << "  [" << i << "] " << all[i] << "\n";
    }
    try {
        int idx = readIntOrCancel("Index to delete (or 'cancel'): ");
        if (idx < 0 || static_cast<size_t>(idx) >= all.size()) {
            cout << "No such index.\n";
            return;
        }
        scheduler.deleteAt(static_cast<size_t>(idx));
        cout << "Deleted.\n";
    } catch (const OperationCancelled &) {
        cout << "Cancelled. Back to menu.\n";
    }
}

void showMenu() {
    cout << "\n========== Date & Time Utility Library + Event Scheduler ==========\n";
    cout << "1. Add event\n";
    cout << "2. View events on a date\n";
    cout << "3. List all events\n";
    cout << "4. Delete an event\n";
    cout << "5. Save schedule to file\n";
    cout << "6. Load schedule from file\n";
    cout << "7. Run DateTime self-tests\n";
    cout << "0. Exit\n";
    cout << "(You can type 'cancel' at any prompt inside an action to abort it.)\n";
}

int main() {
    Scheduler scheduler;
    cout << "Welcome. Loading existing schedule (if any) from " << DATA_FILE << " ...\n";
    scheduler.loadFromFile(DATA_FILE);
    int purged = scheduler.purgeExpired();
    if (purged) cout << "Auto-removed " << purged << " event(s) whose time has already passed.\n";

    while (true) {
        // Auto-delete: every time we come back to the menu, drop any event
        // that has finished. This is a console app with no background
        // thread, so "auto delete" means "checked on every loop", not a
        // live timer ticking in the background.
        scheduler.purgeExpired();

        showMenu();
        int choice = readMenuChoice("Choice: ");

        if (choice == 1) {
            actionAddEvent(scheduler);
        } else if (choice == 2) {
            actionViewOnDate(scheduler);
        } else if (choice == 3) {
            cout << "All events (" << scheduler.count() << "):\n";
            scheduler.listAll();
        } else if (choice == 4) {
            actionDeleteEvent(scheduler);
        } else if (choice == 5) {
            scheduler.saveToFile(DATA_FILE);
        } else if (choice == 6) {
            scheduler.loadFromFile(DATA_FILE);
            scheduler.purgeExpired();
        } else if (choice == 7) {
            runSelfTests();
        } else if (choice == 0) {
            int save = readMenuChoice("Save before exiting? (1 = yes, 0 = no): ");
            if (save == 1) scheduler.saveToFile(DATA_FILE);
            cout << "Goodbye.\n";
            break;
        } else {
            cout << "Unknown option.\n";
        }
    }

    return 0;
}
