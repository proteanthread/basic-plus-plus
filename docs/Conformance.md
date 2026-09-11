# BASIC++ Conformance Statement

**The position this project takes on every standard and law it names.**

| | |
|---|---|
| Version | 1.0 |
| Date | 2026-09-10 |
| Applies to | v6.5.2, and to 7.0.0 as the target |
| Framework | `PROJECT_RULES.md` PART I section 9 |
| Summary table | `PROJECT_RULES.md` PART II section 10 |

---

## Why this document exists

"Follows ISO standards" is not auditable. A named standard with a named position is.

Before this document, the project rules said that all source code "must strictly adhere to" eleven frameworks, including the EU Medical Device Regulation. BASIC++ is not a medical device. A reviewer reading that claim asks for the notified body assessment and the ISO 13485 quality management system, finds neither, and now doubts every other claim in the file. **An overclaimed conformance statement is worse than no conformance statement**, because it converts good engineering into an apparent misrepresentation.

This document replaces adherence language with four verbs, each of which means something specific and testable.

## The four positions

| Position | Meaning |
|---|---|
| **CONFORMS** | The project meets this today **and an automated check in the build proves it**. A CONFORMS claim without a check is not permitted; downgrade it to TARGETS. |
| **TARGETS** | The project intends to meet this. There is a recorded gap in the defect register and a step in the build plan. No conformance may be claimed to a third party. |
| **REFERENCES** | Used as a source of practice, structure and vocabulary. No conformance claim is made or implied. This is the correct position for a standard whose value is its thinking rather than its checklist. |
| **DOES NOT APPLY** | Out of scope. Named here deliberately so that it is not re-added by someone who thinks more standards is better. |

Every standard the project names must carry exactly one of these. A standard with no position does not belong in the file.

---

## 1. Coding rule set - exactly one primary

### MISRA C:2012 with Amendment 3 - PRIMARY - TARGETS

The single rule set this project is measured against. Chosen because it has the most mature tooling for freestanding embedded C, and because it is the coding standard a safety argument is normally built on.

Triage order is **Mandatory**, then **Required**, then **Advisory**. Advisory-level findings are a style conversation and are *not* defects; filing them as defects is how a MISRA adoption fails.

*Status:* never run. No conformance may be claimed until a baseline exists and the Mandatory and Required sets are clean or have recorded, justified deviations.

### ISO/IEC TS 17961 - SUPPLEMENTARY - REFERENCES

Run as a security cross-check where it catches something MISRA does not. Findings are filed under the corresponding MISRA rule where one exists, so the same defect is never counted twice.

### SEI CERT C - REFERENCES

Consulted where its rationale explains a MISRA rule better than MISRA's own text. Never audited against independently.

> **Rule:** a finding is filed against MISRA or it is filed against nothing. Auditing against three overlapping rule sets triplicates every finding and makes the register useless.

---

## 2. The language

### ISO/IEC 9899:2018 (C17) - TARGETS, one gate away from CONFORMS

Rule #2. The engine is freestanding C17. Not C99, not C11, not C23, no C++.

**Evidence:** 842 of 856 files under `engine/src` and `engine/lib` compile clean under `gcc -std=c17 -pedantic-errors` with the project's own defines. Twelve of the fourteen failures are files already recorded as dead or already on the defect register. Two are genuine:

| File | Construct |
|---|---|
| `engine/include/interop/interop_core.h` | forward reference to an `enum` type |
| `engine/src/vm/jit/jit_native.c` | object pointer to function pointer conversion |

The first is a defect and is scheduled. The second is the `dlsym` idiom, is unavoidable in a JIT, and becomes a **recorded deviation** - localised to that file, justified in a comment at the site, and listed here.

This becomes CONFORMS when `-pedantic-errors` is a build gate and the deviation list is in this document. That is one CI step away.

### The freestanding boundary - TARGETS

