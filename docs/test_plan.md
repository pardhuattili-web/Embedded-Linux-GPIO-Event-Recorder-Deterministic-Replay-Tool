# Test plan

## Automated host tests

Run `make test`. The suite checks:

- CSV serialization and round-trip parsing
- rejection of edge/value mismatches
- debounce rejection of short bounce transitions and acceptance after a stable interval
- replay ordering and nondecreasing event timestamps
- end-to-end demo recording followed by replay

## Manual smoke tests

1. Run `./build/gpio-recorder --help` and check CLI help and exit status.
2. Record to a writable file and confirm the CSV header is present.
3. Replay `examples/sample_events.csv` with `--no-sleep`.
4. Try malformed rows (negative timestamp, unknown edge, wrong field count, inconsistent edge/value) and confirm replay exits with an error.
5. Pass a zero or negative speed and confirm it is rejected.
6. For live hardware integration, verify each edge against a known input stimulus and repeat with a bounce-heavy switch.

## Expected sample

The bundled sample has four events in sequence order and nondecreasing elapsed times. Replay output should print the sequence, timestamp, GPIO line offset, edge, and value for all four entries.

## Test boundary

Host tests validate software logic. They do not prove electrical integrity, libgpiod compatibility with a particular distribution, or timing accuracy on a target board. Those require separate hardware-in-the-loop validation.
