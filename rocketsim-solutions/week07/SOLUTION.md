# Week 7 solution — Getting the Rocket to Fly

### References, strings, streams, `new` / `delete`

This walkthrough follows the Week 7 worksheet task by task (E1–E4, M1–M4, H1–H3).

| | |
| --- | --- |
| **Files in this folder** | `src/physics.cpp`, `src/simulation.h/.cpp`, `src/config.cpp`, `src/telemetry.h/.cpp`, `src/main.cpp` (everything else is the unchanged starter) |
| **Run it** | `rocketsim --launch launch.txt --out test` → telemetry table, flight summary, `test.svg`, `test.csv` and the live window |

> [!NOTE]
> The worksheet's Medium tier also asks for a `PITCH_OVER_ALTITUDE` constant (M2), a `printTelemetry` function (M3) and the `report` overloads (M4). The code in `src/` predates those tasks: it hard-codes the 300 m pitch-over and prints telemetry inline. The sections below show the code each of those tasks asks for.

---

## What the week is about

The starter compiles, but the rocket never moves because every function in `physics.cpp` is a stub. The Easy tier makes it fly using only what this week's lectures introduced:
- functions that take the `Rocket` by reference;
- `std::cout` with `<iomanip>`;
- `const`.

Nothing is a class yet. `Rocket` and `Vec2` are plain structs, exactly as the C half of the module would have written them. The point to notice all week is **how data gets into and out of functions**.

---

## Easy — make it fly

### E1 · Mass, gravity and thrust

Every force function takes `const Rocket&`, so it reads the rocket without copying it and cannot change it. Compare with C, where each would take `Rocket*` and use `->` for every access.

```cpp
double totalMass(const Rocket& rocket)
{
    return rocket.dryMass + rocket.fuelMass;
}

bool engineRunning(const Rocket& rocket)
{
    return rocket.fuelMass > 0.0;
}

Vec2 gravityForce(const Rocket& rocket)
{
    Vec2 force;
    force.y = -totalMass(rocket) * GRAVITY;
    return force;
}

Vec2 thrustForce(const Rocket& rocket)        // Easy version, before M2
{
    if (!engineRunning(rocket))
    {
        return Vec2{};
    }
    return scale(unitFromAngle(rocket.launchAngleDeg), rocket.thrust);
}
```

`unitFromAngle` takes **degrees** and converts to radians itself. If you pass radians, the rocket goes almost horizontally.

### E2 · Drag and the time step

Drag has size ½ρC<sub>d</sub>A|v|² and points against the velocity. Multiplying the unit velocity by a *negative* magnitude reverses it. The zero-speed check covers the launch pad, where there is no direction to oppose.

```cpp
Vec2 dragForce(const Rocket& rocket)
{
    const double speed = length(rocket.velocity);
    if (speed == 0.0)
    {
        return Vec2{};
    }
    const double rho = airDensity(rocket.position.y);
    const double magnitude = 0.5 * rho * rocket.dragCoeff * rocket.area * speed * speed;
    return scale(normalised(rocket.velocity), -magnitude);
}
```

`step` is the only function that takes a plain `Rocket&`, because it is the only one that changes the rocket. It applies Euler's method in the order the comment gives. The subtle point is that fuel is burned **after** the forces are computed, so the step uses the mass the rocket actually had during it:

```cpp
void step(Rocket& rocket, double dt)
{
    Vec2 force = add(gravityForce(rocket), thrustForce(rocket));
    force = add(force, dragForce(rocket));

    const Vec2 acceleration = scale(force, 1.0 / totalMass(rocket));
    rocket.velocity = add(rocket.velocity, scale(acceleration, dt));
    rocket.position = add(rocket.position, scale(rocket.velocity, dt));

    if (engineRunning(rocket))
    {
        rocket.fuelMass -= rocket.burnRate * dt;
        if (rocket.fuelMass < 0.0)
        {
            rocket.fuelMass = 0.0;
        }
    }
    rocket.time += dt;
}
```

