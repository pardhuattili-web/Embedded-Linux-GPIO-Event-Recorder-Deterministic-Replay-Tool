# Embedded Linux GPIO Event Recorder & Deterministic Replay Tool

[![C11](https://img.shields.io/badge/C-C11-blue.svg)](https://en.wikipedia.org/wiki/C11) [![Linux](https://img.shields.io/badge/platform-Linux-informational.svg)](https://kernel.org/) [![CI](https://github.com/pardhuattili-web/Embedded-Linux-GPIO-Event-Recorder-Deterministic-Replay-Tool/actions/workflows/host-tests.yml/badge.svg)](https://github.com/pardhuattili-web/Embedded-Linux-GPIO-Event-Recorder-Deterministic-Replay-Tool/actions/workflows/host-tests.yml) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A small, testable Linux systems-programming project that records GPIO edge events with monotonic timestamps, filters mechanical switch bounce, writes portable CSV logs, and replays captured timing for repeatable debugging. Hardware access uses the Linux GPIO character-device API through **libgpiod**; the event-processing and replay core remains independent of hardware so it can be tested on an ordinary Linux computer.

> **Project status:** host-mode capture, CSV event log, debouncing, and replay form the portable core. Live GPIO capture is an optional `libgpiod` integration and requires compatible GPIO hardware, permissions, and a line offset. See [hardware setup](docs/hardware_setup.md).

## Why this project?

Embedded bugs are often hard to reproduce because inputs arrive at inconvenient times or switch contacts bounce. This tool helps engineers capture an input sequence once, then replay its timing through a deterministic software test path. It showcases C, Linux APIs, defensive parsing, timestamp design, hardware abstraction, testing, and operational documentation.

## Architecture

```text
GPIO chip / host event source
          |
          v
  Capture adapter (libgpiod or demo source)
          |
          v
  Debounce + normalized event model
          |
          +------> CSV event logger
          |
          +------> Replay scheduler
                        |
                        v
                 Test callback / trace
```

## Features

- **C11 core** with clear modules and a small public event model.
- **Monotonic timestamps** represented in nanoseconds; wall-clock changes do not distort event intervals.
- **Configurable debouncing** using a minimum stable interval per input line.
- **CSV logging** with sequence number, elapsed nanoseconds, line offset, edge, and logical value.
- **Replay mode** that honours recorded relative delays, with a speed multiplier for faster or slower inspection.
- **Hardware-independent host mode** for development without Raspberry Pi hardware.
- **Live GPIO integration point** designed for Linux GPIO character devices via `libgpiod`.
- **Input validation and graceful shutdown** with clear error messages.
- **Unit tests** for event parsing, debounce boundaries, and CSV round trips.

## Repository layout

```text
src/
  main.c              CLI and application orchestration
  event.c / event.h   Event model, CSV encoding and parsing
  debounce.c / .h     Debounce state machine
  replay.c / .h       Timing-aware replay engine
  gpio_source.c / .h  Host demo source and optional libgpiod adapter
tests/
  test_event.c
  test_debounce.c
  test_replay.c
examples/
  sample_events.csv
docs/
  architecture.md
  hardware_setup.md
  test_plan.md
systemd/
  gpio-event-recorder.service
Makefile
```

## Build and run

### Requirements

- Linux, GCC or Clang with C11 support
- `make`
- Optional live GPIO support: `libgpiod` development headers/tools and a Linux GPIO character device

On Debian/Ubuntu:

```sh
sudo apt update
sudo apt install build-essential pkg-config libgpiod-dev gpiod
```

Build and run host tests:

```sh
make
make test
```

Record a repeatable demo sequence without hardware:

```sh
./build/gpio-recorder record --source demo --duration-ms 1500 --output events.csv
```

Replay a captured log at normal speed:

```sh
./build/gpio-recorder replay --input events.csv
```

Replay twice as fast:

```sh
./build/gpio-recorder replay --input events.csv --speed 2.0
```

Inspect the bundled sample:

```sh
./build/gpio-recorder replay --input examples/sample_events.csv --no-sleep
```

Run `./build/gpio-recorder --help` for supported options. Exact commands may evolve as the live GPIO adapter is enabled for a target board.

## Event log format

`sequence,elapsed_ns,line_offset,edge,value`

Example:

```csv
sequence,elapsed_ns,line_offset,edge,value
0,0,17,rising,1
1,24000000,17,falling,0
2,140000000,17,rising,1
```

- `sequence`: zero-based order in the log
- `elapsed_ns`: monotonic time since the first accepted event
- `line_offset`: GPIO line offset, not a physical pin label
- `edge`: `rising` or `falling`
- `value`: logical value after the transition

Logs are data, not executable commands. Replay emits event records to a callback/trace; it **does not drive physical GPIO outputs**.

## Hardware setup

See [docs/hardware_setup.md](docs/hardware_setup.md). Start in host mode first. Live capture requires selecting the correct `/dev/gpiochipN`, line offset, compatible permissions, and a suitable low-voltage input circuit. Never connect a GPIO directly to mains or out-of-range voltages.

## Testing and validation

`make test` runs host-side unit tests and does not require physical hardware. Hardware smoke tests are documented separately. Don't describe live GPIO capture as verified until it has been run against the target board and GPIO controller.

## Limitations and next steps

- GPIO offsets and permissions vary by board and kernel.
- Scheduler latency means replay timing is best-effort on general-purpose Linux; this is not a hard real-time waveform generator.
- The host demo source validates the core pipeline but does not prove electrical behaviour.
- Optional `libgpiod` APIs differ by major version; keep the board-specific adapter small and verify it against the installed version.

Future enhancements: JSON export, multi-line capture, systemd packaging, trace comparison, and a hardware-in-the-loop CI fixture.

## Portfolio talking points

- Why use `CLOCK_MONOTONIC` instead of wall clock time?
- How does the debounce state machine handle bounce bursts?
- Why separate the capture adapter from the event model?
- What timing guarantees can and cannot be made by standard Linux?
- How would you expand this to several GPIO lines safely?

## License

MIT — see [LICENSE](LICENSE).
