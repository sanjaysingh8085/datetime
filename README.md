# Date & Time Utility Library + Event Scheduler

A small C++ toolkit that mimics what `<chrono>` and calendar apps do
internally: represent dates/times correctly (leap years, month lengths),
compare and add days to them, and use that to power a basic event
scheduler with conflict detection and file persistence.

## Files

| File | Purpose |
|---|---|
| `DateTime.h / .cpp` | Core date+time class: validation, leap years, `addDays`, comparisons |
| `Event.h / .cpp` | An event = title + start `DateTime` + end `DateTime` |
| `Scheduler.h / .cpp` | Holds a list of `Event`s: add, conflict check, query by date, save/load |
| `main.cpp` | Menu-driven console program to use everything above |
| `.vscode/tasks.json` | One-click build task for VS Code |

## How to run in VS Code

1. Install the **C/C++** extension (Microsoft) and make sure `g++` is on
   your PATH (on Windows, install MinGW or use WSL; on Mac, `xcode-select --install`;
   on Linux, `sudo apt install g++`).
2. Open this folder in VS Code (`File > Open Folder`).
3. Press `Ctrl+Shift+B` (or `Cmd+Shift+B` on Mac) — this runs the build
   task and produces a `scheduler` executable.
4. Run it from the VS Code terminal:
   ```
   ./scheduler        # Mac/Linux
   scheduler.exe       # Windows
   ```

Or skip VS Code's task runner and just do it by hand in the terminal:
```bash
g++ -std=c++17 -Wall -o scheduler main.cpp DateTime.cpp Event.cpp Scheduler.cpp
./scheduler
```

## What it does when you run it

- Loads any existing `schedule.csv` on startup (won't error if it doesn't exist yet)
- Menu options: add event, view events on a date, list all, **delete an
  event**, save, load, and a **self-test** option that prints out leap-year
  checks, month-boundary `addDays` checks, and comparison checks with
  expected vs actual values — useful evidence for your project demo/viva.
- **Rejects events in the past.** If the start time you enter is earlier
  than the current system time, the event is not created.
- **Live conflict rejection.** As soon as you finish entering the end time,
  it checks against every existing event and rejects immediately if it
  overlaps — and tells you exactly which event it clashed with.
- **Month input accepts either format.** Type `8`, `Aug`, or `August` —
  all three work, case-insensitive.
- **Type `cancel` at any prompt to abort.** Works mid-way through adding
  an event, viewing by date, or deleting one — you're dropped straight
  back to the main menu, nothing is saved.
- **Delete events by index.** Option 4 lists all events with a number next
  to each; type the number to delete it.
- **Auto-removal of past events.** Any event whose end time has already
  passed is silently dropped from the list — checked on startup, after
  loading a file, and every time you return to the main menu.
  Note: this is a plain console program with no background thread, so
  "auto" means *"re-checked every time you interact with the menu,"* not a
  live timer ticking while the program sits idle. That's a fair tradeoff
  for a project like this — a real background timer would need threads,
  which is out of scope here — but worth knowing so you don't overstate it
  in your report.
- Saves/loads to `schedule.csv` in the project folder

## Design notes (for your report / viva)

- **Why not just store day/month/year separately and compare field by field?**
  That approach breaks at month/year boundaries (e.g. comparing 31-Dec vs
  1-Jan needs special-casing). Instead, every `DateTime` converts to a single
  "minutes since epoch" number (`toMinutes()`) using a well-known
  civil-date-to-day-count algorithm. All comparisons and arithmetic just
  work on that number, so leap years and month lengths are handled once,
  correctly, in one place — not re-implemented in every operator.
- **OOP concepts used:** operator overloading (`<`, `>`, `==`, `+=`, `<<`),
  static members (`isLeapYear`, `daysInMonth`), encapsulation (all fields
  private, validated in the constructor), composition (`Event` has-a
  `DateTime` x2, `Scheduler` has-a `vector<Event>`).
- **Conflict detection:** standard interval-overlap test —
  two events `[s1,e1)` and `[s2,e2)` clash if `s1 < e2 && s2 < e1`.
  `Scheduler::findConflict` returns a pointer to the *specific* clashing
  event (not just true/false) so the rejection message can name it.
- **Cancel-anywhere:** every input prompt goes through `readLineOrCancel`,
  which throws a small `OperationCancelled` marker struct if the user types
  "cancel". Each menu action wraps its steps in one `try/catch` that catches
  that struct and returns to the menu. This is a clean way to handle
  "abort from deep inside a multi-step process" without passing cancel
  flags through every function.

## Roadmap (matches the original 10-day brief)

- [x] Day 1–3: `DateTime` class — construction, validation, leap year logic
- [x] Day 4–5: Date arithmetic (`addDays`, difference between two dates)
- [x] Day 6–7: `Event` + `Scheduler` classes, conflict detection
- [ ] Day 8–9: Recurring events (daily/weekly) — stretch goal, not built yet
- [x] Day 10: File persistence — save/load schedule from a CSV file

## Extending it (if you want to attempt the stretch goal)

Recurring events weren't built — here's the direction if you want to try:
add a `RecurrenceRule` (enum: none/daily/weekly + a count or end date) to
`Event`, and in `Scheduler::getEventsOn`, expand a recurring event into its
virtual occurrences before checking `isOnDate`, rather than storing every
occurrence as a separate `Event`.