There is no `->` anywhere in `physics.cpp`.

### E3 · A telemetry table

The telemetry line is the `printf("%8.1f")` idiom in stream form:

```cpp
if (settings.printEvery > 0.0 && rocket.time >= nextPrint)
{
    std::cout << std::fixed << std::setprecision(1)
              << "t=" << std::setw(7) << rocket.time << " s"
              << "  alt=" << std::setw(9) << rocket.position.y << " m"
              << "  range=" << std::setw(9) << rocket.position.x << " m"
              << "  speed=" << std::setw(7) << length(rocket.velocity) << " m/s"
              << "  fuel=" << std::setw(6) << rocket.fuelMass << " kg\n";
    nextPrint += settings.printEvery;
}
```

`std::fixed` and `std::setprecision` are **sticky**: they stay set on `cout`. `std::setw` applies only to the next item, so it is repeated for every column.

**Compare with C.** Passing a `std::string` to `printf("%f", …)` compiles (at most with a warning) and prints garbage or crashes. With `<<`, the compiler picks the right output for every type, and a type with no `operator<<` is a compile error.

### E4 · `const` everywhere

Every force function's parameter is already `const Rocket&`, and the locals that never change (`speed`, `rho`, `magnitude`, `acceleration`) are `const`. `step` could go one step further by computing the total force in a single `const` expression:

```cpp
const Vec2 force = add(add(gravityForce(rocket), thrustForce(rocket)), dragForce(rocket));
```

Adding `rocket.fuelMass = 0;` inside `gravityForce` fails to compile, with GCC's *"assignment of member 'Rocket::fuelMass' in read-only object"* or the equivalent from other compilers. **In one sentence:** `rocket` is a reference to a `const Rocket`, so the compiler refuses any write through it. This is the guarantee `const Rocket&` gives that C's `Rocket*` does not.

### Sanity checks

| Check | Setting | Result |
| --- | --- | --- |
| Rocket equation | `drag_coeff = 0`, thrust along the launch angle | Speed at burnout (t = 15 s) ≈ **547 m/s** (closed form 546.7 m/s) |
| Drag-free flight | As above | Apogee ≈ 18,000 m at t ≈ 70 s; range ≈ 7,370 m; flight time ≈ 131 s |
| Week 7 Easy | Defaults | Apogee ≈ **6,590 m at 40.3 s**; max speed ≈ 432 m/s; range ≈ 1,720 m; flight time ≈ 85 s |

---

## Medium — flight statistics and a gravity turn

### M1 · Flight statistics and markers

`FlightStats` is filled **inside the loop** by remembering the best value seen so far. Week 9 replaces this bookkeeping with `max_element` over the log.

```cpp
if (rocket.position.y > stats.apogee)
{
    stats.apogee = rocket.position.y;
    stats.apogeeTime = rocket.time;
    apogeeX = rocket.position.x;            // remembered for the marker
}
const double speed = length(rocket.velocity);
if (speed > stats.maxSpeed)
{
    stats.maxSpeed = speed;
}
if (stats.burnoutTime == 0.0 && !physics::engineRunning(rocket))
{
    stats.burnoutTime = rocket.time;        // first step with no fuel
    plot.addMarker(rocket.position.x, rocket.position.y, "burnout");
}
```

After the loop, the apogee marker uses the remembered position:

```cpp
plot.addMarker(apogeeX, stats.apogee, "apogee");
```

### M2 · The gravity turn

Add the constant to `physics.h`, beside the others in `namespace physics`:

```cpp
constexpr double PITCH_OVER_ALTITUDE = 300.0;   // m
```

Then, in `thrustForce`, thrust follows the velocity above that altitude:

```cpp
Vec2 thrustForce(const Rocket& rocket)
{
    if (!engineRunning(rocket))
    {
        return Vec2{};
    }
    const Vec2 direction = (rocket.position.y > PITCH_OVER_ALTITUDE)
                               ? normalised(rocket.velocity)
                               : unitFromAngle(rocket.launchAngleDeg);
    return scale(direction, rocket.thrust);
}
```

