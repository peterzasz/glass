#include "cutter.h"

bool CutGeometry::defect_in_node(const Node& node, const Defect& defect) const
{
    return node.x <= defect.x &&
           node.y <= defect.y &&
           defect.x + defect.w <= node.x + node.w &&
           defect.y + defect.h <= node.y + node.h;
}

bool CutGeometry::no_defect_in_node(const Node& node) const
{
    for(auto& defect: _bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            return false;
        }
    }

    return true;
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

bool CutGeometry::vertical_cut_touches_defect(int x, const Defect& defect) const
{
    return defect.x < x && x < defect.x + defect.w;
}

bool CutGeometry::horizontal_cut_touches_defect(int y, const Defect& defect) const
{
    return defect.y < y && y < defect.y + defect.h;
}

bool CutGeometry::can_cut_vertical_strip(const Item& item, const Node& node, int x, int x_from) const
{
    if(x>x_from && (x-x_from < min_waste || (node.cut == 0 && x-x_from < min_1_cut)) )
    {
        return false;
    }

    if( x + item.w < node.x + node.w && (node.x + node.w - x - item.w < min_waste ||
                                        (node.cut == 0 && 
                                         node.x + node.w - x - item.w  < min_1_cut)))
    {
        return false;
    }

    for(const auto& defect: _bin.defects)
    {
        if(!defect_in_node(node,defect))
        {
            continue;
        }

        if( x > x_from && vertical_cut_touches_defect(x,defect) )
        {
            return false;
        }

        if( x + item.w < node.x + node.w && vertical_cut_touches_defect(x + item.w,defect) )
        {
            return false;
        }
    }

    if(node.cut == 0)
    {
        for(int y = 0; y <= node.h-item.h; ++y)
        {
            if((y == 0 || (y>=min_2_cut)) && (y+item.h == node.h || (node.h-y-item.h >= min_2_cut)) )
            {
                bool item_touches_any_defect = false;

                for(const auto& defect : _bin.defects )
                {
                    if(item_touches_defect(x,y,item,defect))
                    {
                        item_touches_any_defect = true;

                        break;
                    }
                }

                if(!item_touches_any_defect)
                {
                    return true;
                }
            }
        }
    }
    else // node.cut == 2
    {
        if(node.h != item.h && node.h-item.h < min_waste)
        {
            return false;
        }

        bool item_touches_any_defect = false;

        for(const auto& defect: _bin.defects)
        {
            if(item_touches_defect(x,node.y,item,defect))
            {
                item_touches_any_defect = true;
                break;
            }
        }

        if(!item_touches_any_defect)
        {
            return true;
        }

        item_touches_any_defect = false;

        for(const auto& defect: _bin.defects)
        {
            if(item_touches_defect(x,node.y+node.h-item.h,item,defect))
            {
                item_touches_any_defect = true;
                break;
            }
        }

        if(!item_touches_any_defect)
        {
            return true;
        }
    }

    return false;
}

bool CutGeometry::can_cut_horizontal_strip(const Item& item, const Node& node, int y, int y_from) const
{
    if((y > y_from && y-y_from < min_2_cut) || (y + item.h < node.h && node.h-y-item.h < min_2_cut))
    {
        return false;
    }

    for(const auto& defect: _bin.defects)
    {
        if(!defect_in_node(node,defect))
        {
            continue;
        }

        if( y > y_from && horizontal_cut_touches_defect(y,defect) )
        {
            return false;
        }

        if( y + item.h < node.h && horizontal_cut_touches_defect(y + item.h,defect) )
        {
            return false;
        }
    }

    for(int x = node.x; x <= node.x + node.w - item.w; ++x)
    {
        if(!((x>node.x && x-node.x < min_waste) ||
             ( x + item.w < node.x + node.w && node.x + node.w - x - item.w < min_waste)))
        {
            bool item_touches_any_defect = false;

            for(const auto& defect: _bin.defects)
            {
                if(item_touches_defect(x,y,item,defect))
                {
                    item_touches_any_defect = true;
                    break;
                }
            }

            if(!item_touches_any_defect)
            {
                return true;
            }
        }
    }

    return false;
}

