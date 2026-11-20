# Week 9 solution — Containers

### `vector`, `map`, `<algorithm>` and lambdas

This walkthrough follows the Week 9 worksheet task by task (E1–E4, M1–M3, H1–H4).

| | |
| --- | --- |
| **Files in this folder** | `src/rocket_spec.h`, `src/rocket.h/.cpp`, `src/engine.h/.cpp`, `src/wind.h` (new), `src/force.h/.cpp`, `src/telemetry.h/.cpp`, `src/events.h` (new), `src/config.cpp`, `src/simulation.h/.cpp`, `src/main.cpp`; launch files `launch_twostage.txt` and `launch_b.txt` |
| **Run it** | `rocketsim --launch launch.txt --launch launch_twostage.txt --launch launch_b.txt --events` → three rockets on one plot, separation, event and parachute markers, a ranking by apogee |

> [!NOTE]
> Where `src/` differs from the worksheet:
> - **E1:** the CSV is written as `<out>.csv` (default `flight.csv`), not `telemetry.csv`. With a fleet, only the log of the **last rocket in name order** is written. `TelemetryLog` survives as a type alias for `std::vector<Sample>`.
> - **E4:** the reverse-iterator loop prints only the **altitudes** of the last ten samples, on one line. The naive `size_t` loop is kept only as a one-line comment, and that comment describes the failure slightly wrong (see E4).
> - **M2:** the wind profile is always on. `SimSettings` has no switch to turn it off. To compare, give `SimSettings::wind` an empty map.
> - **H1:** events are not read from a file. The `--events` command-line flag adds the two sample events to every rocket. Events are consumed by advancing an index into a sorted copy, not by erasing them. "Fairing off" reduces the **payload** mass by 5 kg, not the dry mass. With the supplied rockets it changes little or nothing (see H1).
> - **H2:** the parachute is not an `Event`. `simulate` checks the state directly, and only when `settings.parachute` is set. A launch file sets it with `parachute = yes`, and of the supplied files only `launch_twostage.txt` does.
> - **H3:** keys are validated with a `std::set`, but dispatch is still an `if` / `else if` chain. The `std::map<std::string, double*>` version is shown below.
> - **H4:** `partial_sort` runs over a vector of **speeds**, not samples, so the five fastest print as bare numbers.

---

## What the week is about

Everything written by hand in Weeks 7 and 8 to hold a growing list is deleted. The log becomes a `std::vector<Sample>`, the stages a `std::vector<Stage>`, the wind profile a `std::map`, the known keys a `std::set` and the fleet a `std::map`. Every loop that searched or summed something becomes an `<algorithm>` call with a lambda.

The theme is the opposite of last week's. Week 8 showed what it costs to manage memory yourself. Week 9 shows that the standard containers already pay that cost, and correctly.

---

## Easy — replace the hand-written log

### E1 · Delete `TelemetryLog`

`telemetry.h` shrinks to the `Sample` struct, a type alias and two functions:

```cpp
struct Sample { double time, x, y, speed, fuel; };

using TelemetryLog = std::vector<Sample>;

bool writeCsv(const TelemetryLog& log, const std::string& path);
void printLogSummary(const TelemetryLog& log, double dt);
```

The alias keeps the old name so the rest of the code reads the same, but `TelemetryLog&` *is* `std::vector<Sample>&`. `simulate` takes it by reference and appends with `push_back`:

```cpp
log.push_back(Sample{rocket.time(), rocket.position().x, rocket.position().y, speed, rocket.fuel()});
```

`writeCsv` uses a range-based `for`:

```cpp
out << "time,x,y,speed,fuel\n";
for (const Sample& s : log)
{
    out << s.time << ',' << s.x << ',' << s.y << ',' << s.speed << ',' << s.fuel << '\n';
}
```

About 60 lines of the Week 8 class are gone: the constructor, the destructor, the copy constructor, the copy assignment with its self-check, and `append` with capacity doubling. Nothing was lost, because `std::vector` does all of it and does it correctly. There is no `new` or `delete` left in `telemetry.h/.cpp`.

> [!NOTE]
> `main` writes the CSV as `outName + ".csv"` (default `flight.csv`), keeping the Week 7 `--out` behaviour. It is not named `telemetry.csv`. When several rockets fly, `main` keeps a copy of the most recent log (`lastLog = log;`, a correct deep copy now that the log is a vector), so the CSV holds the last rocket in name order.

### E2 · Statistics with `max_element`

