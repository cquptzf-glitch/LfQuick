# LfPropertyGrid contract (Qt 6.8.3)

Module: LfQuick, implemented in src/ui/qml/LfPropertyGrid.qml.

## Data model

- Bounded JavaScript array or QML ListModel with count/get(index).
- Each entry has key, label, value and type. Supported types: text, bool, int.
- Optional readOnly=true; integer entries accept from and to bounds.
- Unsupported types fall back to a text editor.
- Application-level readOnly disables every editor; entry-level readOnly protects individual values.

## Writes and validation

- The component never silently writes into the source model.
- On an allowed edit, valueEdited(index,key,value) emits; the application validates, persists and updates its model.
- editValue(index,value) returns false for missing/read-only entries and true when a signal was emitted.
- A UI readOnly property is not an authorization boundary. IVD validation, permissions and audit trails belong in services.

## Usage

```qml
import LfQuick

LfPropertyGrid {
    model: [
        {key: "threshold", label: "Threshold", type: "int", value: backend.threshold, from: 0, to: 100},
        {key: "name", label: "Device", type: "text", value: backend.name},
        {key: "serial", label: "Serial", type: "text", value: backend.serial, readOnly: true}
    ]
    onValueEdited: function(index, key, value) {
        if (key === "threshold") backend.threshold = value
        else if (key === "name") backend.name = value
    }
}
```

The API is not a generic reflective QObject editor or a replacement for C++ model adapters. For large data sets and heterogeneous editors, integrate purpose-built DataView delegates instead.

## Verification

tst_ui_controls.cpp includes a smoke instantiation and a behavior test for emitted edits and read-only rejection. Actual Qt 6.8.3 Build/CTest remains pending.

## Programmatic edit enforcement

The public editValue(index,value) method enforces widget-level type/range/readOnly constraints, even if called outside the visual editor. For integer entries it accepts finite integer numbers only; for bool entries it accepts booleans; for text entries it accepts strings. Unchanged values are rejected to avoid duplicate edit events. The method returns false for rejected edits and true only after emitting valueEdited. This is not application authorization or validation.