C17 clause 4 paragraph 6 permits exactly nine headers in a freestanding implementation: `float.h`, `iso646.h`, `limits.h`, `stdalign.h`, `stdarg.h`, `stdbool.h`, `stddef.h`, `stdint.h`, `stdnoreturn.h`.

Everything else is hosted. The engine legitimately needs hosted headers in its platform layer - `plat_net.c` *should* include `<sys/socket.h>`, that is its job - so the rule is not "no libc", it is "libc only here":

**Sanctioned, may include any system header:**

- `engine/lib/platform/`
- `engine/src/hal/`
- `engine/src/hardware/`
- `engine/iot/src/`
- `engine/include/platform/`

**Everywhere else:** a system header outside the nine is a build failure.

Measured today: 29 of 1,600 files include a hosted header. 19 are inside the sanctioned set. 10 are not. This becomes CONFORMS when the check is in the build; it is eight lines of script.

---

## 3. Product quality - scalability, portability, modularity, readability

### ISO/IEC 25010:2023 - TARGETS

The vocabulary. Findings are filed under these characteristic names so they can be counted rather than merely described.

| What we call it | ISO/IEC 25010:2023 name | Gate |
|---|---|---|
| modularity | Maintainability > Modularity | files <= 400 lines, target 250; functions <= 60 lines |
| readability | Maintainability > Analysability | 6-line header with `NEEDS` and `NEEDED BY`; no Doxygen tags |
| scalability | Flexibility > Scalability | bounded loops, no unbounded allocation |
| portability | Flexibility > Adaptability, Installability, Replaceability | the freestanding boundary above |
| maturity | Reliability > Maturity | the defect register trend |
| safety | Safety | section 4 |
| security | Security | section 5 |

The 2023 revision renamed *Portability* to *Flexibility* and added *Safety* as a top-level characteristic. Text written against 25010:2011 uses the old names and should be updated when touched.

*Status today:* modularity - 82 files and 367 functions over gate. Readability - 630 prohibited tags. Portability - 10 boundary violations.

### Scalability - the standards, the law, and the gates - TARGETS

Scalability has no EU law of its own. It has one regulatory hook, and it is a real one:

> **Regulation (EU) 2024/2847, Annex I Part I, point 2(d)** - products with digital elements shall "protect the availability of essential functions, including the resilience against and mitigation of denial of service attacks".

An interpreter that can be made to allocate without bound, recurse without bound, or loop without bound **by its input** is a denial-of-service surface. That is the CRA hook, and it is why scalability sits beside security here rather than in a performance section.

**The standards:**

| Standard | Role |
|---|---|
| ISO/IEC 25010:2023 | *Flexibility > Scalability* - the characteristic name findings are filed under. |
| ISO/IEC 25023:2016 | Measurement of system and software product quality. The part that turns a 25010 characteristic into a **number**. Adopted for the scalability and maintainability measures specifically. |
| ISO/IEC 5055:2021 | Its *Performance Efficiency* measure is built from exactly the weaknesses that break scalability: unbounded loops, uncontrolled recursion, resource leaks in loops, expensive operations inside loops. |

**The best practices, stated as gates so they can be checked:**

1. **Bounded input.** Every loop whose trip count comes from program text or user input has an explicit ceiling and a defined error when it is reached. Silent truncation is not a ceiling.
2. **No host recursion.** Parser and AST state lives in interpreter-managed structures, never on the host stack. Already a project invariant; this makes it a scalability gate as well as a safety one.
3. **Bounded stack per frame.** No function places an unbounded or large object on the stack. The counter-example already on the register puts 14 KB of stack in one call, on a profile whose target has 4 KB.
4. **Allocation is declared.** Zero-malloc in the engine. Where a module allocates, its size is a function of a declared bound, not of input length.
5. **Complexity is written down.** Any operation worse than O(n log n) on a user-visible collection carries its complexity in the file header. An interpreter is allowed to be slow; it is not allowed to be *surprisingly* slow.
6. **The profile is the unit.** `BASIC_FREESTANDING_16`, the IoT profile and the desktop profile have different ceilings. A scalability claim names its profile or it means nothing.