The apogee and max-speed bookkeeping is gone from the loop. After the loop, two `max_element` calls do the same job:

```cpp
const auto highest = std::max_element(log.begin(), log.end(),
                                      [](const Sample& a, const Sample& b) { return a.y < b.y; });
stats.apogee = highest->y;         // the iterator points at the sample itself
stats.apogeeTime = highest->time;  // so the time comes for free
plot.addMarker(highest->x, highest->y, "apogee");
stats.maxSpeed = std::max_element(log.begin(), log.end(),
                                  [](const Sample& a, const Sample& b) { return a.speed < b.speed; })->speed;
```

The lambda is the "less than" that `max_element` uses. It returns an **iterator**, so the fields are read with `->`. One iterator gives the apogee, its time and its position for the marker. Week 7 needed a separate `apogeeX` variable for that. The call is guarded by `if (!log.empty())`, because dereferencing `end()` of an empty log is undefined. `stats.landingSpeed = log.back().speed` is also taken from the log.

Burnout is the one statistic still recorded in the loop, because it is also where its plot marker goes.

### E3 · `count_if` and `accumulate`

Both are in `printLogSummary`, which `main` calls after each flight summary:

```cpp
const auto above = std::count_if(log.begin(), log.end(), [](const Sample& s) { return s.y > 5000.0; });
const double meanSpeed =
    std::accumulate(log.begin(), log.end(), 0.0, [](double sum, const Sample& s) { return sum + s.speed; }) /
    static_cast<double>(log.size());
std::cout << "  time above 5 km: " << above * dt << " s, mean speed " << meanSpeed << " m/s\n";
```

There is one sample per step, so the number of samples above 5 km times `dt` is the time spent there. The initial value `0.0` matters: with `0` instead, `accumulate` would add in `int` and truncate every partial sum.

### E4 · The `size()` trap

The naive loop:

```cpp
// for (std::size_t i = log.size() - 1; i >= 0; --i)   // never terminates
// for (std::size_t i = log.size() - 1; i >= log.size() - 10; --i)
```

`std::size_t` is unsigned, so `i >= 0` is **always true**. When `i` reaches 0, `--i` wraps around to about 1.8 × 10¹⁹ and the loop reads far past the end of the vector. The second form, with a bound, still fails:
- if `size()` is exactly 10, the bound is 0 and you are back to `i >= 0`;
- if `size()` is below 10, `size() - 10` wraps to a huge number, so the loop silently prints nothing;
- if the log is empty, `size() - 1` wraps as well.

Fixing it means casts to a signed type, or a carefully written `i-- > 0` condition.

Reverse iterators avoid the arithmetic altogether. This is the version in `src/`:

```cpp
int shown = 0;
for (auto it = log.rbegin(); it != log.rend() && shown < 10; ++it, ++shown)
{
    std::cout << ' ' << it->y;
}
```

`rbegin()` points at the last sample and `++` moves backwards. The loop stops at `rend()`, so a short log is handled without any special case.

> [!NOTE]
> In `src/`, the naive loop is only the one-line comment in `telemetry.cpp`. That comment says it "never terminates when `size() < 10`". As explained above, it actually loops forever when `size() == 10` and prints nothing when `size() < 10`. The `src/` loop also prints only the altitudes. To print whole samples in descending time order, as the worksheet asks, print `it->time`, `it->y` and `it->speed` on each line.

---

## Medium — stages, a wind profile and a fleet

### M1 · Multi-stage rockets

`Stage` lives in `rocket_spec.h`, with defaults that make one stage equal to the Week 7 rocket:

```cpp
struct Stage
{
    double dryMass = 50.0;
    double fuelMass = 150.0;
    double thrust = 5000.0;
    double burnRate = 10.0;
};

struct RocketSpec
{
    std::string name = "Starter-1";
    double payloadMass = 0.0;
    std::vector<Stage> stages{Stage{}};   // one default stage = the week 7 rocket
    // drag_coeff, area, launch_angle as before
};
```

`Rocket` copies `spec.stages` into a `std::vector<Stage> stages_` member. The bottom stage is `stages_.front()`, and `mass()` is one `accumulate` with the payload as its starting value:

```cpp
double Rocket::mass() const
{
    return std::accumulate(stages_.begin(), stages_.end(), payloadMass_,
                           [](double sum, const Stage& s) { return sum + s.dryMass + s.fuelMass; });
}
```

In `Rocket::update`, the active stage burns, and when it is empty it is dropped. Its dry mass goes with it:

