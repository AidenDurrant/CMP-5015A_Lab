# Week 8 — Classes in depth

### `Rocket` and `Vec2` become classes

| | |
| --- | --- |
| **Lecture material** | Constructors and member initialiser lists (A, 3–8) · `this` and `const` member functions (A, 9–12) · destructors and RAII (A, 13–16) · `static` members and the `.h`/`.cpp` split (A, 17–20) · copying and the rule of three (B, 1–6) · operator overloading incl. `operator<<` as a friend (B, 7–13) · inheritance, `virtual`, `override`, abstract classes, virtual destructors (B, 14–19) |
| **Starting point** | Your Week 7 code, or the Week 7 model solution copied over the starter. The Medium tier needs the Week 7 Hard `TelemetryLog` — take it from the model solution if you did not write it. |
| **Files you will edit** | `vec2.h`, `rocket.h`, `physics.cpp`, `simulation.cpp`, `main.cpp`, `telemetry.h/.cpp`; new `rocket.cpp`, engine and force files |
| **Reference pages** | [vec2](../code-reference/vec2.md) · [rocket](../code-reference/rocket.md) · [physics](../code-reference/physics.md) |

---

## By the end of this lab

- The flight is **exactly the same** as last week — but the code reads like the maths: `velocity += acceleration * dt;`
- `Rocket` is a proper **class** with a constructor, private state and `const` getters.
- *(Medium)* You have **seen a double-free crash** caused by a compiler-generated copy, and fixed it with the **rule of three**.
- *(Medium)* Engines are a **class hierarchy** with virtual functions, and two engines fly side by side on one plot.
- *(Hard)* Forces are **polymorphic** objects, and a headwind visibly shortens the flight.

---

## Background

### The problem

Last week's code works, but it has three weaknesses:

1. **Vector maths is hard to read.** An expression such as `add(position, scale(velocity, dt))` has to be read inside-out.
2. **Anyone can change anything.** Every field of `Rocket` is public. Nothing stops a function setting `fuelMass` negative or moving the rocket without advancing time.
3. **Ownership is invisible.** Your `TelemetryLog` holds a raw pointer to heap memory. Nothing in the language says who is responsible for deleting it, or what happens when it is copied.

Classes fix all three. **Operators** let user types behave like built-in ones. **Private data plus member functions** let a type guard its own invariants. **Constructors and destructors** tie a resource's lifetime to an object's lifetime (RAII), so cleanup happens automatically.

### Why copying is dangerous

When you don't write a copy constructor, the compiler writes one for you that copies every member. For a pointer member that means copying the **address**, not the data it points to:

```text
   original                     copy
 ┌───────────┐              ┌───────────┐
 │ data ─────┼──────┐   ┌───┼── data    │
 │ size = 3  │      ▼   ▼   │ size = 3  │
 └───────────┘    ┌─────────┐└───────────┘
                  │ 1  2  3 │   ← one array, two owners
                  └─────────┘
```

When both objects are destroyed, both destructors `delete[]` the same array. That is a **double free** and undefined behaviour. The **rule of three** says: if a class needs a custom destructor, it almost certainly needs a custom copy constructor and copy assignment operator too.

### Runtime polymorphism

Some rockets have a solid motor (fixed thrust), others a liquid engine (throttleable). The simulator shouldn't need an `if` for every engine type. Instead, an abstract base class declares *what* an engine can do, derived classes say *how*, and the right version is chosen at run time through a `virtual` call:

```cpp
// A generic illustration -- not the engine classes you will write.
class Shape {
public:
    virtual double area() const = 0;   // pure virtual: no Shape on its own
    virtual ~Shape() = default;        // essential if deleted through a Shape*
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_(r) {}
    double area() const override { return 3.14159 * r_ * r_; }
private:
    double r_;
};
```

---

## Easy — operators and a real `Rocket` class

### Task E1 · `Vec2` operators

In `vec2.h`, replace `add`, `subtract` and `scale` with:

| Operator | Meaning |
| --- | --- |
| `operator+` | `a + b` |
| `operator-` | `a - b` |
| `operator*(const Vec2&, double)` | `v * k` |
| `operator*(double, const Vec2&)` | `k * v` |
| `operator+=` | `a += b` |
| `operator<<` | `std::cout << v` prints `(x, y)` |

