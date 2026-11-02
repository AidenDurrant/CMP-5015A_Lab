# rocketsim — CMP-5015A lab project (weeks 7–12)

A 2-D rocket flight simulator that you build up week by week. The starter
code compiles and runs, but the rocket sits on the pad until you write the
physics (week 7).

## Build

    cmake -B build
    cmake --build build

The first configure downloads and builds SFML 3 (the library behind the live
window), so it needs internet access and takes a few minutes; later builds
are fast. On Linux install the X11/OpenGL/FreeType development packages first
(see `CMakeLists.txt`); Windows and macOS need nothing extra. If the download
is not possible, build without the window and use the SVG instead:

    cmake -B build -DROCKETSIM_LIVE=OFF

Then:

    ./build/rocketsim             # fly the default rocket, write flight.svg
    ./build/rocketsim --demo      # draw a canned trajectory to check the plotter
    ./build/rocketsim --quiet     # no per-second telemetry
    ./build/rocketsim --no-window # write flight.svg only, no window
    ./build/rocketsim --launch launch.txt   # week 7 hard task

On Windows the executable is `build\Debug\rocketsim.exe` (or open the folder
in CLion and press Run).

A window opens after the flight and replays it in real time on a
phosphor-green display with a fixed 1 km grid, so the same change always
looks the same size run after run. Space restarts the replay, + and - change
its speed (1 to 20x), F switches between the fixed 1:1 frame and fit-to-data,
S saves `plot.png`, Esc closes. `flight.svg` is also written every run and
opens in any browser.

## Files

| File | Status | What it is |
| --- | --- | --- |
| `src/main.cpp` | given | Command line, builds a `Rocket`, runs `simulate`, saves the plot |
| `src/vec2.h` | given | `Vec2` struct and free functions `add`, `scale`, `length`, ... |
| `src/rocket.h` | given | `Rocket` struct: design parameters and flight state |
| `src/physics.h` | given | Declarations of the force functions and `step` |
| `src/physics.cpp` | **TODO week 7** | The physics. Every function is a stub |
| `src/simulation.h` | given | `FlightStats`, `SimSettings`, `simulate` |
| `src/simulation.cpp` | partly given | The loop is written; telemetry and stats are TODO |
| `src/config.h/.cpp` | **TODO week 7 (hard)** | Reads `launch.txt` |
| `src/plot.h/.cpp` | given, complete | `Plot` class: SVG output and the ASCII preview |
| `src/plot_window.cpp` | given, complete | The live window (SFML); `Plot::show()` |
| `assets/` | given | Fonts for the window |
| `launch.txt` | given | Example launch file |

Every `.cpp` file in `src/` is compiled automatically. When you add a new file,
re-run `cmake -B build` once.

## Conventions

* x is downrange distance in metres, y is altitude in metres. The ground is y = 0.
* All quantities are SI: kg, m, s, N.
* Search the source for `TODO (Week` to find this week's work.
