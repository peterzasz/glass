#ifndef SOLVE_H
#define SOLVE_H

#include "data.h"
#include "platefiller.h"

class Solver
{
    public:
        Solver(const std::vector<Bin>& bins, const Batch& batch, const FillMethod fill_method, bool area_order) :   _bins(bins), 
                                                                                                                    _batch(batch),
                                                                                                                    _fill_method(fill_method),
                                                                                                                    _area_order(area_order) {}

        void solve();

        Solution& get_solution() { return _s; }

    private:
        std::vector<Bin> _bins;
        Batch _batch;
        Solution _s;
        const FillMethod _fill_method;
        bool _area_order;

        void set_waste(int node_id);

        NextItem next_item();
};

#endif