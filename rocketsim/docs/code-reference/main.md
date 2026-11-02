# `main.cpp` — program entry point

[← Back to the code reference](README.md)

| | |
| --- | --- |
| **File** | `src/main.cpp` |
| **Status** | Given, complete |
| **Includes** | `<cmath>`, `<iostream>`, `<string>`, `config.h`, `plot.h`, `rocket.h`, `simulation.h` |
| **You change it in** | Week 7 Medium (calling `report`), Week 7 Hard (`--out`), and most weeks after that |

---

## Purpose

`main.cpp` is deliberately short. It reads the command-line options, builds the objects the simulation needs, runs one flight, and hands the result to the plotter. All the real work happens in the other files.

---

## Functions

### `void runDemo()` *(anonymous namespace)*

| | |
| --- | --- |
| **Status** | Given |
| **Parameters** | none |
| **Returns** | nothing |
| **Triggered by** | `rocketsim --demo` |

Draws a **canned trajectory** so you can check the build and the plotter before writing any physics. It computes a textbook drag-free projectile — 300 m/s at 60° under gravity of 9.81 m/s² — directly from the closed-form equations of motion, sampling every 0.05 s, and adds the points to a `Plot` series called `demo`. It then:

1. adds an `apogee` marker at the top of the parabola,
2. saves the plot to `flight.svg`,
3. prints the ASCII preview and a confirmation line, and
4. opens the live window with `plot.show()`.

`runDemo` sits in an **anonymous namespace**, which means it is visible only inside `main.cpp` — the C++ equivalent of a `static` function in C.

> [!NOTE]
> The demo never touches `Rocket` or `physics`, so it works on the untouched starter. If `--demo` does not show a parabola, the problem is with your build, not your code.

---

### `int main(int argc, char* argv[])`

| | |
| --- | --- |
| **Status** | Given |
| **Parameters** | `argc` — number of command-line arguments; `argv` — the arguments as C strings |
| **Returns** | `0` on success; `1` on an unknown option or if `flight.svg` cannot be written |

**What it does, in order:**

1. **Creates the defaults.** A `Rocket` (default values from `rocket.h`), a `SimSettings` (default values from `simulation.h`), and a flag `showWindow = true`.
2. **Parses the options.** Each `argv[i]` is copied into a `const std::string` so it can be compared with `==`:

   | Option | Effect |
   | --- | --- |
   | `--demo` | Calls `runDemo()` and returns immediately. |
   | `--quiet` | Sets `settings.printEvery = 0.0`, which turns off telemetry lines. |
   | `--no-window` | Sets `showWindow = false`. |
   | `--launch FILE` | Calls `loadLaunchFile(FILE, rocket, settings)`. Consumes the next argument. |
   | anything else | Prints `Unknown option: …` to `std::cerr` and returns `1`. |

3. **Announces the launch** (`Launching Starter-1...`).
4. **Creates a `Plot`** titled `"Flight path of " + rocket.name` and runs `simulate(rocket, settings, plot)`, keeping the returned `FlightStats`.
5. **Reports** with `printStats(rocket, stats)` and `plot.printAscii()`.
6. **Saves** `flight.svg` with `plot.saveSvg`. On failure it prints an error and returns `1`.
7. **Shows the window** with `plot.show()` if `showWindow` is still true. `show()` returns when the window is closed.

> [!TIP]
> Notice that `rocket` and `settings` are passed to `loadLaunchFile` and `simulate` **by reference**, so those functions change `main`'s own objects rather than copies. This is the core idea of Week 7.

---

## Expected output (starter code)

> **SCREENSHOT PLACEHOLDER — `images/main-starter-output.png`**
> *Capture:* the full terminal output of `./build/rocketsim --no-window` on the unmodified starter: `Launching Starter-1...`, the "never left the pad" hint, the all-zero flight summary, the ASCII plot and the `Wrote flight.svg` line.
<!-- ![Starter output](images/main-starter-output.png) -->
