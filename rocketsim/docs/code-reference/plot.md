# `plot.h` / `plot.cpp` / `plot_window.cpp` — the plotter

[← Back to the code reference](README.md)

| | |
| --- | --- |
| **Files** | `src/plot.h` (interface), `src/plot.cpp` (SVG + ASCII), `src/plot_window.cpp` (live window) |
| **Status** | Given, complete. **You never need to change these files.** |
| **Includes** | `<string>`, `<vector>`; `plot.cpp` also uses `<algorithm>`, `<cmath>`, `<fstream>`, `<iomanip>`, `<iostream>`, `<sstream>`; `plot_window.cpp` uses SFML 3 |

---

## Purpose

`Plot` is the one part of the program you use before you fully understand it. It stores named **series** of `(x, y, t)` points plus labelled **markers**, and can show them three ways:

| Output | Method | Needs SFML? |
| --- | --- | --- |
| Live real-time replay window | `show()` | Yes (otherwise prints a note) |
| SVG file for a web browser | `saveSvg(path)` | No |
| Rough character plot in the terminal | `printAscii()` | No |

`plot.cpp` uses only `std::vector`, `std::string` and file streams, so by Week 9 you can read it as an example. `plot_window.cpp` is SFML graphics code and is **not lab material**.

---

## Typical use

```cpp
Plot plot("My flight");
plot.startSeries("Rocket A");
plot.addPoint(x, y, t);            // once per time step, with the mission time
plot.addMarker(x, y, "apogee");    // optional labelled point
plot.saveSvg("flight.svg");
plot.printAscii();
plot.show();                       // real-time replay window
```

Several rockets on one plot: call `startSeries` again before adding the second rocket's points.

---

## Public interface

### `explicit Plot(const std::string& title, const std::string& xLabel = "Downrange (m)", const std::string& yLabel = "Altitude (m)")`

| | |
| --- | --- |
| **Parameters** | `title` — shown at the top of every output · `xLabel`, `yLabel` — axis labels (default arguments) |

Creates an empty plot. If **both** labels are left at their defaults, the plot is a **flight-path plot** and uses the fixed 1 : 1 frame (see below). Pass any other labels — e.g. `Plot("Range vs launch angle", "Launch angle (deg)", "Range (m)")` — and the plot auto-scales to its data instead.

`explicit` stops a `std::string` being silently converted into a `Plot`.

---

### `void startSeries(const std::string& name)`

| | |
| --- | --- |
| **Parameters** | `name` — the legend label for the new series |

Begins a new line on the plot. Later `addPoint` calls go to this series. Each series gets its own line style (solid, dashed, dotted, dash-dot, at two brightnesses — up to 8 styles before repeating).

---

### `void addPoint(double x, double y, double time = -1.0)`

| | |
| --- | --- |
| **Parameters** | `x`, `y` — position in metres · `time` — mission time in seconds; drives the real-time replay |

Appends a point to the most recently started series. If no series has been started, one called `flight` is created automatically. If `time` is left out (or negative) the point is given a time of 0.05 s per point, so non-flight plots replay at a steady rate.

---

### `void addMarker(double x, double y, const std::string& label)`

| | |
| --- | --- |
| **Parameters** | `x`, `y` — position · `label` — text drawn beside the marker; may be empty |

Adds a labelled open circle — for apogee, burnout, stage separation, parachute deployment and so on. Markers are not tied to a series. In the window they appear when the replay reaches them. An **empty label** draws just the dot, which is how you make a scatter plot (Week 12 Hard).

---

### `bool saveSvg(const std::string& path) const`

| | |
| --- | --- |
| **Parameters** | `path` — file to write |
| **Returns** | `false` if the file could not be opened or written; `true` otherwise |

Writes a 960 × 600 SVG in the same style as the window: grid, axis labels, ground line, every series (clipped to the frame and thinned to about 2,000 points so the file stays small), markers, a legend/readout showing each series' final time and range, and a scanline overlay.

---

### `void show() const`

Opens a 960 × 600 window and replays the flight in real time, with a moving dot per series and a live readout of clock, altitude, range and speed. Returns when the window is closed. Keys: **Space** restart · **+/−** speed (1–20×) · **F** fixed frame ↔ fit-to-data · **S** save `plot.png` · **Esc** close.

If the program was built with `-DROCKETSIM_LIVE=OFF`, `show()` prints `(built without the live window; open the SVG instead)` and returns immediately.

