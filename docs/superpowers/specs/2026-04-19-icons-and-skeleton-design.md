# HyprPDF v0.1 Skeleton + quill-icons Gap Fill — Design

**Date:** 2026-04-19
**Status:** Approved (brainstorm phase); **amended 2026-04-21** — renderer switched from Qt6::Pdf to poppler-qt6 (GPL-3.0 already chosen, enables text select + annotation bake without v1.1 renderer swap). See "Renderer amendment" section below.
**Author:** Jim (soyeb-jim285) + Claude
**Scope:** Two parallel sub-projects — (1) add 20 missing icons to `soyeb-jim285/quill-icons` `main`; (2) finish HyprPDF v0.1 skeleton (CMake + Qt6, ConfigManager, ThemeLoader, models, Quill bridge, tabs, recent files, 5 unit tests, GPL-3.0 license, GitHub repo, push).

## Goals

- Ship a runnable HyprPDF binary that opens PDFs, renders, scrolls, zooms, switches between tabs, and remembers recent files — styled via Quill components and TOML themes.
- Land Quill icon coverage for all UI surfaces planned through v0.6 (annotation tools, page ops, viewer chrome).
- Mirror HyprFM's architecture so future milestones (v0.2 nav, v0.3 annotations, …) drop into the same patterns without re-laying foundations.

## Non-Goals

- Annotations of any kind (v0.3).
- Page operations (qpdf integration) (v0.4).
- Office-format conversion (v0.5).
- Search, thumbnails, outline panel, text selection UI (v0.2).
- Packaging (AppImage / Flatpak / PKGBUILD) (v0.6).
- `Showcase.qml` updates for new icons in `quill-icons` (separate later PR).
- Anything not referenced by the v0.1 milestone in `OUTLINE.md`.

---

## Sub-project 1 — quill-icons gap fill

### Icons to add (20)

