# `simulation.h` / `simulation.cpp` — the flight loop

[← Back to the code reference](README.md)

| | |
| --- | --- |
| **Files** | `src/simulation.h`, `src/simulation.cpp` |
| **Status** | Header given · `simulation.cpp` **partly given — TODO Week 7 Easy and Medium** |
| **Includes** | `plot.h`, `rocket.h`, `physics.h`, `<iomanip>`, `<iostream>` |

---

## Purpose

Runs one rocket from launch to landing, feeding every step to the plotter, and produces a summary of the flight. The loop itself is written for you; the telemetry output and the flight statistics are yours to add.

---

## Types

### `struct FlightStats`

Summary numbers for one flight. Returned by `simulate` and printed by `printStats`.

| Field | Unit | Meaning | Filled in by the starter? |
| --- | --- | --- | --- |
| `apogee` | m | Highest altitude reached | No — TODO Week 7 Medium |
| `apogeeTime` | s | Mission time at apogee | No — TODO Week 7 Medium |
| `maxSpeed` | m/s | Highest speed reached | No — TODO Week 7 Medium |
| `burnoutTime` | s | When the fuel ran out (0 if it never did) | No — TODO Week 7 Medium |
| `range` | m | Downrange distance at touchdown | Yes |
| `flightTime` | s | Total time in the air | Yes |

All fields default to `0.0`.

---

### `struct SimSettings`

| Field | Unit | Default | Meaning |
| --- | --- | --- | --- |
| `dt` | s | `0.01` | Integration time step Δt |
| `maxTime` | s | `300.0` | Safety limit: stop after this much simulated time |
| `printEvery` | s | `1.0` | Print a telemetry line every this many seconds; `0` turns telemetry off (`--quiet`) |

---

## Functions

### `FlightStats simulate(Rocket& rocket, const SimSettings& settings, Plot& plot)`

| | |
| --- | --- |
| **Status** | Loop **given**; telemetry **TODO Week 7 Easy**; statistics and markers **TODO Week 7 Medium** |
| **Parameters** | `rocket` — **modified**: flown in place, so after the call it holds the landed state · `settings` — read only · `plot` — **modified**: a series and points are added |
| **Returns** | A `FlightStats` summary |

**What the given code does:**

1. Starts a new plot series named after the rocket.
2. Works out `maxSteps = maxTime / dt + 1` as a **step-count guard**, so the loop always ends even if `step` forgets to advance the clock.
3. Loops while under both the step limit and `maxTime`. Each iteration:
   - checks whether a telemetry line is due (`rocket.time >= nextPrint`) and, if so, moves `nextPrint` on by `printEvery` — **the printing itself is the Week 7 Easy TODO**;
   - calls `physics::step(rocket, settings.dt)`;
   - adds the new `(x, y, t)` to the plot;
   - *(Week 7 Medium TODO: update the statistics and add markers here)*;
   - records that the rocket has `launched` once its altitude goes above zero, and **breaks out at touchdown** — the first step after launch where altitude is zero or below.
4. If the rocket never got off the ground, prints the hint `The rocket never left the pad. Have you implemented physics::step?`.
5. Sets `flightTime` and `range` from the rocket's final state and returns the stats.

**The TODOs:**

| Tier | What to add |
| --- | --- |
| Week 7 Easy | One telemetry line each time one is due, formatted with `std::cout`, `std::fixed`, `std::setprecision` and `std::setw` so the columns line up. The comment in the code shows the target layout. |
| Week 7 Medium | Track `apogee`, `apogeeTime`, `maxSpeed` and `burnoutTime` as the flight goes on, and add plot markers at burnout and apogee with `plot.addMarker`. |

The expected telemetry layout from the source comment:

```text
t=  12.00 s  alt=  1234.5 m  range=   56.7 m  speed=  123.4 m/s  fuel=  30.0 kg
```

> **SCREENSHOT PLACEHOLDER — `images/simulate-loop.png`**
> *Capture:* the body of `simulate` in the starter `simulation.cpp` (lines 11–60), showing the two `TODO (Week 7, …)` comments in context within the loop.
<!-- ![The simulate loop with its TODOs](images/simulate-loop.png) -->

---

### `void printStats(const Rocket& rocket, const FlightStats& stats)`

| | |
| --- | --- |
| **Status** | **Given** |
| **Parameters** | `rocket` — read only (for its name) · `stats` — read only |
| **Returns** | nothing; prints to `std::cout` |

Prints the flight summary with `<iomanip>`: `std::fixed` and `std::setprecision(1)` for one decimal place, `std::setw(9)` to right-align each number in a nine-character column. It is a worked example of the formatting you need for the telemetry line.

```text
--- Starter-1 flight summary ---
  apogee         0.0 m at t = 0.0 s
  max speed      0.0 m/s
  burnout        0.0 s
  range          0.0 m
  flight time    0.0 s
```

> [!NOTE]
> `std::fixed` is **sticky**: once set on `std::cout` it stays set for every later number. That is why `printStats` sets it again rather than assuming it.

---

## Changes in later weeks

| Week | Change |
| --- | --- |
| 7 Medium | The telemetry line moves into its own `printTelemetry(std::ostream&, const Rocket&)`; overloaded `report` functions are added. |
| 7 Hard | Every step is also recorded into a hand-built telemetry log. |
| 8 | Telemetry is printed with `std::cout << rocket`. |
| 9 | `simulate` fills a `std::vector<Sample>`; the statistics are computed after the loop with `<algorithm>`. |
| 10 | Forces are held in a `std::vector<std::unique_ptr<Force>>`. |
| 12 | `simulate` is run on other threads with `std::async` and `std::thread`. |