```cpp
if (active.fuelMass <= 0.0)
{
    active.fuelMass = 0.0;
    if (stages_.size() > 1)
    {
        stages_.erase(stages_.begin());   // drop the empty stage
        separated_ = true;
    }
}
```

The last stage is never erased, so `front()` always stays valid. `separatedThisStep()` is true for exactly one step, and `simulate` places the marker then:

```cpp
if (rocket.separatedThisStep())
{
    plot.addMarker(rocket.position().x, rocket.position().y, "stage " + std::to_string(stageNumber++) + " separation");
}
```

The `burnout` marker now waits until `stagesRemaining() == 1`, so the first stage running dry does not count as burnout.

`Engine` no longer stores thrust or burn rate. `LiquidEngine::thrust` returns `rocket.activeStage().thrust * throttle_`, so one engine object serves every stage.

**Parser.** `stage = <dry> <fuel> <thrust> <burn>` lines are split with an `std::istringstream` and collected in a local vector. If there were any, they replace the default list at the end. The old single-stage keys (`dry_mass`, `fuel_mass`, …) still work by writing into `spec.stages.front()`, so `launch.txt` is unchanged:

```cpp
else if (key == "stage")
{
    std::istringstream fields(value);
    Stage stage;
    fields >> stage.dryMass >> stage.fuelMass >> stage.thrust >> stage.burnRate;
    stages.push_back(stage);
}
```

`launch_twostage.txt` splits Starter-1's 150 kg of fuel into a 100 kg and a 50 kg stage. The total dry mass is the same 50 kg: 25 + 15 in the stages plus a 10 kg payload. Dropping the 25 kg lower stage halfway through the burn is what gives the higher apogee.

### M2 · A wind profile

`wind.h`:

```cpp
using WindProfile = std::map<double, double>;   // altitude (m) -> wind (m/s, +x)

// operator[] must not be used here: it would insert a zero entry for every
// altitude asked about, silently corrupting the profile.
inline double windAt(const WindProfile& profile, double altitude)
{
    if (profile.empty())
    {
        return 0.0;
    }
    auto upper = profile.lower_bound(altitude);   // first entry >= altitude
    if (upper == profile.begin())
    {
        return upper->second;
    }
    if (upper == profile.end())
    {
        return std::prev(upper)->second;
    }
    auto lower = std::prev(upper);
    const double t = (altitude - lower->first) / (upper->first - lower->first);
    return lower->second + t * (upper->second - lower->second);
}
```

- **At a listed altitude**, `lower_bound` lands exactly on that entry. If it is the first entry, that value is returned directly. Otherwise `t` comes out as 1 and the interpolation returns the entry's own value, so the table values come back exactly.
- **Below the first or above the last entry**, the nearest end value is used.
- **Why not `operator[]`:** `profile[altitude]` inserts a default value (0.0) for any key that is missing. Every altitude asked about would add a zero entry, so the profile would grow with every step and its interpolation would be ruined. It also cannot be called here at all, because `profile` is `const` and `operator[]` is not a `const` member.

The profile is initialised in `SimSettings` with the worksheet's list:

```cpp
WindProfile wind{{0.0, 2.0}, {1000.0, 8.0}, {5000.0, 20.0}, {15000.0, 35.0}};
```

`Drag` keeps a reference to that profile and computes drag from the velocity **relative to the air**:

```cpp
const Vec2 wind{windAt(profile_, rocket.position().y), 0.0};
const Vec2 relative = rocket.velocity() - wind;
// ... magnitude from length(relative) as before ...
return -normalised(relative) * magnitude;
```

The wind blows in +x, the same way the rocket leans, so it carries the rocket downrange. Starter-1's range grows from about **2,890 m** (Week 7 Medium) to about **3,820 m**.

### M3 · A fleet

`main` collects one `--launch` per file into a `std::vector<std::string>`. It then loads each file into a `RocketSpec` and `SimSettings`, both keyed by rocket name. The fleet itself is a `std::map<std::string, Rocket>`:

```cpp
std::map<std::string, LiquidEngine> engines;   // one engine object per rocket
std::map<std::string, Rocket> fleet;
for (const auto& [name, spec] : specs)
{
    engines.emplace(name, LiquidEngine{});
    fleet.emplace(name, Rocket(spec, &engines.at(name)));
}
```

`fleet[name] = Rocket(...)` does not compile. `operator[]` must first default-construct a `Rocket` to assign into, and `Rocket` has no default constructor. `emplace` builds the element directly from its arguments.

