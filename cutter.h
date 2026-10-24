#ifndef CUTTER_H
#define CUTTER_H

#include "data.h"
#include <vector>

enum class CutMethod
{
    NoRotation,
    Rotation
};

class CutGeometry
{
    protected:
        CutGeometry(Solution& solution, const Bin& bin)
            : _s(solution), _bin(bin) {}

        static constexpr int min_1_cut = 100;
        static constexpr int max_1_cut = 3500;
        static constexpr int min_2_cut = 100;
        static constexpr int min_waste = 20;

        Solution& _s;
        const Bin& _bin;

        bool defect_in_node(const Node& node, const Defect& defect) const;
        bool no_defect_in_node(const Node& node) const;
        bool item_in_node(const Node& node, int x, int y, const Item& item) const;
        bool item_touches_defect(int x, int y, const Item& item, const Defect& defect) const;
        bool vertical_cut_touches_defect(int x, const Defect& defect) const;
        bool horizontal_cut_touches_defect(int y, const Defect& defect) const;
        bool can_do_bottom_4_cut(const Node& node, const Item& item) const;
        bool can_do_top_4_cut(const Node& node, const Item& item) const;

        bool can_cut_vertical_strip(const Item& item, const Node& node, int x, int x_from) const;
        bool can_cut_horizontal_strip(const Item& item, const Node& node, int y, int y_from) const;
        bool can_subdivide_by_vertical_strip(const Item& item, const Node& node, int x, int x_from, int x_to) const;
        bool can_subdivide_by_horizontal_strip(const Item& item, const Node& node, int y, int y_from, int y_to) const;
};

class Cutter : private CutGeometry
{
    public:
        Cutter(Solution& solution, const Bin& bin, CutMethod method)
            : CutGeometry(solution, bin), _method(method) {}

        bool cut(Item& item, int prev_item_id, bool& prev_item_visited, int node_id)
        {
            switch(_method)
            {
                case CutMethod::NoRotation:
                    return rotation_cut(item, prev_item_id, prev_item_visited, node_id,true);
                case CutMethod::Rotation:
                    return rotation_cut(item,prev_item_id,prev_item_visited,node_id,false);
            }

            return false;
        }

    private:
        const CutMethod _method;

        int which_child(int node_id);

        bool rotation_cut(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked);
        
        bool rotation_try_vertical_cut(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked
        );

        bool rotation_try_horizontal_cut(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked
        );

        bool rotation_try_4_cut(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked
        );

        bool rotation_try_subdivide_node(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked
        );
};

class PlacementGenerator : private CutGeometry
{
    public:
        PlacementGenerator(Solution& solution, const Bin& bin)
            : CutGeometry(solution, bin) {}

        std::vector<Change> get_placements(Item& item, int prev_item_id, bool& prev_item_visited, int node_id)
        {
            std::vector<Change> placements;
            Change change;

            generate_placements(item, prev_item_id, prev_item_visited, node_id,change,placements);

            return placements;
        }

    private:

        int which_child(int node_id);

        void generate_placements(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            Change change,
            std::vector<Change>& placements);
        
        void try_vertical_cut(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            Change change,
            std::vector<Change>& placements
        );

        void try_horizontal_cut(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            Change change,
            std::vector<Change>& placements
        );

        void try_4_cut(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            Change change,
            std::vector<Change>& placements
        );

        void try_subdivide_node(
            Item& item,
            bool& prev_item_visited,
            int node_id,
            Change change,
            std::vector<Change>& placements
        );

        std::vector<int> possible_x_coords_for_vertical_cut(const Item& item, const Node& node, int x_from, int x_to) const;
        std::vector<int> possible_y_coords_for_horizontal_cut(const Item& item, const Node& node, int y_from, int y_to) const;
        std::vector<int> possible_x_coords_for_subdivision(const Item& item, const Node& node, int x_from, int x_to) const;
        std::vector<int> possible_y_coords_for_subdivision(const Item& item, const Node& node, int y_from, int y_to) const;
};

void apply_change(Solution& s, const Change& c);

#endif