---

### `void printAscii(int width = 72, int height = 20) const`

| | |
| --- | --- |
| **Parameters** | `width`, `height` — size of the character grid (does nothing if smaller than 10 × 4) |

Prints a coarse preview to `std::cout`. The ground is drawn as `-`, series use the glyphs `* + o x # @ % &` in order, and markers are `M`. The axis ranges and a key are printed underneath.

---

### `std::size_t pointCount() const`

| | |
| --- | --- |
| **Returns** | Total number of points across all series |

---

## The fixed flight frame

A flight-path plot does **not** auto-scale. It always shows:

- 1 : 1 scale (a metre is the same size on both axes),
- a 1 km grid, with a heavier line every 5 km,
- 10 km of altitude and about 16 km of range, with a 400 m margin so the pad sits just inside the corner.

If any point falls outside, the whole frame **doubles** (2 km grid, 20 km high, …) until it fits. So the same change to your code always looks the same size on screen; compare runs against the grid. Press **F** in the window for a fit-to-data close-up.

> **SCREENSHOT PLACEHOLDER — `images/plot-fixed-vs-fit.png`**
> *Capture:* two side-by-side window captures of the same flight — left in the fixed 1 km-grid frame, right after pressing **F** (fit-to-data).
<!-- ![Fixed frame vs fit-to-data](images/plot-fixed-vs-fit.png) -->

---

## Private members (for reference)

You do not call these, but you will see them if you read `plot.cpp`.

### Nested types

| Type | Fields | Meaning |
| --- | --- | --- |
| `Point` | `x`, `y`, `time` | One recorded sample |
| `Series` | `name`, `std::vector<Point> points` | One line on the plot |
| `Marker` | `x`, `y`, `label` | One labelled dot |
| `Bounds` | `minX`, `maxX`, `minY`, `maxY`, `gridX`, `gridY` | The visible region and grid spacing, in data units |

### Data members

| Member | Type | Meaning |
| --- | --- | --- |
| `title_`, `xLabel_`, `yLabel_` | `std::string` | Text set by the constructor |
| `flightFrame_` | `bool` | `true` when both axis labels are the defaults |
| `series_` | `std::vector<Series>` | All series, in the order started |
| `markers_` | `std::vector<Marker>` | All markers |

### Member functions

| Function | What it does |
| --- | --- |
| `Bounds fitBounds() const` | The smallest region that holds every finite point and marker, plus a 5–8 % margin, with "nice" grid spacing. |
| `Bounds fixedBounds(double aspect) const` | The fixed 1 : 1 flight frame for a drawing area of the given width/height ratio, doubled up to 12 times until every point fits (falls back to `fitBounds`). |
| `Bounds frame(double aspect, bool fit) const` | Chooses between the two: `fixedBounds` for flight-path plots unless `fit` is requested, otherwise `fitBounds`. |
| `double duration() const` | The latest time stamp across all series — the length of the replay. |
| `void drawWindow() const` | The SFML implementation behind `show()`, in `plot_window.cpp`. |

### Helpers in `plot.cpp` (anonymous namespace)

| Helper | What it does |
| --- | --- |
| `double niceStep(double range, int targetTicks)` | Picks a tick spacing of 1, 2 or 5 × a power of ten. |
| `std::string fmt(double value)` | Formats a number with up to 6 significant figures, printing tiny values as `0`. |
| `std::string fixed1(double value)` | Formats a number with exactly one decimal place. |
| `std::string escapeXml(const std::string& text)` | Replaces `& < > "` with XML entities so labels are safe inside SVG. |
| Style constants | `SERIES_COLOUR`, `SERIES_DASH` (8 styles), the phosphor palette, and the 960 × 600 layout margins shared with the window. |

### Helpers in `plot_window.cpp` (anonymous namespace)

| Helper | What it does |
| --- | --- |
| `fmt`, `fixed`, `clock` | Number and mission-clock (`T+mm:ss.s`) formatting for the readout. |
| `thickSegment`, `dashedPolyline` | Draw thick and dashed lines as triangles (SFML has no line width). |
| `struct Labeller` | Draws text anchored left, centre or right of a point. |
| `saveScreenshot` | Copies the window to an image and saves `plot.png`. |

> [!NOTE]
> `Plot` is **not thread-safe**. In Week 12, keep each thread's data in its own containers and only touch the `Plot` from `main` after the threads have been joined.
