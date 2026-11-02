# `vec2.h` — 2-D vectors

[← Back to the code reference](README.md)

| | |
| --- | --- |
| **File** | `src/vec2.h` (header only — no `.cpp`) |
| **Status** | Given, complete |
| **Includes** | `<cmath>` |
| **You change it in** | Week 8 (free functions become operators) |

---

## Purpose

Positions, velocities and forces in the simulator are all two-dimensional vectors. `vec2.h` provides a small `Vec2` type and the arithmetic you need on it.

In Week 7 it is written C-style — a plain struct plus free functions, like the `Fraction` ADT from the C half of the module — but every function takes its arguments **by `const` reference**, which is how C++ passes read-only values without copying them.

All functions are defined `inline` in the header. That lets several `.cpp` files include `vec2.h` without "multiple definition" link errors.

---

## Types

### `struct Vec2`

```cpp
struct Vec2
{
    double x = 0.0;
    double y = 0.0;
};
```

| Member | Type | Meaning |
| --- | --- | --- |
| `x` | `double` | Horizontal component (downrange) |
| `y` | `double` | Vertical component (altitude) |

Both members have **default member initialisers**, so `Vec2 v;` and `Vec2{}` are the zero vector. You can also write `Vec2{3.0, 4.0}`.

---

## Functions

### `Vec2 add(const Vec2& a, const Vec2& b)`

| | |
| --- | --- |
| **Parameters** | `a`, `b` — the vectors to add |
| **Returns** | A new vector `(a.x + b.x, a.y + b.y)` |

Component-wise sum. Neither argument is changed.

---

### `Vec2 subtract(const Vec2& a, const Vec2& b)`

| | |
| --- | --- |
| **Parameters** | `a`, `b` |
| **Returns** | A new vector `a − b`, i.e. `(a.x − b.x, a.y − b.y)` |

Component-wise difference. Note the order: the result points **from `b` to `a`**.

---

### `Vec2 scale(const Vec2& v, double k)`

| | |
| --- | --- |
| **Parameters** | `v` — the vector; `k` — the scalar multiplier |
| **Returns** | A new vector `(k·v.x, k·v.y)` |

Multiplies a vector by a number. Use a negative `k` to reverse a direction, or `1.0 / m` to divide by a mass.

---

### `double length(const Vec2& v)`

| | |
| --- | --- |
| **Parameters** | `v` |
| **Returns** | The magnitude √(x² + y²) |

Use it for speed (`length` of a velocity) or for the size of a force.

---

### `Vec2 normalised(const Vec2& v)`

| | |
| --- | --- |
| **Parameters** | `v` |
| **Returns** | A **unit vector** (length 1) pointing the same way as `v`, or the zero vector if `v` is zero |

Use it when you need a *direction* without a *size*. The zero check matters: on the launch pad the velocity is zero, and dividing by a length of zero would produce `NaN` values that spread through the whole simulation.

---

### `Vec2 unitFromAngle(double degrees)`

| | |
| --- | --- |
| **Parameters** | `degrees` — angle measured anticlockwise from the +x axis |
| **Returns** | The unit vector `(cos θ, sin θ)` |

Converts an angle in degrees into a direction. `unitFromAngle(90)` is straight up `(0, 1)`; `unitFromAngle(0)` is horizontal `(1, 0)`. The conversion to radians is done for you.

---

## Usage notes

> [!TIP]
> Because these are plain functions, a vector expression nests inside-out. Reading `add(a, scale(b, k))` takes practice; in Week 8 you replace these functions with operators so the same expression is written `a + b * k`.

> [!WARNING]
> In Week 7, do not write `->` anywhere. Every function here takes references, so you use `.` throughout.

---

## Changes in later weeks

| Week | Change |
| --- | --- |
| 8 | `add`, `subtract`, `scale` are replaced by `operator+`, `operator-`, `operator*`, `operator+=`; `operator<<` is added for printing. `length`, `normalised` and `unitFromAngle` stay as free functions. |
| 8 Hard | `operator==` is added. |
| 10 | `Vec2` is used as a template argument (`lerp`), which works because of the Week 8 operators. |
| 11 Hard | The operators are marked `noexcept`. |
