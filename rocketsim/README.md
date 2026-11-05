# rocketsim — CMP-5015A lab project (weeks 7–12)

A 2-D rocket flight simulator that you build up week by week. The starter
code compiles and runs, but the rocket sits on the pad until you write the
physics (week 7).

## Build and run in CLion

CLion only knows how to build and run the code once it has **loaded the
project's `CMakeLists.txt`**. Until then the Run button is greyed out, there
is no `rocketsim` run configuration, and the editor may show red errors on
every `#include`.

1. **Open the project.** Use **File → Open…**. If you open the `rocketsim`
   folder itself, CLion usually finds `CMakeLists.txt` automatically and
   offers to load it as a CMake project; accept, then skip to step 3.
2. **Load `CMakeLists.txt` yourself** if CLion did not offer to, or if you
   opened the whole repository folder rather than `rocketsim`:
   - in the **Project** panel on the left, open `rocketsim/` and find
     `CMakeLists.txt`;
   - **right-click** it and choose **Load CMake Project**. Alternatively, open
     the file and click **Load CMake project** in the bar that appears at the
     top of the editor.
3. **Wait for CMake to finish.** Open the **CMake** tool window (bottom of
   the screen) and watch it run. The first load downloads and builds SFML, so
   it needs internet access and takes a few minutes. It is finished when the
   output ends with `[Finished]` and no errors.
4. **Choose the run configuration.** In the toolbar at the top right, select
   **rocketsim** from the run configuration drop-down.
5. **Run.** Press the green **Run** button (**Ctrl+R** on macOS,
   **Shift+F10** on Windows and Linux). CLion builds the program, and the
   output appears in the **Run** tool window at the bottom.

On the untouched starter you should see `The rocket never left the pad`.
That is correct: writing the physics is the week 7 task.

> **Reload after changing the file list.** When you add a new `.cpp` file,
> or edit `CMakeLists.txt`, right-click `CMakeLists.txt` and choose
> **Reload CMake Project** (or **Tools → CMake → Reload CMake Project**).
> Otherwise the new file is not compiled and you get linker errors.

To pass command-line options such as `--demo` or `--launch launch.txt`, and
to set the working directory so `launch.txt` is found, see
[Running with command-line arguments in CLion](docs/worksheets/week07.md#running-with-command-line-arguments-in-clion)
in the week 7 worksheet.

To build without the live window in CLion, open **Settings → Build,
Execution, Deployment → CMake** and add `-DROCKETSIM_LIVE=OFF` to
**CMake options**, then reload the project.

## Build from a terminal

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

On Windows the executable is `build\Debug\rocketsim.exe`. In CLion, follow
[Build and run in CLion](#build-and-run-in-clion) above instead.

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
re-run `cmake -B build` once (in CLion: right-click `CMakeLists.txt` →
**Reload CMake Project**).

## Conventions

* x is downrange distance in metres, y is altitude in metres. The ground is y = 0.
* All quantities are SI: kg, m, s, N.
* Search the source for `TODO (Week` to find this week's work.
