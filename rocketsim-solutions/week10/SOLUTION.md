# Week 10 solution — Templates and lambdas

### Function templates, class templates and lambdas with captures

This walkthrough follows the Week 10 worksheet task by task (G1–G2, E1–E3, M1–M2, H1–H3). Every task starts from the Week 9 model solution in `../week09/src/`.

| | |
| --- | --- |
| **Files in this folder** | New: `src/util.h`, `src/history.h`, `src/atmosphere.h/.cpp` (replaces Week 9's `physics.h/.cpp` and `wind.h`), `src/mission_control.h`. Changed: `src/rocket_spec.h`, `src/rocket.h/.cpp`, `src/engine.h/.cpp`, `src/force.h/.cpp`, `src/config.cpp`, `src/simulation.h/.cpp`, `src/main.cpp`. Also `launch_b.txt` and `launch_twostage.txt` (which now sets `engine = liquid`) |
| **Run it** | `rocketsim --launch launch.txt --launch launch_twostage.txt --events` → `use_count()` before and after the fleet is built, `Launching … (solid motor)` and `(liquid engine)`, a black box line at each touchdown, the ranking, `Mission control: 2 of 2 rockets alive`, then `0 alive` after the fleet is cleared |

> [!NOTE]
> `src/` is the project with every tier done. The `Rocket` constructor therefore calls `makeEngine(spec.engineType)` from H2, and a rocket with no `engine` key gets a `SolidMotor`. With only G1 done, every rocket gets a `LiquidEngine` as in Week 9. The flights are the same either way.

---

## What the week is about

The Week 9 code had the same logic written more than once:
- range checks for the throttle and for the altitude in the density model;
- a linear blend for the wind, which the apogee marker also wants for a `Vec2`;
- four maps in `main` that every rocket passed through in the same order.

**Function templates** (`clamp`, `lerp`) write the first two once for every type. A **class template** (`History<T, N>`) writes a ring buffer once for any element type and capacity. **Lambdas** turn the repeated fleet code into one call, and turn the engine factory and the mission control count into small inline functions.

The smart pointers that hold the engine, the forces, the atmosphere and the fleet are given code (G1, G2 and the given lines in E3, M2 and H3), not tasks.

The physics is the same as Week 9, so the flights are unchanged: Starter-1 still lands about **3,820 m** downrange with the default wind.

---

## Given code

Each step below matches the step of the same number in the worksheet. "Week 9" is the line in `../week09/src/`, and "Week 10" is where the result is in `src/`. The Week 10 line numbers differ because later tasks add code too.

### G1 · The rocket holds its engine

**`rocket.h`**

| Step | Week 9 | Becomes | Week 10 |
| --- | --- | --- | --- |
| 1 | line 8, `#include <vector>` | `#include <memory>` added beside it | line 6 |
| 2 | line 18, `Rocket(const RocketSpec& spec, Engine* engine);` | `explicit Rocket(const RocketSpec& spec);` | line 19 |
| 3 | (new) | the deleted copy members, the move members and `~Rocket();` | lines 24 to 28 |
| 4 | line 44, `static int liveCount();` | deleted | |
| 5 | line 60, `Engine* engine_;` | `std::unique_ptr<Engine> engine_;` | line 68 |

The class after G1:

```cpp
class Engine;                 // forward declaration only, unchanged

class Rocket
{
public:
    explicit Rocket(const RocketSpec& spec);

    Rocket(const Rocket&) = delete;
    Rocket& operator=(const Rocket&) = delete;
    Rocket(Rocket&&) noexcept;
    Rocket& operator=(Rocket&&) noexcept;
    ~Rocket();   // defined in rocket.cpp, where Engine is a complete type

    Engine& engine() const { return *engine_; }   // unchanged
    // ...
private:
    // ...
    std::unique_ptr<Engine> engine_;
};
```

**`rocket.cpp`**

| Step | Week 9 | Becomes | Week 10 |
| --- | --- | --- | --- |
| 6 | line 12, `Rocket::Rocket(const RocketSpec& spec, Engine* engine)` | `Rocket::Rocket(const RocketSpec& spec)` | line 11 |
| 7 | line 19, `engine_(engine)` | `engine_(std::make_unique<LiquidEngine>())`, then `engine_(makeEngine(spec.engineType))` after H2 | line 18 |
| 8 | (new) | the three `= default` lines | lines 24 to 26 |
| 9 | lines 63 to 66, `Rocket::liveCount()` | deleted | |

```cpp
Rocket::Rocket(const RocketSpec& spec)
    : name_(spec.name),
      // ...
      engine_(makeEngine(spec.engineType))        // G1 alone: std::make_unique<LiquidEngine>()
{
}

Rocket::~Rocket() = default;
Rocket::Rocket(Rocket&&) noexcept = default;
Rocket& Rocket::operator=(Rocket&&) noexcept = default;
```

**`main.cpp`**

| Step | Week 9 | Becomes |
| --- | --- | --- |
| 10 | line 65, `std::map<std::string, LiquidEngine> engines;` | deleted |
| 11 | line 69, `engines.emplace(name, LiquidEngine{});` | deleted |
| 12 | line 70, `fleet.emplace(name, Rocket(spec, &engines.at(name)));` | `fleet.emplace(name, Rocket(spec));` |

Step 12 compiles because `Rocket(spec)` is a temporary, so `emplace` moves it into the map with the move constructor from step 3. E3 then replaces the whole fleet block, so none of these `main.cpp` lines survive into `src/`.

**Why the `= default` lines are in the `.cpp`.** `rocket.h` only forward-declares `Engine`. Destroying a `unique_ptr<Engine>` needs the complete `Engine` type. If `~Rocket()` were left implicit, or defaulted in the header, any file that includes `rocket.h` without `engine.h` fails with *"invalid application of 'sizeof' to incomplete type 'Engine'"*.

**Why the copy members are deleted.** A `unique_ptr` cannot be copied, so the compiler could not write a copy constructor anyway. Writing `= delete` turns a confusing error about `unique_ptr` internals into *"use of deleted function `Rocket::Rocket(const Rocket&)`"*.

### G2 · `simulate` holds its forces

All in **`simulation.cpp`**:

| Step | Week 9 | Becomes | Week 10 |
| --- | --- | --- | --- |
| 1 | line 8, `#include <iostream>` | `#include <memory>` and `#include <vector>` added | line 8 (`<vector>` also arrives through `telemetry.h`) |
| 2 | line 37, `Force* forces[3] = {new Gravity(), new Thrust(), new Drag(settings.wind)};` | the four lines below | lines 43 to 46 |
| 3 | line 63, `for (const Force* force : forces)` | `for (const auto& force : forces)` | line 74 |
| 4 | lines 97 to 100, the `delete force;` loop | deleted | |

```cpp
std::vector<std::unique_ptr<Force>> forces;
forces.push_back(std::make_unique<Gravity>());
forces.push_back(std::make_unique<Thrust>());
forces.push_back(std::make_unique<Drag>(settings.atmosphere));   // settings.wind until M2

Vec2 total;
for (const auto& force : forces)
{
    total += force->apply(rocket);       // unchanged
}
rocket.update(total, settings.dt);
```

`for (auto force : forces)` would try to copy each `unique_ptr` and does not compile. The forces are destroyed when `forces` goes out of scope at the end of `simulate`.

**Check:** after G1 and G2, with no tasks done, the program builds and the flights match Week 9 exactly. Applying the twelve G1 steps and four G2 steps to `../week09/src/` compiles with no warnings.

---

## Easy — function templates and a lambda

### E1 · `clamp`

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `util.h` (new) | The `clamp` template | lines 10 to 15 |
| `engine.cpp` | `#include "util.h"`, and `LiquidEngine::setThrottle` calls `clamp` | lines 4 and 32 |
| `physics.cpp` | `physics::airDensity` calls `clamp` | moved by M2 to `atmosphere.cpp` line 13 |
| `main.cpp` | `#include "util.h"`, and the `--events` throttle calls `clamp` | lines 22 and 83 |

In `util.h`:

```cpp
template <typename T>
T clamp(T value, T low, T high)
{
    static_assert(std::is_arithmetic_v<T>, "clamp needs a number");   // H1
    return value < low ? low : (value > high ? high : value);
}
```

Templates go in a header because the compiler needs the whole definition at every call to generate the version for that type.

The three uses:

| File | Function | Week 9 | Week 10 |
| --- | --- | --- | --- |
| `engine.cpp` | `LiquidEngine::setThrottle` | `throttle_ = fraction < 0.0 ? 0.0 : (fraction > 1.0 ? 1.0 : fraction);` | `throttle_ = clamp(fraction, 0.0, 1.0);` |
| `physics.cpp`, then `atmosphere.cpp` after M2 | `physics::airDensity`, then `Atmosphere::density` | `if (altitude < 0.0) altitude = 0.0;` | `std::exp(-clamp(altitude, 0.0, 1.0e9) / SCALE_HEIGHT)` |
| `main.cpp` | `main`, the `--events` block | `const double throttle = 0.5;` | `const double throttle = clamp(0.5, 0.0, 1.0);` |

All three arguments must be the **same** type. `clamp(altitude, 0, 1e9)` fails to deduce `T`, because it would be both `double` and `int`.

### E2 · `lerp`

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `util.h` | The `lerp` template | lines 19 to 23 |
| `wind.h` | `#include "util.h"`, and `windAt` calls `lerp` with `double` | moved by M2 to `atmosphere.cpp` line 33 |
| `simulation.cpp` | `#include "util.h"`, and `simulate` places the apogee marker with `lerp` on `Vec2` | lines 15 and 124 to 128 |

In `util.h`:

```cpp
template <typename T, typename U>
T lerp(const T& a, const T& b, U t)
{
    return a + (b - a) * t;
}
```

- **`double`**, in `windAt` in **`wind.h`**. The Week 9 line `return lower->second + t * (upper->second - lower->second);` becomes:

  ```cpp
  auto lower = std::prev(upper);
  const double t = (altitude - lower->first) / (upper->first - lower->first);
  return lerp(lower->second, upper->second, t);          // T = double, U = double
  ```

  After M2 this code lives in `Atmosphere::windAt`.

- **`Vec2`**, in `simulate` in **`simulation.cpp`**, after the flight loop. Week 9 had `plot.addMarker(highest->x, highest->y, "apogee");`. Week 10 has:

  ```cpp
  const Vec2 a{highest->x, highest->y};
  const auto next = std::next(highest) != log.end() ? std::next(highest) : highest;
  const Vec2 b{next->x, next->y};
  const Vec2 marker = lerp(a, b, 0.5);                    // T = Vec2, U = double
  plot.addMarker(marker.x, marker.y, "apogee");
  ```

  The check on `std::next(highest)` stops the code reading past the end if the highest sample is the last one.

The `Vec2` version compiles only because Week 8 gave `Vec2` `operator+`, `operator-` and `operator*(Vec2, double)`. A template asks nothing of its type beyond the operations it uses.

### E3 · One lambda builds the fleet

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `main.cpp` | `#include <memory>` in place of `<map>`. The `fleet` vector and the `addRocket` lambda replace the four maps. The two calls to `addRocket`, the flight loop header and the plot title change | include line 10, fleet line 51, lambda lines 53 to 59, calls lines 62 and 70, plot title line 75, loop line 79 |

Week 9's four maps (`specs`, `settingsByName`, `engines`, `fleet`) become one vector and one lambda. Final form, including the M2 and H3 lines:

```cpp
auto atmosphere = std::make_shared<Atmosphere>();                    // M2
std::vector<std::pair<std::shared_ptr<Rocket>, SimSettings>> fleet;
MissionControl control;                                              // H3

// fleet, atmosphere and control are changed or shared by the lambda and
// outlive it, so capture by reference. settings is a by-value parameter
// because the lambda changes its own copy before storing it.
auto addRocket = [&](const RocketSpec& spec, SimSettings settings) {
    settings.atmosphere = atmosphere;                                // M2
    settings.printEvery = quiet ? 0.0 : settings.printEvery;
    auto rocket = std::make_shared<Rocket>(spec);
    control.track(rocket);                                           // H3
    fleet.emplace_back(rocket, settings);
};

if (launchFiles.empty())
{
    addRocket(RocketSpec{}, SimSettings{});
}
for (const std::string& file : launchFiles)
{
    RocketSpec spec;
    SimSettings settings;
    if (loadLaunchFile(file, spec, settings))
    {
        addRocket(spec, settings);
    }
}
```

The flight loop becomes `for (auto& [rocket, settings] : fleet)`, with `simulate(*rocket, settings, plot, log)` and `rocket->name()`. The plot title uses `fleet.front().first->name()`.

**Answers to the comment questions:**
- `fleet` must be captured by reference because the lambda adds to it. A by-value capture would add to a copy inside the lambda, and the copy is also `const` unless the lambda is `mutable`, so `[=]` does not compile.
- `SimSettings` is taken by value because the lambda changes it (quiet, atmosphere) before storing it. A `const&` could not be changed, and changing the caller's object would be surprising.
- The throttle event lambda runs during the flight, long after the `if (withEvents)` block that created `throttle` has ended. A reference to `throttle` would dangle, so it captures a copy.

Order: the Week 9 `std::map` flew rockets in name order. The vector flies them in command line order, so the CSV (written from the last log) now belongs to the last rocket on the command line. The ranking is sorted, so it is unchanged.

---

## Medium — class templates

### M1 · A black box recorder

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `history.h` (new) | The `History<T, N>` class template | the whole file |
| `simulation.cpp` | `#include "history.h"`. In `simulate`, declare the black box, push each sample, print the line at touchdown | include line 14, declaration line 48, push line 83, print lines 136 to 141 |

`history.h`:

```cpp
template <typename T, std::size_t N>
class History
{
public:
    void push(const T& value)
    {
        data_[(start_ + count_) % N] = value;
        if (count_ < N) { ++count_; }
        else            { start_ = (start_ + 1) % N; }   // overwrite the oldest
    }

    std::size_t size() const { return count_; }
    static constexpr std::size_t capacity() { return N; }
    const T& operator[](std::size_t i) const { return data_[(start_ + i) % N]; }   // oldest first
    const T& latest() const { return data_[(start_ + count_ - 1) % N]; }

private:
    std::array<T, N> data_{};
    std::size_t start_ = 0;
    std::size_t count_ = 0;
};
```

`start_` marks the oldest value and `count_` how many are held. When the buffer is full, `push` writes over the oldest value and moves `start_` on one.

**Why `History<Sample, 100>` and `History<Sample, 50>` are different types:** `N` is part of the template arguments, so each value of `N` makes the compiler generate a separate class, with its own `std::array` size fixed at compile time.

In `simulate` in **`simulation.cpp`**:

```cpp
History<Sample, 100> blackBox;
// every step, after the log
log.push_back(Sample{rocket.time(), rocket.position().x, rocket.position().y, speed, rocket.fuel()});
blackBox.push(log.back());

// after the loop
if (settings.printEvery > 0.0 && blackBox.size() > 0)
{
    std::cout << "  black box: last " << blackBox.size() << " samples, from t=" << blackBox[0].time
              << " s (alt " << blackBox[0].y << " m) to t=" << blackBox.latest().time << " s (alt "
              << blackBox.latest().y << " m)\n";
}
```

At `dt = 0.01`, 100 samples cover the last **1 s** of flight, and the last altitude is at or just below zero. `src/` skips the line with `--quiet`.

### M2 · An atmosphere built on your templates

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `atmosphere.h` (new) | The `Atmosphere` class | the whole file |
| `atmosphere.cpp` (new) | The constructors, `density` and `windAt` | the whole file |
| `force.h` | `#include "atmosphere.h"` in place of `"wind.h"`. `Drag` takes and holds `std::shared_ptr<const Atmosphere>` | include line 10, constructor line 37, member line 42 |
| `force.cpp` | `#include "physics.h"` removed (the atmosphere arrives through `force.h`). `Atmosphere::GRAVITY`, `Atmosphere::PITCH_OVER_ALTITUDE`, and `atmosphere_->windAt` and `->density` in `Drag::apply` | lines 7, 18, 25, 29 and 36 |
| `simulation.h` | `#include "atmosphere.h"` in place of `"wind.h"`. `SimSettings::atmosphere` replaces `SimSettings::wind` | include line 12, member line 34 |
| `simulation.cpp` | `Drag` is built from `settings.atmosphere` | line 46 |
| `main.cpp` | The shared atmosphere, `settings.atmosphere = atmosphere;` in `addRocket`, and the three `use_count()` lines | lines 48 and 49, 54, 73 and 108 |
| `physics.h`, `physics.cpp`, `wind.h` | Deleted | |

`atmosphere.h`:

```cpp
class Atmosphere
{
public:
    using WindProfile = std::map<double, double>;   // altitude (m) -> wind (m/s, +x)

    Atmosphere();                                   // the Week 9 default profile
    explicit Atmosphere(WindProfile wind);

    double density(double altitude) const;          // clamp
    double windAt(double altitude) const;           // lower_bound + lerp

    static constexpr double GRAVITY = 9.81;
    static constexpr double SEA_LEVEL_DENSITY = 1.225;
    static constexpr double SCALE_HEIGHT = 8500.0;
    static constexpr double PITCH_OVER_ALTITUDE = 300.0;

private:
    WindProfile wind_;
};
```

`atmosphere.cpp`:

```cpp
Atmosphere::Atmosphere() : wind_{{0.0, 2.0}, {1000.0, 8.0}, {5000.0, 20.0}, {15000.0, 35.0}} {}

double Atmosphere::density(double altitude) const
{
    return SEA_LEVEL_DENSITY * std::exp(-clamp(altitude, 0.0, 1.0e9) / SCALE_HEIGHT);
}

double Atmosphere::windAt(double altitude) const
{
    if (wind_.empty()) return 0.0;
    auto upper = wind_.lower_bound(altitude);
    if (upper == wind_.begin()) return upper->second;
    if (upper == wind_.end())   return std::prev(upper)->second;
    auto lower = std::prev(upper);
    const double t = (altitude - lower->first) / (upper->first - lower->first);
    return lerp(lower->second, upper->second, t);
}
```

`physics.h/.cpp` and `wind.h` are deleted. In `force.cpp`, `Gravity` uses `Atmosphere::GRAVITY`, `Thrust` uses `Atmosphere::PITCH_OVER_ALTITUDE`, and `Drag::apply` calls `atmosphere_->windAt(…)` and `atmosphere_->density(…)`.

The sharing is given. `main` in **`main.cpp`** prints:

```cpp
std::cout << "Atmosphere use_count before the fleet: " << atmosphere.use_count() << '\n';
// ... addRocket calls ...
std::cout << "Atmosphere use_count with " << fleet.size() << " rockets: " << atmosphere.use_count() << '\n';
// ... after fleet.clear() (H3) ...
std::cout << "Atmosphere use_count now: " << atmosphere.use_count() << '\n';
```

With two launch files this prints **1**, then **3** (`main`'s copy plus one in each rocket's settings), then **1** once the fleet is cleared. The `Drag` forces exist only while `simulate` runs, so they add one only during a flight. Without H3 there is no `fleet.clear()`, and the last line prints 3.

---

## Hard — `static_assert`, a table of lambdas and mission control

### H1 · Checking a template with `static_assert`

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `util.h` | `#include <type_traits>` and the `static_assert` in `clamp` | lines 6 and 13 |
| `history.h` | The `static_assert` on `N`, with its comment | lines 13 to 15 |
| `main.cpp` | Temporary test calls only | not in `src/`, because they do not compile |

`util.h`, the first line of `clamp`:

```cpp
static_assert(std::is_arithmetic_v<T>, "clamp needs a number");
```

`history.h`, inside the class above `public:`:

```cpp
// With N == 0, push() would compute % 0, which is undefined. Stop it at
// compile time instead.
static_assert(N > 0, "History needs room for at least one value");
```

`(start_ + count_) % N` divides by `N`, and division by zero is undefined behaviour, so a `History<T, 0>` would crash or worse on its first `push`. The assert turns that into a compile error. A `static_assert` in a class template is checked when the class is generated for a particular `N`, so `History<double, 0> h;` fails with *"static assertion failed … History needs room for at least one value"*.

The two error messages for `clamp(Vec2{}, Vec2{}, Vec2{})`:

| Version | Error |
| --- | --- |
| No `static_assert` | Errors **inside** the template body: no `operator<` for `Vec2`, followed by every candidate `operator<` the compiler tried. Often a page long |
| With `static_assert` | *"static assertion failed: clamp needs a number"*, with a note on where the template was used. Some compilers still report the `operator<` errors after it, but the first line now says what is wrong |

A suitable comment for `util.h`: *without the assert, calling clamp on a Vec2 gives a page of errors about operator< inside the template, with it the first error is "clamp needs a number" at the call.*

**Why `clamp` without the assert compiles for `std::string`:** `std::string` has `<` and `>`, which compare alphabetically, and those are the only operations the body uses. A template checks operations, not meaning.

With `<algorithm>` included, an unqualified `clamp` call on `std::string` values can also find `std::clamp` through argument-dependent lookup and be ambiguous. `::clamp(…)` avoids that.

### H2 · An engine factory from a table of lambdas

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `rocket_spec.h` | `std::string engineType = "solid";` in `RocketSpec` | line 19 |
| `config.cpp` | `"engine"` in `KNOWN_KEYS`, and the `engine` key in `loadLaunchFile` | lines 28 and 81 to 84 |
| `launch_twostage.txt` | `engine = liquid` | line 10 |
| `engine.h` | `#include <memory>` and the `makeEngine` declaration | lines 6 and 44 |
| `engine.cpp` | `#include <functional>`, `<map>` and `<stdexcept>`, and the `makeEngine` definition | lines 6 to 8 and 35 to 50 |
| `rocket.cpp` | The constructor calls `makeEngine` | line 18 |
| `main.cpp` | The `Launching` line prints the engine name | line 87 |

`rocket_spec.h` gains `std::string engineType = "solid";`, and `config.cpp` adds `"engine"` to `KNOWN_KEYS` and stores it:

```cpp
else if (key == "engine")
{
    spec.engineType = value;
}
```

`launch_twostage.txt` sets `engine = liquid`.

The factory in **`engine.cpp`**:

```cpp
std::unique_ptr<Engine> makeEngine(const std::string& type)
{
    // One lambda per engine type. They use nothing from the surrounding
    // function, so they need no captures.
    static const std::map<std::string, std::function<std::unique_ptr<Engine>()>> makers = {
        {"solid", [] { return std::make_unique<SolidMotor>(); }},
        {"liquid", [] { return std::make_unique<LiquidEngine>(); }},
    };
    const auto found = makers.find(type);
    if (found == makers.end())
    {
        throw std::invalid_argument("unknown engine type '" + type + "'");
    }
    return found->second();
}
```

Each lambda returns a `unique_ptr<SolidMotor>` or `unique_ptr<LiquidEngine>`, which converts to the `std::function`'s `unique_ptr<Engine>` return type. `find` is used, not `operator[]`, because `operator[]` would insert an empty `std::function` for an unknown name (the same trap as `windAt` in Week 9). `static` builds the table once rather than on every call.

In **`main.cpp`**, the Week 9 line `std::cout << "Launching " << name << "...\n";` becomes:

```cpp
std::cout << "Launching " << rocket->name() << " (" << rocket->engine().name() << ")...\n";
```

giving `Launching Starter-1 (solid motor)...` and `Launching TwoStage (liquid engine)...`. The throw becomes a `RocketSpecError` in Week 11.

### H3 · Mission control

| File | What changes | Where in `src/` |
| --- | --- | --- |
| `mission_control.h` (new) | The given class, with `alive()` written as `std::count_if` | the whole file, `alive()` at lines 20 to 24 |
| `main.cpp` | `#include "mission_control.h"`, `MissionControl control;`, `control.track(rocket)` in `addRocket`, and the two mission control lines around `fleet.clear()` | lines 17, 52, 57 and 105 to 107 |

`alive()` in **`mission_control.h`**:

```cpp
int alive() const
{
    return static_cast<int>(std::count_if(tracked_.begin(), tracked_.end(),
                                          [](const std::weak_ptr<Rocket>& w) { return !w.expired(); }));
}
```

`count_if` returns a `std::ptrdiff_t`, so the result is cast to `int`. The lambda captures nothing because it only reads its parameter.

In **`main.cpp`**, after the ranking:

```cpp
std::cout << "Mission control: " << control.alive() << " of " << control.tracked() << " rockets alive\n";
fleet.clear();
std::cout << "Mission control after clearing the fleet: " << control.alive() << " alive\n";
```

With two launch files the output is `2 of 2 rockets alive`, then `0 alive`. `fleet.clear()` drops the last `shared_ptr` to each rocket, so the rocket and its engine are deleted, and each `weak_ptr` reports `expired()`.

---

## Checkpoint

| Tier | Expected |
| --- | --- |
| Easy | Flights unchanged from Week 9 (Starter-1 range about **3,820 m**). `clamp` in `setThrottle`, `density` and the events throttle. `lerp` with `double` in `windAt` and with `Vec2` for the apogee marker. One `addRocket` lambda, called for the default rocket and for each launch file |
| Medium | `black box: last 100 samples, from t=… to t=…` at each touchdown. `Atmosphere use_count` prints **1**, then **1 + number of rockets**, then **1** after the fleet is cleared |
| Hard | `Launching TwoStage (liquid engine)...` and `(solid motor)` for Starter-1. `Mission control: 2 of 2 rockets alive`, then `0 alive` |

---

## Common mistakes

- **Defining a template in a `.cpp`.** The result is "undefined reference" errors at link time. Templates live entirely in headers.
- **Mixing argument types in `clamp`.** `clamp(x, 0, 1.0)` fails to deduce `T`. Write `0.0`.
- **`[=]` on `addRocket`.** The captured `fleet` is a `const` copy, so `emplace_back` does not compile. And with `mutable`, the rockets go into a copy that is thrown away.
- **Capturing `throttle` by reference in the event lambda.** It compiles, but the variable has gone by the time the event fires at 20 s.
- **Forgetting to reload CMake** after adding `atmosphere.cpp` and deleting `physics.cpp`. The result is link errors for `Atmosphere::density` or for the old `physics::airDensity`.
- **Defaulting `~Rocket()` in the header** while `Engine` is only forward-declared. The result is an incomplete-type error.
- **`operator[]` on the engine table.** An unknown name inserts an empty `std::function`, and calling it throws `std::bad_function_call` rather than the clear `invalid_argument`.
