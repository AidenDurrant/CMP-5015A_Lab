# rocketsim — Code Reference

> **CMP-5015A · Weeks 7–12**
> This reference describes the **starter code** you receive at the start of Week 7: what every file is for, and what every function does. Use it alongside the weekly worksheets in [`../worksheets/`](../worksheets/README.md).

---

## Contents

| Page | Covers |
| --- | --- |
| [Build, run and project files](build-and-project.md) | `CMakeLists.txt`, `launch.txt`, `assets/`, command-line options, the window controls |
| [`main.cpp`](main.md) | Program entry point, command-line parsing, the demo trajectory |
| [`vec2.h`](vec2.md) | The `Vec2` 2-D vector and its helper functions |
| [`rocket.h`](rocket.md) | The `Rocket` struct: design parameters and flight state |
| [`physics.h` / `physics.cpp`](physics.md) | Physical constants, forces, and the time-step function |
| [`simulation.h` / `simulation.cpp`](simulation.md) | `FlightStats`, `SimSettings`, the simulation loop and the flight summary |
| [`config.h` / `config.cpp`](config.md) | Reading a launch file |
| [`plot.h` / `plot.cpp` / `plot_window.cpp`](plot.md) | The `Plot` class: live window, SVG and ASCII output |

---

## What the program does

`rocketsim` is a 2-D rocket flight simulator. It models a rocket as a single point with a mass, pushes it around with three forces (gravity, thrust and drag), and advances it through time in small steps until it lands. Each step is recorded and the flight path is:

- replayed in a **live window** styled like a ground-station monitor,
- written to **`flight.svg`**, which opens in any web browser, and
- printed as a rough **ASCII plot** in the terminal.

In the starter code the physics is not written yet, so the rocket stays on the launch pad. Writing that physics is your first job in Week 7.

> **SCREENSHOT PLACEHOLDER — `images/starter-on-pad.png`**
> *Capture:* the terminal after running `./build/rocketsim` on the unmodified starter, showing the message `The rocket never left the pad. Have you implemented physics::step?` and the empty ASCII plot.
<!-- ![Starter code: the rocket never leaves the pad](images/starter-on-pad.png) -->

---

## How the pieces fit together

```
                 ┌──────────────┐
  command line ─►│   main.cpp   │── --launch FILE ──►  config.cpp  (loadLaunchFile)
                 └──────┬───────┘                         │ fills Rocket + SimSettings
                        │ builds Rocket, SimSettings, Plot ◄┘
                        ▼
                 ┌──────────────┐   once per time step   ┌──────────────┐
                 │simulation.cpp│ ─────────────────────► │ physics.cpp  │
                 │  simulate()  │ ◄───────────────────── │   step()     │
                 └──────┬───────┘   rocket moved on dt   └──────┬───────┘
                        │ addPoint / addMarker                   │ uses
                        ▼                                        ▼
                 ┌──────────────┐                         ┌──────────────┐
                 │   plot.cpp   │                         │ vec2.h       │
                 │  Plot class  │                         │ rocket.h     │
                 └──────┬───────┘                         └──────────────┘
                        ▼
          flight.svg · ASCII preview · live window (plot_window.cpp)
```

**One run, in order:**

1. `main` creates a default `Rocket` and `SimSettings`, then reads the command-line options (which may load a launch file into them).
2. `main` creates a `Plot` and calls `simulate(rocket, settings, plot)`.
3. `simulate` loops: it calls `physics::step` to move the rocket forward by `dt` seconds, then adds the new position to the plot. It stops when the rocket returns to the ground or the time limit is reached.
4. `simulate` returns a `FlightStats` summary, which `main` prints with `printStats`.
5. `main` prints the ASCII plot, saves `flight.svg` and (unless `--no-window` was given) opens the live window.

---

## File status at a glance

| File | Status in the starter | You first change it in |
| --- | --- | --- |
| `src/main.cpp` | Given, complete | Week 7 (Medium/Hard) |
| `src/vec2.h` | Given, complete | Week 8 |
| `src/rocket.h` | Given, complete | Week 8 |
| `src/physics.h` | Given, complete | Week 7 (Medium) |
| `src/physics.cpp` | **Stubs — TODO Week 7** | Week 7 (Easy) |
| `src/simulation.h` | Given, complete | Week 7 (Medium) |
| `src/simulation.cpp` | **Partly given — TODO Week 7** | Week 7 (Easy) |
| `src/config.h` | Given | Week 7 (Hard) |
| `src/config.cpp` | **Stub — TODO Week 7 Hard** | Week 7 (Hard) |
| `src/plot.h` / `plot.cpp` | Given, complete | Never (read-only) |
| `src/plot_window.cpp` | Given, complete | Never (not lab material) |

> [!TIP]
> Every unfinished part of the starter is marked with a comment beginning `TODO (Week`. Search the project for that text to find exactly what each week asks of you.

---

## Conventions used throughout the code

| Convention | Meaning |
| --- | --- |
| **Coordinates** | `x` is downrange distance, `y` is altitude. The launch pad is at the origin and the ground is `y = 0`. |
| **Units** | SI throughout: kilograms, metres, seconds, newtons. Angles are in **degrees** from the horizontal (90° = straight up). |
| **Passing objects** | Read-only objects are passed as `const T&`; objects a function changes are passed as `T&`. No raw pointers are used for parameters. |
| **Constants** | Physical constants are `constexpr` values inside `namespace physics`. |
| **Headers** | Every header starts with `#pragma once`. Declarations live in `.h` files, definitions in `.cpp` files (except small `inline` helpers in `vec2.h`). |
| **Status comments** | `GIVEN` means the code is complete and you should read it. `TODO (Week N, Tier)` marks work for you. |

---

## How to read the function pages

Each function in this reference is documented in the same layout:

> ### `returnType functionName(parameters)`
> | | |
> | --- | --- |
> | **Status** | Given / TODO (week and tier) |
> | **Parameters** | what each argument means |
> | **Returns** | what comes back |
>
> A description of the behaviour, plus any notes on how it is used elsewhere.

For functions you have to write, the page states the **contract** — what the function must do — but not how to write it. Working that out is the lab.

> [!NOTE]
> This reference describes the starter code. As you work through the weeks your own code will grow beyond it (new files such as `telemetry.h`, `rocket.cpp`, `history.h`, …). Those are described in each week's worksheet rather than here.
