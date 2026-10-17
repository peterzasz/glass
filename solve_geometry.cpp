#include "solve.h"
#include "data.h"

constexpr int min_1_cut = 100;
constexpr int max_1_cut = 3500;
constexpr int min_2_cut = 100;
constexpr int min_waste = 20;

bool Solver::defect_in_node(const Node& node, const Defect& defect) const
{
    return node.x <= defect.x &&
           node.y <= defect.y && 
           defect.x + defect.w <= node.x + node.w &&
           defect.y + defect.h <= node.y + node.h;
}

bool Solver::item_in_node(const Node& node, int x, int y, const Item& it) const
{
    return node.x <= x &&
           node.y <= y &&
           x + it.w <= node.x + node.w &&
           y + it.h <= node.y + node.h;
}

bool Solver::item_touches_defect(int x, int y, const Item& it, const Defect& defect) const
{
    return x < defect.x + defect.w &&
           y < defect.y + defect.h &&
           defect.x < x + it.w &&
           defect.y < y + it.h;
}

bool Solver::x_cut_touches_defect(int x, const Defect& defect) const
{
    return defect.x < x && x < defect.x + defect.w;
}

bool Solver::y_cut_touches_defect(int y, const Defect& defect) const
{
    return defect.y < y && y < defect.y + defect.h;
}

bool Solver::can_cut(const Bin& bin, const Node& node, int x, int y, const Item& it)
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

bool Solver::valid_x_cut(int x, int y, int x_from, int y_from, const Node& node, const Item& it) const
{
    return !(   (x > x_from && x - x_from < min_waste) ||
                (x + it.w < node.x + node.w && node.x+node.w-x-it.w < min_waste) ||
                (y > y_from && y - y_from < min_waste) ||
                (y + it.h < node.y + node.h && node.y + node.h - y - it.h < min_waste) ||
                (node.cut == 0 && x > x_from && (x-x_from < min_1_cut || x-x_from > max_1_cut))
            );
}

bool Solver::valid_y_cut(int x, int y, int x_from, int y_from, const Node& node, const Item& it) const
{
    return !(   (x > x_from && x - x_from < min_waste) ||
                (x + it.w < node.x + node.w && node.x+node.w-x-it.w < min_waste) ||
                (y > y_from && y - y_from < min_waste) ||
                (y + it.h < node.y + node.h && node.y + node.h - y - it.h < min_waste) ||
                (node.cut == 1 && y > y_from && y-y_from < min_2_cut)
            );
}

bool Solver::valid_4_cut(int node_h, int it_h) const
{
    return node_h-it_h >= min_waste;
}