Each rocket needs its own engine, because throttle is engine state. The engines live in a second map. Map nodes never move once inserted, so the `Engine*` that each `Rocket` keeps stays valid. A `std::vector<LiquidEngine>` would not be safe for this, because growing it moves its elements.

Each rocket flies on the same `Plot` (`simulate` calls `plot.startSeries(rocket.name())`). The results are ranked with one `sort` call:

```cpp
std::vector<std::pair<std::string, FlightStats>> results;
// ... results.emplace_back(name, stats); after each flight ...
std::sort(results.begin(), results.end(),
          [](const auto& a, const auto& b) { return a.second.apogee > b.second.apogee; });
```

`>` puts the highest apogee first. Like the word-frequency example, the map gives you name order for free, but to rank by value you copy the data into a vector and sort it.

`simulation.h` also defines `operator<` on `FlightStats` (by apogee), but the ranking does not use it. Two files with the same `name` collide: `emplace` keeps the first and ignores the second.

---

## Hard — scheduled events and a parachute

### H1 · An event queue

`events.h`:

```cpp
struct Event
{
    double time;
    std::string label;
    std::function<void(Rocket&)> action;
};

using EventList = std::vector<Event>;
```

`SimSettings` holds an `EventList`. `simulate` sorts a **copy** of it, because the settings are `const` and the list is consumed. On every step, it runs each event whose time has passed and marks the plot:

```cpp
EventList pending = settings.events;
std::sort(pending.begin(), pending.end(), [](const Event& a, const Event& b) { return a.time < b.time; });
std::size_t nextEvent = 0;
// in the loop:
while (nextEvent < pending.size() && pending[nextEvent].time <= rocket.time())
{
    const Event& event = pending[nextEvent++];
    event.action(rocket);
    plot.addMarker(rocket.position().x, rocket.position().y, event.label);
}
```

Advancing `nextEvent` does the job of "remove". Each event runs once, and nothing is erased from the vector while it is being walked. The two sample events are added in `main` when `--events` is given:

```cpp
const double throttle = 0.5;
settings.events.push_back({20.0, "throttle 50%", [throttle](Rocket& r) { r.engine().setThrottle(throttle); }});
settings.events.push_back({30.0, "fairing off", [](Rocket& r) { r.jettison(5.0); }});
```

The throttle lambda captures `throttle` **by value**. The event is stored and runs long after this block has ended. With `[&]` it would hold a reference to a dead local variable. `setThrottle` is virtual on `Engine`, and every rocket here has a `LiquidEngine`, the Week 8 class that honours it.

> [!NOTE]
> The two events differ from the worksheet in what they actually do.
> - `src/` reads no events from a file. They are added in code behind the `--events` flag.
> - `Rocket::jettison` reduces `payloadMass_`, clamped at zero, not the dry mass. Only `TwoStage` has a payload (10 kg). For Starter-1 and Heavy-B, "fairing off" fires and marks the plot but changes nothing. To match the worksheet, subtract the 5 kg from the active stage's `dryMass` instead.
> - All three supplied rockets burn out before 20 s: Starter-1 at 15 s, Heavy-B at about 18 s, TwoStage at about 18 s. The throttle event therefore fires after burnout and has no visible effect. Fire it at, say, 10 s to see the burn stretch out.

### H2 · A parachute

The worksheet's trigger is **state, not time**, so `src/` does not use an `Event`. `simulate` checks it every step:

```cpp
if (settings.parachute && launched && !rocket.parachuteDeployed() && rocket.velocity().y < 0.0)
{
    rocket.deployParachute();
    plot.addMarker(rocket.position().x, rocket.position().y, "parachute");
}
```

The condition is the first step after leaving the pad where vertical velocity is negative, which is apogee. `launched` stops it firing on the pad. On the rocket:

```cpp
void deployParachute(double areaMultiplier = 20.0) { dragAreaMultiplier_ = areaMultiplier; }
double area() const { return area_ * dragAreaMultiplier_; }
bool parachuteDeployed() const { return dragAreaMultiplier_ > 1.0; }
```

The default `20.0` is a drogue. `Drag` already uses `rocket.area()`, so the force code is unchanged. The landing speed is printed in the flight summary (`landing … m/s`, taken from `log.back().speed`). It drops from about **190 m/s** to about **25 m/s**.

A 200× main canopy lands under 10 m/s, but it takes over twelve minutes to come down, well past the 300 s `max_time`. If you try it, raise `max_time`.

