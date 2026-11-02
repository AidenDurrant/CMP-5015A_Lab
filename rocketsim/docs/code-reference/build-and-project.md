# Build, run and project files

[← Back to the code reference](README.md)

This page covers everything in the `rocketsim/` folder that is not C++ source: the build script, the example launch file, the assets folder, and how to drive the program from the command line.

---

## Folder layout

```
rocketsim/
├── CMakeLists.txt        build script (C++20, fetches SFML for the window)
├── README.md             short project readme
├── launch.txt            example launch file (one rocket)
├── assets/
│   ├── DejaVuSans.ttf            font used by the live window
│   ├── DejaVuSansMono.ttf        monospace font for the window readout
│   └── DejaVuSans-LICENSE.txt    free font licence
├── docs/                 this documentation
└── src/                  all C++ source (see the other reference pages)
```

---

## Requirements

| Tool | Version |
| --- | --- |
| C++ standard | C++20 |
| CMake | 3.16 or newer |
| Compiler | GCC 11+, Clang 14+ or MSVC 2022 |
| IDE (recommended) | CLion |
| Libraries | Standard library only. SFML 3 (for the live window) is downloaded automatically. |

---

## Building

```sh
cd rocketsim
cmake -B build          # first time only: downloads and builds SFML (a few minutes)
cmake --build build     # compile
```

On Windows the executable is `build\Debug\rocketsim.exe`; in CLion just press **Run**.

### Building without the window

If your machine cannot download SFML (no internet, missing system packages), turn the window off:

```sh
cmake -B build -DROCKETSIM_LIVE=OFF
cmake --build build
```

The program is otherwise identical. Every run still writes `flight.svg`, and every lab task can be completed from the SVG.

> [!IMPORTANT]
> **Adding a new `.cpp` file?** The build collects every `src/*.cpp` file automatically, but only when CMake is *configured*. After creating a new source file, run `cmake -B build` again. A "function not found" **linker** error after adding a file is almost always this.

---

## `CMakeLists.txt`

| Setting | What it does |
| --- | --- |
| `CMAKE_CXX_STANDARD 20` | Compiles as C++20, with compiler extensions turned off so your code is portable. |
| `file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)` | Every `.cpp` file in `src/` becomes part of the program. |
| `find_package(Threads)` + `Threads::Threads` | Links the threading library. It has no effect until Week 12, when you use `std::thread`. |
| `-Wall -Wextra -Wpedantic` (or `/W4` on MSVC) | Turns warnings on. Treat every warning as something to fix. |
| `option(ROCKETSIM_LIVE … ON)` | When ON, fetches SFML 3.0.0 with `FetchContent`, links `SFML::Graphics`, and defines `ROCKETSIM_LIVE` and `ROCKETSIM_ASSET_DIR` for the window code. |

**Linux only:** the window needs the X11, OpenGL and FreeType development packages (on Ubuntu: `libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev libfreetype-dev`). Windows and macOS need nothing extra.

> [!TIP]
> To catch memory errors (useful from Week 7 Hard onwards), configure a separate build with AddressSanitizer on GCC or Clang:
> ```sh
> cmake -B build-asan -DCMAKE_CXX_FLAGS="-fsanitize=address -g"
> cmake --build build-asan
> ```

---

## Running

| Command | What happens |
| --- | --- |
| `./build/rocketsim` | Flies the default rocket, prints telemetry and a summary, writes `flight.svg`, opens the window. |
| `./build/rocketsim --demo` | Plots a canned parabola (no physics needed). Use this first to check that the build and the plotter work. |
| `./build/rocketsim --quiet` | As the default, but with no per-second telemetry lines. |
| `./build/rocketsim --no-window` | Writes `flight.svg` only; no window. Use this over SSH, in scripts, and for the Week 12 sweeps. |
| `./build/rocketsim --launch FILE` | Flies the rocket described in `FILE`. Prints a "not implemented" message until you complete Week 7 Hard. |

