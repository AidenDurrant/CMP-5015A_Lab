# Week 7 — Getting the Rocket to Fly!

### References, strings, streams, `new` / `delete`

| | |
| --- | --- |
| **Lecture material** | Pass by `const&` and `&` · `cout` / `<iomanip>` · `std::string` · overloading and default arguments· `const` and `constexpr` · `new[]` / `delete[]` · headers and the two-file model |
| **Starting point** | The `rocketsim/` starter folder |
| **Files you will edit** | `physics.cpp`, `physics.h`, `simulation.cpp`, `main.cpp`, `config.cpp`; new `telemetry.h` / `telemetry.cpp` (Hard) |
| **Reference pages** | [physics](../code-reference/physics.md) · [simulation](../code-reference/simulation.md) · [vec2](../code-reference/vec2.md) · [config](../code-reference/config.md) |

---

## By the end of this lab

- The rocket **flies**: the stationary dot in the starter code becomes a full arc from the launch pad to a landing.
- The console prints a neatly aligned **telemetry table** and a **flight summary**.
- *(Medium)* The flight performs a **gravity turn** and the plot marks **burnout** and **apogee**.
- *(Hard)* The rocket can be loaded from a **launch file**, and every step is recorded in a hand-built **dynamic array** written out as a CSV.

---

## Background

### The problem

You have a working program that does nothing interesting yet. 

It parses its command line, builds a rocket, runs a loop, and plots the result but the rocket never moves, because every physics function is a currently a stub (a small, short piece of code that acts as a temporary placeholder for a method, function, or routine that has not been fully written yet). This week your task is to write the physics that makes the rocket fly.

This week you are tasked to write the code the way you would have in C (i.e., plain functions acting on a struct) but with the C++ tools from this week's lectures. The difference to look for is **how data gets into and out of functions**.

### Pointers vs references

In C, a function that needs to read a large struct without copying it, or needs to change it, takes a pointer:

```c
/* C: lecture 4 style */
double area(const Shape* s) { return s->width * s->height; }
void   grow(Shape* s, double k) { s->width *= k; }
```

In C++ the same intent is written with **references**:

```cpp
// C++
double area(const Shape& s) { return s.width * s.height; }
void   grow(Shape& s, double k) { s.width *= k; }
```

A reference cannot be null, cannot be re-seated to point at something else, and needs no `->` or `&` at the call. A `const&` additionally makes the compiler **refuse** any attempt to change the object.

Look at `physics.h`: every function that only *reads* the rocket takes `const Rocket&`. Exactly one takes a plain `Rocket&`. Before you write anything, work out which one and why.

### The physics model

You do not need any physics beyond this box. The rocket is a point mass; `x` is downrange and `y` is altitude, both in metres.

$$
\begin{aligned}
\mathbf{F}_g &= (0,\; -m g) \\
\mathbf{F}_t &= T\,\hat{\mathbf{d}} \quad \text{while fuel remains, else } \mathbf{0} \\
\mathbf{F}_d &= -\tfrac{1}{2}\,\rho(y)\,C_d\,A\,|\mathbf{v}|\,\mathbf{v} \\
\rho(y) &= \rho_0\, e^{-y/H}, \quad \rho_0 = 1.225\ \text{kg/m}^3,\ H = 8500\ \text{m}
\end{aligned}
$$

Each call to `step(rocket, dt)` advances time with **Euler's method**, in this order:

$$
\begin{aligned}
\mathbf{a} &= (\mathbf{F}_g + \mathbf{F}_t + \mathbf{F}_d) / m \\
\mathbf{v} &\leftarrow \mathbf{v} + \mathbf{a}\,\Delta t \\
\mathbf{p} &\leftarrow \mathbf{p} + \mathbf{v}\,\Delta t \\
m_{fuel} &\leftarrow \max(0,\; m_{fuel} - \dot{m}\,\Delta t) \\
t &\leftarrow t + \Delta t
\end{aligned}
$$

Mass *m* is dry mass plus remaining fuel. The default rocket weighs 200 kg at lift-off, has 5000 N of thrust and burns for 15 s.

---

## Before you start

1. Unzip the `rocketsim` folder and configure it **straight away** — the first `cmake -B build` downloads SFML and takes a few minutes.
   ```sh
   cd rocketsim
   cmake -B build
   cmake --build build
   ```
2. Run `./build/rocketsim --demo`. A window replays a parabola in real time. Press **+** to speed it up, **Space** to restart, **Esc** to close. Note the **1 km grid** — it is the same on every run, so compare flights against the grid, not the window edges.
3. Run `./build/rocketsim` on its own and confirm it prints **`The rocket never left the pad`**.
4. Read `vec2.h` and `physics.h` **in full**. They are short, and you will use every function in them.

