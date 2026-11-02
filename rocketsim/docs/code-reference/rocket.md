# `rocket.h` — the rocket

[← Back to the code reference](README.md)

| | |
| --- | --- |
| **File** | `src/rocket.h` (header only in Week 7) |
| **Status** | Given, complete |
| **Includes** | `<string>`, `vec2.h` |
| **You change it in** | Week 8 (struct becomes a class, with a new `rocket.cpp`) |

---

## Purpose

Everything the simulator knows about one rocket. In Week 7 it is a plain `struct` with every field public, like the C `Fraction` struct from lecture 7; the physics functions in `physics.h` read and write it directly.

---

## `struct Rocket`

The fields fall into two groups: **design parameters**, which are fixed when the rocket is built, and **state**, which changes on every time step.

### Design parameters (fixed for the whole flight)

| Field | Type | Unit | Default | Meaning |
| --- | --- | --- | --- | --- |
| `name` | `std::string` | — | `"Starter-1"` | Used in plot titles, legends and the summary |
| `dryMass` | `double` | kg | `50.0` | Structure + payload. Never changes in Week 7. |
| `thrust` | `double` | N | `5000.0` | Engine force while fuel remains |
| `burnRate` | `double` | kg/s | `10.0` | How fast fuel is consumed while the engine runs |
| `dragCoeff` | `double` | — | `0.5` | Drag coefficient *C*<sub>d</sub> |
| `area` | `double` | m² | `0.05` | Frontal area *A* used for drag |
| `launchAngleDeg` | `double` | degrees | `85.0` | Angle from horizontal; 90 = straight up |

### State (changes every time step)

| Field | Type | Unit | Default | Meaning |
| --- | --- | --- | --- | --- |
| `fuelMass` | `double` | kg | `150.0` | Fuel remaining on board |
| `position` | `Vec2` | m | `(0, 0)` | `x` = downrange, `y` = altitude |
| `velocity` | `Vec2` | m/s | `(0, 0)` | |
| `time` | `double` | s | `0.0` | Mission elapsed time |

Every field has a **default member initialiser**, so `Rocket rocket;` in `main` gives you the default rocket with no further set-up.

---

## The default rocket

The defaults describe a small sounding rocket:

| Quantity | Value | Derived from |
| --- | --- | --- |
| Lift-off mass | 200 kg | 50 kg dry + 150 kg fuel |
| Burn time | 15 s | 150 kg ÷ 10 kg/s |
| Thrust-to-weight at lift-off | ≈ 2.5 | 5000 N ÷ (200 kg × 9.81 m/s²) |
| Exhaust velocity | 500 m/s | thrust ÷ burn rate |

> [!NOTE]
> `rocket.h` has no functions in Week 7. Everything that *does* something to a rocket lives in `physics.h` as a free function taking a `Rocket&` or `const Rocket&`.

---

## Changes in later weeks

| Week | Change |
| --- | --- |
| 8 | Becomes a `class` with private data, a constructor using a member initialiser list, `const` getters, an `update(totalForce, dt)` member function and a destructor. Member functions are defined in a new `rocket.cpp`. |
| 8 Medium | Holds a pointer to an `Engine` object instead of plain `thrust`/`burnRate` numbers. |
| 8 Hard | Gains a `static` live-object counter. |
| 9 Medium | Gains a `std::vector<Stage>` for multi-stage flight. |
| 10 | The engine pointer becomes `std::unique_ptr<Engine>`; copying a rocket is disabled with `= delete`. |
| 11 Medium | The constructor throws `RocketSpecError` for impossible specifications. |
