# HyprPDF — Project Outline

A Qt6/QML PDF + office document viewer with annotation and page editing. Sibling to HyprFM.

## Goals

- Fast, keyboard-first PDF reader for Linux/Wayland.
- Annotate (highlight, underline, strikeout, sticky notes, freehand ink).
- Edit page structure (delete, rotate, reorder, merge, split, extract).
- View office formats (PPTX/DOCX/ODP/ODT) by converting to PDF transparently.
- Match HyprFM look/feel via shared Quill component library and TOML themes.

## Non-Goals

- Full office editor. PPTX/DOCX are view-only; edits go to sidecar or new PDF.
- Form filling with digital signatures (v1). May add later.
- OCR / scanned-page text recognition (v1). Optional later via Tesseract.
- Cloud sync. Local files only.
- Windows/macOS builds. Linux/Wayland first, X11 best-effort.

## Feature Matrix

### Core viewing (MVP)

| Feature | Status | Engine |
|---------|--------|--------|
| Open PDF | v1 | Qt PDF |
| Multi-page continuous scroll | v1 | `PdfMultiPageView` |
| Single-page / two-page / facing modes | v1 | QML layout |
| Zoom (fit-width, fit-page, custom, pinch) | v1 | Qt PDF |
| Page thumbnails sidebar | v1 | Qt PDF render |
| Outline / TOC / bookmarks panel | v1 | `PdfBookmarkModel` |
| Text search with match highlight + next/prev | v1 | `PdfSearchModel` |
| Text selection + copy to clipboard | v1 | Qt PDF ≥6.6 |
| Tabs (multi-doc) | v1 | reuse Quill Tabs |
| Keyboard nav (j/k/g/G/PgUp/PgDn/Ctrl±) | v1 | QML Shortcut |
| Recent files | v1 | JSON |
| Dark mode / invert colors | v1 | QML shader |
| Theme loader (TOML) | v1 | reuse HyprFM `ThemeLoader` |

### Annotations (MVP)

Stored in sidecar `~/.local/share/hyprpdf/annotations/<sha256>.json` keyed by file content hash, so renames/moves do not lose notes.

| Annotation | v1 | Notes |
|------------|----|-------|
| Highlight (text) | yes | rect coords per page |
| Underline | yes | same |
| Strikeout | yes | same |
| Sticky note | yes | anchor point + text body |
| Freehand ink | yes | polyline list |
| Rectangle / ellipse | v1.1 | shape prim |
| Arrow / line | v1.1 | shape prim |
| Text box (typewriter) | v1.1 | |
| Stamp (approved/reviewed) | v2 | |
| Color picker per annot | v1 | 6-color palette + custom |
| Per-annot author + timestamp | v1 | |
| Comment thread on annot | v1.1 | reply list |
| Export annotations → JSON / Markdown | v1 | |
| Export annotated PDF (bake into PDF) | v1.1 | Poppler or QPDF |

### Page operations (v1.1)

Powered by QPDF (subprocess or linked lib).

| Op | Command |
|----|---------|
| Delete page | `qpdf --pages in.pdf 1-4,6-z -- out.pdf` |
| Rotate page | `qpdf --rotate=+90:3 in.pdf out.pdf` |
| Reorder pages | drag thumbnail → QPDF page selection |
| Extract pages → new PDF | `qpdf --pages in.pdf 3-5 -- out.pdf` |
| Merge multiple PDFs | `qpdf --empty --pages a.pdf b.pdf -- out.pdf` |
| Split by range | qpdf loop |
| Insert blank page | qpdf + blank template |
| Crop / margin trim | MediaBox edit via qpdf |

Safety: always write temp file then atomic rename. Undo stack in-session only (keeps previous file snapshots up to N MB).

### Document conversion (v1.1)

| Input | Converter | Reversible? |
|-------|-----------|-------------|
| PPTX, PPT, ODP | `soffice --headless --convert-to pdf` | no (view only) |
| DOCX, DOC, ODT, RTF | same | no |
| XLSX, ODS, CSV | same | no |
| EPUB | optional — `pandoc` or Calibre | no |
| Markdown | `pandoc -o out.pdf` | no |
| HTML | `wkhtmltopdf` or `chromium --headless --print-to-pdf` | no |

