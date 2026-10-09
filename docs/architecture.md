# Architecture and design notes

## Design goals

1. Keep event processing independent of a particular board or GPIO library.
2. Use monotonic time for event intervals.
3. Keep input parsing strict: malformed or inconsistent rows fail replay.
4. Make the portable core testable without electrical hardware.
5. Make boundaries and limitations explicit rather than implying hard real-time guarantees.

## Modules

- `event.c`: normalized event type, edge name conversion, strict CSV reader/writer.
- `debounce.c`: state machine that accepts a transition only after the candidate input has remained unchanged for the configured interval.
- `gpio_source.c`: deterministic host demo event source; it is a seam for the future libgpiod adapter.
- `replay.c`: reads validated records, maintains order checks, applies relative delays, and dispatches events through a callback.
- `main.c`: command line, file lifetime, error reporting, and host-mode orchestration.

## Time model

All log times are nanoseconds relative to the first captured/accepted event. Live capture should take timestamps from `clock_gettime(CLOCK_MONOTONIC, ...)` or the GPIO event timestamp source where supported. Never derive intervals by subtracting wall-clock timestamps.

Replay waits for each delta divided by the positive speed multiplier. `--no-sleep` is intended for tests and inspection. On a general-purpose Linux kernel, scheduling jitter means this is best-effort timing, not a hard-real-time output engine.

## CSV contract

Header: `sequence,elapsed_ns,line_offset,edge,value`

Rows must have exactly five fields. Sequence numbers must increase during replay and elapsed time must be nondecreasing. Rising edges use logical value 1; falling edges use value 0. GPIO offsets are controller line offsets, not board header pin numbers.

## Safety and error policy

The current application does not output GPIO levels; replay emits events to a software callback. A future hardware adapter must not silently turn replay logs into actuator commands. Do not use this diagnostic utility to actuate safety-critical systems without an independent safety design.
