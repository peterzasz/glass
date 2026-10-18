#include "cutter.h"

bool CutGeometry::defect_in_node(const Node& node, const Defect& defect) const
{
    return node.x <= defect.x &&
           node.y <= defect.y &&
           defect.x + defect.w <= node.x + node.w &&
           defect.y + defect.h <= node.y + node.h;
}

bool CutGeometry::item_in_node(const Node& node, int x, int y, const Item& item) const
{
    return node.x <= x &&
           node.y <= y &&
           x + item.w <= node.x + node.w &&
           y + item.h <= node.y + node.h;
}

bool CutGeometry::item_touches_defect(
    int x,
    int y,
    const Item& item,
    const Defect& defect) const
{
    return x < defect.x + defect.w &&
           y < defect.y + defect.h &&
           defect.x < x + item.w &&
           defect.y < y + item.h;
}

bool CutGeometry::x_cut_touches_defect(int x, const Defect& defect) const
{
    return defect.x < x && x < defect.x + defect.w;
}

bool CutGeometry::y_cut_touches_defect(int y, const Defect& defect) const
{
    return defect.y < y && y < defect.y + defect.h;
}

bool CutGeometry::can_cut(const Node& node, int x, int y, const Item& item) const
{
    if(!item_in_node(node, x, y, item))
    {
        return false;
    }

    for(const Defect& defect : _bin.defects)
    {
        if(defect_in_node(node, defect) && item_touches_defect(x, y, item, defect))
        {
            return false;
        }
    }

    if(node.cut == 4)
    {
        return item.w == node.w && item.h == node.h;
    }

    if(node.cut % 2 == 0)
    {
        if(!x_cut_avoids_defects(x, node) || !x_cut_avoids_defects(x + item.w, node))
        {
            return false;
        }
    }
    else
    {
        if(!y_cut_avoids_defects(y, node) || !y_cut_avoids_defects(y + item.h, node))
        {
            return false;
        }
    }

    // A fourth-level cut can only separate the item from the bottom or top
    // of a full-width third-level strip.
    if(node.cut == 3)
    {
        if(node.x != x || node.x + node.w != x + item.w)
        {
            return false;
        }

        if(node.y != y && node.y + node.h != y + item.h)
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::no_defect_in_node(const Node& node) const
{
    for(const Defect& defect : _bin.defects)
    {
        if(defect_in_node(node, defect))
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::x_cut_avoids_defects(int x, const Node& node) const
{
    for(const Defect& defect : _bin.defects)
    {
        if(defect_in_node(node, defect) && x_cut_touches_defect(x, defect))
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::y_cut_avoids_defects(int y, const Node& node) const
{
    for(const Defect& defect : _bin.defects)
    {
        if(defect_in_node(node, defect) && y_cut_touches_defect(y, defect))
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::valid_x_cut(
    int x,
    int y,
    int x_from,
    int y_from,
    const Node& node,
    const Item& item) const
{
    return !(
        (x > x_from && x - x_from < min_waste) ||
        (x + item.w < node.x + node.w && node.x + node.w - x - item.w < min_waste) ||
        (y > y_from && y - y_from < min_waste) ||
        (y + item.h < node.y + node.h && node.y + node.h - y - item.h < min_waste) ||
        (node.cut == 0 && x > x_from &&
            (x - x_from < min_1_cut || x - x_from > max_1_cut)) ||
        (node.cut == 0 && (item.w < min_1_cut || item.w > max_1_cut))
    );
}

bool CutGeometry::valid_y_cut(
    int x,
    int y,
    int x_from,
    int y_from,
    const Node& node,
    const Item& item) const
{
    return !(
        (x > x_from && x - x_from < min_waste) ||
        (x + item.w < node.x + node.w && node.x + node.w - x - item.w < min_waste) ||
        (y > y_from && y - y_from < min_waste) ||
        (y + item.h < node.y + node.h && node.y + node.h - y - item.h < min_waste) ||
        (node.cut == 1 && item.h < min_2_cut)
    );
}

bool CutGeometry::valid_4_cut(int node_h, int item_h) const
{
    return node_h - item_h >= min_waste;
}

bool CutGeometry::passes_min_waste_constraint(int coord, int coord_from) const
{
    return !(coord > coord_from && coord - coord_from < min_waste);
}

bool CutGeometry::rotations_valid_y_cut(
    const Node& node,
    const Item& item,
    int y) const
{
    return item_in_node(node, node.x, y, item) &&
           !(y + item.h < node.y + node.h &&
             node.y + node.h - y - item.h < min_waste) &&
           !(node.cut == 1 && item.h < min_2_cut) &&
           y_cut_avoids_defects(y, node) &&
           y_cut_avoids_defects(y + item.h, node);
}

bool CutGeometry::rotations_valid_x_cut(
    const Node& node,
    const Item& item,
    int x,
    int x_from) const
{
    return item_in_node(node, x, node.y, item) &&
           !(x + item.w < node.x + node.w &&
             node.x + node.w - x - item.w < min_waste) &&
           !(node.cut == 0 && (item.w < min_1_cut || item.w > max_1_cut)) &&
           !(node.cut == 0 && x > x_from &&
             (x - x_from < min_1_cut || x - x_from > max_1_cut)) &&
           x_cut_avoids_defects(x, node) &&
           x_cut_avoids_defects(x + item.w, node);
}

bool CutGeometry::can_do_bottom_4_cut(const Node& node, const Item& item) const
{
    for(const Defect& defect : _bin.defects)
    {
        if(defect_in_node(node, defect) &&
           item_touches_defect(node.x, node.y, item, defect))
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::can_do_top_4_cut(const Node& node, const Item& item) const
{
    for(const Defect& defect : _bin.defects)
    {
        if(defect_in_node(node, defect) &&
           item_touches_defect(node.x, node.y + node.h - item.h, item, defect))
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::passes_4_cut_min_waste_constraint(
    const Node& node,
    const Item& item) const
{
    return node.h - item.h >= min_waste;
}
