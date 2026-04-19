# HyprPDF v0.1 Skeleton + quill-icons Gap Fill — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship runnable HyprPDF v0.1 (PDF viewer with tabs, recent files, themed via Quill) and add 20 missing icons to `soyeb-jim285/quill-icons`, both pushed to `soyeb-jim285` GitHub.

**Architecture:** Two phases. Phase 1 ships icons to `quill-icons` `main` (independent repo, no HyprPDF dep). Phase 2 builds HyprPDF v0.1 mirroring HyprFM (`/stuff/Study/projects/hyprfm`): `ConfigManager` + `ThemeLoader` (TOML, Catppuccin defaults + annot palette), `RecentFilesModel` (JSON persisted), `DocumentModel` (open PDFs as tabs), `DocumentController` (open dispatcher), `Theme.qml` singleton bridged into `Quill.Theme` in `Main.qml`. Phase 2 adds `Quill` + `quill-icons` as git submodules **after** Phase 1 push so the new icons are available.

**Tech Stack:** C++20, Qt 6.6+ (Core, Gui, Qml, Quick, QuickControls2, Pdf, Svg, Widgets, DBus, Test), CMake >= 3.20, header-only `toml.hpp` (vendored from HyprFM), GPL-3.0 license, Quill QML component library + quill-icons (both as git submodules).

**Repo conventions (CRITICAL):**
- **Never** add `Co-Authored-By: Claude` (or any Claude variant) trailer to any commit.
- **Never** add the `Generated with Claude Code` footer.
- Commits use HEREDOC for multi-line messages.
- Sequential commits, logical chunks (no giant "wip" commits).

---

## Reference paths

- HyprPDF (this work): `/stuff/Study/projects/hyprpdf`
- HyprFM (pattern source): `/stuff/Study/projects/hyprfm`
- Spec: `/stuff/Study/projects/hyprpdf/docs/superpowers/specs/2026-04-19-icons-and-skeleton-design.md`
- Working clone for quill-icons: `/tmp/quill-icons-work`

---

# Phase 1 — quill-icons gap fill (20 icons)

## Task 1.1: Clone quill-icons working tree

**Files:**
- Create: `/tmp/quill-icons-work/` (clone target)

- [ ] **Step 1: Clone via SSH**

```bash
rm -rf /tmp/quill-icons-work
git clone git@github.com:soyeb-jim285/quill-icons.git /tmp/quill-icons-work
cd /tmp/quill-icons-work
git status
```

Expected: clean working tree on `main`, remote `origin` set to SSH URL.

- [ ] **Step 2: Confirm template by reading one existing icon**

Read `/tmp/quill-icons-work/IconSearch.qml`. Expected: file exists with `Shape { ShapePath { PathSvg { path: "..." } } ... }` structure. This is the template for new icons.

## Task 1.2: Write 20 new icon QML files

Each icon = one `.qml` file in `/tmp/quill-icons-work/`. Pattern is identical except for name + `PathSvg.path` value(s). Stroke-only, 24x24 viewBox, multi-subpath glyphs use multiple `ShapePath` blocks.

**Files to create (all under `/tmp/quill-icons-work/`):**

`IconHighlighter.qml`, `IconPen.qml`, `IconUnderline.qml`, `IconStrikethrough.qml`, `IconType.qml`, `IconEraser.qml`, `IconCircle.qml`, `IconArrowRight.qml`, `IconBookmark.qml`, `IconBookOpen.qml`, `IconZoomIn.qml`, `IconZoomOut.qml`, `IconMaximize.qml`, `IconHand.qml`, `IconPrinter.qml`, `IconRotateCw.qml`, `IconRotateCcw.qml`, `IconFilePlus.qml`, `IconFiles.qml`, `IconLayoutGrid.qml`.

- [ ] **Step 1: Generate all 20 files via Lucide source**

Lucide is MIT-licensed (`https://github.com/lucide-icons/lucide`). Each Lucide SVG file at `https://raw.githubusercontent.com/lucide-icons/lucide/main/icons/<lucide-name>.svg` contains `<path d="..." />` elements. We extract the `d` attribute(s) and embed in our `Shape`.

Lucide-name mapping for our 20:

| Our QML | Lucide SVG |
|---|---|
| IconHighlighter | highlighter |
| IconPen | pen |
| IconUnderline | underline |
| IconStrikethrough | strikethrough |
| IconType | type |
| IconEraser | eraser |
| IconCircle | circle |
| IconArrowRight | arrow-right |
| IconBookmark | bookmark |
| IconBookOpen | book-open |
| IconZoomIn | zoom-in |
| IconZoomOut | zoom-out |
| IconMaximize | maximize |
| IconHand | hand |
| IconPrinter | printer |
| IconRotateCw | rotate-cw |
| IconRotateCcw | rotate-ccw |
| IconFilePlus | file-plus |
| IconFiles | files |
| IconLayoutGrid | layout-grid |

Save the Python generator below to `/tmp/gen_icons.py`, then run it with `python3 /tmp/gen_icons.py`:

```python
import re, sys, urllib.request, pathlib

ICONS = {
    "IconHighlighter":"highlighter","IconPen":"pen","IconUnderline":"underline",
    "IconStrikethrough":"strikethrough","IconType":"type","IconEraser":"eraser",
    "IconCircle":"circle","IconArrowRight":"arrow-right","IconBookmark":"bookmark",
    "IconBookOpen":"book-open","IconZoomIn":"zoom-in","IconZoomOut":"zoom-out",
    "IconMaximize":"maximize","IconHand":"hand","IconPrinter":"printer",
    "IconRotateCw":"rotate-cw","IconRotateCcw":"rotate-ccw","IconFilePlus":"file-plus",
    "IconFiles":"files","IconLayoutGrid":"layout-grid",
}

OUT = pathlib.Path("/tmp/quill-icons-work")

PATH_RX  = re.compile(r'<path[^>]*\sd="([^"]+)"')
CIRC_RX  = re.compile(r'<circle[^>]*cx="([\d.]+)"[^>]*cy="([\d.]+)"[^>]*r="([\d.]+)"')
LINE_RX  = re.compile(r'<line[^>]*x1="([\d.\-]+)"[^>]*y1="([\d.\-]+)"[^>]*x2="([\d.\-]+)"[^>]*y2="([\d.\-]+)"')
RECT_RX  = re.compile(r'<rect[^>]*x="([\d.\-]+)"[^>]*y="([\d.\-]+)"[^>]*width="([\d.\-]+)"[^>]*height="([\d.\-]+)"')
POLY_RX  = re.compile(r'<polyline[^>]*points="([^"]+)"')

def fetch_paths(lucide_name):
    url = f"https://raw.githubusercontent.com/lucide-icons/lucide/main/icons/{lucide_name}.svg"
    svg = urllib.request.urlopen(url).read().decode()
    paths = []
    for m in PATH_RX.finditer(svg):
        paths.append(m.group(1))
    for cx,cy,r in CIRC_RX.findall(svg):
        paths.append(f"M{cx} {cy}m-{r} 0a{r} {r} 0 1 0 {float(r)*2} 0a{r} {r} 0 1 0 -{float(r)*2} 0")
    for x1,y1,x2,y2 in LINE_RX.findall(svg):
        paths.append(f"M{x1} {y1}L{x2} {y2}")
    for m in RECT_RX.finditer(svg):
        x,y,w,h = m.groups()
        paths.append(f"M{x} {y}h{w}v{h}h-{w}z")
    for pts in POLY_RX.findall(svg):
        coords = pts.strip().split()
        if coords:
            d = "M" + coords[0] + "".join("L"+c for c in coords[1:])
            paths.append(d)
    return paths

def emit(qml_name, paths):
    blocks = "\n\n".join(f'''    ShapePath {{
        strokeColor: root.color
        strokeWidth: root.strokeWidth
        fillColor: "transparent"
        capStyle: ShapePath.RoundCap
        joinStyle: ShapePath.RoundJoin
        scale: Qt.size(root.size / 24, root.size / 24)
        PathSvg {{ path: "{p}" }}
    }}''' for p in paths)
    body = f'''import QtQuick
import QtQuick.Shapes

Shape {{
    id: root
    property real size: 24
    property color color: "#ffffff"
    property real strokeWidth: Math.max(1, size / 12)
    width: size
    height: size
    clip: false
    preferredRendererType: Shape.CurveRenderer

{blocks}
}}
'''
    (OUT / f"{qml_name}.qml").write_text(body)

for qml, lucide in ICONS.items():
    paths = fetch_paths(lucide)
    if not paths:
        print(f"WARN no paths for {lucide}", file=sys.stderr); continue
    emit(qml, paths)
    print(f"wrote {qml}.qml ({len(paths)} subpaths)")
```

Expected output: `wrote IconHighlighter.qml (3 subpaths)` and similar lines for all 20. No `WARN no paths` lines.

- [ ] **Step 2: Verify each file was generated**

```bash
cd /tmp/quill-icons-work
ls Icon*.qml | wc -l
for f in IconHighlighter IconPen IconUnderline IconStrikethrough IconType \
         IconEraser IconCircle IconArrowRight IconBookmark IconBookOpen \
         IconZoomIn IconZoomOut IconMaximize IconHand IconPrinter \
         IconRotateCw IconRotateCcw IconFilePlus IconFiles IconLayoutGrid; do
  test -s "$f.qml" && echo "OK $f.qml" || echo "MISSING $f.qml"
done
```

Expected: 20 `OK ...` lines, total 118 `Icon*.qml` files (was 98).

## Task 1.3: Update qmldir

**Files:**
- Modify: `/tmp/quill-icons-work/qmldir`

- [ ] **Step 1: Append 20 module entries and re-sort**

```bash
cd /tmp/quill-icons-work
{
  echo "IconHighlighter 1.0 IconHighlighter.qml"
  echo "IconPen 1.0 IconPen.qml"
  echo "IconUnderline 1.0 IconUnderline.qml"
  echo "IconStrikethrough 1.0 IconStrikethrough.qml"
  echo "IconType 1.0 IconType.qml"
  echo "IconEraser 1.0 IconEraser.qml"
  echo "IconCircle 1.0 IconCircle.qml"
  echo "IconArrowRight 1.0 IconArrowRight.qml"
  echo "IconBookmark 1.0 IconBookmark.qml"
  echo "IconBookOpen 1.0 IconBookOpen.qml"
  echo "IconZoomIn 1.0 IconZoomIn.qml"
  echo "IconZoomOut 1.0 IconZoomOut.qml"
  echo "IconMaximize 1.0 IconMaximize.qml"
  echo "IconHand 1.0 IconHand.qml"
  echo "IconPrinter 1.0 IconPrinter.qml"
  echo "IconRotateCw 1.0 IconRotateCw.qml"
  echo "IconRotateCcw 1.0 IconRotateCcw.qml"
  echo "IconFilePlus 1.0 IconFilePlus.qml"
  echo "IconFiles 1.0 IconFiles.qml"
  echo "IconLayoutGrid 1.0 IconLayoutGrid.qml"
} >> qmldir

head -n1 qmldir > /tmp/qmldir.new
tail -n +2 qmldir | sort >> /tmp/qmldir.new
mv /tmp/qmldir.new qmldir

head -3 qmldir
wc -l qmldir
```

