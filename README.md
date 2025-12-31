# Restaurant Management CLI (DSA Edition)

A console app in C++ that manages restaurant orders while showcasing core data structures and algorithms. It uses only manual implementations for queues, heaps, linked lists, BSTs, graphs, and merge sort (no STL containers for these structures), making it a good educational reference.

## What it does
- Take customer orders (normal or VIP), auto-queue them, and track status through PLACED → QUEUED → PREPPING → READY → SERVED (or CANCELLED).
- VIP scheduling uses a custom min-heap; normal orders use a circular FIFO queue.
- Modify or cancel orders before completion; items can be attached and estimates auto-computed from menu defaults.
- Menu managed as a BST (add/find/list/remove) to supply default prep times.
- Status validation via a directed graph of allowed transitions.
- Persistence: save/load state to JSON to resume after crash/exit.
- Reporting: active and completed orders, sorted automatically (merge sort) with table output.

## Build & Run
```bash
make          # build
make run      # build + run
./restaurant  # run if already built
```

## Quick start
```bash
./restaurant
load db.json          # auto-loaded if present; can load explicitly
load data_demo.json   # optional: demo data
help                  # show commands
```

## Commands (TUI)
- `new` — create order (prompts for customer, VIP, items, estimate)
- `edit <id>` — edit an existing order
- `next` — kitchen pulls next order (VIP first)
- `start <id>` — mark PREPPING
- `ready <id>` — mark READY
- `serve <id>` — mark SERVED
- `cancel <id>` — cancel if still active
- `show <id>` — show one order
- `list [status]` — list all or by status; table is sorted by placed time
- `report active` — placed/queued/prepping/ready, sorted by placed time
- `report completed` — served orders, sorted by served time
- `find <id>` — quick lookup by id
- `menu add|remove|find|list` — manage menu defaults (BST)
- `save [path]` — persist to JSON (default `db.json`)
- `load [path]` — load from JSON (default `db.json`)
- `clear`, `help`, `exit`

### Status tokens
`PLACED | QUEUED | PREPPING | READY | SERVED | CANCELLED`

## Sorting behavior
Lists and reports are automatically merge-sorted:
- Active/all: by placed time
- Completed: by served time
No user choice needed; the app picks the sensible default.

## Persistence
State saves to a compact JSON (orders, queues, nextId). Load it back to resume after a crash or restart. A sample dataset is provided: `data_demo.json`.

## Data structure highlights
- FIFO: custom circular queue (normal orders)
- Priority: custom min-heap (VIP orders)
- Registry: doubly linked list (active orders)
- Menu: binary search tree
- Workflow: adjacency-matrix directed graph for allowed transitions
- Sorting: merge sort for listings/reports

## Demo workflow
1) Load sample: `load data_demo.json`
2) Inspect: `list`, `report active`, `report completed`
3) Advance work: `next`, `ready <id>`, `serve <id>`
4) Add menu + order: `menu add`, then `new`
5) Save: `save`

## Notes
- Input parsing is forgiving but expects integers for IDs/quantities/estimates.
- Timestamps are shown as seconds since epoch when present; `-` if missing.
