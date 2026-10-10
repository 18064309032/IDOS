#ifndef IDOS_RENDER_TYPES_H
#define IDOS_RENDER_TYPES_H

enum class IDOSItemState
{
    Shown,
    Hidden,
    Selected,
    Unselected,
    Highlighted,
    Unhighlighted
};

enum class IDOSColorMap
{
    Grayscale,
    Rainbow,
    BlueRed,
    Viridis,
    Plasma,
    Thermal
};

enum class IDOSDisplayMode
{
    Surface,
    Wireframe,
    SurfaceWithEdges,
    Points
};

enum class IDOSOrientation
{
    Perspective,
    Top,
    Bottom,
    Front,
    Back,
    Left,
    Right,
    Isometric
};

#endif // IDOS_RENDER_TYPES_H