Expected: first line is `module Icons` (verify with `head -1`); file has roughly 119 lines.

## Task 1.4: Commit and push

- [ ] **Step 1: Stage, commit, push**

```bash
cd /tmp/quill-icons-work
git add Icon*.qml qmldir
git status
git commit -m "$(cat <<'EOF'
add: PDF/annotation icons (Highlighter, Pen, Underline, Strikethrough, Type, Eraser, Circle, ArrowRight, Bookmark, BookOpen, ZoomIn, ZoomOut, Maximize, Hand, Printer, RotateCw, RotateCcw, FilePlus, Files, LayoutGrid)

20 Lucide-derived stroke icons covering annotation tools (Highlighter, Pen,
Underline, Strikethrough, Type, Eraser), shape primitives (Circle, ArrowRight),
viewer chrome (Bookmark, BookOpen, ZoomIn/Out, Maximize, Hand, Printer,
LayoutGrid), and page operations (RotateCw/Ccw, FilePlus, Files).

Some originally-filled Lucide icons (Highlighter, Pen) are rendered as outlines
to match the existing stroke-only icon style.
EOF
)"
git log --oneline -1
git push origin main
```

Expected: one new commit on `main`, no `Co-Authored-By` trailer, push succeeds.

---

# Phase 2 — HyprPDF v0.1 skeleton

## Task 2.1: Initialize git repo + .gitignore

**Files:**
- Modify: `/stuff/Study/projects/hyprpdf/` (init)
- Create: `/stuff/Study/projects/hyprpdf/.gitignore`

- [ ] **Step 1: Init repo**

```bash
cd /stuff/Study/projects/hyprpdf
git init -b main
git status
```

Expected: `Initialized empty Git repository`; untracked: existing `CLAUDE.md`, `OUTLINE.md`, `README.md`, `CMakeLists.txt`, `src/`, `themes/`, `dist/`, `tests/`, `docs/`.

- [ ] **Step 2: Write .gitignore at `/stuff/Study/projects/hyprpdf/.gitignore`**

Contents:

```
build/
build-*/
.cache/
compile_commands.json
.qmlls.ini
.qtc_clangd/
*.user
*.user.*
*.swp
*.swo
*~
CMakeUserPresets.json
.vscode/
.idea/
```

## Task 2.2: Create GitHub repo with GPL-3.0 license

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/LICENSE` (gh-written)
- Modify: git remote `origin`

- [ ] **Step 1: Create remote + LICENSE**

```bash
cd /stuff/Study/projects/hyprpdf
gh repo create soyeb-jim285/hyprpdf \
  --public \
  --source=. \
  --remote=origin \
  --license=gpl-3.0 \
  --description "Qt6/QML PDF + office viewer with annotations"
```

Expected: `Created repository soyeb-jim285/hyprpdf on GitHub`. `LICENSE` may or may not appear locally depending on `gh` version.

- [ ] **Step 2: Verify LICENSE + remote**

```bash
cd /stuff/Study/projects/hyprpdf
ls LICENSE 2>/dev/null && head -3 LICENSE
git remote -v
```

Expected: remote `origin` shows SSH URL `git@github.com:soyeb-jim285/hyprpdf.git`.

If `LICENSE` is missing locally, pull it from the freshly created remote:

```bash
cd /stuff/Study/projects/hyprpdf
git fetch origin
git checkout origin/main -- LICENSE
ls LICENSE
```

## Task 2.3: First commit — import existing skeleton + spec + plan + LICENSE

- [ ] **Step 1: Stage + commit existing files**

```bash
cd /stuff/Study/projects/hyprpdf
git add CLAUDE.md OUTLINE.md README.md CMakeLists.txt \
        src/main.cpp src/CMakeLists.txt src/qml/Main.qml src/qml/PdfView.qml \
        themes/default.toml dist/hyprpdf.desktop tests/CMakeLists.txt \
        docs/superpowers/specs/2026-04-19-icons-and-skeleton-design.md \
        docs/superpowers/plans/2026-04-19-icons-and-skeleton.md \
        .gitignore LICENSE
git status
git commit -m "$(cat <<'EOF'
chore: import skeleton, OUTLINE, README, design spec, plan

Initial import of pre-existing v0.1 skeleton (open/render/scroll/zoom),
project documentation (OUTLINE.md feature matrix and roadmap, CLAUDE.md
Claude Code guide), GPL-3.0 LICENSE, .gitignore, and the v0.1 brainstorm
artefacts (design spec + implementation plan) under docs/superpowers/.
EOF
)"
git log --oneline -1
```

Expected: one commit, no `Co-Authored-By` trailer, no Claude attribution footer.

## Task 2.4: Add Quill + quill-icons submodules

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/.gitmodules`
- Create: `/stuff/Study/projects/hyprpdf/src/qml/Quill/` (submodule)
- Create: `/stuff/Study/projects/hyprpdf/src/qml/icons/` (submodule)

**Pre-req:** Phase 1 must have pushed icons to `quill-icons` `main` first.

- [ ] **Step 1: Add Quill submodule**

```bash
cd /stuff/Study/projects/hyprpdf
git submodule add git@github.com:soyeb-jim285/quill.git src/qml/Quill
ls src/qml/Quill/qmldir
```

Expected: `qmldir` exists in `src/qml/Quill/`.

- [ ] **Step 2: Add quill-icons submodule**

```bash
cd /stuff/Study/projects/hyprpdf
git submodule add git@github.com:soyeb-jim285/quill-icons.git src/qml/icons
ls src/qml/icons/IconHighlighter.qml
```

Expected: `IconHighlighter.qml` exists (verifying Phase 1 push landed in this submodule pointer).

- [ ] **Step 3: Commit submodule additions**

```bash
cd /stuff/Study/projects/hyprpdf
git status
git add .gitmodules src/qml/Quill src/qml/icons
git commit -m "$(cat <<'EOF'
chore: add Quill and quill-icons submodules

src/qml/Quill   -> github.com/soyeb-jim285/quill   (component library)
src/qml/icons   -> github.com/soyeb-jim285/quill-icons (Lucide-derived icons)

Pinned to current main HEADs. CMake will set QT_QML_IMPORT_PATH to
src/qml/ so both modules resolve from this prefix.
EOF
)"
```

## Task 2.5: Vendor toml.hpp

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/src/third_party/toml.hpp` (copied from HyprFM)

- [ ] **Step 1: Copy from HyprFM**

```bash
mkdir -p /stuff/Study/projects/hyprpdf/src/third_party
cp /stuff/Study/projects/hyprfm/src/third_party/toml.hpp \
   /stuff/Study/projects/hyprpdf/src/third_party/toml.hpp
ls -la /stuff/Study/projects/hyprpdf/src/third_party/toml.hpp
head -10 /stuff/Study/projects/hyprpdf/src/third_party/toml.hpp
```

Expected: file size ~489 KB, opens with the `toml++` header banner.

- [ ] **Step 2: Commit**

```bash
cd /stuff/Study/projects/hyprpdf
git add src/third_party/toml.hpp
git commit -m "$(cat <<'EOF'
chore: vendor toml.hpp from HyprFM

Header-only TOML parser (toml++) used by ThemeLoader and ConfigManager.
Source: /stuff/Study/projects/hyprfm/src/third_party/toml.hpp at HEAD.
EOF
)"
```

## Task 2.6: Rewrite themes/default.toml + add latte.toml

**Files:**
- Modify: `/stuff/Study/projects/hyprpdf/themes/default.toml`
- Create: `/stuff/Study/projects/hyprpdf/themes/latte.toml`

- [ ] **Step 1: Rewrite default.toml — replace contents with:**

```
name = "Catppuccin Mocha"

[colors]
base    = "#1e1e2e"
mantle  = "#181825"
crust   = "#11111b"
surface = "#313244"
overlay = "#6c7086"
text    = "#cdd6f4"
subtext = "#bac2de"
muted   = "#a6adc8"
accent  = "#89b4fa"
success = "#a6e3a1"
warning = "#f9e2af"
error   = "#f38ba8"

[annotations]
highlight = "#f9e2af"
underline = "#a6e3a1"
strikeout = "#f38ba8"
ink       = "#89b4fa"
note      = "#fab387"
```

- [ ] **Step 2: Create latte.toml with contents:**

```
name = "Catppuccin Latte"

[colors]
base    = "#eff1f5"
mantle  = "#e6e9ef"
crust   = "#dce0e8"
surface = "#ccd0da"
overlay = "#7c7f93"
text    = "#4c4f69"
subtext = "#5c5f77"
muted   = "#6c6f85"
accent  = "#1e66f5"
success = "#40a02b"
warning = "#df8e1d"
error   = "#d20f39"

[annotations]
highlight = "#df8e1d"
underline = "#40a02b"
strikeout = "#d20f39"
ink       = "#1e66f5"
note      = "#fe640b"
```

(Commit deferred — bundled with ThemeLoader commit in Task 2.7.)

## Task 2.7: ThemeLoader (TDD)

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/src/services/themeloader.h`
- Create: `/stuff/Study/projects/hyprpdf/src/services/themeloader.cpp`
- Create: `/stuff/Study/projects/hyprpdf/tests/tst_themeloader.cpp`
- Modify: `/stuff/Study/projects/hyprpdf/tests/CMakeLists.txt`

- [ ] **Step 1: Write the failing test at `tests/tst_themeloader.cpp`**

