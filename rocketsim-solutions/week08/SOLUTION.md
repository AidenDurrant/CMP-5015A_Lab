# Week 8 solution — Classes in depth

### `Rocket` and `Vec2` become classes

This walkthrough follows the Week 8 worksheet task by task (E1–E4, M1–M3, H1–H4).

| | |
| --- | --- |
| **Files in this folder** | `src/vec2.h`, `src/rocket_spec.h` (new), `src/rocket.h/.cpp` (new `.cpp`), `src/engine.h/.cpp` (new), `src/force.h/.cpp` (new), `src/physics.h/.cpp`, `src/telemetry.h/.cpp`, `src/config.h/.cpp`, `src/simulation.h/.cpp`, `src/main.cpp` |
| **Run it** | `rocketsim --launch launch.txt --compare` → telemetry for both rockets, two flight summaries, "The ... flew higher", the deep-copy count, `Rocket … destroyed` once per rocket, `Live rockets at end: 0`, `flight.svg`, `flight.csv` and the live window |

> [!NOTE]
> `src/` is the finished state after all three tiers, so some Easy-tier steps are no longer visible in it, and a few names and signatures differ from the worksheet:
> - **E2 constructor.** `Rocket` is built from a `RocketSpec` plus an `Engine*` (`Rocket(const RocketSpec&, Engine*)`), not from eight separate values. Thrust and burn rate live in the engine (M2), not in the rocket.
> - **E2 `physics::step`.** It no longer exists. After H1, `physics.cpp` holds only `airDensity`, and the "two-line step" is the force sum plus `rocket.update(...)` inside `simulate`. The free force functions have moved into the `Force` classes.
> - **E4 telemetry format.** `operator<<` for `Rocket` prints the Week 7 column layout with units and range (`Starter-1  t=   12.0 s  alt=   1234.5 m  range= ... m  speed= ... m/s  fuel= ... kg`), not the shorter line in the worksheet.
> - **M2 engine pointer.** `Rocket` holds `Engine*`, not `const Engine*`. The base `Engine` also has an extra virtual `setThrottle` with an empty default body, so that `simulate` can throttle any engine.
> - **M3 is behind a flag.** The second (liquid-engine) flight only happens with `--compare`. A plain run flies the solid motor only.
> - **H2 wind.** Wind is set in the launch file (`wind = -15` for a headwind, `wind = 15` for a tailwind). There is no command-line option, and no single run that plots still air against headwind: run twice and compare.
> - **H4 test.** `operator<` is used in `main` to say which engine flew higher. `main` has no test of `Vec2::operator==`. A small test is shown below.
> - **Extras not asked for:** `RocketSpec`, `TelemetryLog::copyCount()`, `operator<<` for `FlightStats`, `Vec2` `-=`, `*=`, `/`, unary `-` and `!=`, the getters `dryMass()`, `engineRunning()` and `engine()`, and the options `--quiet`, `--no-window`, `--out` and `--compare`.

---

## What the week is about

The flight is **identical to Week 7** (with the Week 7 Medium gravity turn). Only the code is restructured:
- each thing that owns state is a **class** with a constructor, private data and `const` getters;
- each thing that varies by kind (engine, force) is a small **hierarchy** with virtual functions;
- the Week 7 hand-built log becomes a class that **owns memory**, and that is where the rule of three is needed and demonstrated.

---

## Easy — operators and a real `Rocket` class

### E1 · `Vec2` operators

`Vec2` keeps its two public doubles. It is a value with nothing to protect, so a class with private data would add nothing. The compound operators are members. The binary operators are free functions written in terms of them, so `a + b` and `a += b` can never disagree:

```cpp
struct Vec2
{
    double x = 0.0;
    double y = 0.0;

    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    // operator-= and operator*= (double) follow the same pattern
};

inline Vec2 operator+(Vec2 a, const Vec2& b) { return a += b; }   // a is already a copy
inline Vec2 operator-(Vec2 a, const Vec2& b) { return a -= b; }
inline Vec2 operator*(Vec2 v, double k) { return v *= k; }        // v * k
inline Vec2 operator*(double k, Vec2 v) { return v *= k; }        // k * v

inline std::ostream& operator<<(std::ostream& out, const Vec2& v)
{
    return out << '(' << v.x << ", " << v.y << ')';
}
```

The left operand is taken **by value** rather than as `const Vec2&`, as the worksheet table writes it. The function needs a copy to modify and return anyway, so it takes the copy in the parameter. Both forms are correct. `length`, `normalised` and `unitFromAngle` stay as free functions, and `normalised` now uses `/`.