The constant has a type, a name and a scope, unlike a `#define`, and it is visible to anything that includes `physics.h`. The arc bends over, and range grows from about **1,720 m** to about **2,890 m** (apogee ≈ 6,475 m at 40.0 s).

### M3 · Telemetry to any stream

Move the E3 line into its own function and call it from the loop with `std::cout`:

```cpp
// simulation.h
#include <iosfwd>
void printTelemetry(std::ostream& out, const Rocket& rocket);

// simulation.cpp
void printTelemetry(std::ostream& out, const Rocket& rocket)
{
    out << std::fixed << std::setprecision(1)
        << "t=" << std::setw(7) << rocket.time << " s"
        << "  alt=" << std::setw(9) << rocket.position.y << " m"
        << "  range=" << std::setw(9) << rocket.position.x << " m"
        << "  speed=" << std::setw(7) << length(rocket.velocity) << " m/s"
        << "  fuel=" << std::setw(6) << rocket.fuelMass << " kg\n";
}

// in simulate()
printTelemetry(std::cout, rocket);
```

**Without the `&`:** the call fails with *"use of deleted function `std::basic_ostream(const std::basic_ostream&)`"* (or similar). Passing by value needs a copy, and stream copy constructors are deleted. A stream owns a buffer and a connection to a device, and two copies writing to the same device would make no sense. Because `out` is an `std::ostream&`, the same function also works with a `std::ofstream` or a `std::ostringstream`.

### M4 · Overloading `report`

Declare the three overloads in `simulation.h` (where `FlightStats` lives) and define them in `simulation.cpp`. The default argument goes on the **declaration only**:

```cpp
// simulation.h
void report(const Vec2& v);
void report(const Rocket& r);
void report(const FlightStats& s, int precision = 1);

// simulation.cpp
void report(const Vec2& v)
{
    std::cout << std::fixed << std::setprecision(1) << "(" << v.x << ", " << v.y << ")\n";
}

void report(const Rocket& r)
{
    std::cout << r.name << ": ";
    printTelemetry(std::cout, r);
}

void report(const FlightStats& s, int precision)
{
    std::cout << std::fixed << std::setprecision(precision)
              << "apogee " << s.apogee << " m at " << s.apogeeTime << " s, "
              << "max speed " << s.maxSpeed << " m/s, "
              << "range " << s.range << " m\n";
}
```

Calls from `main`:

```cpp
report(rocket.position);   // Vec2 overload: where it landed
report(rocket);            // Rocket overload
report(stats);             // FlightStats, default precision 1
report(stats, 3);          // FlightStats, precision 3
```

The compiler chooses the overload from the **argument types** at compile time. Writing the default `= 1` on the definition as well is a compile error ("default argument given for parameter 2 … redefinition").

---

## Hard — a launch file and a hand-built log

### H1 · Reading a launch file

`loadLaunchFile` uses `std::ifstream`, `std::getline` and string surgery. Each line is cut at `#`, trimmed, split at `=`, and its value converted with `std::stod`:

```cpp
std::string line;
int lineNumber = 0;
while (std::getline(in, line))
{
    ++lineNumber;
    const std::size_t hash = line.find('#');
    if (hash != std::string::npos)
    {
        line = line.substr(0, hash);
    }
    line = trim(line);
    if (line.empty())
    {
        continue;
    }

    const std::size_t equals = line.find('=');
    const std::string key = trim(line.substr(0, equals));
    const std::string value = trim(line.substr(equals + 1));

    if (key == "name") { rocket.name = value; continue; }
    const double number = std::stod(value);
    if (key == "dry_mass")          rocket.dryMass = number;
    // ... one branch per key ...
    else
    {
        std::cerr << path << ":" << lineNumber << ": unknown key '" << key << "'\n";
    }
}
```