```cpp
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "services/themeloader.h"

class TestThemeLoader : public QObject {
    Q_OBJECT
private slots:
    void defaultsWithoutTomlFile() {
        ThemeLoader t;
        QCOMPARE(t.color("base"),    QColor("#1e1e2e"));
        QCOMPARE(t.color("text"),    QColor("#cdd6f4"));
        QCOMPARE(t.color("accent"),  QColor("#89b4fa"));
        QCOMPARE(t.annotation("highlight"), QColor("#f9e2af"));
        QCOMPARE(t.annotation("ink"),       QColor("#89b4fa"));
    }

    void loadOverridesFromToml() {
        QTemporaryDir dir;
        QFile f(dir.filePath("custom.toml"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(R"(name = "custom"
[colors]
base   = "#000000"
text   = "#ffffff"
[annotations]
highlight = "#ff0000"
)");
        f.close();
        ThemeLoader t;
        t.loadTheme("custom", dir.path());
        QCOMPARE(t.color("base"),  QColor("#000000"));
        QCOMPARE(t.color("text"),  QColor("#ffffff"));
        QCOMPARE(t.annotation("highlight"), QColor("#ff0000"));
        QCOMPARE(t.color("accent"), QColor("#89b4fa"));
    }

    void themeChangedSignalFires() {
        QTemporaryDir dir;
        QFile f(dir.filePath("x.toml"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("name = \"x\"\n[colors]\nbase=\"#abcdef\"\n");
        f.close();
        ThemeLoader t;
        QSignalSpy spy(&t, &ThemeLoader::themeChanged);
        t.loadTheme("x", dir.path());
        QCOMPARE(spy.count(), 1);
    }

    void availableThemesListsTomlFiles() {
        QTemporaryDir dir;
        for (auto n : {"a.toml", "b.toml", "ignore.txt"}) {
            QFile f(dir.filePath(n)); QVERIFY(f.open(QIODevice::WriteOnly)); f.close();
        }
        ThemeLoader t;
        auto list = t.availableThemes(dir.path());
        QCOMPARE(list.size(), 2);
        QVERIFY(list.contains("a"));
        QVERIFY(list.contains("b"));
    }
};
QTEST_MAIN(TestThemeLoader)
#include "tst_themeloader.moc"
```

- [ ] **Step 2: Add test target — replace `tests/CMakeLists.txt` with**

```cmake
function(hyprpdf_test name)
    qt_add_executable(${name} ${name}.cpp ${ARGN})
    target_link_libraries(${name} PRIVATE Qt6::Test Qt6::Core Qt6::Gui Qt6::Pdf)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_compile_definitions(${name} PRIVATE
        HYPRPDF_THEMES_DIR="${CMAKE_SOURCE_DIR}/themes"
        HYPRPDF_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

hyprpdf_test(tst_themeloader ${CMAKE_SOURCE_DIR}/src/services/themeloader.cpp)
```

- [ ] **Step 3: Verify FAILS (no themeloader.h yet)**

```bash
cd /stuff/Study/projects/hyprpdf
cmake -B build -G Ninja 2>&1 | tail -20
cmake --build build --target tst_themeloader 2>&1 | tail -10
```

Expected: build error like `fatal error: services/themeloader.h: No such file or directory`.

- [ ] **Step 4: Write `src/services/themeloader.h`**

```cpp
#pragma once
#include <QObject>
#include <QColor>
#include <QHash>
#include <QString>
#include <QStringList>

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
    explicit ThemeLoader(QObject *parent = nullptr);

    QColor base()    const { return color("base"); }
    QColor mantle()  const { return color("mantle"); }
    QColor crust()   const { return color("crust"); }
    QColor surface() const { return color("surface"); }
    QColor overlay() const { return color("overlay"); }
    QColor text()    const { return color("text"); }
    QColor subtext() const { return color("subtext"); }
    QColor muted()   const { return color("muted"); }
    QColor accent()  const { return color("accent"); }
    QColor success() const { return color("success"); }
    QColor warning() const { return color("warning"); }
    QColor error()   const { return color("error"); }

    Q_INVOKABLE QColor color(const QString &name) const;
    Q_INVOKABLE QColor annotation(const QString &name) const;
    Q_INVOKABLE void loadTheme(const QString &nameOrPath, const QString &themesDir);
    Q_INVOKABLE QStringList availableThemes(const QString &themesDir) const;

signals:
    void themeChanged();

private:
    QHash<QString, QColor> m_colors;
    QHash<QString, QColor> m_annotations;
};
```

- [ ] **Step 5: Write `src/services/themeloader.cpp`**

```cpp
#include "themeloader.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QDebug>

#define TOML_HEADER_ONLY 1
#include "third_party/toml.hpp"

namespace {

const QHash<QString, QColor> kDefaultColors = {
    {"base",    QColor("#1e1e2e")},
    {"mantle",  QColor("#181825")},
    {"crust",   QColor("#11111b")},
    {"surface", QColor("#313244")},
    {"overlay", QColor("#6c7086")},
    {"text",    QColor("#cdd6f4")},
    {"subtext", QColor("#bac2de")},
    {"muted",   QColor("#a6adc8")},
    {"accent",  QColor("#89b4fa")},
    {"success", QColor("#a6e3a1")},
    {"warning", QColor("#f9e2af")},
    {"error",   QColor("#f38ba8")},
};

const QHash<QString, QColor> kDefaultAnnotations = {
    {"highlight", QColor("#f9e2af")},
    {"underline", QColor("#a6e3a1")},
    {"strikeout", QColor("#f38ba8")},
    {"ink",       QColor("#89b4fa")},
    {"note",      QColor("#fab387")},
};

QString resolveThemePath(const QString &nameOrPath, const QString &themesDir) {
    if (nameOrPath.isEmpty()) return {};
    QFileInfo fi(nameOrPath);
    if (fi.isAbsolute() && fi.exists()) return nameOrPath;
    QString candidate = QDir(themesDir).filePath(nameOrPath.endsWith(".toml")
                          ? nameOrPath : nameOrPath + ".toml");
    return QFile::exists(candidate) ? candidate : QString{};
}

}  // namespace

ThemeLoader::ThemeLoader(QObject *parent) : QObject(parent) {}

QColor ThemeLoader::color(const QString &name) const {
    if (auto it = m_colors.constFind(name); it != m_colors.cend()) return *it;
    return kDefaultColors.value(name);
}

QColor ThemeLoader::annotation(const QString &name) const {
    if (auto it = m_annotations.constFind(name); it != m_annotations.cend()) return *it;
    return kDefaultAnnotations.value(name);
}

void ThemeLoader::loadTheme(const QString &nameOrPath, const QString &themesDir) {
    const QString path = resolveThemePath(nameOrPath, themesDir);
    if (path.isEmpty()) {
        qWarning() << "ThemeLoader: theme not found" << nameOrPath << "in" << themesDir;
        emit themeChanged();
        return;
    }
    try {
        auto tbl = toml::parse_file(path.toStdString());
        m_colors.clear();
        m_annotations.clear();
        if (auto colors = tbl["colors"].as_table()) {
            for (auto &[k, v] : *colors) {
                if (auto s = v.value<std::string>()) {
                    m_colors.insert(QString::fromStdString(std::string(k.str())),
                                    QColor(QString::fromStdString(*s)));
                }
            }
        }
        if (auto annots = tbl["annotations"].as_table()) {
            for (auto &[k, v] : *annots) {
                if (auto s = v.value<std::string>()) {
                    m_annotations.insert(QString::fromStdString(std::string(k.str())),
                                         QColor(QString::fromStdString(*s)));
                }
            }
        }
    } catch (const toml::parse_error &e) {
        qWarning() << "ThemeLoader: parse error" << e.what();
    }
    emit themeChanged();
}

QStringList ThemeLoader::availableThemes(const QString &themesDir) const {
    QStringList out;
    QDir dir(themesDir);
    for (const auto &fi : dir.entryInfoList({"*.toml"}, QDir::Files)) {
        out << fi.completeBaseName();
    }
    return out;
}
```

- [ ] **Step 6: Run test — should PASS**

```bash
cd /stuff/Study/projects/hyprpdf
cmake --build build --target tst_themeloader 2>&1 | tail -10
ctest --test-dir build -R tst_themeloader --output-on-failure
```

Expected: build succeeds, `1/1 Test #1: tst_themeloader ........... Passed`.

- [ ] **Step 7: Commit**

```bash
cd /stuff/Study/projects/hyprpdf
git add src/services/themeloader.h src/services/themeloader.cpp \
        tests/tst_themeloader.cpp tests/CMakeLists.txt \
        themes/default.toml themes/latte.toml
git commit -m "$(cat <<'EOF'
feat: ThemeLoader with Catppuccin defaults and annotation palette

ThemeLoader parses [colors] and [annotations] tables from a TOML theme
file, falling back to a baked-in Catppuccin Mocha palette per-key when
absent. Exposes Q_PROPERTY accessors for QML binding plus Q_INVOKABLE
annotation(name) and availableThemes(dir) helpers. themeChanged signal
fires on every loadTheme() call, including failures, so QML re-binds
even on fallback to defaults.

Bundled themes: default.toml (Mocha) and latte.toml (Latte light).
Annotation palette covers highlight, underline, strikeout, ink, note.
EOF
)"
```

## Task 2.8: ConfigManager (TDD)

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/src/services/configmanager.h`
- Create: `/stuff/Study/projects/hyprpdf/src/services/configmanager.cpp`
- Create: `/stuff/Study/projects/hyprpdf/tests/tst_configmanager.cpp`
- Modify: `/stuff/Study/projects/hyprpdf/tests/CMakeLists.txt`

- [ ] **Step 1: Write the failing test at `tests/tst_configmanager.cpp`**

```cpp
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "services/configmanager.h"

class TestConfigManager : public QObject {
    Q_OBJECT
private slots:
    void firstRunWritesDefaults() {
        QTemporaryDir dir;
        ConfigManager c(dir.filePath("config.toml"));
        QVERIFY(QFile::exists(dir.filePath("config.toml")));
        QCOMPARE(c.theme(), QString("default"));
        QCOMPARE(c.iconTheme(), QString("default"));
        QCOMPARE(c.defaultZoom(), 1.0);
        QCOMPARE(c.continuousScroll(), true);
        QCOMPARE(c.radiusSm(), 4);
        QCOMPARE(c.radiusMd(), 8);
        QCOMPARE(c.radiusLg(), 12);
        QCOMPARE(c.animationsEnabled(), true);
        QCOMPARE(c.animDurationFast(), 100);
        QCOMPARE(c.animDurationNormal(), 200);
        QCOMPARE(c.animDurationSlow(), 350);
        QCOMPARE(c.transparencyEnabled(), false);
        QCOMPARE(c.transparencyLevel(), 0.85);
    }

    void setterSavesAndReloads() {
        QTemporaryDir dir;
        const QString path = dir.filePath("config.toml");
        {
            ConfigManager c(path);
            c.setTheme("latte");
            c.setDefaultZoom(1.5);
            c.save();
        }
        ConfigManager c2(path);
        QCOMPARE(c2.theme(), QString("latte"));
        QCOMPARE(c2.defaultZoom(), 1.5);
    }

    void configChangedSignalFires() {
        QTemporaryDir dir;
        ConfigManager c(dir.filePath("config.toml"));
        QSignalSpy spy(&c, &ConfigManager::configChanged);
        c.setTheme("latte");
        QVERIFY(spy.count() >= 1);
    }

