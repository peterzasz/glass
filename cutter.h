#ifndef CUTTER_H
#define CUTTER_H

#include "data.h"
#include <vector>

enum class CutMethod
{
    Fast,
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
        bool item_in_node(const Node& node, int x, int y, const Item& item) const;
        bool item_touches_defect(int x, int y, const Item& item, const Defect& defect) const;
        bool x_cut_touches_defect(int x, const Defect& defect) const;
        bool y_cut_touches_defect(int y, const Defect& defect) const;

        bool can_cut(const Node& node, int x, int y, const Item& item) const;
        bool no_defect_in_node(const Node& node) const;
        bool x_cut_avoids_defects(int x, const Node& node) const;
        bool y_cut_avoids_defects(int y, const Node& node) const;
        bool valid_x_cut(int x, int y, int x_from, int y_from, const Node& node, const Item& item) const;
        bool valid_y_cut( int x, int y, int x_from, int y_from, const Node& node, const Item& item) const;
        bool valid_4_cut(int node_h, int item_h) const;
        bool passes_min_waste_constraint(int coord, int coord_from) const;
        bool rotations_valid_y_cut(const Node& node, const Item& item, int y) const;
        bool rotations_valid_x_cut(const Node& node, const Item& item, int x, int x_from) const;
        bool can_do_bottom_4_cut(const Node& node, const Item& item) const;
        bool can_do_top_4_cut(const Node& node, const Item& item) const;
        bool passes_4_cut_min_waste_constraint(const Node& node, const Item& item) const;
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
                case CutMethod::Fast:
                    return fast_cut(item, prev_item_id, prev_item_visited, node_id);

                case CutMethod::Rotation:
                    return rotation_cut(item, prev_item_id, prev_item_visited, node_id, false);
            }

            return false;
        }

    private:
        const CutMethod _method;

        bool fast_cut(Item& item, int prev_item_id, bool& prev_item_visited, int node_id);
        bool fast_try_vertical_cut(Item& item, int prev_item_id, bool& prev_item_visited, int node_id);
        bool fast_try_horizontal_cut(Item& item, int prev_item_id, bool& prev_item_visited, int node_id);
        bool fast_try_4_cut(Item& item, int prev_item_id, bool& prev_item_visited, int node_id);

        bool rotation_cut(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked);

        bool rotation_try_vertical_cut(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked);

        bool rotation_try_horizontal_cut(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked);

        bool rotation_try_4_cut(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked);

        bool rotation_try_subdivide_node(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked);

        bool rotation_try_subdivide_orientation(
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id);
};

class CutPlacementGenerator : private CutGeometry
{
    public:
        CutPlacementGenerator(Solution& solution, const Bin& bin)
            : CutGeometry(solution, bin) {}

        std::vector<Change> get_placements(
            Item& item,
            int prev_item_id,
            int node_id,
            bool allow_rotation = true);

    private:
        Solution _base_solution;

        void generate(
            std::vector<Change>& placements,
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id,
            bool orientation_locked);

        void try_vertical_cut(
            std::vector<Change>& placements,
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id);

        void try_horizontal_cut(
            std::vector<Change>& placements,
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id);

        void try_4_cut(
            std::vector<Change>& placements,
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id);

        void try_subdivide_node(
            std::vector<Change>& placements,
            Item& item,
            int prev_item_id,
            bool& prev_item_visited,
            int node_id);

        std::vector<int> possible_x_cut_positions(const Item& item, const Node& node) const;
        std::vector<int> possible_y_cut_positions(const Item& item, const Node& node) const;
        std::vector<int> possible_x_subdivision_positions(const Item& item, const Node& node) const;
        std::vector<int> possible_y_subdivision_positions(const Item& item, const Node& node) const;

        void record_placement(std::vector<Change>& placements) const;
        static bool same_node(const Node& a, const Node& b);
};

void apply_change(Solution& solution, const Change& change);

#endif