Keep `length`, `normalised` and `unitFromAngle` as free functions.

**Done when:** `step` contains the line `velocity += acceleration * dt;` (or its equivalent for your code) and the flight is unchanged.

> **SCREENSHOT PLACEHOLDER — `images/week08/step-before-after.png`**
> *Capture:* your `step` function from Week 7 (nested `add`/`scale` calls) next to the same function after E1, showing how the operators make it read like the maths.
<!-- ![step before and after operators](images/week08/step-before-after.png) -->

---

### Task E2 · `Rocket` becomes a class

1. Make the data members **private**.
2. Add a **constructor** taking name, dry mass, fuel, thrust, burn rate, Cd, area and launch angle, written with a **member initialiser list**.
3. Add **`const` getters**: `name()`, `position()`, `velocity()`, `fuel()`, `mass()`, `time()`, `launchAngle()`, `dragCoeff()`, `area()`.
4. Move the integration into a member function:
   ```cpp
   void Rocket::update(const Vec2& totalForce, double dt);
   ```
   It performs steps 2–6 of Euler (everything after summing the forces).
5. The force functions in `physics.cpp` stay as free functions, but now read the rocket through its getters. `physics::step` shrinks to **two lines**.

**Done when:** the program builds and the flight summary matches Week 7.

---

### Task E3 · Split into `.h` and `.cpp`

Declare the class in `rocket.h` and define every member function in a new `rocket.cpp`.

> [!IMPORTANT]
> Re-run `cmake -B build` so the build picks up `rocket.cpp`.

**Done when:** `rocket.h` contains only declarations (and any trivial inline getters you choose to keep), and the program links.

---

### Task E4 · Printing a rocket, and watching it die

1. Add `operator<<` for `Rocket` as a **`friend`** free function, printing:
   ```text
   Starter-1 t=12.0 alt=1234.5 speed=123.4 fuel=30.0
   ```
   Use it for the telemetry line.
2. Add a **destructor** that prints `Rocket <name> destroyed`. Run the program: the message must appear **exactly once**.
3. Now pass the rocket **by value** to any function and run again. It prints **twice**. Where did the second rocket come from?

**Done when:** you can name the function the compiler wrote for you that created the second rocket.

> **SCREENSHOT PLACEHOLDER — `images/week08/destroyed-twice.png`**
> *Capture:* the end of the console output with `Rocket Starter-1 destroyed` printed twice after passing the rocket by value.
<!-- ![Destructor runs twice](images/week08/destroyed-twice.png) -->

---

## Medium — the rule of three and virtual engines

### Task M1 · The rule of three, the hard way

1. Rewrite `TelemetryLog` as a **class**:
   - a constructor taking a capacity,
   - a destructor doing `delete[]`,
   - `append`, `size()`, and `const Sample& operator[](int) const`.
2. In `main`, write:
   ```cpp
   TelemetryLog copy = log;
   ```
   and run. The program **crashes on exit** with a double free.
3. Write the **copy constructor** and **copy assignment operator**: a deep copy, with a self-assignment check.
4. Run again. No crash.

> [!TIP]
> If your platform exits silently instead of crashing, build with `-fsanitize=address` (GCC/Clang) to make the error loud. See [Build, run and project files](../code-reference/build-and-project.md).

**Done when:** the copy line runs with no crash and no sanitizer error, and `copy` holds the same samples as `log`.