    void brokenTomlRotates() {
        QTemporaryDir dir;
        const QString path = dir.filePath("config.toml");
        QFile f(path); QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("this is not valid toml [[[\n");
        f.close();
        ConfigManager c(path);
        QCOMPARE(c.theme(), QString("default"));
        QVERIFY(QFile::exists(path));
        QDir d(dir.path());
        bool foundBroken = false;
        for (const auto &n : d.entryList({"config.toml.broken-*"}, QDir::Files))
            foundBroken = !n.isEmpty();
        QVERIFY(foundBroken);
    }
};
QTEST_MAIN(TestConfigManager)
#include "tst_configmanager.moc"
```

- [ ] **Step 2: Add test target — append to `tests/CMakeLists.txt`**

```cmake
hyprpdf_test(tst_configmanager ${CMAKE_SOURCE_DIR}/src/services/configmanager.cpp)
```

- [ ] **Step 3: Verify FAILS**

```bash
cd /stuff/Study/projects/hyprpdf
cmake --build build --target tst_configmanager 2>&1 | tail -10
```

Expected: `fatal error: services/configmanager.h: No such file or directory`.

- [ ] **Step 4: Write `src/services/configmanager.h`**

```cpp
#pragma once
#include <QObject>
#include <QString>
#include <QTimer>

class ConfigManager : public QObject {
    Q_OBJECT
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

public:
    explicit ConfigManager(const QString &path = {}, QObject *parent = nullptr);

    QString theme()            const { return m_theme; }
    QString iconTheme()        const { return m_iconTheme; }
    double  defaultZoom()      const { return m_defaultZoom; }
    bool    continuousScroll() const { return m_continuousScroll; }
    int     radiusSm()         const { return m_radiusSm; }
    int     radiusMd()         const { return m_radiusMd; }
    int     radiusLg()         const { return m_radiusLg; }
    bool    animationsEnabled()   const { return m_animationsEnabled; }
    int     animDurationFast()    const { return m_animDurationFast; }
    int     animDurationNormal()  const { return m_animDurationNormal; }
    int     animDurationSlow()    const { return m_animDurationSlow; }
    bool    transparencyEnabled() const { return m_transparencyEnabled; }
    double  transparencyLevel()   const { return m_transparencyLevel; }

    void setTheme(const QString &v);
    void setIconTheme(const QString &v);
    void setDefaultZoom(double v);
    void setContinuousScroll(bool v);
    void setRadiusSm(int v);
    void setRadiusMd(int v);
    void setRadiusLg(int v);
    void setAnimationsEnabled(bool v);
    void setAnimDurationFast(int v);
    void setAnimDurationNormal(int v);
    void setAnimDurationSlow(int v);
    void setTransparencyEnabled(bool v);
    void setTransparencyLevel(double v);

    Q_INVOKABLE void save();
    Q_INVOKABLE void reload();

signals:
    void configChanged();

private:
    void load();
    void scheduleSave();
    void rotateBroken();

    QString m_path;
    QTimer  m_saveTimer;

    QString m_theme = "default";
    QString m_iconTheme = "default";
    double  m_defaultZoom = 1.0;
    bool    m_continuousScroll = true;
    int     m_radiusSm = 4;
    int     m_radiusMd = 8;
    int     m_radiusLg = 12;
    bool    m_animationsEnabled = true;
    int     m_animDurationFast = 100;
    int     m_animDurationNormal = 200;
    int     m_animDurationSlow = 350;
    bool    m_transparencyEnabled = false;
    double  m_transparencyLevel = 0.85;
};
```

- [ ] **Step 5: Write `src/services/configmanager.cpp`**

```cpp
#include "configmanager.h"
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

#define TOML_HEADER_ONLY 1
#include "third_party/toml.hpp"

ConfigManager::ConfigManager(const QString &path, QObject *parent)
    : QObject(parent) {
    if (path.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                          + "/hyprpdf";
        QDir().mkpath(dir);
        m_path = dir + "/config.toml";
    } else {
        m_path = path;
        QDir().mkpath(QFileInfo(path).absolutePath());
    }
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(300);
    connect(&m_saveTimer, &QTimer::timeout, this, &ConfigManager::save);

    load();
}

void ConfigManager::load() {
    if (!QFile::exists(m_path)) {
        save();
        return;
    }
    try {
        auto tbl = toml::parse_file(m_path.toStdString());
        if (auto v = tbl["theme"].value<std::string>())             m_theme = QString::fromStdString(*v);
        if (auto v = tbl["iconTheme"].value<std::string>())         m_iconTheme = QString::fromStdString(*v);
        if (auto v = tbl["defaultZoom"].value<double>())            m_defaultZoom = *v;
        if (auto v = tbl["continuousScroll"].value<bool>())         m_continuousScroll = *v;
        if (auto v = tbl["radiusSm"].value<int64_t>())              m_radiusSm = static_cast<int>(*v);
        if (auto v = tbl["radiusMd"].value<int64_t>())              m_radiusMd = static_cast<int>(*v);
        if (auto v = tbl["radiusLg"].value<int64_t>())              m_radiusLg = static_cast<int>(*v);
        if (auto v = tbl["animationsEnabled"].value<bool>())        m_animationsEnabled = *v;
        if (auto v = tbl["animDurationFast"].value<int64_t>())      m_animDurationFast = static_cast<int>(*v);
        if (auto v = tbl["animDurationNormal"].value<int64_t>())    m_animDurationNormal = static_cast<int>(*v);
        if (auto v = tbl["animDurationSlow"].value<int64_t>())      m_animDurationSlow = static_cast<int>(*v);
        if (auto v = tbl["transparencyEnabled"].value<bool>())      m_transparencyEnabled = *v;
        if (auto v = tbl["transparencyLevel"].value<double>())      m_transparencyLevel = *v;
    } catch (const toml::parse_error &e) {
        qWarning() << "ConfigManager: parse error" << e.what() << "rotating";
        rotateBroken();
        save();
    }
}

void ConfigManager::rotateBroken() {
    const QString rotated = m_path + ".broken-"
        + QString::number(QDateTime::currentSecsSinceEpoch());
    QFile::rename(m_path, rotated);
}

void ConfigManager::save() {
    QSaveFile f(m_path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "ConfigManager: cannot write" << m_path << f.errorString();
        return;
    }
    QString s;
    s += "theme               = \"" + m_theme + "\"\n";
    s += "iconTheme           = \"" + m_iconTheme + "\"\n";
    s += "defaultZoom         = " + QString::number(m_defaultZoom) + "\n";
    s += QString("continuousScroll    = %1\n").arg(m_continuousScroll ? "true" : "false");
    s += "radiusSm            = " + QString::number(m_radiusSm) + "\n";
    s += "radiusMd            = " + QString::number(m_radiusMd) + "\n";
    s += "radiusLg            = " + QString::number(m_radiusLg) + "\n";
    s += QString("animationsEnabled   = %1\n").arg(m_animationsEnabled ? "true" : "false");
    s += "animDurationFast    = " + QString::number(m_animDurationFast) + "\n";
    s += "animDurationNormal  = " + QString::number(m_animDurationNormal) + "\n";
    s += "animDurationSlow    = " + QString::number(m_animDurationSlow) + "\n";
    s += QString("transparencyEnabled = %1\n").arg(m_transparencyEnabled ? "true" : "false");
    s += "transparencyLevel   = " + QString::number(m_transparencyLevel) + "\n";
    f.write(s.toUtf8());
    f.commit();
}

void ConfigManager::reload() {
    load();
    emit configChanged();
}

void ConfigManager::scheduleSave() {
    emit configChanged();
    m_saveTimer.start();
}

#define SETTER(Type, Name, Member) \
    void ConfigManager::set##Name(Type v) { if (Member == v) return; Member = v; scheduleSave(); }

SETTER(const QString &, Theme,            m_theme)
SETTER(const QString &, IconTheme,        m_iconTheme)
SETTER(double,          DefaultZoom,      m_defaultZoom)
SETTER(bool,            ContinuousScroll, m_continuousScroll)
SETTER(int,             RadiusSm,         m_radiusSm)
SETTER(int,             RadiusMd,         m_radiusMd)
SETTER(int,             RadiusLg,         m_radiusLg)
SETTER(bool,            AnimationsEnabled,   m_animationsEnabled)
SETTER(int,             AnimDurationFast,    m_animDurationFast)
SETTER(int,             AnimDurationNormal,  m_animDurationNormal)
SETTER(int,             AnimDurationSlow,    m_animDurationSlow)
SETTER(bool,            TransparencyEnabled, m_transparencyEnabled)
SETTER(double,          TransparencyLevel,   m_transparencyLevel)
```

- [ ] **Step 6: Build + run**

```bash
cd /stuff/Study/projects/hyprpdf
cmake --build build --target tst_configmanager 2>&1 | tail -10
ctest --test-dir build -R tst_configmanager --output-on-failure
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
cd /stuff/Study/projects/hyprpdf
git add src/services/configmanager.h src/services/configmanager.cpp \
        tests/tst_configmanager.cpp tests/CMakeLists.txt
git commit -m "$(cat <<'EOF'
feat: ConfigManager with trimmed v0.1 schema

Persists user settings to ~/.config/hyprpdf/config.toml (path overridable
for tests). Schema covers theme, iconTheme, view defaults (defaultZoom,
continuousScroll), Quill-bridge tunables (radiusSm/Md/Lg, animation
durations, animationsEnabled), and transparency. Setters mark dirty +
debounce save by 300ms. Broken TOML is rotated to .broken-<ts> and
defaults are re-written. Heavier keys (shortcutMap, sidebar widths,
bookmarks) deferred to v0.2.
EOF
)"
```

## Task 2.9: RecentFilesModel (TDD)

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/src/models/recentfilesmodel.h`
- Create: `/stuff/Study/projects/hyprpdf/src/models/recentfilesmodel.cpp`
- Create: `/stuff/Study/projects/hyprpdf/tests/tst_recentfilesmodel.cpp`
- Modify: `/stuff/Study/projects/hyprpdf/tests/CMakeLists.txt`

- [ ] **Step 1: Write the failing test at `tests/tst_recentfilesmodel.cpp`**

