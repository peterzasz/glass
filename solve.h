#ifndef SOLVE_H
#define SOLVE_H

#include "data.h"

class Solver
{
    public:
        Solver(const std::vector<Bin>& bins, const Batch& batch) : _bins(bins), _batch(batch) {}

        void solve();

        Solution& get_solution() { return _s; }

    private:
        std::vector<Bin> _bins;
        Batch _batch;
        Solution _s;

        void set_waste(int node_id);

        NextItem next_item();
};

#endif