The "done when" line is in `Rocket::update` (see E2):

```cpp
velocity_ += acceleration * dt;
position_ += velocity_ * dt;
```

Compare with Week 7's `rocket.velocity = add(rocket.velocity, scale(acceleration, dt));`.

### E2 · `Rocket` becomes a class

The data members are private (with a trailing `_`). The getters are `const` and defined inline in the header. `name()` returns `const std::string&` so that reading the name does not copy it:

```cpp
class Rocket
{
public:
    Rocket(const RocketSpec& spec, Engine* engine);
    // ...
    const std::string& name() const { return name_; }
    double fuel() const { return fuelMass_; }
    double mass() const { return dryMass_ + fuelMass_; }
    const Vec2& position() const { return position_; }
    const Vec2& velocity() const { return velocity_; }
    double time() const { return time_; }
    double launchAngle() const { return launchAngleDeg_; }
    double dragCoeff() const { return dragCoeff_; }
    double area() const { return area_; }

    void update(const Vec2& totalForce, double dt);

private:
    std::string name_;
    double dryMass_;
    double fuelMass_;
    // ... dragCoeff_, area_, launchAngleDeg_, position_, velocity_
    double time_ = 0.0;
    Engine* engine_;
};
```

The constructor uses a **member initialiser list**:

```cpp
Rocket::Rocket(const RocketSpec& spec, Engine* engine)
    : name_(spec.name),
      dryMass_(spec.dryMass),
      fuelMass_(spec.fuelMass),
      dragCoeff_(spec.dragCoeff),
      area_(spec.area),
      launchAngleDeg_(spec.launchAngleDeg),
      engine_(engine)
{
    ++liveCount_;   // H3
}
```

> [!NOTE]
> **`src/` differs from the worksheet here.** The worksheet's constructor takes eight values (name, dry mass, fuel, thrust, burn rate, Cd, area, angle). `src/` passes a `RocketSpec`, which is the plain struct the launch-file parser fills, and the engine. Thrust and burn rate are the engine's job after M2. Keeping the numbers read from disk separate from the object in flight keeps the parser out of the class, and this pays off in Week 11. An Easy-only solution would have `thrust_` and `burnRate_` members and the eight-parameter constructor.

`update` performs steps 2–6 of Euler. It is the **only** mutator:

```cpp
void Rocket::update(const Vec2& totalForce, double dt)
{
    const Vec2 acceleration = totalForce / mass();
    velocity_ += acceleration * dt;
    position_ += velocity_ * dt;

    if (engineRunning())
    {
        fuelMass_ -= engine_->burnRate(*this) * dt;
        if (fuelMass_ < 0.0)
        {
            fuelMass_ = 0.0;
        }
    }
    time_ += dt;
}
```

At the Easy stage, `physics::step` shrinks to two lines. The Week 7 force functions are kept, but they now read the rocket through its getters:

```cpp
// Easy-tier version (not in src/: H1 replaced it)
void step(Rocket& rocket, double dt)
{
    const Vec2 force = gravityForce(rocket) + thrustForce(rocket) + dragForce(rocket);
    rocket.update(force, dt);
}
```

In `src/` the same two steps appear inside `simulate`, as a sum over the `Force` objects followed by `rocket.update(total, settings.dt);` (see H1).

### E3 · Split into `.h` and `.cpp`

`rocket.h` declares the class. `rocket.cpp` defines the constructors, the destructor, `update`, `liveCount` and `operator<<`, and initialises the static member (`int Rocket::liveCount_ = 0;`). Only the one-line getters stay inline in the header.

`rocket.h` uses a **forward declaration** `class Engine;` rather than `#include "engine.h"`. The header only stores an `Engine*` and returns an `Engine&`, and neither needs the full class. `rocket.cpp` includes `engine.h` because it calls `engine_->burnRate(...)`. Without re-running `cmake -B build`, the new `.cpp` files are not compiled and the link fails with "undefined reference to `Rocket::Rocket(...)`".

### E4 · Printing a rocket, and watching it die

`operator<<` is a `friend`, so it can read the private members directly. It takes and returns `std::ostream&`:

```cpp
// rocket.h, inside the class
friend std::ostream& operator<<(std::ostream& out, const Rocket& rocket);

// rocket.cpp
std::ostream& operator<<(std::ostream& out, const Rocket& rocket)
{
    return out << std::fixed << std::setprecision(1) << rocket.name_ << "  t=" << std::setw(7)
               << rocket.time_ << " s  alt=" << std::setw(9) << rocket.position_.y
               << " m  range=" << std::setw(9) << rocket.position_.x << " m  speed=" << std::setw(7)
               << length(rocket.velocity_) << " m/s  fuel=" << std::setw(6) << rocket.fuelMass_
               << " kg";
}
```