```cpp
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "models/recentfilesmodel.h"

class TestRecentFilesModel : public QObject {
    Q_OBJECT
private slots:
    void addRecentDedupesAndMovesToTop() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/a/b/foo.pdf");
        m.addRecent("/a/b/bar.pdf");
        m.addRecent("/a/b/foo.pdf");
        QCOMPARE(m.rowCount(), 2);
        QCOMPARE(m.index(0).data(RecentFilesModel::PathRole).toString(),
                 QString("/a/b/foo.pdf"));
        QCOMPARE(m.index(1).data(RecentFilesModel::PathRole).toString(),
                 QString("/a/b/bar.pdf"));
    }

    void capsAt20() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        for (int i = 0; i < 25; ++i)
            m.addRecent(QString("/x/file%1.pdf").arg(i));
        QCOMPARE(m.rowCount(), 20);
        QCOMPARE(m.index(0).data(RecentFilesModel::PathRole).toString(),
                 QString("/x/file24.pdf"));
    }

    void clearEmpties() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/a"); m.addRecent("/b");
        m.clear();
        QCOMPARE(m.rowCount(), 0);
    }

    void persistRoundtrip() {
        QTemporaryDir dir;
        const QString p = dir.filePath("recent.json");
        {
            RecentFilesModel m(p);
            m.addRecent("/x/one.pdf");
            m.addRecent("/x/two.pdf");
        }
        RecentFilesModel m2(p);
        QCOMPARE(m2.rowCount(), 2);
        QCOMPARE(m2.index(0).data(RecentFilesModel::PathRole).toString(),
                 QString("/x/two.pdf"));
    }

    void existsRoleFalseForMissingPath() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/definitely/does/not/exist.pdf");
        QCOMPARE(m.index(0).data(RecentFilesModel::ExistsRole).toBool(), false);
    }

    void openEmitsRequest() {
        QTemporaryDir dir;
        RecentFilesModel m(dir.filePath("recent.json"));
        m.addRecent("/p/q.pdf");
        QSignalSpy spy(&m, &RecentFilesModel::requestOpen);
        m.open(0);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().first().toString(), QString("/p/q.pdf"));
    }
};
QTEST_MAIN(TestRecentFilesModel)
#include "tst_recentfilesmodel.moc"
```

- [ ] **Step 2: Add test target — append to `tests/CMakeLists.txt`**

```cmake
hyprpdf_test(tst_recentfilesmodel ${CMAKE_SOURCE_DIR}/src/models/recentfilesmodel.cpp)
```

- [ ] **Step 3: Verify FAILS**

Build the target — expect missing-header error.

- [ ] **Step 4: Write `src/models/recentfilesmodel.h`**

```cpp
#pragma once
#include <QAbstractListModel>
#include <QDateTime>
#include <QHash>
#include <QString>
#include <QVector>

class RecentFilesModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        NameRole,
        SizeRole,
        ModifiedRole,
        IconNameRole,
        ExistsRole
    };

    explicit RecentFilesModel(const QString &path = {}, QObject *parent = nullptr);
    ~RecentFilesModel() override;

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addRecent(const QString &path);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void open(int index);

signals:
    void requestOpen(const QString &path);

private:
    struct Entry { QString path; QDateTime addedAt; };
    void load();
    void save();
    static QString iconNameFor(const QString &path);

    QString m_storePath;
    QVector<Entry> m_entries;
    static constexpr int kMax = 20;
};
```

- [ ] **Step 5: Write `src/models/recentfilesmodel.cpp`**

```cpp
#include "recentfilesmodel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDebug>

RecentFilesModel::RecentFilesModel(const QString &path, QObject *parent)
    : QAbstractListModel(parent) {
    if (path.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QDir().mkpath(dir);
        m_storePath = dir + "/recent.json";
    } else {
        m_storePath = path;
        QDir().mkpath(QFileInfo(path).absolutePath());
    }
    load();
}

RecentFilesModel::~RecentFilesModel() { save(); }

int RecentFilesModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_entries.size();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_entries.size()) return {};
    const auto &e = m_entries[index.row()];
    QFileInfo fi(e.path);
    switch (role) {
        case PathRole:     return e.path;
        case NameRole:     return fi.fileName();
        case SizeRole:     return fi.size();
        case ModifiedRole: return fi.lastModified();
        case IconNameRole: return iconNameFor(e.path);
        case ExistsRole:   return fi.exists();
    }
    return {};
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const {
    return {
        {PathRole,     "path"},
        {NameRole,     "name"},
        {SizeRole,     "size"},
        {ModifiedRole, "modified"},
        {IconNameRole, "iconName"},
        {ExistsRole,   "exists"}
    };
}

void RecentFilesModel::addRecent(const QString &path) {
    if (path.isEmpty()) return;
    int existing = -1;
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].path == path) { existing = i; break; }
    }
    if (existing >= 0) {
        beginRemoveRows({}, existing, existing);
        m_entries.remove(existing);
        endRemoveRows();
    }
    beginInsertRows({}, 0, 0);
    m_entries.prepend({path, QDateTime::currentDateTime()});
    endInsertRows();
    while (m_entries.size() > kMax) {
        beginRemoveRows({}, m_entries.size() - 1, m_entries.size() - 1);
        m_entries.removeLast();
        endRemoveRows();
    }
    save();
}

void RecentFilesModel::remove(int index) {
    if (index < 0 || index >= m_entries.size()) return;
    beginRemoveRows({}, index, index);
    m_entries.remove(index);
    endRemoveRows();
    save();
}

void RecentFilesModel::clear() {
    if (m_entries.isEmpty()) return;
    beginResetModel();
    m_entries.clear();
    endResetModel();
    save();
}

void RecentFilesModel::open(int index) {
    if (index < 0 || index >= m_entries.size()) return;
    emit requestOpen(m_entries[index].path);
}

void RecentFilesModel::load() {
    QFile f(m_storePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) return;
    beginResetModel();
    m_entries.clear();
    for (const auto &v : doc.array()) {
        const auto o = v.toObject();
        Entry e;
        e.path    = o.value("path").toString();
        e.addedAt = QDateTime::fromString(o.value("addedAt").toString(), Qt::ISODate);
        if (!e.path.isEmpty()) m_entries.append(e);
    }
    endResetModel();
}

void RecentFilesModel::save() {
    QJsonArray arr;
    for (const auto &e : m_entries) {
        arr.append(QJsonObject{
            {"path", e.path},
            {"addedAt", e.addedAt.toString(Qt::ISODate)}
        });
    }
    QSaveFile f(m_storePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "RecentFilesModel: cannot write" << m_storePath << f.errorString();
        return;
    }
    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    f.commit();
}

QString RecentFilesModel::iconNameFor(const QString &path) {
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "pdf") return "IconFileText";
    if (ext == "pptx" || ext == "ppt" || ext == "odp") return "IconLayoutGrid";
    if (ext == "docx" || ext == "doc"  || ext == "odt") return "IconFileText";
    if (ext == "epub" || ext == "md"   || ext == "html") return "IconBookOpen";
    return "IconFileText";
}
```

- [ ] **Step 6: Build + run**

```bash
cd /stuff/Study/projects/hyprpdf
cmake --build build --target tst_recentfilesmodel 2>&1 | tail -10
ctest --test-dir build -R tst_recentfilesmodel --output-on-failure
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
cd /stuff/Study/projects/hyprpdf
git add src/models/recentfilesmodel.h src/models/recentfilesmodel.cpp \
        tests/tst_recentfilesmodel.cpp tests/CMakeLists.txt
git commit -m "$(cat <<'EOF'
feat: RecentFilesModel with JSON persistence

QAbstractListModel of recently-opened documents. addRecent dedupes by
path and moves the entry to the front, capped at 20 entries. Persisted
to ~/.local/share/hyprpdf/recent.json (path overridable for tests).
ExistsRole is false for paths whose file no longer resolves; the entry
is kept (not deleted) so the user can recover after a remount.
IconNameRole maps file extension to a quill-icons name.
EOF
)"
```

## Task 2.10: DocumentModel + sample.pdf fixture (TDD)

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/tests/fixtures/sample.pdf` (hand-written)
- Create: `/stuff/Study/projects/hyprpdf/src/models/documentmodel.h`
- Create: `/stuff/Study/projects/hyprpdf/src/models/documentmodel.cpp`
- Create: `/stuff/Study/projects/hyprpdf/tests/tst_documentmodel.cpp`
- Modify: `/stuff/Study/projects/hyprpdf/tests/CMakeLists.txt`

- [ ] **Step 1: Create the minimal sample.pdf fixture**

Write `/stuff/Study/projects/hyprpdf/tests/fixtures/sample.pdf` with this exact text content:

```
%PDF-1.4
1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj
2 0 obj<</Type/Pages/Count 1/Kids[3 0 R]>>endobj
3 0 obj<</Type/Page/Parent 2 0 R/MediaBox[0 0 612 792]/Resources<<>>>>endobj
xref
0 4
0000000000 65535 f
0000000009 00000 n
0000000052 00000 n
0000000093 00000 n
trailer<</Size 4/Root 1 0 R>>
startxref
158
%%EOF
```

Verify:

```bash
file /stuff/Study/projects/hyprpdf/tests/fixtures/sample.pdf
```

Expected: `PDF document, version 1.4`.

- [ ] **Step 2: Write the failing test at `tests/tst_documentmodel.cpp`**

```cpp
#include <QTest>
#include <QSignalSpy>
#include <QFile>
#include "models/documentmodel.h"

class TestDocumentModel : public QObject {
    Q_OBJECT
private:
    QString fixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
private slots:
    void initTestCase() {
        if (!QFile::exists(fixture()))
            QSKIP("sample.pdf fixture missing");
    }

    void openAddsRowAndDedupes() {
        DocumentModel m;
        const int idx1 = m.openDocument(fixture());
        QCOMPARE(idx1, 0);
        QCOMPARE(m.rowCount(), 1);
        const int idx2 = m.openDocument(fixture());
        QCOMPARE(idx2, 0);
        QCOMPARE(m.rowCount(), 1);
    }

    void closeTabRemoves() {
        DocumentModel m;
        m.openDocument(fixture());
        QCOMPARE(m.rowCount(), 1);
        m.closeTab(0);
        QCOMPARE(m.rowCount(), 0);
    }

    void currentIndexClamps() {
        DocumentModel m;
        m.openDocument(fixture());
        m.setCurrentIndex(99);
        QCOMPARE(m.currentIndex(), 0);
        m.setCurrentIndex(-5);
        QCOMPARE(m.currentIndex(), 0);
    }

    void openFailedEmitsOnBogusPath() {
        DocumentModel m;
        QSignalSpy spy(&m, &DocumentModel::openFailed);
        const int idx = m.openDocument("/no/such/file.pdf");
        QCOMPARE(idx, -1);
        QCOMPARE(spy.count(), 1);
    }
};
QTEST_MAIN(TestDocumentModel)
#include "tst_documentmodel.moc"
```

- [ ] **Step 3: Add test target — append to `tests/CMakeLists.txt`**

```cmake
hyprpdf_test(tst_documentmodel ${CMAKE_SOURCE_DIR}/src/models/documentmodel.cpp)
```

- [ ] **Step 4: Verify FAILS**

Build target — expect missing-header error.

- [ ] **Step 5: Write `src/models/documentmodel.h`**

```cpp
#pragma once
#include <QAbstractListModel>
#include <QPdfDocument>
#include <QString>
#include <QVector>
#include <memory>

class DocumentModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int count        READ rowCount     NOTIFY countChanged)
public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        TitleRole,
        PageCountRole,
        IsDirtyRole,
        DocumentRole
    };

    explicit DocumentModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int  currentIndex() const { return m_current; }
    void setCurrentIndex(int idx);

    Q_INVOKABLE int  openDocument(const QString &path);
    Q_INVOKABLE void closeTab(int index);

signals:
    void currentIndexChanged();
    void countChanged();
    void openFailed(QString path, QString reason);

private:
    struct Tab {
        QString path;
        QString title;
        std::unique_ptr<QPdfDocument> doc;
        bool dirty = false;
    };
    QVector<Tab*> m_tabs;
    int m_current = -1;
};
```

- [ ] **Step 6: Write `src/models/documentmodel.cpp`**

```cpp
#include "documentmodel.h"
#include <QFileInfo>
#include <QDebug>

DocumentModel::DocumentModel(QObject *parent) : QAbstractListModel(parent) {}

int DocumentModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_tabs.size();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_tabs.size()) return {};
    const Tab *t = m_tabs[index.row()];
    switch (role) {
        case PathRole:      return t->path;
        case TitleRole:     return t->title;
        case PageCountRole: return t->doc ? t->doc->pageCount() : 0;
        case IsDirtyRole:   return t->dirty;
        case DocumentRole:  return QVariant::fromValue<QObject*>(t->doc.get());
    }
    return {};
}

QHash<int, QByteArray> DocumentModel::roleNames() const {
    return {
        {PathRole,      "path"},
        {TitleRole,     "title"},
        {PageCountRole, "pageCount"},
        {IsDirtyRole,   "dirty"},
        {DocumentRole,  "document"}
    };
}

void DocumentModel::setCurrentIndex(int idx) {
    if (m_tabs.isEmpty()) {
        if (m_current != -1) { m_current = -1; emit currentIndexChanged(); }
        return;
    }
    const int clamped = qBound(0, idx, m_tabs.size() - 1);
    if (clamped == m_current) return;
    m_current = clamped;
    emit currentIndexChanged();
}

int DocumentModel::openDocument(const QString &path) {
    if (path.isEmpty()) {
        emit openFailed(path, "empty path");
        return -1;
    }
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i]->path == path) {
            setCurrentIndex(i);
            return i;
        }
    }
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isReadable()) {
        emit openFailed(path, "file not found or unreadable");
        return -1;
    }
    auto doc = std::make_unique<QPdfDocument>(this);
    auto err = doc->load(path);
    if (err != QPdfDocument::Error::None) {
        emit openFailed(path, QString("QPdfDocument error %1").arg(static_cast<int>(err)));
        return -1;
    }
    auto *t = new Tab{path, fi.fileName(), std::move(doc), false};
    beginInsertRows({}, m_tabs.size(), m_tabs.size());
    m_tabs.append(t);
    endInsertRows();
    emit countChanged();
    setCurrentIndex(m_tabs.size() - 1);
    return m_tabs.size() - 1;
}

void DocumentModel::closeTab(int index) {
    if (index < 0 || index >= m_tabs.size()) return;
    beginRemoveRows({}, index, index);
    delete m_tabs.takeAt(index);
    endRemoveRows();
    emit countChanged();
    if (m_tabs.isEmpty()) {
        m_current = -1;
        emit currentIndexChanged();
    } else {
        setCurrentIndex(qMin(m_current, m_tabs.size() - 1));
    }
}
```

- [ ] **Step 7: Build + run**

```bash
cd /stuff/Study/projects/hyprpdf
cmake --build build --target tst_documentmodel 2>&1 | tail -10
ctest --test-dir build -R tst_documentmodel --output-on-failure
```

Expected: PASS.

- [ ] **Step 8: Commit deferred — bundled with DocumentController in Task 2.11**

## Task 2.11: DocumentController (TDD)

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/src/services/documentcontroller.h`
- Create: `/stuff/Study/projects/hyprpdf/src/services/documentcontroller.cpp`
- Create: `/stuff/Study/projects/hyprpdf/tests/tst_documentcontroller.cpp`
- Modify: `/stuff/Study/projects/hyprpdf/tests/CMakeLists.txt`

- [ ] **Step 1: Write the failing test at `tests/tst_documentcontroller.cpp`**

```cpp
#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QFile>
#include "services/documentcontroller.h"
#include "models/documentmodel.h"

class TestDocumentController : public QObject {
    Q_OBJECT
private:
    QString fixture() const { return QString(HYPRPDF_FIXTURES_DIR) + "/sample.pdf"; }
private slots:
    void initTestCase() {
        if (!QFile::exists(fixture()))
            QSKIP("sample.pdf fixture missing");
    }

    void openPdfRoutesToModel() {
        DocumentModel model;
        DocumentController c(&model);
        c.open(fixture());
        QCOMPARE(model.rowCount(), 1);
    }

    void openOfficeEmitsFailed() {
        DocumentModel model;
        DocumentController c(&model);
        QSignalSpy spy(&c, &DocumentController::openFailed);
        c.open("/tmp/example.docx");
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().first().toString().contains("v0.5"));
    }

    void openFromUrlStripsFilePrefix() {
        DocumentModel model;
        DocumentController c(&model);
        c.openFromUrl(QUrl::fromLocalFile(fixture()));
        QCOMPARE(model.rowCount(), 1);
    }
};
QTEST_MAIN(TestDocumentController)
#include "tst_documentcontroller.moc"
```

- [ ] **Step 2: Add test target — append to `tests/CMakeLists.txt`**

```cmake
hyprpdf_test(tst_documentcontroller
    ${CMAKE_SOURCE_DIR}/src/services/documentcontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/models/documentmodel.cpp)
```

- [ ] **Step 3: Verify FAILS**

Build the target — expect missing-header error.

- [ ] **Step 4: Write `src/services/documentcontroller.h`**

```cpp
#pragma once
#include <QObject>
#include <QString>
#include <QUrl>

class DocumentModel;

class DocumentController : public QObject {
    Q_OBJECT
public:
    explicit DocumentController(DocumentModel *model, QObject *parent = nullptr);

    Q_INVOKABLE void open(const QString &path);
    Q_INVOKABLE void closeCurrent();
    Q_INVOKABLE void openFromUrl(const QUrl &url);

signals:
    void openFailed(QString reason);

private:
    DocumentModel *m_model;
};
```

- [ ] **Step 5: Write `src/services/documentcontroller.cpp`**

```cpp
#include "documentcontroller.h"
#include "models/documentmodel.h"
#include <QFileInfo>

DocumentController::DocumentController(DocumentModel *model, QObject *parent)
    : QObject(parent), m_model(model) {
    connect(m_model, &DocumentModel::openFailed, this,
            [this](const QString &p, const QString &r) {
                emit openFailed(p + ": " + r);
            });
}

void DocumentController::open(const QString &path) {
    if (path.isEmpty()) { emit openFailed("empty path"); return; }
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "pdf") {
        m_model->openDocument(path);
        return;
    }
    static const QStringList kOffice = {
        "pptx","ppt","odp","docx","doc","odt","rtf","xlsx","ods","csv",
        "epub","md","html"
    };
    if (kOffice.contains(ext)) {
        emit openFailed(QString("%1: office formats land in v0.5").arg(path));
        return;
    }
    emit openFailed(QString("%1: unsupported file type .%2").arg(path, ext));
}

void DocumentController::closeCurrent() {
    if (m_model->currentIndex() >= 0)
        m_model->closeTab(m_model->currentIndex());
}

void DocumentController::openFromUrl(const QUrl &url) {
    open(url.toLocalFile());
}
```

- [ ] **Step 6: Build + run**

```bash
cd /stuff/Study/projects/hyprpdf
cmake --build build --target tst_documentcontroller 2>&1 | tail -10
ctest --test-dir build -R tst_documentcontroller --output-on-failure
```

Expected: PASS.

- [ ] **Step 7: Bundle commit (DocumentModel + DocumentController + fixture)**

```bash
cd /stuff/Study/projects/hyprpdf
git add src/models/documentmodel.h src/models/documentmodel.cpp \
        src/services/documentcontroller.h src/services/documentcontroller.cpp \
        tests/tst_documentmodel.cpp tests/tst_documentcontroller.cpp \
        tests/fixtures/sample.pdf tests/CMakeLists.txt
git commit -m "$(cat <<'EOF'
feat: DocumentModel and DocumentController

DocumentModel is a QAbstractListModel of open PDF tabs. openDocument
loads via QPdfDocument, dedupes by path, and emits openFailed on errors.
closeTab destroys the document; setCurrentIndex clamps to the row range.
DocumentController is a thin facade that routes by extension: PDF goes
to the model, recognised office formats emit "land in v0.5", unknown
extensions emit "unsupported file type". openFromUrl strips file://.

Adds tests/fixtures/sample.pdf (minimal 1-page PDF, hand-written).
EOF
)"
```

## Task 2.12: Theme.qml singleton + register Quill module in CMake

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/src/qml/Theme.qml`
- Modify: `/stuff/Study/projects/hyprpdf/src/CMakeLists.txt`
- Modify: `/stuff/Study/projects/hyprpdf/CMakeLists.txt`

- [ ] **Step 1: Write `src/qml/Theme.qml`**

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

    function annot(name) { return theme.annotation(name) }
    function containerColor(c, a) {
        return Qt.rgba(c.r, c.g, c.b,
                       transparencyEnabled ? a * transparencyLevel : 1)
    }
}
```

- [ ] **Step 2: Replace `src/CMakeLists.txt` with**

```cmake
qt_add_executable(hyprpdf
    main.cpp
    services/themeloader.cpp
    services/configmanager.cpp
    services/documentcontroller.cpp
    models/documentmodel.cpp
    models/recentfilesmodel.cpp
)

target_link_libraries(hyprpdf PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
    Qt6::Pdf Qt6::Svg Qt6::Widgets Qt6::DBus
)

target_include_directories(hyprpdf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(hyprpdf
    URI HyprPDF
    VERSION 1.0
    NO_CACHEGEN
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/PdfView.qml
        qml/Toolbar.qml
        qml/TabsBar.qml
        qml/RecentFilesView.qml
    RESOURCE_PREFIX /
)

install(TARGETS hyprpdf RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/themes/
        DESTINATION share/hyprpdf/themes
        FILES_MATCHING PATTERN "*.toml")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/qml/Quill
        DESTINATION share/hyprpdf/src/qml)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/qml/icons
        DESTINATION share/hyprpdf/src/qml)
install(FILES ${CMAKE_SOURCE_DIR}/dist/hyprpdf.desktop
        DESTINATION share/applications)
```

- [ ] **Step 3: Update top-level `CMakeLists.txt` — add right after `find_package(Qt6 ...)`**

