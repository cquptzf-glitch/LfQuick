# LfQuick Medical Demo

Reference application for validating LfQuick in biochemistry/immunoassay analyzer UI scenarios.

> **Important:** this is not medical diagnostic software. All data and workflows are synthetic and must not be used for diagnosis, treatment, or operation of a real instrument.

Current page groups:

- Dashboard
- Samples
- Results
- Results Review
- Instrument
- Alarms
- Developer

## Results Review validation slice

`ResultsReviewPage.qml` is intentionally a framework-integration page rather than a visual mock. It exercises:

- DataGrid stable columns and persistence;
- typed cell/editor delegates;
- validated edit transactions;
- extended/batch row selection;
- right-click context actions;
- proxy-to-source row mapping after filtering;
- QML permission evaluation backed by a C++ `IPermissionProvider`;
- NavigationGuard confirm/hard-block workflows;
- Busy Overlay during synthetic save operations.

The demo-only permission id `demo.results.review` remains in the example/product layer and is not part of LfQuick Core/UI contracts.

## Development rule

A UI capability should not be treated as complete merely because a QML component exists. For this demo, major capabilities should have a working implementation path, error/edge handling, relevant tests, and an actual integration scenario before being marked complete.

See `../../docs/LfQuick_Medical_Demo_详细设计方案.md` and `../../docs/IMPLEMENTATION_STATUS.md` for the detailed design and current status.