PDF/annotation set Lucide-derived (https://lucide.dev, MIT):

`IconHighlighter`, `IconPen`, `IconUnderline`, `IconStrikethrough`, `IconType`, `IconEraser`, `IconCircle`, `IconArrowRight`, `IconBookmark`, `IconBookOpen`, `IconZoomIn`, `IconZoomOut`, `IconMaximize`, `IconHand`, `IconPrinter`, `IconRotateCw`, `IconRotateCcw`, `IconFilePlus`, `IconFiles`, `IconLayoutGrid`.

### Pattern (verbatim from `IconSearch.qml`)

```qml
import QtQuick
import QtQuick.Shapes

Shape {
    id: root
    property real size: 24
    property color color: "#ffffff"
    property real strokeWidth: Math.max(1, size / 12)
    width: size; height: size
    clip: false
    preferredRendererType: Shape.CurveRenderer

    ShapePath {
        strokeColor: root.color; strokeWidth: root.strokeWidth
        fillColor: "transparent"
        capStyle: ShapePath.RoundCap; joinStyle: ShapePath.RoundJoin
        scale: Qt.size(root.size / 24, root.size / 24)
        PathSvg { path: "<lucide d attr>" }
    }
    // additional ShapePath blocks for multi-subpath glyphs
}
```

24×24 viewBox assumed. Multi-subpath icons (Highlighter, Pen, BookOpen) use multiple `ShapePath` blocks. Stroke style preserved — Lucide fill-style icons (e.g. Highlighter) become outline. Acceptable visual divergence.

### Steps

1. `git clone git@github.com:soyeb-jim285/quill-icons.git /tmp/quill-icons-work`.
2. For each icon, fetch the Lucide source SVG, extract `d` attribute(s), build the `.qml` file from the template above.
3. Append 20 lines to `qmldir`, alphabetised within the existing list.
4. One commit on `main`: subject `add: PDF/annotation icons (Highlighter, Pen, Underline, Strikethrough, Type, Eraser, Circle, ArrowRight, Bookmark, BookOpen, ZoomIn, ZoomOut, Maximize, Hand, Printer, RotateCw, RotateCcw, FilePlus, Files, LayoutGrid)`. **No `Co-Authored-By` trailer.**
5. `git push origin main`.

### Risks

- Some Lucide icons changed paths between versions; pin to a known commit when fetching `d` attrs.
- Highlighter/Pen visual mismatch (fill→stroke). Document in commit body.
- `Showcase.qml` will not display new icons until separately updated — out of scope here.

### Verification

Smoke-test rendering by importing `Icons` in a scratch `.qml`, instantiating each new icon, eyeballing in `qml /tmp/test.qml`. No automated test (Quill repo has no test suite; visual-only).

---

## Sub-project 2 — HyprPDF v0.1 skeleton

### Decisions

| Axis | Decision |
|---|---|
| License | GPL-3.0 (overrides MIT recommendation; matches Linux-ecosystem norm and forward-compat with Poppler link in v1.1) |
| Theme schema | Hybrid — Catppuccin core (`[colors]`) + annotation palette (`[annotations]`) |
| ThemeLoader | Verbatim port of HyprFM ThemeLoader pattern + extra `[annotations]` parse |
| ConfigManager | Trimmed v0.1 schema — theme/iconTheme, view defaults, radii, animation, transparency. Defer shortcuts/sidebar widths to v0.2 |
| Recent files | JSON at `~/.local/share/hyprpdf/recent.json`, separate from config |
| Tabs | `Quill.Tabs` backed by `DocumentModel : QAbstractListModel` |
| Submodules | `git submodule add` SSH URLs, `QT_QML_IMPORT_PATH = src/qml` |

### File tree (delta vs current `/stuff/Study/projects/hyprpdf`)

```
hyprpdf/
├── LICENSE                              NEW (GPL-3.0)
├── .gitignore                           NEW (build/, .cache/, compile_commands.json, .qmlls.ini)
├── .gitmodules                          NEW
├── CMakeLists.txt                       UPDATE (QT_QML_IMPORT_PATH, install layout, qmldir prefer fix)
├── src/
│   ├── CMakeLists.txt                   UPDATE (new files, IMPORT_PATH, install)
│   ├── main.cpp                         REWRITE (context props, addImportPath fallbacks)
│   ├── third_party/
│   │   └── toml.hpp                     NEW (copied from HyprFM)
│   ├── models/
│   │   ├── documentmodel.{h,cpp}        NEW
│   │   └── recentfilesmodel.{h,cpp}     NEW
│   ├── services/
│   │   ├── documentcontroller.{h,cpp}   NEW
│   │   ├── configmanager.{h,cpp}        NEW
│   │   └── themeloader.{h,cpp}          NEW
│   └── qml/
│       ├── Main.qml                     REWRITE
│       ├── Theme.qml                    NEW (singleton)
│       ├── PdfView.qml                  KEEP (already opens/scrolls/zooms)
│       ├── Toolbar.qml                  NEW
│       ├── TabsBar.qml                  NEW
│       ├── RecentFilesView.qml          NEW
│       ├── Quill/                       SUBMODULE
│       └── icons/                       SUBMODULE
├── tests/
│   ├── CMakeLists.txt                   UPDATE (5 targets)
│   ├── fixtures/sample.pdf              NEW (1-page tiny PDF, generated in configure step)
│   └── tst_{themeloader,configmanager,recentfilesmodel,documentmodel,documentcontroller}.cpp  NEW
├── themes/
│   ├── default.toml                     REWRITE (Catppuccin Mocha + [annotations])
│   └── latte.toml                       NEW (Catppuccin Latte light variant)
└── docs/
    ├── superpowers/specs/
    │   └── 2026-04-19-icons-and-skeleton-design.md   THIS FILE
    └── annotation-format.md             NEW (sidecar JSON spec stub)
```

### Backend services + models

#### `services/themeloader.{h,cpp}`

```cpp
class ThemeLoader : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor base    READ base    NOTIFY themeChanged)
    Q_PROPERTY(QColor mantle  READ mantle  NOTIFY themeChanged)
    Q_PROPERTY(QColor crust   READ crust   NOTIFY themeChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY themeChanged)
    Q_PROPERTY(QColor overlay READ overlay NOTIFY themeChanged)
    Q_PROPERTY(QColor text    READ text    NOTIFY themeChanged)
    Q_PROPERTY(QColor subtext READ subtext NOTIFY themeChanged)
    Q_PROPERTY(QColor muted   READ muted   NOTIFY themeChanged)
    Q_PROPERTY(QColor accent  READ accent  NOTIFY themeChanged)
    Q_PROPERTY(QColor success READ success NOTIFY themeChanged)
    Q_PROPERTY(QColor warning READ warning NOTIFY themeChanged)
    Q_PROPERTY(QColor error   READ error   NOTIFY themeChanged)
public:
    Q_INVOKABLE QColor color(const QString &name) const;
    Q_INVOKABLE QColor annotation(const QString &name) const;   // highlight, underline, strikeout, ink, note
    Q_INVOKABLE void loadTheme(const QString &nameOrPath, const QString &themesDir);
    Q_INVOKABLE QStringList availableThemes(const QString &themesDir) const;
signals:
    void themeChanged();
private:
    QHash<QString,QColor> m_colors;
    QHash<QString,QColor> m_annotations;
};
```

Defaults baked in (Catppuccin Mocha for core, distinct annot palette). Falls back per-key if TOML omits a key.

#### `services/configmanager.{h,cpp}` — trimmed v0.1 schema

```cpp
Q_PROPERTY(QString theme               READ theme               WRITE setTheme               NOTIFY configChanged)
Q_PROPERTY(QString iconTheme           READ iconTheme           WRITE setIconTheme           NOTIFY configChanged)
Q_PROPERTY(double  defaultZoom         READ defaultZoom         WRITE setDefaultZoom         NOTIFY configChanged)
Q_PROPERTY(bool    continuousScroll    READ continuousScroll    WRITE setContinuousScroll    NOTIFY configChanged)
Q_PROPERTY(int     radiusSm            READ radiusSm            WRITE setRadiusSm            NOTIFY configChanged)
Q_PROPERTY(int     radiusMd            READ radiusMd            WRITE setRadiusMd            NOTIFY configChanged)
Q_PROPERTY(int     radiusLg            READ radiusLg            WRITE setRadiusLg            NOTIFY configChanged)
Q_PROPERTY(bool    animationsEnabled   READ animationsEnabled   WRITE setAnimationsEnabled   NOTIFY configChanged)
Q_PROPERTY(int     animDurationFast    READ animDurationFast    WRITE setAnimDurationFast    NOTIFY configChanged)
Q_PROPERTY(int     animDurationNormal  READ animDurationNormal  WRITE setAnimDurationNormal  NOTIFY configChanged)
Q_PROPERTY(int     animDurationSlow    READ animDurationSlow    WRITE setAnimDurationSlow    NOTIFY configChanged)
Q_PROPERTY(bool    transparencyEnabled READ transparencyEnabled WRITE setTransparencyEnabled NOTIFY configChanged)
Q_PROPERTY(double  transparencyLevel   READ transparencyLevel   WRITE setTransparencyLevel   NOTIFY configChanged)

Q_INVOKABLE void save();      // QSaveFile to ~/.config/hyprpdf/config.toml, 300ms debounce on setters
Q_INVOKABLE void reload();
signals: void configChanged();
```

First run: creates `~/.config/hyprpdf/config.toml` populated with defaults. Setters mark dirty + queue debounced save.

#### `models/recentfilesmodel.{h,cpp}` (`QAbstractListModel`)

```cpp
enum Roles {
    PathRole = Qt::UserRole + 1,
    NameRole,
    SizeRole,
    ModifiedRole,
    IconNameRole,    // "IconFileText", "IconImage", etc. — derived from extension
    ExistsRole       // false if path no longer resolves
};

Q_INVOKABLE void addRecent(const QString &path);   // dedupe + push to top
Q_INVOKABLE void remove(int index);
Q_INVOKABLE void clear();
Q_INVOKABLE void open(int index);   // emits requestOpen(path)

signals: void requestOpen(const QString &path);
```

Persist to `~/.local/share/hyprpdf/recent.json` as `[{ "path": "...", "addedAt": "ISO-8601" }, ...]`. Cap = 20. On load, dead paths kept but `ExistsRole` = false (UI greys them out).

#### `models/documentmodel.{h,cpp}` (`QAbstractListModel`, open tabs)

```cpp
enum Roles {
    PathRole = Qt::UserRole + 1,
    TitleRole,
    PageCountRole,
    IsDirtyRole,        // reserved for v0.3 annotations dirty-flag
    DocumentRole        // QPdfDocument* via QObject*, bound to PdfView in QML
};

Q_INVOKABLE int  openDocument(const QString &path);   // returns tab index, dedupes by path
Q_INVOKABLE void closeTab(int index);
Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
Q_PROPERTY(int count        READ rowCount     NOTIFY countChanged)

signals:
    void currentIndexChanged();
    void countChanged();
    void openFailed(QString path, QString reason);
```

Each tab owns a `QPdfDocument` instance (parented to the model row). Closing a tab destroys the doc.

#### `services/documentcontroller.{h,cpp}` — thin façade

```cpp
Q_INVOKABLE void open(const QString &path);
Q_INVOKABLE void closeCurrent();
Q_INVOKABLE void openFromUrl(const QUrl &url);   // strips file:// for FileDialog use

signals:
    void openFailed(QString reason);
```

v0.1 routing: PDF → `DocumentModel::openDocument` directly; non-PDF (`.docx`, `.pptx`, …) → emit `openFailed("Office formats land in v0.5")`. No `ConverterService` yet.

### Threading

- Main thread: all of above. TOML parse < 10 ms.
- v0.3+ adds SHA-256 hashing → `QtConcurrent::run`. Not v0.1.

### QML

#### `Theme.qml` singleton

```qml
pragma Singleton
import QtQuick
QtObject {
    property color base:    theme.base
    property color mantle:  theme.mantle
    property color crust:   theme.crust
    property color surface: theme.surface
    property color overlay: theme.overlay
    property color text:    theme.text
    property color subtext: theme.subtext
    property color muted:   theme.muted
    property color accent:  theme.accent
    property color success: theme.success
    property color warning: theme.warning
    property color error:   theme.error

    property int radiusSmall:  config.radiusSm
    property int radiusMedium: config.radiusMd
    property int radiusLarge:  config.radiusLg

    readonly property int spacing: 8
    readonly property int fontSmall: 11
    readonly property int fontNormal: 13
    readonly property int fontLarge: 16
    readonly property int controlSize: 32
    readonly property int toolbarRowHeight: 40

    property bool animationsEnabled: config.animationsEnabled
    readonly property int animDurationFast: animationsEnabled ? config.animDurationFast   : 0
    readonly property int animDuration:     animationsEnabled ? config.animDurationNormal : 0
    readonly property int animDurationSlow: animationsEnabled ? config.animDurationSlow   : 0

    property bool transparencyEnabled: config.transparencyEnabled
    property real transparencyLevel:   config.transparencyLevel

    function annot(name)            { return theme.annotation(name) }
    function containerColor(c, a)   { return Qt.rgba(c.r, c.g, c.b,
                                          transparencyEnabled ? a * transparencyLevel : 1) }
}
```

#### `Main.qml` — Quill bridge (verbatim port of HyprFM pattern)

```qml
import Quill 1.0 as Q
import HyprPDF 1.0
ApplicationWindow {
    Component.onCompleted: {
        Q.Theme.background       = Qt.binding(() => Theme.base)
        Q.Theme.backgroundAlt    = Qt.binding(() => Theme.mantle)
        Q.Theme.backgroundDeep   = Qt.binding(() => Theme.crust)
        Q.Theme.surface0         = Qt.binding(() => Theme.surface)
        Q.Theme.surface1         = Qt.binding(() => Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.10))
        Q.Theme.surface2         = Qt.binding(() => Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.15))
        Q.Theme.textPrimary      = Qt.binding(() => Theme.text)
        Q.Theme.textSecondary    = Qt.binding(() => Theme.subtext)
        Q.Theme.textTertiary     = Qt.binding(() => Theme.muted)
        Q.Theme.primary          = Qt.binding(() => Theme.accent)
        Q.Theme.success          = Qt.binding(() => Theme.success)
        Q.Theme.warning          = Qt.binding(() => Theme.warning)
        Q.Theme.error            = Qt.binding(() => Theme.error)
        Q.Theme.radiusSm         = Qt.binding(() => Theme.radiusSmall)
        Q.Theme.radius           = Qt.binding(() => Theme.radiusMedium)
        Q.Theme.radiusLg         = Qt.binding(() => Theme.radiusLarge)
        Q.Theme.fontSize         = Qt.binding(() => Theme.fontNormal)
        Q.Theme.fontSizeSmall    = Qt.binding(() => Theme.fontSmall)
        Q.Theme.fontSizeLarge    = Qt.binding(() => Theme.fontLarge)
        Q.Theme.animDuration     = Qt.binding(() => Theme.animDuration)
        Q.Theme.animDurationFast = Qt.binding(() => Theme.animDurationFast)
        Q.Theme.animDurationSlow = Qt.binding(() => Theme.animDurationSlow)
    }
}
```

Layout: `ColumnLayout { Toolbar; TabsBar; StackLayout { currentIndex: documentModel.currentIndex; Repeater { model: documentModel; PdfView { document: model.document } } } }`. When `documentModel.count === 0`, swap the StackLayout for `RecentFilesView`.

#### `Toolbar.qml`

`RowLayout` of `Quill.IconButton` instances using `icons.IconFolder` (open), `icons.IconZoomIn`, `IconZoomOut`, `IconMaximize` (fit-page), `IconPanelLeft` (sidebar toggle stub for v0.2), `IconSearch` (stub), `IconSettings` (stub).

Icons depend on Sub-project 1 push.

#### `TabsBar.qml`

Wraps `Quill.Tabs`. Model = `documentModel`. Per-tab close (`icons.IconX`). `+` button at end → fires `openDialog.open()`.

#### `RecentFilesView.qml`

Visible when `documentModel.count === 0`. `Quill.ScrollableList` of `Quill.Card` instances bound to `recentFiles` model. Click → `recentFiles.open(index)`. Empty state: "No recent documents — Ctrl+O to open."

### CMake

Top-level adds:

- `set(QT_QML_IMPORT_PATH ${CMAKE_SOURCE_DIR}/src/qml CACHE PATH "" FORCE)` so QML tooling resolves Quill + icons.
- Install layout matching HyprFM:
    - `bin/hyprpdf`
    - `share/hyprpdf/themes/*.toml`
    - `share/hyprpdf/src/qml/{Quill,icons}/...`
    - `share/hyprpdf/HyprPDF/...` (generated qmldir + qml tree + qmltypes)
    - `share/applications/hyprpdf.desktop`
- Post-install step: strip `prefer :/HyprPDF/` from installed `qmldir` (HyprFM bug fix — see HyprFM commit history for context).

`src/CMakeLists.txt` `qt_add_qml_module(hyprpdf URI HyprPDF VERSION 1.0 NO_CACHEGEN QML_FILES Main.qml Theme.qml PdfView.qml Toolbar.qml TabsBar.qml RecentFilesView.qml RESOURCE_PREFIX /)`. `Quill/` and `icons/` are NOT listed — they resolve via `QT_QML_IMPORT_PATH`.

`main.cpp` registers context properties (`config`, `theme`, `recentFiles`, `documentModel`, `documentController`) and adds engine import paths in this order:

1. `${HYPRPDF_DATA_DIR}/src/qml` (installed Quill/icons location)
2. `${HYPRPDF_DATA_DIR}` (installed HyprPDF module dir)
3. `${HYPRPDF_SOURCE_DIR}/src/qml` (in-tree dev fallback)
4. `${HYPRPDF_SOURCE_DIR}` (in-tree dev fallback)

### Tests

5 files. Pattern: per-class executable, `Qt6::Test`, `target_compile_definitions` for fixture/themes paths.

| File | Asserts |
|---|---|
| `tst_themeloader` | Defaults present without TOML; TOML override changes returned colors; `themeChanged` fires once per `loadTheme`; `annotation("highlight")` returns expected colour. |
| `tst_configmanager` | First-run creates file with defaults; setter→save roundtrip via `QTemporaryDir`; reload picks up external edit; broken TOML → `.broken-<ts>` rename + fresh defaults. |
| `tst_recentfilesmodel` | `addRecent` dedupes existing path and moves to front; cap at 20; `clear` empties; JSON persist roundtrip via `QTemporaryDir`; missing file → `ExistsRole == false`. |
| `tst_documentmodel` | `openDocument` adds row; dedupe by path returns existing index; `closeTab` removes row + destroys `QPdfDocument`; `currentIndex` clamps to range; `openFailed` emits on bogus path. |
| `tst_documentcontroller` | `open(pdf)` routes to model; `open(docx)` emits `openFailed("Office formats land in v0.5")`; `openFromUrl` strips `file://` prefix. |

PDF fixture: `tests/fixtures/sample.pdf` — single-page tiny PDF generated by `qpdf --empty --pages` at CMake configure step (skipped if `qpdf` absent; tests requiring fixture are then skipped via `QSKIP`).

### Error handling

- Theme TOML parse fail → log warning via `qWarning`, fall back to baked defaults, `themeChanged` still emits.
- Config TOML parse fail → rename to `config.toml.broken-<unix-ts>`, write fresh defaults, log path.
- Recent JSON corrupt → log + start with empty list. **Do NOT delete** — user may want to recover.
- PDF open fail (missing/corrupt) → `openFailed(path, reason)` signal → QML `console.warn` (toast widget deferred to v0.2).
- Submodule missing at runtime → engine load fails with clear `qWarning` listing import paths and the missing module name.

### Git flow

1. `cd /stuff/Study/projects/hyprpdf`
2. `git init -b main` (initialize local repo).
3. Write `.gitignore`: `build/`, `.cache/`, `compile_commands.json`, `.qmlls.ini`, `*.swp`, `*.user`, `CMakeUserPresets.json`.
4. `gh repo create soyeb-jim285/hyprpdf --public --source=. --remote=origin --license=gpl-3.0 --description "Qt6/QML PDF + office viewer with annotations"` (creates remote, writes `LICENSE`, adds remote `origin`; does NOT push).
5. Verify `LICENSE` = GPL-3.0 text (gh-written) and `git remote -v` shows `origin`.
6. Stage existing pre-skeleton files (current `CLAUDE.md`, `OUTLINE.md`, `README.md`, existing `src/`, `themes/`, `dist/`, the spec at `docs/superpowers/specs/2026-04-19-icons-and-skeleton-design.md`) for the initial commit chain.
7. `git submodule add git@github.com:soyeb-jim285/quill.git src/qml/Quill`
8. `git submodule add git@github.com:soyeb-jim285/quill-icons.git src/qml/icons` — **after Sub-project 1 push** so the latest 20 icons are present.
9. Sequential commits on `main` (logical chunks, **no `Co-Authored-By` trailer, no Claude attribution footer, no `🤖 Generated with` footer**):
    1. `chore: import skeleton, OUTLINE, CLAUDE.md, README, design spec` (existing files at session start + this spec)
    2. `chore: add LICENSE, .gitignore`
    3. `chore: add Quill + quill-icons submodules`
    4. `chore: vendor toml.hpp from HyprFM`
    5. `feat: ThemeLoader with Catppuccin defaults and annotation palette`
    6. `feat: ConfigManager (trimmed v0.1 schema)`
    7. `feat: RecentFilesModel with JSON persistence`
    8. `feat: DocumentModel + DocumentController`
    9. `feat: Theme.qml singleton + Quill bridge in Main.qml`
    10. `feat: Toolbar, TabsBar, RecentFilesView QML`
    11. `test: 5 backend unit tests + sample.pdf fixture`
    12. `build: install layout + qmldir prefer fix`
10. `git push -u origin main`.

### Verification (post-implementation)

- `cmake -B build -G Ninja && cmake --build build` → no warnings, no errors.
- `ctest --test-dir build` → all 5 tests pass.
- `./build/src/hyprpdf tests/fixtures/sample.pdf` → window opens, page renders, scroll works, zoom works, theme bridge applies (verify by toggling `transparencyEnabled` in `~/.config/hyprpdf/config.toml` and reloading).
- `./build/src/hyprpdf` (no arg) → RecentFilesView visible.
- Open second file via Ctrl+O → second tab appears, switching works.
- Quit + relaunch → recent files list persists.

### Renderer amendment (2026-04-21)

**Changed:** Qt6::Pdf dropped in favor of `poppler-qt6` (Arch pkg `poppler-qt6`, linked via `pkg_check_modules(POPPLER_QT6 REQUIRED IMPORTED_TARGET poppler-qt6)`).

**Rationale:**
- GPL-3.0 already chosen → Poppler's GPL licensing compatible.
- Poppler gives text selection, search, annotation read/write, digital sig verify, forms — features Qt6::Pdf lacks or ships minimally.
- Avoids the v1.1 renderer swap for annotation-bake (Poppler is the bake path the original OUTLINE named).

**Cost:** no QML `PdfMultiPageView` equivalent. Added components:
- `PdfDoc : QObject` — wraps `std::unique_ptr<Poppler::Document>`, stable int id, exposes `pageCount`, `pageSize(i)`, `title`, `text(i)` as Q_INVOKABLE.
- `PdfPageImageProvider : QQuickImageProvider` — URL scheme `image://pdf/<docId>/<pageIndex>/<targetW>`, looks up `PdfDoc` via registry, renders page via `Poppler::Document::page(i)->renderToImage(dpi, dpi)`.
- `PdfView.qml` — `Flickable { ListView { model: pdfDoc.pageCount; delegate: Image { source: "image://pdf/<id>/<index>/<targetW>"; asynchronous: true; cache: false } } }`. Zoom re-renders at new `targetW` (vector-sharp).

**DocumentModel impact:** stores `PdfDoc*` per tab (not `QPdfDocument`). `DocumentRole` returns `QObject*` pointing at `PdfDoc`.

**Tests impact:** load via `Poppler::Document::load(path)` returning `std::unique_ptr<Poppler::Document>`. Sample.pdf fixture unchanged.

### Out-of-scope reminders

- No annotations.
- No qpdf.
- No soffice.
- No search/thumbnails/outline.
- No packaging.

These wait for v0.2–v1.0 milestones, each of which gets its own brainstorm → design → plan cycle.