Cache: `~/.cache/hyprpdf/converted/<sha256>.pdf`, key = content hash + mtime. Annotations attach to source file hash, not cached PDF.

Long-running `soffice --headless` daemon with UNO socket for fast repeat conversions (v1.2).

### Advanced (v2+)

- Form filling (AcroForm, XFA basic).
- Digital signature view + verify (v2).
- Digital signature apply (v2.1).
- OCR layer via Tesseract for scanned PDFs.
- Redaction tool (black-box + remove underlying text).
- Optimize / shrink (ghostscript subprocess).
- Image extraction.
- Compare two PDFs (diff-pdf).
- Presentation mode (full-screen, laser pointer, timer).
- Read-aloud (TTS via espeak-ng or piper).

## Tech Stack

- **Language:** C++20, QML.
- **GUI:** Qt 6.6+ (Core, Gui, Qml, Quick, QuickControls2, Pdf, Svg, Widgets, DBus).
- **PDF render + search:** `QtPdf` module.
- **PDF structural edit:** `libqpdf` (link) or `qpdf` CLI (subprocess).
- **Office convert:** `libreoffice` subprocess (`soffice --headless`).
- **Annotation bake-to-PDF:** `libpoppler-qt6` (v1.1).
- **TOML config:** header-only `toml.hpp` (reuse HyprFM copy).
- **Hashing (annot keying):** QCryptographicHash SHA-256.
- **Build:** CMake ≥3.20.
- **Test:** Qt6::Test.

## Architecture

Three-layer mirror of HyprFM:

```
QML Frontend           C++ Backend                System
─────────────────      ──────────────────────     ───────────────
Main.qml               DocumentController         libqpdf
PdfView.qml            DocumentModel (tabs)       soffice (QProcess)
ThumbnailList.qml      AnnotationStore            poppler-qt6
OutlinePanel.qml       SearchController           QtPdf engine
AnnotToolbar.qml       PageOpsService             Wayland clipboard
AnnotLayer.qml         ConverterService
Toolbar.qml            ThemeLoader
```

### Data flow

1. User opens file → `DocumentController::open(path)`.
2. If non-PDF, `ConverterService::convert(path)` returns cached PDF path.
3. `DocumentModel` adds tab, loads `QPdfDocument`.
4. `AnnotationStore` looks up sidecar by content hash, exposes list model per page.
5. `PdfView.qml` binds to current `QPdfDocument`; `AnnotLayer.qml` overlays per page.
6. User draws highlight → QML emits rect + text → `AnnotationStore::add()` → model insert → re-render.
7. Page op → `PageOpsService` runs qpdf on temp → swaps file → reloads doc → remaps annotations by stable page UUID.

### Stable page identity

Problem: delete/reorder breaks `pageIndex`-keyed annotations.
Solution: on first open assign UUID per page, store map `{pageIndex → uuid}`. Annotations key by uuid. On page op, recompute index map from qpdf output. Deleted pages → orphaned annots moved to "detached" bin, user can recover.

### Threading

- GUI thread: QML + Qt PDF render (fast).
- Worker thread: SHA-256 hashing, JSON I/O.
- Subprocess: qpdf, soffice, poppler (via QProcess, non-blocking).

## File Layout

