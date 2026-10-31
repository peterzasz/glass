#ifndef FILLERTYPES_H
#define FILLERTYPES_H

#include <ostream>

enum class FillMethod
{
    NoRotation,
    Rotation,
    PlacementLookahead,
    PlacementLookahead2,
    OrderLookahead
};

std::ostream& operator<<(std::ostream& os, FillMethod method);

enum class Ordering
{
    NoOrder,
    AreaOrder,
    WidthHeight,
    HeightWidth
};

std::ostream& operator<<(std::ostream& os, Ordering ordering);

struct FillerType
{
    Ordering ordering;
    FillMethod method;
};

std::ostream& operator<<(std::ostream& os, FillerType filler);

enum class CutMethod
{
    NoRotation,
    Rotation
};

std::ostream& operator<<(std::ostream& os, CutMethod method);

#endif