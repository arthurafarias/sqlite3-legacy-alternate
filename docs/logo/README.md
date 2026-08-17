# Logo

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="mark-dark.svg">
    <img src="mark-light.svg" alt="sqlite3-legacy-amalgamation mark" width="220">
  </picture>
</p>

## Concept

SQLite ships as **one file**: the amalgamation, `sqlite3.c`. This project's whole
job is to take that single file apart — one struct, function, or typedef per
`.c`/`.h` pair — and keep it building as a real library while it does.

The mark is a database drawn as a mosaic instead of a solid shape: the same
tile grid that fills the cylinder also appears, individually, breaking off to
its right — some still tethered by a dotted line back to the slot they came
from, the furthest ones fully loose and fading. That is the project in one
image: a monolith being read apart into standalone, individually-named
pieces, mid-process rather than finished.

It is an original mark for this repository, not a reuse or reinterpretation
of the official SQLite logo/wordmark.

## Files

| File | Use |
|---|---|
| [`mark-light.svg`](mark-light.svg) | Default mark, tuned for light backgrounds |
| [`mark-dark.svg`](mark-dark.svg) | Same mark, tuned for dark backgrounds |

Both are plain vector shapes (no embedded fonts, no external references), so
they render anywhere an `<img>` or inline SVG works, including GitHub's
Markdown renderer.

## Usage

Pair the two files with a `<picture>` element so the mark follows the
viewer's color scheme automatically:

```html
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/logo/mark-dark.svg">
  <img src="docs/logo/mark-light.svg" alt="sqlite3-legacy-amalgamation" width="160">
</picture>
```

This is exactly how the root [README.md](../../README.md) uses it.

## Palette

| Role | Light | Dark |
|---|---|---|
| Monolith tiles | `#2f6fb2` / `#3a7dc4` | `#5b9bd9` / `#6ba8e0` |
| Cylinder outline | `#1c4d80` | `#bcdcf7` |
| Detached files | `#e0862c` / `#ec9a45` | `#f6a94f` / `#f8bb70` |
| Detached-file outline | `#b5661a` | `#ffd9a3` |
| Leader lines | `#b7c7d8` | `#3a4a5c` |

Blue stays reserved for the still-amalgamated mass; the warm accent is only
ever used for a piece that has (or is about to) become its own file. Keep
that split if the mark is ever extended (e.g. an animated or interactive
variant) — mixing the two palettes across the same element muddies the "still
combined vs. already split out" reading the mark exists to carry.

## Regenerating

Both files are generated, not hand-drawn — tile positions are computed
programmatically to stay evenly spaced inside the cylinder silhouette. There
is no build-tracked generator script in this repository; treat the two SVGs
as the source of truth and hand-edit them (or write a fresh generator) if the
mark needs to change.