> [!NOTE]
> The parachute only deploys if `settings.parachute` is true. It is off by default and set by `parachute = yes` in a launch file. `launch_twostage.txt` has that line; `launch.txt` and `launch_b.txt` do not. To see it on Starter-1, add `parachute = yes` to `launch.txt` (no rebuild needed).

### H3 · A table-driven parser

**Validation (in `src/`).** The known keys live in a `std::set<std::string>`, and an unknown key is reported with its line number:

```cpp
const std::set<std::string> KNOWN_KEYS = {"name", "dry_mass", "fuel_mass", "thrust", "burn_rate",
                                          "drag_coeff", "area", "launch_angle", "dt", "max_time",
                                          "payload_mass", "stage", "parachute"};
// ...
if (KNOWN_KEYS.count(key) == 0)
{
    std::cerr << path << ":" << lineNumber << ": unknown key '" << key << "'\n";
    continue;
}
```

**Dispatch (not in `src/`).** `src/` still uses an `if` / `else if` chain for the numeric keys. The table-driven version builds the map inside `loadLaunchFile`, because the pointers are to *this call's* `spec` and `settings`:

```cpp
Stage& single = spec.stages.front();   // the legacy single-stage keys
const std::map<std::string, double*> fields = {
    {"dry_mass", &single.dryMass},       {"fuel_mass", &single.fuelMass},
    {"thrust", &single.thrust},          {"burn_rate", &single.burnRate},
    {"drag_coeff", &spec.dragCoeff},     {"area", &spec.area},
    {"launch_angle", &spec.launchAngleDeg}, {"payload_mass", &spec.payloadMass},
    {"dt", &settings.dt},                {"max_time", &settings.maxTime}};

// in the loop, after the name / stage / parachute special cases:
const auto field = fields.find(key);
if (field != fields.end())
{
    *field->second = std::stod(value);
}
```

The pointers into `spec.stages.front()` stay valid because `stage =` lines go into a separate local vector, and that vector is only assigned to `spec.stages` after the loop. Adding a numeric key is now one line in the table. `name`, `stage` and `parachute` are not doubles, so they stay as explicit cases. `launch.txt` still reproduces the default flight, and a misspelt key is still reported by the `set` check.

### H4 · `transform` and `partial_sort`

In `printLogSummary`:

```cpp
std::vector<double> altitudes(log.size());
std::transform(log.begin(), log.end(), altitudes.begin(), [](const Sample& s) { return s.y; });
std::vector<double> speeds(log.size());
std::transform(log.begin(), log.end(), speeds.begin(), [](const Sample& s) { return s.speed; });
std::partial_sort(speeds.begin(), speeds.begin() + 5, speeds.end(), std::greater<double>());
```

`transform` writes into a vector that already has the right size; it does not grow the destination. `partial_sort` puts the five largest values, in order, at the front, and leaves the rest unsorted. That costs about *n* log 5 instead of *n* log *n* for a full sort.

> [!NOTE]
> `src/` partial-sorts the extracted **speeds**, so it prints five numbers rather than five samples. It also computes `speeds.begin() + 5` without checking the size, which is undefined for a log shorter than five samples. To print the samples themselves, in descending speed order, partial-sort a copy of the log (the log is `const`):
>
> ```cpp
> std::vector<Sample> fastest = log;
> const auto middle = fastest.begin() + std::min<std::size_t>(5, fastest.size());
> std::partial_sort(fastest.begin(), middle, fastest.end(),
>                   [](const Sample& a, const Sample& b) { return a.speed > b.speed; });
> for (auto it = fastest.begin(); it != middle; ++it)
> {
>     std::cout << "  t=" << it->time << " s  speed=" << it->speed << " m/s\n";
> }
> ```

---

## Checkpoint

| Tier | Expected |
| --- | --- |
| Easy | The CSV (`flight.csv`, or `<out>.csv`) is written from a `std::vector<Sample>`; apogee and max speed come from `max_element` after the loop; the summary shows time above 5 km and mean speed. With the wind on by default, Starter-1's range is about **3,820 m** |
| Medium | `TwoStage` shows a `stage 1 separation` marker and beats Starter-1's apogee on the same fuel; a ranking of the three rockets by apogee from one `sort` call |
| Hard | `throttle 50%` and `fairing off` markers at 20 s and 30 s with `--events`; a `parachute` marker at apogee and a landing speed of about **25 m/s** (instead of about 190 m/s) for any rocket whose launch file has `parachute = yes` |