*Status:* no measurement. Gates 1, 3 and 4 have known violations already on the register. Gate 5 is not being followed anywhere.

### ISO/IEC 25023:2016 - TARGETS

Measurement of system and software product quality. Adopted for the scalability and maintainability measures; not adopted wholesale, because most of its measures need operational data this project does not collect.

### Directive (EU) 2022/2555 (NIS2) - DOES NOT APPLY to the interpreter

Availability and resilience obligations bind essential and important **entities**, not software vendors. A deployer running BASIC++ in scope of NIS2 carries them. Named here because "EU law for availability" leads here first, and the answer needs to be on the record.

### ISO/IEC 5055:2021 - TARGETS

Automated Source Code Quality Measures: Reliability, Security, Performance Efficiency, Maintainability, each defined as a set of countable weaknesses. This is the one that becomes a numeric build gate, because unlike 25010 it is machine-checkable by construction.

*Status:* no baseline. The number matters less than having one, because the gate is "must not regress", not "must be low".

---

## 4. Safety

### IEC 61508 (all parts) - REFERENCES

The generic functional-safety standard. BASIC++ is **not** developed under a 61508 lifecycle, has no SIL claim, and has had no independent assessment. What it takes from 61508 is the structure of a safety argument and the reason MISRA C is the coding standard in section 1.

> A SIL claim requires an independent assessor. **Nothing in this project may be described as SIL-anything.**

### ISO/IEC/IEEE 12207 - REFERENCES

Software life cycle processes. The reference for how the audit, build and release steps are organised. No process conformance is claimed.

### ISO/IEC/IEEE 29119 parts 1-4 - TARGETS

Software testing. This is the standard the `tests/` directory is measured against, and today it fails on the most basic point: test suites report pass and fail in their output text and exit 0 regardless, and several assert success against subsystems that are stubs. The directory is also absent from the repository.

> Nothing about the current test suite may be offered as evidence of anything until the exit-status defect is fixed.

### Determinism and bounded execution - TARGETS

Good practice **derived from** safety-critical work. These are not conformance to any regulation:

- every search loop, tokenizer scan and interpreter tick is bounded
- no host recursion: parser and AST state lives in interpreter-managed structures, never on the host stack
- single-exit cleanup or scoped release on every path
- no silent error swallowing

The last of those is currently violated in several recorded ways.

---

## 5. Security and regulatory law

### Regulation (EU) 2024/2847 - Cyber Resilience Act - TARGETS, binding status UNRESOLVED

Reporting obligations began 11 September 2026. Full application is 11 December 2027.

**Whether the Act binds this project is still an open question**, and it turns on three facts: is the project monetised, is there a legal entity providing sustained support for commercial use, and will the engine ever ship inside someone else's product. Free and open-source software outside the course of a commercial activity is carved out.

Until those three are answered, the project TARGETS Annex I as a quality exercise. It does not claim compliance and **must not be described as CRA-compliant**.

*Annex I work already done:* `SECURITY.md` exists and provides a coordinated vulnerability disclosure policy with 24-hour, 72-hour and 14-day timelines.

*Annex I work outstanding:* the crypto key generator has no entropy source, there is no transport security anywhere, and the support period in `SECURITY.md` is unset.

### Directive (EU) 2024/2853 - Product Liability Directive - REFERENCES, pending the same determination

Transposition deadline 9 December 2026. Software is a product under this Directive, and defectiveness is judged against what a user is entitled to expect. **Silent wrong answers are the exposure**, which is why the defect register ranks a silent wrong result above a crash.

Retained and relabelled: strict reference-counting lifecycle on `VAL_STRING`, `VAL_MAP` and `VAL_OBJECT`; descriptive standardised BASIC error codes with exact source line attribution.

### Regulation (EU) 2016/679 (GDPR) - DOES NOT APPLY to the interpreter

BASIC++ processes no personal data as a product. GDPR binds whoever **deploys** it in a way that processes personal data, not the interpreter.