The telemetry line in `simulate` is then just `std::cout << rocket << '\n';`. (The format keeps the Week 7 columns. This differs from the shorter example line on the worksheet.)

The destructor prints a message so you can watch objects die. Week 9 removes it again.

```cpp
Rocket::~Rocket()
{
    --liveCount_;
    std::cout << "Rocket " << name_ << " destroyed\n";
}
```

In a plain run, `Rocket Starter-1 destroyed` appears **once**, at the closing brace of the inner block in `main`. With `--compare`, `Starter-1-liquid` is destroyed as well (once each).

**Passing by value prints it twice.** Change any function to take `Rocket` instead of `Rocket&` and you get a second message. The second rocket is the parameter. It was made by the **copy constructor** `Rocket(const Rocket&)`, which the compiler wrote implicitly and which copies every member. It is destroyed when the function returns.

> [!NOTE]
> In `src/` the copy constructor is written by hand because of H3, which needs every constructor to count. At the E4 stage it is the compiler-generated one.

---

## Medium — the rule of three and virtual engines

### M1 · The rule of three, the hard way

`TelemetryLog` is now a class that owns its `Sample*`:

```cpp
class TelemetryLog
{
public:
    explicit TelemetryLog(int capacity = 1024);
    TelemetryLog(const TelemetryLog& other);
    TelemetryLog& operator=(const TelemetryLog& other);
    ~TelemetryLog();

    void append(const Sample& sample);
    int size() const { return size_; }
    const Sample& operator[](int i) const { return samples_[i]; }

    static int copyCount();   // lab demo: how many deep copies were made
private:
    Sample* samples_;
    int size_;
    int capacity_;
    static int copyCount_;
};
```

The destructor is `delete[] samples_;`. `append` is the Week 7 doubling logic, now acting on members.

**The crash.** Without the two copy functions, `TelemetryLog copy = log;` runs the compiler's member-wise copy. That copies the pointer, so two objects own one array, and both destructors `delete[]` it: a double free. Under `-fsanitize=address` this is reported as `attempting double-free`.

> [!NOTE]
> In `src/main.cpp` the copy sits inside the inner block, so it is destroyed **before** `writeCsv(log, ...)` runs. If you delete the copy functions from `src/`, AddressSanitizer will therefore report a **heap-use-after-free** in `writeCsv` first, because the sanitizer stops at the first error. Put the copy line at the end of `main` (as the worksheet does) to see the double free itself.

**The fix** is a deep copy. Assignment includes a self-assignment check, and it allocates the new array before freeing the old one:

```cpp
TelemetryLog::TelemetryLog(const TelemetryLog& other)
    : samples_(new Sample[other.capacity_]), size_(other.size_), capacity_(other.capacity_)
{
    for (int i = 0; i < size_; ++i)
    {
        samples_[i] = other.samples_[i];
    }
    ++copyCount_;
}

TelemetryLog& TelemetryLog::operator=(const TelemetryLog& other)
{
    if (this != &other)   // self-assignment would delete the array we then copy from
    {
        Sample* fresh = new Sample[other.capacity_];
        for (int i = 0; i < other.size_; ++i)
        {
            fresh[i] = other.samples_[i];
        }
        delete[] samples_;
        samples_ = fresh;
        size_ = other.size_;
        capacity_ = other.capacity_;
        ++copyCount_;
    }
    return *this;
}
```

`main` makes the copy on purpose and proves that the deep copy ran:

```cpp
TelemetryLog copy = log;
std::cout << "Deep copies made so far: " << TelemetryLog::copyCount()
          << " (copy has " << copy.size() << " samples)\n";
```

`writeCsv` becomes a free function that reads the log through `size()` and `operator[]`.

### M2 · An engine hierarchy

`Engine` is abstract. Its virtual destructor is what makes `delete enginePtr` through a base pointer safe:

```cpp
class Engine
{
public:
    virtual ~Engine() = default;
    virtual double thrust(const Rocket& rocket) const = 0;
    virtual double burnRate(const Rocket& rocket) const = 0;
    virtual std::string name() const = 0;

    // Not on the worksheet: most engines cannot be throttled, so ignore it.
    virtual void setThrottle(double /*fraction*/) {}
};
```

