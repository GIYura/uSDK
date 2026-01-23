# Codex Review Guide — uSDK (Embedded C, CMSIS-first)

## Project intent
uSDK is a reusable embedded library with:
- HAL-style MCU-agnostic APIs in `hal/include/`
- MCU-specific implementations in `platforms/`
- portable drivers/services in `lib/`
- core utilities in `core/`, `utils/`, `buffer/`, `event/`, `delay/`

## Review priorities
### P0 (must fix)
- Undefined behavior in C (out-of-bounds, invalid shifts, strict aliasing violations, uninitialized reads)
- ISR/concurrency issues: missing `volatile`, missing critical sections, race conditions on shared state, non-ISR-safe calls from ISR
- Broken register access (wrong bit fields, write-one-to-clear mistakes, ordering issues)
- API breaking changes in `hal/include/` without migration plan
- Memory safety: stack overflow risks, recursion, uncontrolled buffer writes, missing bounds checks
- Timing bugs: wrong units, overflow in tick math, busy loops in paths that must be non-blocking

### P1 (should fix)
- Error-handling gaps: ignored return codes, missing propagation, unclear ownership/lifetime
- Portability regressions across MCUs (assumptions about word size, endianness, alignment)
- Inconsistent naming/style in public HAL APIs; missing/incorrect docs for public functions
- Logging/printf in low-level drivers where it shouldn’t be (unless behind a compile-time flag)

## Embedded constraints
- Avoid dynamic allocation (`malloc/free`) in library code unless explicitly justified and guarded.
- Avoid heavy libc in drivers (snprintf/vsnprintf) unless already accepted by module design.
- Keep ISR handlers minimal: no blocking waits, no long loops, no non-ISR-safe calls.
- Prefer clear integer-width types (`uint32_t`, etc.), watch for implicit promotions.

## What to include in reviews
- Point to exact file/line and propose a concrete fix.
- Suggest minimal tests (unit tests for pure logic; hardware test steps for drivers).
- If behavior depends on HW, ask for a brief comment describing the assumption.

## Repo-specific checks
- `hal/include/`: treat as stable public interface; changes must be justified and documented.
- `platforms/`: ensure CMSIS/register-level correctness; verify clock enables, reset states, IRQ config.
- `lib/`: must stay portable; no direct MCU register access; only use HAL APIs.