Two rules are retained as good practice, not as compliance:

- secure zeroization of buffers holding user data on deallocation
- no background log file or telemetry without an explicit flag (`--log`, `--debug`)

The second is worth keeping on its own merits regardless of GDPR.

---

## 6. Arithmetic

### IEEE 754-2019 - TARGETS

Binary floating point. Two recorded gaps: `CINT` rounds half away from zero where 754's default is `roundTiesToEven` (which is also what QBASIC does); and overflow yields `inf` rather than an error.

Where Rule #1 and IEEE 754 disagree, **Rule #1 wins** and the deviation is recorded here. On `CINT` they agree, which is why that one is an easy fix rather than a tradeoff.

### ISO/IEC 10967 parts 1-3 - Language Independent Arithmetic - REFERENCES

The reference for what arithmetic must do at the edges. A BASIC lives or dies on those edges. Consulted, not conformed to.

---

## 7. The BASIC language itself

| Standard | Position |
|---|---|
| ISO/IEC 10279:1991 Full BASIC | REFERENCES - deliberately **not** a target |
| ECMA-55 Minimal BASIC | REFERENCES |
| ECMA-116 Standard BASIC | REFERENCES |

Rule #1 is absolute backward compatibility with GW-BASIC, BASICA, QBASIC and VB DOS. Those dialects are not conformant to 10279 and never were. Conformance to 10279 is therefore **impossible** while Rule #1 holds, and Rule #1 is not negotiable.

These standards are the reference for which construct means what, and for the ANSI Full BASIC features BASIC++ chooses to add on top. See `docs/Structured_BASIC.md` section 6.

> **Nothing in this project may be described as ANSI BASIC compliant.**

---

## 8. Design and documentation

| Standard | Position | Role |
|---|---|---|
| IEEE 1016 | REFERENCES | What the `Freestanding_*_Architecture` documents ought to contain. |
| ISO/IEC/IEEE 26514 | REFERENCES | What `docs/` and `help/` are measured against. |

---

## 9. Explicitly out of scope

Named here so that they are not re-added.

### MDR (EU) 2017/745 and IVDR (EU) 2017/746 - DOES NOT APPLY

BASIC++ is not a medical device and is not software intended by its manufacturer for a medical purpose. There is no ISO 13485 quality management system, no technical documentation under Annex II, no notified body and no clinical evaluation. Claiming adherence to MDR invites a reviewer to ask for all four.

The bounded-execution invariants previously filed under MDR are retained in section 4 as derived practice.

### Regulation (EU) 2023/1230 - Machinery Regulation - DOES NOT APPLY

BASIC++ is not safety software placed on the market as part of machinery. Same reasoning, same retention.

### ISO 26262, DO-178C, IEC 62304 - DOES NOT APPLY

Automotive, airborne and medical-software lifecycle respectively.

If BASIC++ is ever embedded in a product in one of these sectors, **that product's manufacturer carries the obligation, not this project** - but they will ask for the 61508-shaped evidence in section 4, which is the practical reason for keeping it.

---

## 10. What would make each claim true

The point of a TARGETS position is that it names what is missing.

| Claim | What is missing |
|---|---|
| C17 | `-pedantic-errors` as a build gate; `interop_core.h` fixed; `jit_native.c` deviation recorded here |
| freestanding | the eight-line boundary check in the build |
| 25010 modularity | 82 files and 367 functions split, or documented exceptions written |
| 25010 readability | 630 Doxygen tags removed |
| 5055 | one baseline measurement, then a no-regress gate |
| MISRA | one baseline run; Mandatory and Required clean or deviated with justification |
| 29119 | test suites that exit non-zero on failure |
| IEEE 754 | `CINT` rounds half to even; overflow raises Error 6 |
| CRA | the three questions answered; entropy in the key generator; a support period in `SECURITY.md` |

Nine items. Six of them are build-plan steps that already exist. **None of them requires a new standard to be adopted.**
