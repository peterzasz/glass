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
        long long can_cut_calls = 0;

        bool can_cut(const Bin& bin, const Node& node, int x, int y, const Item& it);
        bool defect_in_node(const Node& node, const Defect& defect) const;
        bool item_in_node(const Node& node, int x, int y, const Item& it) const;
        bool item_touches_defect(int x, int y, const Item& it, const Defect& defect) const;
        bool x_cut_touches_defect(int x, const Defect& defect) const;
        bool y_cut_touches_defect(int y, const Defect& defect) const;

        bool cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at);
        void set_waste(int node_id, int& node_id_at);

        NextItem next_item();
};

#endif