```cmake
set(QT_QML_IMPORT_PATH "${CMAKE_SOURCE_DIR}/src/qml" CACHE PATH "" FORCE)
set(QML_IMPORT_PATH    "${CMAKE_SOURCE_DIR}/src/qml" CACHE STRING "" FORCE)
```

(Leave existing `HYPRPDF_DATA_DIR` / `HYPRPDF_SOURCE_DIR` cache vars in place.)

- [ ] **Step 4: Build verification (no run yet — needs main.cpp + QML files in next tasks)**

```bash
cd /stuff/Study/projects/hyprpdf
rm -rf build
cmake -B build -G Ninja 2>&1 | tail -20
```

Expected: configure succeeds, no warnings about Theme.qml singleton registration. (Build will fail later because Main.qml references not-yet-written Toolbar/TabsBar/RecentFilesView — fine for now.)

(Commit deferred — bundled with QML in Task 2.14.)

## Task 2.13: Rewrite main.cpp with context properties + import paths

**Files:**
- Modify: `/stuff/Study/projects/hyprpdf/src/main.cpp`

- [ ] **Step 1: Replace `src/main.cpp` with**

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>

#include "services/themeloader.h"
#include "services/configmanager.h"
#include "services/documentcontroller.h"
#include "models/documentmodel.h"
#include "models/recentfilesmodel.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("HyprPDF");
    QGuiApplication::setApplicationVersion(HYPRPDF_VERSION);
    QGuiApplication::setOrganizationName("hyprpdf");
    QQuickStyle::setStyle("Basic");

    QCommandLineParser parser;
    parser.setApplicationDescription("HyprPDF - Qt6/QML PDF viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "PDF or office document to open", "[file]");
    parser.process(app);

    QString initialFile;
    const auto positional = parser.positionalArguments();
    if (!positional.isEmpty())
        initialFile = positional.first();

    auto *config        = new ConfigManager(QString{}, &app);
    auto *theme         = new ThemeLoader(&app);
    auto *recentFiles   = new RecentFilesModel(QString{}, &app);
    auto *documentModel = new DocumentModel(&app);
    auto *documentCtl   = new DocumentController(documentModel, &app);

    const QString themesDir = QString(HYPRPDF_DATA_DIR) + "/themes";
    theme->loadTheme(config->theme(), themesDir);
    QObject::connect(config, &ConfigManager::configChanged, theme, [theme, config, themesDir]() {
        theme->loadTheme(config->theme(), themesDir);
    });

    QQmlApplicationEngine engine;
    engine.addImportPath(QString(HYPRPDF_DATA_DIR) + "/src/qml");
    engine.addImportPath(QString(HYPRPDF_DATA_DIR));
    engine.addImportPath(QString(HYPRPDF_SOURCE_DIR) + "/src/qml");
    engine.addImportPath(QString(HYPRPDF_SOURCE_DIR));

    auto *ctx = engine.rootContext();
    ctx->setContextProperty("config",             config);
    ctx->setContextProperty("theme",              theme);
    ctx->setContextProperty("recentFiles",        recentFiles);
    ctx->setContextProperty("documentModel",      documentModel);
    ctx->setContextProperty("documentController", documentCtl);

    QObject::connect(recentFiles, &RecentFilesModel::requestOpen,
                     documentCtl, &DocumentController::open);
    QObject::connect(documentModel, &DocumentModel::countChanged,
                     recentFiles, [recentFiles, documentModel]() {
                         if (documentModel->rowCount() == 0) return;
                         const auto idx = documentModel->index(documentModel->rowCount() - 1, 0);
                         const auto path = idx.data(DocumentModel::PathRole).toString();
                         if (!path.isEmpty()) recentFiles->addRecent(path);
                     });

    engine.setInitialProperties({{"initialFile", initialFile}});
    engine.loadFromModule("HyprPDF", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
```

(Build will still fail until Task 2.14 lands the QML files. Commit deferred — bundled in Task 2.14.)

## Task 2.14: QML chrome — Toolbar, TabsBar, RecentFilesView, Main rewrite, PdfView update

**Files:**
- Create: `/stuff/Study/projects/hyprpdf/src/qml/Toolbar.qml`
- Create: `/stuff/Study/projects/hyprpdf/src/qml/TabsBar.qml`
- Create: `/stuff/Study/projects/hyprpdf/src/qml/RecentFilesView.qml`
- Modify: `/stuff/Study/projects/hyprpdf/src/qml/Main.qml`
- Modify: `/stuff/Study/projects/hyprpdf/src/qml/PdfView.qml`

**IMPORTANT BEFORE STARTING THIS TASK:** Read `/stuff/Study/projects/hyprpdf/src/qml/Quill/components/IconButton.qml` to confirm the property API. The plan below assumes:
- `IconButton` accepts `tooltip: string`, `onClicked: signal`, and either `icon: string` or a `contentItem` override.

If `IconButton` does NOT accept a `contentItem`, fall back to a plain composition: `Rectangle { MouseArea { onClicked: ... }; Q.Tooltip { text: ... }; <iconComponent> { ... } }`. Update Toolbar.qml/TabsBar.qml accordingly.

- [ ] **Step 1: Verify Quill IconButton API**

```bash
cat /stuff/Study/projects/hyprpdf/src/qml/Quill/components/IconButton.qml | head -40
```

Decide whether to use `contentItem` override or fallback composition based on what you see.

- [ ] **Step 2: Write `src/qml/Toolbar.qml`**

```qml
import QtQuick
import QtQuick.Layouts
import Quill 1.0 as Q
import icons 1.0 as Ico
import HyprPDF 1.0

Rectangle {
    id: root
    implicitHeight: Theme.toolbarRowHeight
    color: Theme.mantle

    signal openRequested()
    signal zoomInRequested()
    signal zoomOutRequested()
    signal fitPageRequested()

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing
        anchors.rightMargin: Theme.spacing
        spacing: Theme.spacing

        Q.IconButton {
            tooltip: "Open (Ctrl+O)"
            onClicked: root.openRequested()
            contentItem: Ico.IconFolder { size: 18; color: Q.Theme.textPrimary }
        }
        Q.IconButton {
            tooltip: "Zoom in (Ctrl++)"
            onClicked: root.zoomInRequested()
            contentItem: Ico.IconZoomIn { size: 18; color: Q.Theme.textPrimary }
        }
        Q.IconButton {
            tooltip: "Zoom out (Ctrl+-)"
            onClicked: root.zoomOutRequested()
            contentItem: Ico.IconZoomOut { size: 18; color: Q.Theme.textPrimary }
        }
        Q.IconButton {
            tooltip: "Fit page (Ctrl+2)"
            onClicked: root.fitPageRequested()
            contentItem: Ico.IconMaximize { size: 18; color: Q.Theme.textPrimary }
        }

        Item { Layout.fillWidth: true }

        Q.IconButton {
            tooltip: "Settings (stub - v0.6)"
            enabled: false
            contentItem: Ico.IconSettings { size: 18; color: Q.Theme.textTertiary }
        }
    }
}
```

- [ ] **Step 3: Write `src/qml/TabsBar.qml`**

```qml
import QtQuick
import QtQuick.Layouts
import Quill 1.0 as Q
import icons 1.0 as Ico
import HyprPDF 1.0

Rectangle {
    id: root
    visible: documentModel.count > 0
    implicitHeight: visible ? 32 : 0
    color: Theme.crust

    signal openRequested()

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Repeater {
            model: documentModel
            delegate: Rectangle {
                Layout.preferredWidth: 180
                Layout.fillHeight: true
                color: index === documentModel.currentIndex ? Theme.surface : Theme.mantle
                border.color: Theme.crust
                border.width: 1
                MouseArea {
                    anchors.fill: parent
                    onClicked: documentModel.currentIndex = index
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 4
                    spacing: 6
                    Text {
                        Layout.fillWidth: true
                        text: model.title
                        color: Theme.text
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    Q.IconButton {
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18
                        tooltip: "Close tab"
                        onClicked: documentModel.closeTab(index)
                        contentItem: Ico.IconX { size: 12; color: Q.Theme.textSecondary }
                    }
                }
            }
        }

        Q.IconButton {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            tooltip: "Open another document (Ctrl+T)"
            onClicked: root.openRequested()
            contentItem: Ico.IconPlus { size: 14; color: Q.Theme.textPrimary }
        }

        Item { Layout.fillWidth: true }
    }
}
```

- [ ] **Step 4: Write `src/qml/RecentFilesView.qml`**

```qml
import QtQuick
import QtQuick.Layouts
import Quill 1.0 as Q
import icons 1.0 as Ico
import HyprPDF 1.0

Item {
    id: root
    signal openRequested()

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(640, parent.width - 64)
        spacing: 16

        Text {
            text: "HyprPDF"
            color: Theme.text
            font.pixelSize: 28
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            visible: recentFiles.rowCount === 0
            text: "No recent documents.  Press Ctrl+O to open one."
            color: Theme.subtext
            Layout.alignment: Qt.AlignHCenter
        }
        Q.Button {
            visible: recentFiles.rowCount === 0
            text: "Open document..."
            onClicked: root.openRequested()
            Layout.alignment: Qt.AlignHCenter
        }

        Q.ScrollableList {
            visible: recentFiles.rowCount > 0
            Layout.fillWidth: true
            Layout.preferredHeight: 360
            model: recentFiles
            delegate: Q.Card {
                width: ListView.view.width
                opacity: model.exists ? 1.0 : 0.55
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: recentFiles.open(index)
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12
                    Loader {
                        sourceComponent: model.iconName === "IconLayoutGrid" ? layoutGrid
                                       : model.iconName === "IconBookOpen"   ? bookOpen
                                       : fileText
                    }
                    Component { id: fileText;   Ico.IconFileText  { size: 24; color: Q.Theme.textPrimary } }
                    Component { id: layoutGrid; Ico.IconLayoutGrid{ size: 24; color: Q.Theme.textPrimary } }
                    Component { id: bookOpen;   Ico.IconBookOpen  { size: 24; color: Q.Theme.textPrimary } }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text { text: model.name; color: Theme.text;    font.pixelSize: Theme.fontNormal }
                        Text { text: model.path; color: Theme.subtext; font.pixelSize: Theme.fontSmall;
                               elide: Text.ElideMiddle; Layout.fillWidth: true }
                    }
                }
            }
        }
    }
}
```

- [ ] **Step 5: Replace `src/qml/Main.qml`**

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quill 1.0 as Q
import HyprPDF 1.0

ApplicationWindow {
    id: root
    width: 1200
    height: 800
    visible: true
    color: Theme.base
    title: documentModel.count === 0 ? "HyprPDF"
        : (documentModel.index(documentModel.currentIndex, 0).data(258) + " - HyprPDF")

    property string initialFile: ""

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

        if (initialFile.length > 0)
            documentController.open(initialFile)
    }

    Connections {
        target: documentController
        function onOpenFailed(reason) { console.warn("HyprPDF open failed:", reason) }
    }

    FileDialog {
        id: openDialog
        title: "Open document"
        nameFilters: [
            "Documents (*.pdf *.pptx *.ppt *.odp *.docx *.doc *.odt *.rtf *.epub *.md *.html)",
            "PDF (*.pdf)",
            "All files (*)"
        ]
        onAccepted: documentController.openFromUrl(selectedFile)
    }

    function curView() {
        if (documentModel.count === 0) return null
        if (stack.currentIndex <= 0) return null
        return stack.children[stack.currentIndex]
    }

    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem { text: "Open...";    shortcut: "Ctrl+O"; onTriggered: openDialog.open() }
            MenuItem { text: "New tab";    shortcut: "Ctrl+T"; onTriggered: openDialog.open() }
            MenuItem { text: "Close tab";  shortcut: "Ctrl+W"; onTriggered: documentController.closeCurrent() }
            MenuSeparator {}
            MenuItem { text: "Quit";       shortcut: "Ctrl+Q"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "&View"
            MenuItem { text: "Zoom in";    shortcut: "Ctrl++"; onTriggered: { var v=curView(); if (v && v.zoomIn) v.zoomIn() } }
            MenuItem { text: "Zoom out";   shortcut: "Ctrl+-"; onTriggered: { var v=curView(); if (v && v.zoomOut) v.zoomOut() } }
            MenuItem { text: "Fit width";  shortcut: "Ctrl+1"; onTriggered: { var v=curView(); if (v && v.fitWidth) v.fitWidth() } }
            MenuItem { text: "Fit page";   shortcut: "Ctrl+2"; onTriggered: { var v=curView(); if (v && v.fitPage) v.fitPage() } }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Toolbar {
            Layout.fillWidth: true
            onOpenRequested: openDialog.open()
            onZoomInRequested:  { var v=curView(); if (v) v.zoomIn() }
            onZoomOutRequested: { var v=curView(); if (v) v.zoomOut() }
            onFitPageRequested: { var v=curView(); if (v) v.fitPage() }
        }
        TabsBar {
            Layout.fillWidth: true
            onOpenRequested: openDialog.open()
        }

        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: documentModel.count === 0 ? 0 : (documentModel.currentIndex + 1)

            RecentFilesView { onOpenRequested: openDialog.open() }

            Repeater {
                model: documentModel
                delegate: PdfView { document: model.document }
            }
        }
    }

    Shortcut { sequence: "j";      onActivated: { var v=curView(); if (v && v.scrollBy) v.scrollBy(80) } }
    Shortcut { sequence: "k";      onActivated: { var v=curView(); if (v && v.scrollBy) v.scrollBy(-80) } }
    Shortcut { sequence: "PgDown"; onActivated: { var v=curView(); if (v && v.nextPage) v.nextPage() } }
    Shortcut { sequence: "PgUp";   onActivated: { var v=curView(); if (v && v.prevPage) v.prevPage() } }
}
```

