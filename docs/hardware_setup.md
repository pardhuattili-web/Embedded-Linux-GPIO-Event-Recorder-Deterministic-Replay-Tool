# Hardware setup and live GPIO integration

## Host-only workflow

You can build and test the portable core on any Linux computer with a C11 compiler and Make:

```sh
make
make test
./build/gpio-recorder record --source demo --output events.csv
./build/gpio-recorder replay --input events.csv --no-sleep
```

This path uses a deterministic host event source. It does not access physical GPIO.

## Optional live GPIO capture

The repository keeps the live capture adapter as a hardware-specific integration point. To implement it for your board:

1. Install `libgpiod` and its development headers for your distribution.
2. Inspect available controllers with `gpiodetect` and available line names with `gpioinfo`.
3. Confirm the correct line offset from the board documentation; do not assume the offset equals a Raspberry Pi physical header pin.
4. Wire a supported low-voltage digital input with a suitable pull-up or pull-down and common ground as appropriate.
5. Request the line as input with rising/falling edge detection and use the API version installed on the target.
6. Convert library events into `gpio_event_t`, apply debounce, and pass accepted events to `event_write_csv`.
7. Test with a signal generator or a safely wired switch and compare edge counts against expected changes.

The exact libgpiod API differs between major versions. Keep all version-specific calls isolated in the capture adapter rather than spreading them into the event and replay modules.

## Permissions

Access to `/dev/gpiochipN` is controlled by the OS. Prefer a dedicated group or udev rule appropriate to the target distribution over running the entire application as root. Avoid broad permissions such as world-writable GPIO devices.

## Electrical safety

- Use only voltage levels supported by the target board.
- Never connect a GPIO to mains voltage.
- Use appropriate level shifting, isolation, input protection, and common-ground strategy where necessary.
- Disconnect power before changing wiring.
- Validate the signal voltage with suitable measurement equipment before connecting it.

## Verification checklist

- [ ] Correct gpiochip and line offset recorded.
- [ ] Input voltage range confirmed from board documentation.
- [ ] Rising/falling edges verified against a known stimulus.
- [ ] Bounce test run with both clean and noisy switch transitions.
- [ ] CSV log reviewed for monotonic timestamps and increasing sequence numbers.
- [ ] Board-specific live testing noted in the README only after it has actually been performed.
