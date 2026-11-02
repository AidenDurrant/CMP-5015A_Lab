# `config.h` / `config.cpp` — reading a launch file

[← Back to the code reference](README.md)

| | |
| --- | --- |
| **Files** | `src/config.h`, `src/config.cpp` |
| **Status** | Header given · `config.cpp` **stub — TODO Week 7 Hard** |
| **Includes** | `<string>`, `rocket.h`, `simulation.h`, `<iostream>` |
| **Used by** | `main.cpp` for the `--launch FILE` option |

---

## Purpose

Lets a rocket be described in a text file instead of being compiled in, so you can change the launch angle or the thrust without rebuilding. The file format is described in [Build, run and project files → `launch.txt`](build-and-project.md#launchtxt).

---

## Functions

### `bool loadLaunchFile(const std::string& path, Rocket& rocket, SimSettings& settings)`

| | |
| --- | --- |
| **Status** | **TODO — Week 7 Hard**. Rewritten in **Week 11**. |
| **Parameters** | `path` — file to read, read only · `rocket` — **modified**: fields named in the file are overwritten · `settings` — **modified**: `dt` and `max_time` |
| **Returns** | `false` if the file cannot be opened (and `rocket` and `settings` are left unchanged); `true` otherwise |

**Current behaviour (stub):** ignores its arguments, prints `loadLaunchFile is not implemented yet; using the default rocket` to `std::cerr`, and returns `false`. `rocketsim --launch …` therefore flies the default rocket.

**Contract (Week 7 Hard):**

- Opens `path` with `std::ifstream`; if that fails, return `false`.
- Reads the file **one line at a time**.
- Ignores everything after a `#`, and skips lines that are then blank.
- Splits each line at `=` into a key and a value, with surrounding spaces removed.
- Assigns the value to the matching field. `name` is the only text value; all others are numbers.
- Reports an **unknown key** on `std::cerr`, with its **line number**, and carries on.

The comment in `config.cpp` lists the steps and the standard-library tools to use (`std::getline`, `find`, `substr`, `std::stod`).

> [!WARNING]
> `std::stod` **throws an exception** if the value is not a number (for example `thrust = lots`), and in Week 7 that will end the program. Leave it that way. Week 11 replaces this parser with stream extraction and proper exception handling.

---

## Changes in later weeks

| Week | Change |
| --- | --- |
| 9 Medium | Accepts repeated `stage = …` lines for multi-stage rockets. |
| 9 Hard | Validates keys with a `std::set` and dispatches with a `std::map` from key to field. |
| 10 Hard | Accepts an `engine = solid` / `engine = liquid` key. |
| 11 | Rewritten with `std::istringstream` and `>>`; throws `std::runtime_error` (later `LaunchFileError`) with the file name and line number instead of returning `false`. |