`trim` is a small helper in an anonymous namespace, built on `find_first_not_of` / `find_last_not_of`. A line with no `=` is also reported with its line number. A file that will not open returns `false` and leaves the rocket untouched.

**Deliberately not handled:** `thrust = fifty` makes `std::stod` throw `std::invalid_argument`, and the program ends. Week 11 introduces exceptions, and this is the motivating example.

**Check:** `rocketsim --launch launch.txt` reproduces the default flight exactly, and editing `launch_angle` changes the plot with no rebuild.

### H2 · A hand-built telemetry log

`telemetry.h` declares the structs and functions given on the worksheet, plus a `writeCsv` helper:

```cpp
struct Sample { double time, x, y, speed, fuel; };
struct TelemetryLog { Sample* samples; int size; int capacity; };

TelemetryLog createLog(int capacity);
void append(TelemetryLog& log, const Sample& sample);
bool writeCsv(const TelemetryLog& log, const std::string& path);
void destroyLog(TelemetryLog& log);
```

`append` doubles the capacity when the array is full: it allocates a new array, copies the samples across, and `delete[]`s the old one.

```cpp
void append(TelemetryLog& log, const Sample& sample)
{
    if (log.size == log.capacity)
    {
        const int newCapacity = log.capacity * 2;
        Sample* bigger = new Sample[newCapacity];
        for (int i = 0; i < log.size; ++i)
        {
            bigger[i] = log.samples[i];
        }
        delete[] log.samples;
        log.samples = bigger;
        log.capacity = newCapacity;
    }
    log.samples[log.size] = sample;
    ++log.size;
}
```

`simulate` takes the log as `TelemetryLog&` and appends a `Sample` after every step. `main` creates it with `createLog(1024)`, writes the CSV (header `time,x,y,speed,fuel`), and calls `destroyLog` before returning.

**Why copying is dangerous:** a `TelemetryLog` holds only a *pointer*, so a copy shares the same array. Destroying both copies would `delete[]` it twice. `createLog` returns the struct by value, which is safe only because nothing keeps the original. That double-free is Week 8's opening exercise.

> [!NOTE]
> The worksheet names the file `telemetry.csv`. This solution writes it as `<out>.csv` alongside `<out>.svg`, so a plain run writes `flight.csv` and `--out test` writes `test.csv`.

### H3 · An `--out` option

`main` keeps the base name in a `std::string` (default `"flight"`) and builds both file names by concatenation:

```cpp
else if (arg == "--out" && i + 1 < argc)
{
    outName = argv[++i];
}
// ...
writeCsv(log, outName + ".csv");
plot.saveSvg(outName + ".svg");
```

`rocketsim --launch launch.txt --out test` writes `test.svg` and `test.csv`.

---

## Checkpoint

| Tier | Expected |
| --- | --- |
| Easy | Telemetry table; summary with apogee ≈ **6,590 m at 40.3 s**; an arc on `flight.svg` from the pad to the landing point |
| Medium | Apogee ≈ **6,475 m**, range ≈ **2,890 m**, `burnout` and `apogee` markers on the plot |
| Hard | `rocketsim --launch launch.txt --out test` writes `test.svg` and `test.csv` |

---

## Common mistakes

- **`step` not advancing `rocket.time`.** The loop guard stops after `maxTime / dt` steps and prints "never left the pad".
- **Burning fuel before computing the forces.** The first step then uses the wrong mass: a small error, but visible against the 547 m/s check.
- **Passing radians to `unitFromAngle`.** It takes degrees; radians send the rocket sideways.
- **Forgetting `std::fixed`.** The table comes out in scientific notation.
- **Dropping the `&` from `std::ostream&`** in `printTelemetry`. It does not compile, because streams cannot be copied.
- **Repeating a default argument** on the definition of `report`. Defaults go on the declaration only.
- **Copying a `TelemetryLog` by value.** It "works" until both copies are destroyed.
