#ifndef SOLVE_H
#define SOLVE_H

#include "data.h"

class Solver
{
    public:
        Solver(const std::vector<Bin>& bins, const Batch& batch);

        void solve();

        Solution& get_solution() { return _s; }

    private:
        std::vector<Bin> _bins;
        Batch _batch;
        Solution _s;

        bool defect_in_node(const Node& node, const Defect& defect) const;
        bool item_in_node(const Node& node, int x, int y, const Item& it) const;
        bool item_touches_defect(int x, int y, const Item& it, const Defect& defect) const;
        bool x_cut_touches_defect(int x, const Defect& defect) const;
        bool y_cut_touches_defect(int y, const Defect& defect) const;
        bool no_defect_in_node(const Node& node, const Bin& bin) const;
        bool valid_x_cut(int x, const Node& node, const Bin& bin) const;
        bool valid_y_cut(int y, const Node& node, const Bin& bin) const;
        bool can_do_bottom_4_cut(const Bin& bin, const Node& node, const Item& it) const;
        bool can_do_top_4_cut(const Bin& bin, const Node& node, const Item& it) const;

        bool cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked);
        bool try_vertical_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked);
        bool try_4_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked);
        bool try_horizontal_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked);

        void initialize_stack_order();
        void order_stack();

        void set_waste(int node_id, int& node_id_at);

        NextItem next_item();
};

#endif