> **SCREENSHOT PLACEHOLDER — `images/week08/double-free.png`**
> *Capture:* the crash report (e.g. AddressSanitizer's `attempting double-free` message) produced by `TelemetryLog copy = log;` **before** the copy constructor is written.
<!-- ![Double-free crash](images/week08/double-free.png) -->

---

### Task M2 · An engine hierarchy

Write an **abstract** `Engine`:

```cpp
class Engine {
public:
    virtual double thrust(const Rocket&) const = 0;
    virtual double burnRate(const Rocket&) const = 0;
    virtual std::string name() const = 0;
    virtual ~Engine() = default;
};
```

Derive two engines, marking each overriding function `override`:

| Class | Behaviour |
| --- | --- |
| `SolidMotor` | Constant thrust while fuel remains — the Week 7 behaviour |
| `LiquidEngine` | Same, plus `setThrottle(double)`; thrust is scaled by the throttle |

`Rocket` holds a `const Engine*` passed to its constructor. In `main`, create the engine **on the stack** and pass `&motor`.

> [!NOTE]
> A raw pointer to something the rocket does not own is fine for now. Week 10 replaces it with a smart pointer that expresses ownership.

**Done when:** a rocket with a `SolidMotor` gives exactly the same flight as before.

---

### Task M3 · Two engines, one plot

1. In `simulate`, throttle a `LiquidEngine` **down to 60 %** whenever speed exceeds 300 m/s, and back to **100 %** below it.
2. Fly the same rocket design twice — once with each engine — on **one plot** (two `startSeries` calls, two `Rocket` objects).

**Done when:** the plot shows two trajectories and you can read the difference in apogee from the readout.

> **SCREENSHOT PLACEHOLDER — `images/week08/two-engines.png`**
> *Capture:* the live window replaying both flights side by side — one solid line, one dashed — with both readouts visible.
<!-- ![Solid vs liquid engine](images/week08/two-engines.png) -->

---

## Hard — polymorphic forces and class-wide state

### Task H1 · Forces as objects

Introduce an abstract `Force`:

```cpp
class Force {
public:
    virtual Vec2 apply(const Rocket&) const = 0;
    virtual std::string name() const = 0;
    virtual ~Force() = default;
};
```

Derive `Gravity`, `Thrust` and `Drag` by moving the Week 7 functions into their `apply` bodies. In `simulate`, keep a fixed array `Force* forces[3]`, created with `new` before the loop and `delete`d after it, and sum `apply` over the array.

**Done when:** the flight is unchanged and the loop no longer names any specific force.

---

### Task H2 · Wind

Give `Drag` a constructor `Drag(const Vec2& wind)` and compute drag on the velocity **relative to the air** (`velocity() - wind`).

**Done when:** a 15 m/s headwind visibly shortens the range, and a tailwind lengthens it.

> **SCREENSHOT PLACEHOLDER — `images/week08/headwind.png`**
> *Capture:* a plot comparing still air with a 15 m/s headwind (two series), showing the headwind run landing short.
<!-- ![Headwind comparison](images/week08/headwind.png) -->

---

### Task H3 · Counting live rockets

Add `static int Rocket::liveCount()`, backed by a **static data member** that is incremented in **every** constructor (including the copy constructor) and decremented in the destructor. Print it at the start and end of `main`.

**Done when:** the count returns to **zero** at the end of `main`.

---

### Task H4 · Comparison operators

Add `bool operator<(const FlightStats&, const FlightStats&)` comparing apogee, and `operator==` on `Vec2`. Next week needs `<` to sort flights.

**Done when:** both operators compile and give the expected answer on a small test in `main`.

---

## Checkpoint

Before moving on, run your program and check each of the following yourself:

| Tier | Check that… |
| --- | --- |
| Easy | **Same numbers as Week 7**, produced by `rocket.update(force, dt)` and printed with `cout << rocket`. `Rocket … destroyed` appears **exactly once**. |
| Medium | The copied `TelemetryLog` no longer crashes, and the two-engine plot shows **two lines** (solid and dashed) replaying side by side with their own readouts. |
| Hard | `liveCount()` ends at **0**, and the headwind run lands short. |

---

## Watch out for

- **Getter return types.** Returning a `Vec2` by value is fine (it's 16 bytes); `name()` should return `const std::string&`.
- **Virtual destructors are not optional** on `Engine` and `Force` once anything is deleted through a base-class pointer.
- **`operator<<` must take and return `std::ostream&`.** Returning by value won't compile — streams can't be copied, as you found last week.
- **The double-free is reported differently on each platform.** On MSVC it may be a silent non-zero exit code. Use `-fsanitize=address` on GCC/Clang.
- **New `rocket.cpp` not linking?** Re-run `cmake -B build`.

---

## Reflect

1. Which invariants of a rocket can `Rocket` now protect that the Week 7 struct could not?
2. Why must `operator*(double, const Vec2&)` be a free function rather than a member of `Vec2`?
3. In M2, why is `name()` pure virtual rather than a data member set by each derived class's constructor? Is either design wrong?
