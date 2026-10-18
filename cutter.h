#ifndef CUTTER_H
#define CUTTER_H

#include "data.h"
#include <vector>

enum class CutMethod
{
    Fast,
    Rotation
};

class Cutter
{
    public:
        Cutter(Solution& s, const Bin& bin, CutMethod method) : _s(s), _bin(bin), _method(method) {}

        bool cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id)
        {
            if(_method == CutMethod::Fast)
            {
                return fast_cut(it,prev_item_id,prev_item_visited,node_id);
            }
            else if(_method == CutMethod::Rotation)
            {
                return rotation_cut();
            }
            else
            {
                return false;
            }
        }

    private:
        Solution& _s;
        const Bin& _bin;
        const CutMethod _method;

        bool rotation_cut();

        bool fast_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id);
        bool fast_try_vertical_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id);
        bool fast_try_4_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id);
        bool fast_try_horizontal_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id);

        //cut_geometry.cpp
        bool can_cut(const Bin& bin, const Node& node, int x, int y, const Item& it);
        bool defect_in_node(const Node& node, const Defect& defect) const;
        bool item_in_node(const Node& node, int x, int y, const Item& it) const;
        bool item_touches_defect(int x, int y, const Item& it, const Defect& defect) const;
        bool x_cut_touches_defect(int x, const Defect& defect) const;
        bool y_cut_touches_defect(int y, const Defect& defect) const;
        bool valid_x_cut(int x, int y, int x_from, int y_from, const Node& node, const Item& it) const;
        bool valid_y_cut(int x, int y, int x_from, int y_from, const Node& node, const Item& it) const;
        bool valid_4_cut(int node_h, int it_h) const;
};

class CutPlacementGenerator
{
    public:
        CutPlacementGenerator(Solution& s, const Bin& bin) : _s(s), _bin(bin) {}
        
        std::vector<Change> get_placements();

    private:
        Solution& _s;
        const Bin& _bin;
};

#endif