- [ ] **Step 6: Replace `src/qml/PdfView.qml`**

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Pdf
import HyprPDF 1.0

Item {
    id: root

    property var document: null
    property string documentTitle: document && document.status === PdfDocument.Ready ? document.title : ""

    function zoomIn()   { view.renderScale *= 1.2 }
    function zoomOut()  { view.renderScale /= 1.2 }
    function fitWidth() {
        if (!document) return
        view.renderScale = width / Math.max(1, document.maxPageWidth)
    }
    function fitPage() {
        if (!document) return
        const s1 = width  / Math.max(1, document.maxPageWidth)
        const s2 = height / Math.max(1, document.maxPageHeight)
        view.renderScale = Math.min(s1, s2)
    }
    function scrollBy(dy)  { view.contentY = Math.max(0, view.contentY + dy) }
    function nextPage()    { if (document) view.goToPage(Math.min(document.pageCount - 1, view.currentPage + 1)) }
    function prevPage()    { if (document) view.goToPage(Math.max(0, view.currentPage - 1)) }

    PdfMultiPageView {
        id: view
        anchors.fill: parent
        document: root.document
    }

    Label {
        anchors.centerIn: parent
        visible: !root.document || root.document.status !== PdfDocument.Ready
        text: !root.document ? "" : (root.document.status === PdfDocument.Null ? "No document" : "Loading...")
        opacity: 0.6
        color: Theme.subtext
    }
}
```

- [ ] **Step 7: Build + smoke run**

```bash
cd /stuff/Study/projects/hyprpdf
cmake --build build --target hyprpdf 2>&1 | tail -20
./build/src/hyprpdf tests/fixtures/sample.pdf &
APP_PID=$!
sleep 3
kill $APP_PID 2>/dev/null
```

Expected: window opens, no QML errors in stderr (a few warnings about empty MediaBox content from the minimal sample.pdf are acceptable), sample.pdf visible briefly.

If QML errors mention `Quill.IconButton: Cannot assign to non-existent property "contentItem"`, swap the Toolbar.qml/TabsBar.qml IconButton blocks for the fallback composition described in Step 1 and rebuild.

- [ ] **Step 8: Commit Theme.qml + Main.qml + QML files + main.cpp + CMake updates**

```bash
cd /stuff/Study/projects/hyprpdf
git add src/qml/Theme.qml src/qml/Main.qml src/qml/PdfView.qml \
        src/qml/Toolbar.qml src/qml/TabsBar.qml src/qml/RecentFilesView.qml \
        src/CMakeLists.txt CMakeLists.txt src/main.cpp
git commit -m "$(cat <<'EOF'
feat: Theme.qml singleton, Quill bridge, Toolbar, TabsBar, RecentFilesView

Theme.qml is a QtObject singleton bound to the ThemeLoader (theme.*) and
ConfigManager (config.*) context properties. Main.qml's Component.onCompleted
copies Theme.* into Quill.Theme via Qt.binding(() => ...) so all Quill
components inherit the active palette and re-render on theme change.

ColumnLayout: Toolbar (open + zoom + fit) -> TabsBar (per-tab close, +
button) -> StackLayout. When documentModel.count == 0 the stack shows
RecentFilesView; otherwise it shows one PdfView per open tab.

PdfView no longer owns its QPdfDocument; it accepts a `document` property
bound from documentModel rows. main.cpp wires backend services as engine
context properties and adds source-tree + installed import paths. Top-level
CMake sets QT_QML_IMPORT_PATH so Quill + icons submodules resolve.
EOF
)"
```

## Task 2.15: Final clean rebuild + ctest + push

- [ ] **Step 1: Clean rebuild + full test suite**

```bash
cd /stuff/Study/projects/hyprpdf
rm -rf build
cmake -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected: 5/5 tests pass.

- [ ] **Step 2: Smoke test main binary against fixture**

```bash
cd /stuff/Study/projects/hyprpdf
./build/src/hyprpdf tests/fixtures/sample.pdf &
APP_PID=$!
sleep 3
kill $APP_PID 2>/dev/null
```

Expected: window opens with sample.pdf visible, dark Catppuccin Mocha background, no QML import errors in stderr.

- [ ] **Step 3: Smoke test no-arg launch**

```bash
cd /stuff/Study/projects/hyprpdf
./build/src/hyprpdf &
APP_PID=$!
sleep 3
kill $APP_PID 2>/dev/null
```

Expected: window opens with RecentFilesView; the previously-opened sample.pdf appears in the recent list.

- [ ] **Step 4: Push everything to GitHub**

```bash
cd /stuff/Study/projects/hyprpdf
git status
git log --oneline
git push -u origin main
```

Expected: push succeeds. `gh repo view soyeb-jim285/hyprpdf` shows all commits.

- [ ] **Step 5: Verify no Co-Authored-By trailers + no Claude attribution in any commit**

```bash
cd /stuff/Study/projects/hyprpdf
git log --all --pretty=format:'%B' | grep -i 'co-authored-by' && echo "FAIL: trailer found" || echo "OK: no trailer"
git log --all --pretty=format:'%B' | grep -i 'claude' && echo "FAIL: Claude attribution found" || echo "OK: no Claude attribution"
```

Expected: both lines print `OK: ...`.

---

## Self-Review Notes

**Spec coverage:**
- Sub-project 1 — Tasks 1.1-1.4.
- ThemeLoader, ConfigManager, DocumentController — Tasks 2.7, 2.8, 2.11.
- DocumentModel, RecentFilesModel — Tasks 2.10, 2.9.
- Theme.qml + Quill bridge — Tasks 2.12, 2.14.
- Toolbar, TabsBar, RecentFilesView — Task 2.14.
- main.cpp rewrite — Task 2.13.
- Themes (default + latte) — Task 2.6.
- Tests (5 files) — Tasks 2.7, 2.8, 2.9, 2.10, 2.11.
- toml.hpp vendor — Task 2.5.
- Submodules — Task 2.4.
- License + .gitignore + git init + GitHub repo — Tasks 2.1, 2.2, 2.3.
- CMake install layout — Task 2.12.
- `prefer :/HyprPDF/` strip — NOT in plan; spec says it's a HyprFM bug fix relevant only after `make install` to a system prefix. v0.1 dev workflow uses in-tree build, so deferred to v0.6 packaging milestone. Acceptable.
- Verification (cmake build + ctest + smoke) — Task 2.15.
- Final push — Task 2.15.

**Placeholder scan:** no TBD/TODO. All test/header/cpp/CMake snippets are concrete. Toolbar.qml has a flagged-NOTE step (Task 2.14 Step 1 + Step 7 fallback) about IconButton API — this is conditional, not a placeholder, and the fallback is named.

**Type consistency:** verified. `RecentFilesModel::PathRole == Qt::UserRole + 1`, `DocumentModel::PathRole == Qt::UserRole + 1` (TitleRole == 258 used in Main.qml title binding). Signal signatures: `RecentFilesModel::requestOpen(QString)`, `DocumentModel::openFailed(QString,QString)`, `DocumentController::openFailed(QString)` — distinct on purpose; the controller flattens the model's (path, reason) pair into one string.

**Commit chain:** 9 commits in Phase 2 (chore-import, chore-submodules, chore-toml, feat-themeloader, feat-configmanager, feat-recentfiles, feat-document-model+controller, feat-qml-everything, plus the LICENSE that lands inside chore-import via gh repo create). Slight rearrangement from spec for git-flow practicality.

---

## Plan complete

Plan saved to `/stuff/Study/projects/hyprpdf/docs/superpowers/plans/2026-04-19-icons-and-skeleton.md`.

Two execution options:

1. **Subagent-Driven (recommended)** — fresh subagent per task, two-stage review between tasks, fast iteration.

2. **Inline Execution** — execute tasks in this session using `superpowers:executing-plans`, batched checkpoints for review.

Which approach?
