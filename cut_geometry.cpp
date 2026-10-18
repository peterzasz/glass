#include "solve.h"
#include "data.h"
#include "cutter.h"

constexpr int min_1_cut = 100;
constexpr int max_1_cut = 3500;
constexpr int min_2_cut = 100;
constexpr int min_waste = 20;

bool Cutter::defect_in_node(const Node& node, const Defect& defect) const
{
    return node.x <= defect.x &&
           node.y <= defect.y && 
           defect.x + defect.w <= node.x + node.w &&
           defect.y + defect.h <= node.y + node.h;
}

bool Cutter::item_in_node(const Node& node, int x, int y, const Item& it) const
{
    return node.x <= x &&
           node.y <= y &&
           x + it.w <= node.x + node.w &&
           y + it.h <= node.y + node.h;
}

bool Cutter::item_touches_defect(int x, int y, const Item& it, const Defect& defect) const
{
    return x < defect.x + defect.w &&
           y < defect.y + defect.h &&
           defect.x < x + it.w &&
           defect.y < y + it.h;
}

bool Cutter::x_cut_touches_defect(int x, const Defect& defect) const
{
    return defect.x < x && x < defect.x + defect.w;
}

bool Cutter::y_cut_touches_defect(int y, const Defect& defect) const
{
    return defect.y < y && y < defect.y + defect.h;
}

bool Cutter::can_cut(const Bin& bin, const Node& node, int x, int y, const Item& it)
{
    if(!item_in_node(node,x,y,it))
    {
        return false;
    }

    for(const auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect) && item_touches_defect(x,y,it,defect))
        {
            return false;
        }
    }

    if(node.cut == 4)
    {
        if(it.w == node.w && it.h == node.h)
        {
            return true;
        }

        return false;
    }

    if( node.cut%2 == 0 )
    {
        for(const auto& defect : bin.defects)
        {
            if(defect_in_node(node,defect) && (x_cut_touches_defect( x, defect ) || x_cut_touches_defect( x + it.w, defect )))
            {
                return false;
            }
        }
    }
    else
    {
        for(const auto& defect : bin.defects)
        {
            if(defect_in_node(node,defect) && (y_cut_touches_defect( y, defect ) || y_cut_touches_defect( y + it.h, defect )))
            {
                return false;
            }
        }
    }

    if( node.cut == 3 )
    {
        if( node.x != x || node.x + node.w != x + it.w )
        {
            return false;
        }

        if( node.y != y && node.y + node.h != y + it.h )
        {
            return false;
        }
    }

    return true;
}

bool Cutter::valid_x_cut(int x, int y, int x_from, int y_from, const Node& node, const Item& it) const
{
    return !(   (x > x_from && x - x_from < min_waste) ||
                (x + it.w < node.x + node.w && node.x+node.w-x-it.w < min_waste) ||
                (y > y_from && y - y_from < min_waste) ||
                (y + it.h < node.y + node.h && node.y + node.h - y - it.h < min_waste) ||
                (node.cut == 0 && x > x_from && (x-x_from < min_1_cut || x-x_from > max_1_cut))
            );
}

bool Cutter::valid_y_cut(int x, int y, int x_from, int y_from, const Node& node, const Item& it) const
{
    return !(   (x > x_from && x - x_from < min_waste) ||
                (x + it.w < node.x + node.w && node.x+node.w-x-it.w < min_waste) ||
                (y > y_from && y - y_from < min_waste) ||
                (y + it.h < node.y + node.h && node.y + node.h - y - it.h < min_waste) ||
                (node.cut == 1 && y > y_from && y-y_from < min_2_cut)
            );
}

bool Cutter::valid_4_cut(int node_h, int it_h) const
{
    return node_h-it_h >= min_waste;
}


bool Cutter::no_defect_in_node(const Node& node, const Bin& bin) const
{
    for(auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            return false;
        }
    }

    return true;
}

bool Cutter::x_cut_touches_any_defect(int x, const Node& node, const Bin& bin) const
{
    for(auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect) && x_cut_touches_defect(x,defect))
        {
            return false;
        }
    }

    return true;
}

bool Cutter::y_cut_touches_any_defect(int y, const Node& node, const Bin& bin) const
{
    for(auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect) && y_cut_touches_defect(y,defect))
        {
            return false;
        }
    }

    return true;
}

bool Cutter::passes_min_waste_constraint(int coord, int coord_from) const
{
    return !(coord > coord_from && coord-coord_from < min_waste);
}

bool Cutter::rotations_valid_y_cut(Node& node, Item& it, int y) const
{
    return  item_in_node(node,node.x,y,it) &&
            !(y + it.h < node.y + node.h && node.y + node.h - y - it.h < min_waste) &&
            !(node.cut == 1 && it.h < min_2_cut) && 
            y_cut_touches_any_defect(y,node,_bin) && 
            y_cut_touches_any_defect(y+it.h,node,_bin);
}

bool Cutter::rotations_valid_x_cut(Node& node, Item& it, int x, int x_from) const
{
    return  item_in_node(node,x,node.y,it) &&
            !(x + it.w < node.x + node.w && node.x + node.w - x - it.w < min_waste) &&
            !(node.cut == 0 && (it.w < min_1_cut || it.w > max_1_cut)) && 
            !(node.cut == 0 && x > x_from && (x - x_from < min_1_cut || x - x_from > max_1_cut)) &&
            x_cut_touches_any_defect(x,node,_bin) && 
            x_cut_touches_any_defect(x+it.w,node,_bin);
}

bool Cutter::can_do_bottom_4_cut(const Bin& bin, const Node& node, const Item& it) const
{
    for(auto& defect : bin.defects)
    {
        if( defect_in_node(node,defect) && item_touches_defect(node.x,node.y,it,defect))
        {
            return false;
        }
    }

    return true;
}

bool Cutter::can_do_top_4_cut(const Bin& bin, const Node& node, const Item& it) const
{
    for(auto& defect : bin.defects)
    {
        if( defect_in_node(node,defect) && item_touches_defect(node.x,node.y+node.h-it.h,it,defect))
        {
            return false;
        }
    }

    return true;
}

bool Cutter::passes_4_cut_min_waste_constraint(Node& node, Item& it) const
{
    return node.h-it.h >= min_waste;
}