`SolidMotor` reproduces the Week 7 behaviour. `LiquidEngine` scales both thrust and burn rate by a throttle setting, which is clamped to [0, 1]:

```cpp
double SolidMotor::thrust(const Rocket& rocket) const
{
    return rocket.engineRunning() ? thrust_ : 0.0;
}

double LiquidEngine::thrust(const Rocket& rocket) const
{
    return rocket.engineRunning() ? thrust_ * throttle_ : 0.0;
}
```

Every overriding function is marked `override`. `main` creates the engine on the stack and passes its address:

```cpp
SolidMotor motor(spec.thrust, spec.burnRate);
Rocket rocket(spec, &motor);
```

`Rocket::update` asks `engine_->burnRate(*this)`, and `Thrust::apply` (H1) asks `rocket.engine().thrust(rocket)`. A `SolidMotor` therefore gives exactly the Week 7 flight.

> [!NOTE]
> The worksheet says `const Engine*`. `src/` stores a plain `Engine*`, and `Rocket::engine()` returns `Engine&`, because `simulate` calls the non-`const` `setThrottle` through the rocket (M3). With `const Engine*`, the throttle would have to be set on the `LiquidEngine` object directly, outside the rocket.

### M3 · Two engines, one plot

The throttle rule is one line in `simulate`, after each step:

```cpp
rocket.engine().setThrottle(speed > 300.0 ? 0.6 : 1.0);
```

Because `setThrottle` is virtual with a do-nothing default in `Engine`, this line is safe for **any** rocket. A `SolidMotor` ignores it, and a `LiquidEngine` throttles. Dynamic dispatch does the `if (engine is liquid)` for you.

With `--compare`, `main` flies the same design a second time with a `LiquidEngine`, onto the same `Plot`. Each `simulate` call starts its own series with `plot.startSeries(rocket.name())`:

```cpp
LiquidEngine engine(spec.thrust, spec.burnRate);
RocketSpec liquidSpec = spec;
liquidSpec.name += "-liquid";
Rocket rocket2(liquidSpec, &engine);
TelemetryLog log2;
const FlightStats stats2 = simulate(rocket2, settings, plot, log2);
std::cout << (stats < stats2 ? "The liquid engine flew higher.\n"
                             : "The solid motor flew higher.\n");
```

Throttling scales thrust and burn rate together, so the total impulse from the fuel is unchanged. What changes is *when* it is delivered: more slowly, at higher speed, where drag would take more of it. Read the two apogees from the two summaries or from the live-window readouts.

---

## Hard — polymorphic forces and class-wide state

### H1 · Forces as objects

`Force` has the same shape as `Engine`. `Gravity`, `Thrust` and `Drag` are the Week 7 functions moved into `apply` bodies:

```cpp
Vec2 Gravity::apply(const Rocket& rocket) const
{
    return Vec2{0.0, -rocket.mass() * physics::GRAVITY};
}

Vec2 Thrust::apply(const Rocket& rocket) const
{
    const double magnitude = rocket.engine().thrust(rocket);
    if (magnitude == 0.0)
    {
        return Vec2{};
    }
    Vec2 direction = unitFromAngle(rocket.launchAngle());
    if (rocket.position().y > physics::PITCH_OVER_ALTITUDE && length(rocket.velocity()) > 1.0)
    {
        direction = normalised(rocket.velocity());   // gravity turn
    }
    return direction * magnitude;
}
```

`simulate` holds the forces as raw pointers in a fixed array. It creates them with `new` before the loop and deletes them after it. Week 10 replaces this with `vector<unique_ptr<Force>>`.

```cpp
Force* forces[3] = {new Gravity(), new Thrust(), new Drag(settings.wind)};
// ...
Vec2 total;
for (const Force* force : forces)
{
    total += force->apply(rocket);
}
rocket.update(total, settings.dt);
// ... after the loop:
for (Force* force : forces)
{
    delete force;
}
```

The loop never names a specific force. `delete force` through a `Force*` is only correct because `~Force()` is virtual.

### H2 · Wind

`Drag` takes the wind in an `explicit` constructor. The default argument means `Drag()` still means still air. Drag acts on the velocity **relative to the air**:

```cpp
explicit Drag(const Vec2& wind = Vec2{});

Vec2 Drag::apply(const Rocket& rocket) const
{
    const Vec2 relative = rocket.velocity() - wind_;
    const double speed = length(relative);
    if (speed == 0.0)
    {
        return Vec2{};
    }
    const double rho = physics::airDensity(rocket.position().y);
    const double magnitude = 0.5 * rho * rocket.dragCoeff() * rocket.area() * speed * speed;
    return -normalised(relative) * magnitude;
}
```

