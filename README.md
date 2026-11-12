# CMP-5015A rocket flight simulator — lab worksheets

## This week's worksheet

**[Week 8 — Classes in depth](rocketsim/docs/worksheets/week08.md)**

---

## Getting started

| | |
| --- | --- |
| [Starter project README](rocketsim/README.md) | How to build and run `rocketsim` |
| [Build, run and project files](rocketsim/docs/code-reference/build-and-project.md) | CMake, command-line options, window controls, the `launch.txt` format |
| [Worksheet overview](rocketsim/docs/worksheets/README.md) | How the labs work: tiers, checkpoints, the carry-over rule |

---

## Weekly worksheets

| Week | Worksheet | Theme |
| --- | --- | --- |
| 7 | [Getting the Rocket to Fly!](rocketsim/docs/worksheets/week07.md) | References, strings, streams, `new` / `delete` |
| 8 | [Classes in depth](rocketsim/docs/worksheets/week08.md) | Constructors, RAII, the rule of three, operators, inheritance |

---

## Code reference

What every file and function in the starter code does.

| Page | Covers |
| --- | --- |
| [Overview](rocketsim/docs/code-reference/README.md) | How the pieces fit together, file status, conventions |
| [`main.cpp`](rocketsim/docs/code-reference/main.md) | Entry point, command-line parsing, the demo |
| [`vec2.h`](rocketsim/docs/code-reference/vec2.md) | The `Vec2` 2-D vector |
| [`rocket.h`](rocketsim/docs/code-reference/rocket.md) | The `Rocket` struct |
| [`physics.h` / `physics.cpp`](rocketsim/docs/code-reference/physics.md) | Forces and the time step |
| [`simulation.h` / `simulation.cpp`](rocketsim/docs/code-reference/simulation.md) | The flight loop and summary |
| [`config.h` / `config.cpp`](rocketsim/docs/code-reference/config.md) | Reading a launch file |
| [`plot.h` / `plot.cpp`](rocketsim/docs/code-reference/plot.md) | The plotter: live window, SVG, ASCII |

---

## Model solutions

Each week's solution is released before the next lab. If you missed a week, copy its `src/` over your project and carry on.

| Week | Walkthrough | Code |
| --- | --- | --- |
| — | [How to use the solutions](rocketsim-solutions/README.md) | |
| 7 | [SOLUTION.md](rocketsim-solutions/week07/SOLUTION.md) | [src/](rocketsim-solutions/week07/src) |
| 8 | TBC | TBC |

---

## Repository layout

| Folder | Contents |
| --- | --- |
| [`rocketsim/`](rocketsim) | The starter project you receive in week 7 ([source](rocketsim/src), [docs](rocketsim/docs)) |
| [`rocketsim-solutions/`](rocketsim-solutions) | One folder per week: `SOLUTION.md` walkthrough and `src/` with all tiers |
