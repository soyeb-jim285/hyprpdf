# HyprPDF

A fast, keyboard-first Qt6/QML PDF viewer with annotation and page-editing tools. Sibling project to [HyprFM](../hyprfm).

## Status

Early scaffold. See [OUTLINE.md](OUTLINE.md) for the full plan, feature matrix, and roadmap.

## Features (planned)

- View PDF, PPTX, DOCX, ODT, ODP and more (office formats convert via LibreOffice).
- Annotate: highlight, underline, strikeout, sticky notes, freehand ink.
- Edit page structure: delete, rotate, reorder, merge, split, extract pages.
- Full-text search, outline/bookmarks panel, thumbnail sidebar.
- Tabs, dark mode, TOML themes, Wayland-first.
- Shared Quill QML components and icon set with HyprFM.

## Build

```bash
cmake -B build -G Ninja
cmake --build build
./build/src/hyprpdf
```

## Dependencies

Arch:

```bash
sudo pacman -S qt6-base qt6-declarative qt6-quickcontrols2 qt6-pdf qt6-svg \
               qpdf poppler-qt6 libreoffice-fresh cmake ninja
```

## License

TBD — see [OUTLINE.md §Licensing](OUTLINE.md#licensing).
