# `physics.h` / `physics.cpp` — forces and the time step

[← Back to the code reference](README.md)

| | |
| --- | --- |
| **Files** | `src/physics.h` (declarations), `src/physics.cpp` (definitions) |
| **Status** | `physics.h` given · `physics.cpp` **stubs — TODO Week 7 Easy** (except `airDensity`) |
| **Includes** | `rocket.h`, `vec2.h`, `<cmath>` |
| **Namespace** | Everything is inside `namespace physics` |

---

## Purpose

This is the physics of the simulator: the three forces acting on the rocket and the function that moves the rocket forward by one small time step. In the starter, every function except `airDensity` is a **stub** that returns a dummy value, which is why the rocket never leaves the pad.

> [!IMPORTANT]
> The declarations in `physics.h` are the **interface**. Do not change them in Week 7 Easy — `simulation.cpp` and the later weeks depend on them. (Week 7 Medium asks you to add one constant.)

---

## The model

The rocket is a **point mass** in two dimensions. With *m* the current mass, **v** the velocity, *C*<sub>d</sub> the drag coefficient, *A* the frontal area and ρ the air density:

$$
\begin{aligned}
\mathbf{F}_g &= (0,\; -m g) \\
\mathbf{F}_t &= T\,\hat{\mathbf{d}} \quad \text{while fuel remains, otherwise } \mathbf{0} \\
\mathbf{F}_d &= -\tfrac{1}{2}\,\rho(y)\,C_d\,A\,|\mathbf{v}|\,\mathbf{v} \\
\rho(y) &= \rho_0\, e^{-y/H}
\end{aligned}
$$

where **d̂** is the unit thrust direction and *T* is the thrust. Mass is dry mass plus remaining fuel.

---

## Constants

Defined in `physics.h` as `constexpr` — the C++ replacement for C's `#define`. Unlike a macro, each has a type and obeys scope, so it is written `physics::GRAVITY` outside the namespace.

| Constant | Type | Value | Meaning |
| --- | --- | --- | --- |
| `GRAVITY` | `double` | `9.81` | *g*, m/s² |
| `SEA_LEVEL_DENSITY` | `double` | `1.225` | ρ<sub>0</sub>, air density at sea level, kg/m³ |
| `SCALE_HEIGHT` | `double` | `8500.0` | *H*, m — the atmosphere thins as e<sup>−y/H</sup> |

---

## Functions

### `double airDensity(double altitude)`

| | |
| --- | --- |
| **Status** | **Given** |
| **Parameters** | `altitude` — height above the ground, m |
| **Returns** | Air density ρ in kg/m³ |

Implements the exponential atmosphere ρ(y) = ρ<sub>0</sub> e<sup>−y/H</sup>. Negative altitudes are treated as zero, so the rocket never sees denser-than-sea-level air if it dips slightly below ground on the final step. Density roughly halves every 5.9 km.

> [!TIP]
> `airDensity` takes `altitude` **by value** and changes its local copy. Contrast this with the other functions, which take the rocket **by reference**. Ask yourself why a `double` is passed by value but a `Rocket` is not.

---

### `double totalMass(const Rocket& rocket)`

| | |
| --- | --- |
| **Status** | **TODO — Week 7 Easy** (stub returns `1.0`) |
| **Parameters** | `rocket` — read only |
| **Returns** | The rocket's current mass, kg |

**Contract:** returns the mass of the rocket as it is *now*: the structure plus whatever fuel remains. It falls during the burn and is constant afterwards.

---

### `bool engineRunning(const Rocket& rocket)`

| | |
| --- | --- |
| **Status** | **TODO — Week 7 Easy** (stub returns `false`) |
| **Parameters** | `rocket` — read only |
| **Returns** | `true` while the engine is producing thrust |

**Contract:** the engine runs for as long as there is fuel left to burn.

---

### `Vec2 gravityForce(const Rocket& rocket)`

| | |
| --- | --- |
| **Status** | **TODO — Week 7 Easy** (stub returns the zero vector) |
| **Parameters** | `rocket` — read only |
| **Returns** | The weight of the rocket as a force vector, N |

**Contract:** **F**<sub>g</sub> from the model above. Always points straight down; its size depends on the current mass.

---

### `Vec2 thrustForce(const Rocket& rocket)`

| | |
| --- | --- |
| **Status** | **TODO — Week 7 Easy**; changed in **Week 7 Medium** (stub returns the zero vector) |
| **Parameters** | `rocket` — read only |
| **Returns** | The engine force vector, N |

**Contract (Easy):** while the engine is running, a force of `rocket.thrust` newtons along the launch angle; once the fuel is gone, zero. `unitFromAngle` in `vec2.h` converts the launch angle into a direction.

**Contract (Medium — gravity turn):** as above, but once the rocket is above a pitch-over altitude the thrust direction follows the direction of travel instead of the fixed launch angle.

---

### `Vec2 dragForce(const Rocket& rocket)`

| | |
| --- | --- |
| **Status** | **TODO — Week 7 Easy** (stub returns the zero vector) |
| **Parameters** | `rocket` — read only |
| **Returns** | The aerodynamic drag force vector, N |

**Contract:** **F**<sub>d</sub> from the model above. Its size is ½ρ*C*<sub>d</sub>*A*|**v**|², it always points **against** the velocity, and it is zero when the rocket is stationary. Density is evaluated at the rocket's current altitude.

---

### `void step(Rocket& rocket, double dt)`

| | |
| --- | --- |
| **Status** | **TODO — Week 7 Easy** (stub does nothing) |
| **Parameters** | `rocket` — **modified**; `dt` — time step, s |
| **Returns** | nothing; the rocket passed in is moved forward by `dt` seconds |

**Contract:** advances the rocket by one step of **Euler's method**, in this order:

$$
\begin{aligned}
\mathbf{a} &= (\mathbf{F}_g + \mathbf{F}_t + \mathbf{F}_d) / m \\
\mathbf{v} &\leftarrow \mathbf{v} + \mathbf{a}\,\Delta t \\
\mathbf{p} &\leftarrow \mathbf{p} + \mathbf{v}\,\Delta t \\
m_{fuel} &\leftarrow \max(0,\; m_{fuel} - \dot{m}\,\Delta t) \quad \text{(only while the engine runs)} \\
t &\leftarrow t + \Delta t
\end{aligned}
$$

This is the only function in `physics` that takes a **non-`const`** `Rocket&`, because it is the only one that changes the rocket. It is called once per loop iteration by `simulate`.

> [!WARNING]
> - `step` **must** advance `rocket.time`. If it does not, the simulation loop runs until its step-count guard stops it, and the program reports that the rocket never left the pad.
> - Compute the forces **before** burning fuel, or the first step uses the wrong mass.

---

## Changes in later weeks

| Week | Change |
| --- | --- |
| 7 Medium | Add a `constexpr` pitch-over altitude; `thrustForce` performs a gravity turn. |
| 8 | The force functions read the rocket through getters; `step` hands the total force to `Rocket::update` and shrinks to a couple of lines. |
| 8 Hard | The three forces become classes derived from an abstract `Force` (`Gravity`, `Thrust`, `Drag`). |
| 9 Medium | Drag uses the air velocity from a wind profile. |
| 10 Medium | `airDensity` and the wind profile move into a shared `Atmosphere` class. |