```
hyprpdf/
├── CMakeLists.txt
├── README.md
├── CLAUDE.md
├── OUTLINE.md                 ← this file
├── LICENSE                    (GPL-3 or MIT, decide)
├── .gitignore
├── src/
│   ├── main.cpp
│   ├── CMakeLists.txt
│   ├── models/
│   │   ├── documentmodel.{h,cpp}      (tab list)
│   │   ├── annotationmodel.{h,cpp}
│   │   └── recentfilesmodel.{h,cpp}
│   ├── services/
│   │   ├── documentcontroller.{h,cpp}
│   │   ├── annotationstore.{h,cpp}
│   │   ├── converterservice.{h,cpp}   (soffice wrap)
│   │   ├── pageopsservice.{h,cpp}     (qpdf wrap)
│   │   ├── searchcontroller.{h,cpp}
│   │   ├── configmanager.{h,cpp}
│   │   └── themeloader.{h,cpp}
│   └── qml/
│       ├── Main.qml
│       ├── PdfView.qml
│       ├── AnnotLayer.qml
│       ├── AnnotToolbar.qml
│       ├── ThumbnailList.qml
│       ├── OutlinePanel.qml
│       ├── SearchBar.qml
│       ├── Toolbar.qml
│       ├── Theme.qml                  (singleton)
│       ├── Quill/                     (submodule)
│       └── icons/                     (submodule: quill-icons)
├── tests/
│   ├── CMakeLists.txt
│   ├── tst_annotationstore.cpp
│   ├── tst_pageopsservice.cpp
│   ├── tst_converterservice.cpp
│   └── tst_documentmodel.cpp
├── themes/
│   ├── default.toml
│   └── dark.toml
├── docs/
│   ├── keybindings.md
│   └── annotation-format.md
└── dist/
    ├── hyprpdf.desktop
    └── hyprpdf.svg
```

## Roadmap

### v0.1 — skeleton (week 1)
- CMake build, Qt6 deps.
- Open PDF, render, scroll, zoom.
- Tabs + recent files.
- Theme load from TOML.

### v0.2 — navigation (week 2)
- Thumbnails sidebar.
- Outline/bookmarks panel.
- Text search.
- Text selection + copy.
- Keyboard shortcuts.

### v0.3 — annotations (week 3)
- Highlight, underline, strikeout, sticky note, freehand.
- Sidecar JSON store, SHA-256 keying.
- Color palette, undo per-annotation.
- Export annotations → Markdown.

### v0.4 — page ops (week 4)
- QPDF integration.
- Delete, rotate, reorder via drag in thumbnails.
- Merge, split, extract.
- Stable page UUIDs + annotation remap.

### v0.5 — office conversion (week 5)
- ConverterService + soffice subprocess.
- PPTX, DOCX, ODT support.
- Cache manager + invalidation on mtime.
- Progress indicator.

### v0.6 — polish (week 6)
- Presentation mode.
- Print dialog.
- Config UI.
- Packaging: AUR + AppImage (reuse HyprFM GH Actions).

### v1.0 — annotation bake + release
- Poppler-Qt6 integration to bake annotations back into PDF spec.
- Export annotated PDF.
- Documentation, screenshots, README.

### Future (v1.1+)

- Form fill, sig verify, OCR, redaction, compare, TTS.

## Risks & Open Questions

1. **Qt PDF text selection maturity** — need 6.6+. Test early; fall back to Poppler render if too limited.
2. **soffice first-start latency** — ~2 s. Acceptable? If not, daemon mode in v1.2.
3. **qpdf CLI vs libqpdf link** — start with CLI (simpler packaging), link later if perf matters.
4. **License** — QtPdf is LGPL/commercial; Poppler is GPL; qpdf is Apache. If link Poppler, project forced GPL. Decide MIT-with-poppler-optional vs GPL-all.
5. **Annotation spec interop** — sidecar JSON is simple but not portable to Acrobat/Foxit. Baking into PDF (Poppler) fixes but is write-path complexity.
6. **Stable page UUIDs after external edit** — if user edits PDF outside HyprPDF, UUID map breaks. Need to recompute on load + user confirm for orphan annots.

## Dependencies (packages)

Arch:
```
qt6-base qt6-declarative qt6-quickcontrols2 qt6-pdf qt6-svg qt6-tools
qpdf poppler-qt6 libreoffice-fresh
cmake ninja
```

Runtime optional:
- `tesseract` (OCR future)
- `ghostscript` (shrink/optimize future)
- `pandoc` (EPUB/Markdown convert future)

## Licensing

TBD. Recommendation: **GPL-3.0** to align with Poppler dependency and Linux ecosystem norms. Sidecar annotation format stays open JSON so other tools can interop.