![The demo parabola](../images/week07/demo-parabola.png)

> [!NOTE]
> Can't build with the window? Use `cmake -B build -DROCKETSIM_LIVE=OFF` and open `flight.svg` in a web browser instead. Every task works this way.

---

## Easy — make it fly

### Task E1 · Mass, gravity and thrust

In `physics.cpp`, implement `totalMass`, `engineRunning`, `gravityForce` and `thrustForce`.

- Thrust acts along `unitFromAngle(rocket.launchAngleDeg)` while fuel remains.
- Use the `Vec2` helpers in `vec2.h` (`add`, `scale`, …).
- **Do not write `->` anywhere.**

Build and run. With no drag and no `step` yet nothing will move — that comes next. Make sure it compiles cleanly with no warnings.

**Done when:** the four functions compile, and each one does exactly what its comment in `physics.h` says.

---

### Task E2 · Drag and the time step

Implement `dragForce` using `airDensity` and the drag formula above. It must point **against** the velocity.

Then implement `step` using Euler's method, in the order given in its comment.

> [!TIP]
> Try implementing `step` *before* `dragForce` (leave drag returning zero). Gravity will eventually win and the rocket will come down — check the summary to see how high it got. Then add drag and watch the apogee drop.

**Done when:** the rocket flies an arc and lands. The summary's flight time and range are no longer zero.

