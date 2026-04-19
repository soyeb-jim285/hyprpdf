# CLAUDE.md

This file provides guidance to Claude Code when working with HyprPDF.

## Project

HyprPDF is a Qt6/QML PDF and office-document viewer with annotation and page-editing tools. Sibling to HyprFM — reuses its architecture, theme system, and Quill component library.

See `OUTLINE.md` for the feature matrix, roadmap, architecture, and open questions. Treat it as the source of truth for scope decisions.

## Build & Test

```bash
cmake -B build -G Ninja && cmake --build build
./build/src/hyprpdf
ctest --test-dir build
./build/tests/tst_annotationstore
```

Tests use Qt6::Test (QCOMPARE, QSignalSpy). One file per backend class in `tests/tst_*.cpp`.

## Architecture (summary — see OUTLINE.md for detail)

Three layers, mirror of HyprFM:

- **QML Frontend** (`src/qml/`) — rendering only. `Main.qml` wires tabs, shortcuts, panels. `PdfView.qml` wraps `PdfMultiPageView`. `AnnotLayer.qml` overlays highlights/notes on each page. `Theme.qml` is a singleton pulling colors from the active TOML theme.
- **C++ Backend** (`src/models/`, `src/services/`) — exposed to QML as context properties in `main.cpp`. Models are `QAbstractListModel` subclasses. Services wrap subprocesses (qpdf, soffice) via `QProcess`.
- **System Layer** — `qpdf` for page ops, `soffice --headless` for PPTX/DOCX conversion, `libpoppler-qt6` for annotation bake-to-PDF (v1.1+).

### Key conventions

- Annotations key by **SHA-256 of file content**, not path. Renames/moves preserve notes. Sidecar at `~/.local/share/hyprpdf/annotations/<hash>.json`.
- Pages key by **stable UUID** assigned on first load, stored next to annotations. Page ops update `{pageIndex → uuid}` map so annotations survive delete/reorder.
- Converted-from-office PDFs cache at `~/.cache/hyprpdf/converted/<hash>.pdf`. Annotations attach to **source file hash**, not cached PDF.
- Config at `~/.config/hyprpdf/config.toml`. Themes in `themes/`. Reuse HyprFM's `ThemeLoader` pattern.
- All long-running work (qpdf, soffice, hashing) off the GUI thread — `QProcess` or `QtConcurrent`.
- Page ops write to temp file, then atomic rename. Keep one snapshot for in-session undo.

## Commit Rules

Never add Co-Authored-By lines to commits.

## Shared Submodules (planned)

Same as HyprFM:

- `src/qml/icons/` → [quill-icons](https://github.com/soyeb-jim285/quill-icons)
- `src/qml/Quill/` → [quill](https://github.com/soyeb-jim285/quill)

Quill's `Theme.qml` singleton must be bridged from HyprPDF's theme in `Main.qml` `Component.onCompleted`. Directory must be capitalized `Quill/`.

## Dependencies

Qt6 modules: Core, Gui, Qml, Quick, QuickControls2, Pdf, Svg, Widgets, DBus.
Runtime CLI: `qpdf`, `soffice` (LibreOffice, optional but needed for non-PDF formats), `wl-copy`.
Optional link: `libpoppler-qt6` for annotation bake (v1.1+).
