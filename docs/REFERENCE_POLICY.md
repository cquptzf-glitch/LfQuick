# LfQuick External Reference & Clean-room Policy

## 1. Purpose

LfQuick may study existing open-source and publicly observable UI frameworks, but external references must never compromise LfQuick's license, ownership, modular architecture, or ability to be used in proprietary medical-device products.

This policy applies to source code, QML, APIs, binaries, demos, screenshots, design systems, documentation, and generated metadata used during implementation.

## 2. Reference classes

### Class A — Public design concepts and official Qt/Microsoft documentation

Allowed for architecture and behavior reference. Examples:

- Qt documentation and examples;
- Microsoft Fluent Design guidance;
- standard Qt Quick Controls behavior and APIs.

Prefer these sources for generic concepts.

### Class B — zhuzichu520/FluentUI public MIT source

The original public FluentUI repository is MIT licensed. Its public source may be inspected and may inform implementation decisions.

Rules:

1. Prefer extracting ideas, behavior, edge cases and test scenarios rather than copying files wholesale.
2. If code is copied or substantially derived, retain the applicable MIT copyright/license notice and record the source path and commit in `THIRD_PARTY_NOTICES.md`.
3. Do not preserve legacy FluentUI coupling when LfQuick has a cleaner abstraction.
4. Public FluentUI is a reference implementation, not LfQuick's architecture contract.

Useful reference areas include accessibility/focus handling, control interaction states, frameless-window behavior, router UX and gallery scenarios. Known architectural issues such as hard-coded colors, monolithic navigation/router/window components and global theme coupling should not be copied.

## 3. FluentUI2 / FluentUI Pro

The public `zhuzichu520/FluentUI2` repository exposes a compiled plugin/demo and public API surface; its README states that commercial/source access is purchased separately. The public repository is licensed GPL-3.0.

LfQuick therefore uses a strict clean-room rule:

**Allowed:**

- observe public demo behavior;
- inspect public QML files, `qmldir`, generated `qmltypes`, documented properties/signals/types and repository structure;
- infer architectural requirements from public API behavior;
- implement independently against Qt public APIs.

**Forbidden unless a separate compatible source license has explicitly been obtained for LfQuick:**

- decompiling/disassembling/reconstructing the Pro binary implementation;
- copying private or paid Pro source into LfQuick;
- reproducing non-public implementation obtained through unauthorized access;
- copying GPL implementation into LfQuick while representing LfQuick as license-independent/proprietary-compatible.

Public API compatibility is not implementation copying. Any Pro-inspired implementation must be written independently from the public contract and Qt documentation.

## 4. PR requirements

A PR that materially uses an external framework as reference must include:

```text
Reference-Type: A | B | C
Reference-Project: ...
Reference-Path/API: ...
Reference-Commit/Version: ...
Copied-Code: yes/no
License-Impact: none / MIT notice added / review required
```

Class C means clean-room reference to a public API/behavior with no source implementation copied.

## 5. Architectural priority

When reference implementations conflict with LfQuick architecture, the following order wins:

```text
LfQuick architecture boundary
> Qt supported mechanism
> product stability/performance
> compatibility behavior
> source-level similarity to reference project
```

## 6. Current guidance

For LfQuick 0.x:

- Use public FluentUI MIT source mainly to learn control behavior and known edge cases.
- Use FluentUI2/Pro only as a public API/behavior benchmark.
- Rebuild Theme, Router, Windowing, Navigation and DataView around LfQuick's own contracts instead of porting legacy implementation.