![The rocket's first flight](../images/week07/easy-arc.png)

---

### Task E3 · A telemetry table

In `simulate`, print one telemetry line every `settings.printEvery` seconds. Use `std::cout` with `std::fixed`, `std::setprecision(1)` and `std::setw` so the columns line up:

```text
t=  12.00 s  alt=  1234.5 m  range=   56.7 m  speed=  123.4 m/s  fuel=  30.0 kg
```

`printStats` at the bottom of the same file is a worked example of the formatting.

**Compare with C:** in lecture 5 you would have written `printf("%8.1f", …)`. The output is the same, but with `<<` the **compiler checks the types**. Try passing a `std::string` to `%f` in `printf` and to `<<` in C++, and compare what happens.

**Done when:** a table of aligned columns scrolls past during the flight, one row per second.

![Telemetry table](../images/week07/telemetry-table.png)

---

### Task E4 · `const` everywhere

1. Mark every parameter and local variable that is never modified as `const`.
2. Then **deliberately** add `rocket.fuelMass = 0;` inside `gravityForce` and build.
3. Read the compiler error. Then remove the line.

This is what `const Rocket&` gives you over C's `Rocket*`. The compiler enforces that this function only reads.

**Done when:** you can explain the error message.

---

### Sanity checks for your physics

Use these to check your `step` and forces against known answers (default rocket, `dt = 0.01`).

| Check | How | Expected |
| --- | --- | --- |
| **Rocket equation** | Temporarily set `dragCoeff = 0` | Speed at burnout (t = 15 s) ≈ **547 m/s** |
| **Drag-free flight** | As above | Apogee ≈ **18,000 m** at t ≈ 70 s; range ≈ 7,370 m; flight time ≈ 131 s |
| **Week 7 Easy** | Defaults | Apogee ≈ **6,590 m** at **40.3 s**; max speed ≈ 432 m/s; range ≈ 1,720 m; flight time ≈ 85 s |

> [!NOTE]
> The first check has a closed-form answer. Exhaust velocity is *v*<sub>e</sub> = *T* / ṁ = 500 m/s, so the ideal Δv = *v*<sub>e</sub> ln(200 / 50) ≈ 693 m/s along the launch direction, minus *g* × 15 s lost vertically to gravity. If you are within a few m/s, your `step` is correct.

---

## Medium — flight statistics and a gravity turn

### Task M1 · Flight statistics and markers

Fill in the remaining `FlightStats` fields **inside the loop**:

| Field | Meaning |
| --- | --- |
| `apogee`, `apogeeTime` | The highest `position.y` seen, and when |
| `maxSpeed` | The highest speed (`length(velocity)`) seen |
| `burnoutTime` | The first step at which `engineRunning` becomes false |

Call `plot.addMarker` at burnout (during the loop) and at apogee (after the loop). Both labels must appear on the SVG.

**Done when:** the summary shows the Easy sanity-check numbers, and the plot shows `burnout` and `apogee` markers.

---

### Task M2 · The gravity turn

Real rockets do not hold a fixed angle all the way up. After clearing the pad they **pitch over** and let the thrust follow the direction of travel, so gravity gradually bends the path into an efficient arc.

1. Add `constexpr double PITCH_OVER_ALTITUDE = 300.0;` to `physics.h`.
2. Change `thrustForce` so that **above** that altitude, thrust points along `normalised(velocity)` instead of the launch angle.

**Done when:** the 85° launch bends into a proper arc and lands about **2,890 m** downrange instead of 1,720 m.

![Gravity turn with markers](../images/week07/gravity-turn.png)

---

### Task M3 · Telemetry to any stream

Move the telemetry line into its own function in `simulation.cpp`:

```cpp
void printTelemetry(std::ostream& out, const Rocket& rocket);
```

Call it with `std::cout`. Then **remove the `&`** from `std::ostream&` and read the error.

Streams cannot be copied, so this only compiles when `out` is a reference. As a bonus, the same function can now write to a file stream as well as the console.

**Done when:** output is unchanged, and you can explain why the version without `&` fails.

---

### Task M4 · Overloading `report`

Write three functions with the **same name** and use them from `main`:

```cpp
void report(const Vec2& v);
void report(const Rocket& r);
void report(const FlightStats& s, int precision = 1);
```

One name, three parameter lists, one default argument. Call each from `main` (e.g. report the final position, the rocket, and the stats at two different precisions).

**Done when:** all three are called from `main` and the compiler picks the right one each time.

---

## Hard — a launch file and a hand-built log

### Task H1 · Reading a launch file

Implement `loadLaunchFile` in `config.cpp` using `std::ifstream`, `std::getline`, `find`, `substr` and `std::stod`.

- Strip `#` comments, skip blank lines, and trim spaces around keys and values.
- Report unknown keys on `std::cerr` **with the line number**.
- See the [launch file format](../code-reference/build-and-project.md#launchtxt) for every key.

**Done when:** `rocketsim --launch launch.txt` reproduces the default flight **exactly**, and changing `launch_angle` in the file changes the plot without rebuilding.

> [!WARNING]
> Try setting a value to `fifty`. `std::stod` **throws**, and the program dies. Leave it like that — Week 11 fixes it properly with exceptions.

---

### Task H2 · A hand-built telemetry log

Create `telemetry.h` / `telemetry.cpp` with a **C-style dynamic array**, written the way lecture 3 would, but with `new[]` / `delete[]` instead of `malloc` / `free`:

```cpp
struct Sample { double time, x, y, speed, fuel; };

struct TelemetryLog { Sample* samples; int size; int capacity; };

TelemetryLog createLog(int capacity);
void append(TelemetryLog& log, const Sample& s);
void destroyLog(TelemetryLog& log);
```

- `append` must **double the capacity** when the array is full: allocate a new array, copy across, `delete[]` the old one.
- Record every step in `simulate`.
- Write `telemetry.csv` with `std::ofstream` and open it in a spreadsheet.
- Call `destroyLog` before `main` returns.

> [!CAUTION]
> Do **not** copy a `TelemetryLog` by value anywhere. Think about what a copy of this struct contains. Next week shows exactly why that would crash.

**Done when:** `telemetry.csv` opens in a spreadsheet with one row per time step, and a chart of altitude against time matches your plot.

![telemetry.csv in a spreadsheet](../images/week07/telemetry-csv.png)

---

### Task H3 · An `--out` option

Add `--out NAME` so the SVG and CSV are written as `NAME.svg` and `NAME.csv`. Build the file names with `std::string` concatenation.

**Done when:** `rocketsim --out test` writes `test.svg` and `test.csv`.

---

## Checkpoint

Before moving on, run your program and check each of the following yourself:

| Tier | Check that… |
| --- | --- |
| Easy | A telemetry table, a summary with **apogee ≈ 6,590 m at 40.3 s**, and `flight.svg` showing an arc from the pad to the landing point with the ground line under it |
| Medium | Apogee **≈ 6,475 m**, range **≈ 2,890 m**, with `burnout` and `apogee` markers on the plot |
| Hard | `rocketsim --launch launch.txt --out test` writes `test.svg` and `test.csv` |

---

## Watch out for

- **`step` must advance `rocket.time`.** If it does not, the loop stops after its step-count guard and prints the "never left the pad" hint.
- **Burn the fuel after computing the forces**, not before, or the first step runs with the wrong mass.
- **`std::fixed` is sticky.** Once set on `cout` it stays set — which is why `printStats` sets it again.
- **Resist `printf`.** It still works in C++, but the point of this week is that `<<` is type-checked. Habits formed now matter in Week 11, when `<<` and `>>` are the whole point.

---