bool CutGeometry::can_do_bottom_4_cut(const Node& node, const Item& item) const
{
    if(node.h - item.h < min_waste)
    {
        return false;
    }

    for(const auto& defect: _bin.defects)
    {
        if(item_touches_defect(node.x,node.y,item,defect))
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::can_do_top_4_cut(const Node& node, const Item& item) const
{
    if(node.h - item.h < min_waste)
    {
        return false;
    }

    for(const auto& defect: _bin.defects)
    {
        if(item_touches_defect(node.x,node.y+node.h-item.h,item,defect))
        {
            return false;
        }
    }

    return true;
}

bool CutGeometry::can_subdivide_by_vertical_strip(const Item& item, const Node& node, int x, int x_from, int x_to) const
{
    int min_remainder = node.cut == 1 ? min_1_cut : min_waste;

    if((x > x_from && (x - x_from < min_remainder)) || (x < x_to && ( x_to - x < min_remainder)) )
    {
        return false;
    }

    if(x == x_from)
    {
        for(auto& defect: _bin.defects)
        {
            if(defect_in_node(node,defect) && vertical_cut_touches_defect(x+item.w,defect))
            {
                return false;
            }
        }
    }
    else if(x == x_to)
    {
        for(auto& defect: _bin.defects)
        {
            if(defect_in_node(node,defect) && vertical_cut_touches_defect(x,defect))
            {
                return false;
            }
        }
    }
    else
    {
        for(auto& defect: _bin.defects)
        {
            if(defect_in_node(node,defect) && (vertical_cut_touches_defect(x,defect) || vertical_cut_touches_defect(x+item.w,defect)))
            {
                return false;
            }
        }
    }

    if(node.cut == 1)
    {
        for(int y = 0; y <= node.h-item.h; ++y)
        {
            if((y>0 && y < min_2_cut) || (y+item.h < node.h && node.h-y-item.h < min_2_cut))
            {
                continue;
            }

            bool item_touches_any_defect = false;

            for(auto& defect: _bin.defects)
            {
                if(item_touches_defect(x,y,item,defect))
                {
                    item_touches_any_defect = true;
                    break;
                }
            }

            if(!item_touches_any_defect)
            {
                return true;
            }
        }
    }
    else
    {
        if(node.h != item.h && node.h-item.h < min_waste)
        {
            return false;
        }

        bool item_touches_any_defect = false;

        for(auto& defect: _bin.defects)
        {
            if(item_touches_defect(x,node.y,item,defect))
            {
                item_touches_any_defect = true;
                break;
            }
        }

        if(!item_touches_any_defect)
        {
            return true;
        }

        item_touches_any_defect = false;

        for(auto& defect: _bin.defects)
        {
            if(item_touches_defect(x,node.y + node.h-item.h,item,defect))
            {
                item_touches_any_defect = true;
                break;
            }
        }

        if(!item_touches_any_defect)
        {
            return true;
        }
    }

    return false;
}

bool CutGeometry::can_subdivide_by_horizontal_strip(const Item& item, const Node& node, int y, int y_from, int y_to) const
{
    if((y>y_from && y-y_from < min_2_cut) || (y<y_to && y_to-y < min_2_cut))
    {
        return false;
    }

    if(y == y_from)
    {
        for(auto& defect: _bin.defects)
        {
            if(defect_in_node(node,defect) && horizontal_cut_touches_defect(y+item.h,defect))
            {
                return false;
            }
        }
    }
    else if(y == y_to)
    {
        for(auto& defect: _bin.defects)
        {
            if(defect_in_node(node,defect) && horizontal_cut_touches_defect(y,defect))
            {
                return false;
            }
        }
    }
    else
    {
        for(auto& defect: _bin.defects)
        {
            if(defect_in_node(node,defect) && (horizontal_cut_touches_defect(y,defect) || horizontal_cut_touches_defect(y+item.h,defect)))
            {
                return false;
            }
        }
    }

    for(int x = node.x; x <= node.x + node.w-item.w; ++x)
    {
        if((x>node.x && x-node.x < min_waste) || (x+item.w < node.x + node.w && node.x + node.w-x-item.w < min_waste))
        {
            continue;
        }

        bool item_touches_any_defect = false;

        for(auto& defect: _bin.defects)
        {
            if(item_touches_defect(x,y,item,defect))
            {
                item_touches_any_defect = true;
                break;
            }
        }

        if(!item_touches_any_defect)
        {
            return true;
        }
    }

    return false;
}