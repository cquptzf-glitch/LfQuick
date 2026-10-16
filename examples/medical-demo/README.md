# LfQuick Medical Demo

Reference application for validating LfQuick in biochemistry/immunoassay analyzer UI scenarios.

> **Important:** this is not medical diagnostic software. All data and workflows are synthetic and must not be used for diagnosis, treatment, or operation of a real instrument.

## Current pages

- Dashboard
- Samples
- Sample Register
- Sample Rack
- Results
- Results Review
- Instrument
- Alarms
- Developer

## Application Shell validation slice

The Medical Demo now uses the framework-level model-driven shell instead of a demo-local hand-written navigation rail:

- `NavigationItemModel` owns route metadata, label, section/group, badge, parameters and selected-route state;
- `NavigationView` drives `QuickRuntime.push()` / `back()` and synchronizes selection from `QuickRuntime.currentRoute`;
- `ApplicationShell` provides the navigation surface, page host and shared `OverlayHost`;
- compact/expanded navigation is exercised from the demo header;
- `Alarms` demonstrates badge metadata;
- `Results Review` demonstrates navigation permission integration through the existing `PermissionEvaluator`;
- navigation still flows through the existing `NavigationGuard` and overlay confirmation path rather than a shell-specific state machine.

The shell API contains no medical route ids; the demo supplies all route definitions from the product/example layer.

## Sample Register validation slice

`SampleRegisterPage.qml` exercises a complete form workflow rather than a visual mock:

- `KeyboardWedgeScanner` / `ScannerSession` / `InputRouter`;
- scanner focus isolation so keyboard-wedge input is captured only while the barcode field is active;
- active-page isolation so cached inactive pages do not consume global scanner signals;
- `ValidationEvaluator` backed by the Core `validateValue()` implementation;
- duplicate barcode/sample-id lookup through generic `DataTableModel.findFirst()`;
- `FocusNavigator`;
- dirty `NavigationGuard` confirm/cancel flow;
- busy `NavigationGuard` hard block during submission;
- `OverlayCoordinator` busy state;
- synthetic submit into `DataTableModel`, followed by reset/focus restore.

## Sample Rack validation slice

`SampleRackPage.qml` exercises table/scanner interaction:

- scanner-driven rack switching;
- exact rack filtering through `DataFilterProxyModel.exactMatch`, so `R1` does not match `R10`;
- stable DataGrid columns and persistence;
- typed `DataGridProgressDelegate`;
- extended/batch selection;
- safe proxy-to-source row snapshots before batch mutation;
- context-menu actions;
- keyboard navigation/focus entry;
- temporary scanner-session minimum-length override with restoration on page deactivation.

## Results Review validation slice

`ResultsReviewPage.qml` exercises:

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

## Windows CI/CD package

The repository contains CI/release workflows that build both the Medical Demo and Hybrid Demo on Windows x64 with Qt 6.8.3 + MSVC2022, run tests, use Qt's CMake deployment API, and package deployable demo directories.

Expected executables after a successful package build:

```text
bin/LfQuickMedicalDemo.exe
bin/LfQuickHybridDemo.exe
```

Main/PR/manual CI uploads a snapshot artifact. A `v*` tag runs the release workflow and is designed to publish a zip to GitHub Releases after Build/Test/Deploy succeeds.

See `../../docs/CI_CD.md` for the exact workflow and local reproduction commands.

> Current GitHub Actions runners are still failing before any workflow steps start (`steps = null` / no jobs), so the workflow/deployment source is implemented but a real Windows artifact/release has not yet been verified.

## Development rule

A UI capability is not considered complete merely because a QML component exists. Major capabilities need a working implementation path, edge/error handling, relevant tests, and an actual integration scenario before they are marked complete.

See `../../docs/LfQuick_Medical_Demo_详细设计方案.md` and `../../docs/IMPLEMENTATION_STATUS.md` for the detailed design and current status.
