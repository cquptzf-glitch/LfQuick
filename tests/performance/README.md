# DataView Performance Benchmark

This benchmark is intentionally opt-in and does not participate in normal CTest runs.
It measures model-side regression telemetry without imposing machine-specific pass/fail thresholds.

Build:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_DATAVIEW=ON \
  -DLFQUICK_BUILD_BENCHMARKS=ON
cmake --build build --target lfquick_dataview_benchmark --parallel
```

Run:

```bash
./build/tests/lfquick_dataview_benchmark
```

Default matrix:

- rows: 20k / 50k
- columns: 8 / 20 / 40
- synthetic update batches: 1 / 10 / 100 updates

Reported telemetry:

- model population time
- cross-column filter time
- sort time
- update batch latency

The benchmark deliberately avoids absolute timing assertions because CI hardware and developer machines differ significantly. Track the CSV output over time to detect regressions. QML render/FPS/scroll latency remains a separate GUI benchmark because it requires a stable graphics environment.
