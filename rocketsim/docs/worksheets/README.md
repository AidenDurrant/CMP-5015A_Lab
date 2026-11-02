# rocketsim — Lab Worksheets (Weeks 7–12)

> **CMP-5015A · Programming 2 — C++ lab series**

Over six weeks you build **one** program, `rocketsim`: a 2-D rocket flight simulator that replays its flight in a live window and writes it to an SVG file. Each week you extend the same code base with that week's lecture material, so by Week 12 you have a finished project rather than six throwaway exercises.

| Week | Worksheet | Theme |
| --- | --- | --- |
| 7 | [Get Flying](week07.md) | References, strings, streams, `new` / `delete` |
| 8 | [Classes in depth](week08.md) | Constructors, RAII, the rule of three, operators, inheritance |
| 9 | [Collections](week09.md) | `vector`, `map`, `<algorithm>`, lambdas |
| 10 | [Generics and ownership](week10.md) | Templates, `unique_ptr`, `shared_ptr`, move semantics |
| 11 | [Serialisation and errors](week11.md) | Streams, binary I/O, exceptions, RTTI |
| 12 | [Threads](week12.md) | `std::thread`, mutexes, atomics, `async`/`future`, condition variables |

For what every file and function in the starter does, see the [code reference](../code-reference/README.md).

---

## How each lab works

Every week has three **tiers** of tasks:

| Tier | Purpose | Who | Time in a 1 h 45 lab |
| --- | --- | --- | --- |
| **Easy** | Keeps the simulator working end to end; uses the week's core idea | Everyone | First ~50 minutes |
| **Medium** | Adds a real feature; uses the week's material properly | Most students | Remaining ~50 minutes |
| **Hard** | Stretch work; often previews next week | If you finish early | At home |

If you only ever do the Easy tasks, your simulator will still work at the end of Week 12.

**Checkpoints.** Each week ends with a **checkpoint**: specific things your plot or console output must show. Run your program and check each item yourself before moving on to the next week; if something doesn't match, the "Watch out for" section is the first place to look.

**Missed a week?** The model solution for each week is released before the next lab. Copy it over your project and carry on; nobody is left behind.

**Formative work.** These labs are not assessed coursework. They are your practice for the module, so the more you attempt yourself, the more you get out of them.

---

## The carry-over rule

Code you write in an earlier week is never just thrown away — it is **replaced by the C++ facility that does the same job better**. For example:

| You write by hand in… | …and replace it in | …with |
| --- | --- | --- |
| Week 7: a `new[]`/`delete[]` telemetry log | Week 9 | `std::vector` |
| Week 8: raw `Force*` pointers | Week 10 | `std::unique_ptr` |
| Week 7: a `getline` + `substr` parser | Week 11 | `>>` with exceptions |

When the replacement arrives, notice how much code disappears. That is the point.

---

## Reading the worksheets

Each worksheet uses the same layout:

| Section | Meaning |
| --- | --- |
| **By the end of this lab** | What you will have achieved |
| **Background** | The problem, and why this week's C++ solves it |
| **Easy / Medium / Hard** | The tasks, numbered E1, M1, H1, … |
| **Done when** | How to tell an individual task is finished |
| **Screenshot** | What you should see at that point |
| **Checkpoint** | A self-check of the week's results before moving on |
| **Watch out for** | Common mistakes |
| **Reflect** | Questions to test your understanding |

> [!TIP]
> **Build and run often.** After every task, rebuild and run. Small steps make compiler errors short and bugs easy to find.

> [!IMPORTANT]
> **New `.cpp` file?** Re-run `cmake -B build` so the build picks it up. A linker error about a function you *know* you wrote is almost always this.
