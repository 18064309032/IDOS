# IDOS SVG icons

Original project vector assets. Filenames use `<module>-<purpose>.svg`.

- 24 x 24 viewBox, transparent background, 1.65 px rounded strokes.
- Simple SVG primitives with explicit colors; no scripts, external resources, fonts or filters.
- Designed for 16/24/32 px UI use. Review `preview.html` for all sizes.
- `gui-*`: business trees and well/grid/case nodes; `app-*`: application actions.
- `core-*`, `providers-*`, `analysis-*`, `render-*`, `python-*`, `assistant-*`: module capabilities.
- `render-*` represents the current render module; future split modules can reuse these assets.
- All entries and their purposes are listed in `manifest.json`.
- `images.qrc` embeds all icons in `idos_gui` under `:/images/`.
- Existing data/well/grid/case tree nodes, New Project and New Well actions, and dock panels use these icons.
- Icons for features without an existing UI entry remain available for future use.