Options can be combined, e.g. `./build/rocketsim --quiet --no-window`. An unrecognised option prints `Unknown option: …` and exits with status 1.

> **SCREENSHOT PLACEHOLDER — `images/demo-window.png`**
> *Capture:* the live window after `./build/rocketsim --demo` has finished its replay — the green parabola on the 1 km grid, the `apogee` marker and the readout panel.
<!-- ![The --demo trajectory in the live window](images/demo-window.png) -->

---

## The live window

The window replays the flight in real time (1 second of mission time = 1 second on screen) like an old ground-station monitor: phosphor green on black, with a moving dot and a live readout of mission clock, altitude, range and speed.

| Key | Action |
| --- | --- |
| **Space** | Restart the replay |
| **+** / **−** | Replay speed: 1×, 2×, 5×, 10×, 20× |
| **F** | Toggle between the fixed 1 : 1 frame and fit-to-data (close-up) |
| **S** | Save the current view as `plot.png` |
| **Esc** | Close the window (the program then exits) |

**The fixed frame.** Flight-path plots always use the same frame: 1 : 1 scale, a 1 km grid, 10 km high and about 16 km wide, with the launch pad at the origin. It does **not** auto-scale, so a change that adds 500 m of range always looks like 500 m. Compare runs against the grid, not the window edges. If a flight leaves the frame, the frame doubles (2 km grid, 20 km high, and so on).

**Several rockets.** When more than one series is plotted, they are told apart by brightness and dash pattern (solid, dashed, dotted, dash-dot), not by colour.

---

## `launch.txt`

An example **launch file**: a plain-text description of one rocket and its simulation settings. It is read by `loadLaunchFile` (which you write in Week 7 Hard) when you run `rocketsim --launch launch.txt`.

```text
# rocketsim launch file.  One rocket per file.
# Each line is  key = value.  Anything after '#' is a comment.
# Units: kg, N, kg/s, m^2, degrees, seconds.

name         = Starter-1
dry_mass     = 50
fuel_mass    = 150
thrust       = 5000
burn_rate    = 10
drag_coeff   = 0.5
area         = 0.05
launch_angle = 85        # degrees from horizontal, 90 = straight up
dt           = 0.01      # integration time step
max_time     = 300       # give up after this many simulated seconds
```

### Format rules

- One `key = value` pair per line.
- Everything from `#` to the end of the line is a comment.
- Blank lines are ignored.
- Spaces around the key, the `=` and the value are not significant.
- `name` is the only text value; every other value is a number.

### Keys and where they go

| Key | Unit | Sets | Default |
| --- | --- | --- | --- |
| `name` | — | `Rocket::name` | `Starter-1` |
| `dry_mass` | kg | `Rocket::dryMass` | 50 |
| `fuel_mass` | kg | `Rocket::fuelMass` | 150 |
| `thrust` | N | `Rocket::thrust` | 5000 |
| `burn_rate` | kg/s | `Rocket::burnRate` | 10 |
| `drag_coeff` | — | `Rocket::dragCoeff` | 0.5 |
| `area` | m² | `Rocket::area` | 0.05 |
| `launch_angle` | degrees | `Rocket::launchAngleDeg` | 85 |
| `dt` | s | `SimSettings::dt` | 0.01 |
| `max_time` | s | `SimSettings::maxTime` | 300 |

The values in `launch.txt` match the defaults in `rocket.h` and `simulation.h` exactly, so `rocketsim --launch launch.txt` must produce the same flight as `rocketsim` on its own.

---

## Output files

| File | Written by | Contents |
| --- | --- | --- |
| `flight.svg` | `Plot::saveSvg` on every run | The flight-path plot in the same style as the window. Open it in a web browser. |
| `plot.png` | Pressing **S** in the window | A screenshot of the current window view. |

Later weeks add further outputs of your own (`telemetry.csv`, `flight.log`, `flight.bin`, …). Files are written to the **current working directory** — in CLion that is usually the build folder.
