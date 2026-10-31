#include "fillertypes.h"

std::ostream& operator<<(std::ostream& os, CutMethod method)
{
    switch(method)
    {
        case CutMethod::NoRotation:
            return os << "NoRotation";

        case CutMethod::Rotation:
            return os << "Rotation";
    }

    return os << "UnknownOrdering";
}

std::ostream& operator<<(std::ostream& os, FillerType filler)
{
    return os << "Ordering: " << filler.ordering << " Method: " << filler.method;
}

std::ostream& operator<<(std::ostream& os, Ordering ordering)
{
    switch(ordering)
    {
        case Ordering::AreaOrder:
            return os << "AreaOrder";

        case Ordering::HeightWidth:
            return os << "HeightWidth";

        case Ordering::WidthHeight:
            return os << "WidthHeight";
        
        case Ordering::NoOrder:
            return os << "NoOrder";
    }

    return os << "UnknownOrdering";
}

std::ostream& operator<<(std::ostream& os, FillMethod method)
{
    switch(method)
    {
        case FillMethod::NoRotation:
            return os << "NoRotation";

        case FillMethod::Rotation:
            return os << "Rotation";

        case FillMethod::PlacementLookahead:
            return os << "PlacementLookahead";
        
        case FillMethod::PlacementLookahead2:
            return os << "PlacementLookahead2";

        case FillMethod::OrderLookahead:
            return os << "OrderLookahead";
    }

    return os << "UnknownOrdering";
}