The wind comes from the launch file as `wind = <m/s>` and is stored as `Vec2{number, 0.0}`. The rocket flies towards +x, so `wind = -15` is a **headwind** and shortens the range, while `wind = 15` is a tailwind and lengthens it. To get the comparison plot, run once without a `wind` line and once with `wind = -15`. (`src/` has no option that puts both on one plot.)

### H3 · Counting live rockets

A static data member counts the rockets, behind a static member function:

```cpp
// rocket.h
static int liveCount();
private:
static int liveCount_;

// rocket.cpp
int Rocket::liveCount_ = 0;
int Rocket::liveCount() { return liveCount_; }
```

Both constructors do `++liveCount_`, and the destructor does `--liveCount_`. The copy constructor **has** to be written by hand for this, because the compiler's version would copy without counting, and the count would go negative. Copy assignment stays `= default`, because assigning does not create an object.

`main` prints `Live rockets at start: 0`. The rockets live in an inner block that closes before the final line, so `Live rockets at end: 0` appears after the `destroyed` messages.

### H4 · Comparison operators

```cpp
// simulation.h / simulation.cpp
bool operator<(const FlightStats& a, const FlightStats& b)
{
    return a.apogee < b.apogee;
}

// vec2.h
inline bool operator==(const Vec2& a, const Vec2& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const Vec2& a, const Vec2& b) { return !(a == b); }
```

`main` uses `stats < stats2` in the `--compare` branch to report which engine flew higher. `src/` has no explicit test of `operator==`. A minimal test for `main` would be:

```cpp
FlightStats low, high;
low.apogee = 100.0;
high.apogee = 200.0;
std::cout << std::boolalpha << (low < high) << ' ' << (high < low) << '\n';        // true false
std::cout << (Vec2{1, 2} == Vec2{1, 2}) << ' ' << (Vec2{1, 2} == Vec2{2, 1}) << '\n';  // true false
```

Exact `==` on doubles is fine for values that were copied, but not for results of arithmetic. That limitation is acceptable here.

`src/` also adds `operator<<` for `FlightStats`, which prints the summary block (`std::cout << stats`).

---

## Checkpoint

| Tier | Expected |
| --- | --- |
| Easy | Same numbers as Week 7 (with its gravity turn): apogee ≈ **6,475 m at 40.0 s**, range ≈ **2,890 m**. The telemetry is printed with `cout << rocket`, and `Rocket Starter-1 destroyed` appears **exactly once** in a plain run |
| Medium | `Deep copies made so far: 1` with no crash and no sanitizer error. With `--compare`, two trajectories on one plot and two summaries, plus a line saying which engine flew higher |
| Hard | `Live rockets at end: 0`. A `wind = -15` run lands short of the still-air range, and `wind = 15` lands long |

---

## Common mistakes

- **A getter returning `std::string` by value.** It copies the name every time telemetry is printed. Use `const std::string&`.
- **Missing `virtual ~Engine()` / `virtual ~Force()`.** The code still compiles, but `delete` through a base pointer is undefined behaviour.
- **`operator<<` returning `std::ostream` by value.** It does not compile, because streams cannot be copied, as you saw in Week 7.
- **Copy assignment without the self-assignment check**, or freeing the old array before copying from `other`.
- **Counting in only one constructor.** The copy constructor must count too, or `liveCount()` ends negative.
- **Forgetting `override`.** A typo such as `burnrate` then quietly declares a new function instead of overriding.
- **Not re-running `cmake -B build`** after adding `rocket.cpp`, `engine.cpp` and `force.cpp`, which gives "undefined reference" link errors.

---

## Reflect — suggested answers

1. **Invariants.** Fuel can never go negative (`update` clamps it). Position, velocity and time can only change together, in one Euler step, so nothing can move the rocket without advancing the clock. Dry mass, Cd, area and angle cannot change after construction. And `liveCount` is always accurate, because only the class's own constructors and destructor touch it.
2. **`operator*(double, const Vec2&)` must be a free function** because a member operator always has the class object as its *left* operand. In `k * v` the left operand is a `double`, which has no members, so the only way to define it is as a free function.
3. **`name()` as pure virtual vs a data member.** Pure virtual forces every concrete engine to provide a name (forgetting is a compile error), and stores nothing per object. A `name_` member set by each constructor works just as well and allows names chosen at run time. Neither design is wrong. The virtual version shows the kind of the class, while a data member describes an individual object.
