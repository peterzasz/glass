#ifndef SOLVE_H
#define SOLVE_H

#include "data.h"
#include "platefiller.h"

class Solver
{
    public:
        Solver(const std::vector<Bin>& bins, const Batch& batch, std::vector<FillerType>& fillers) : _bins(bins), 
                                                                                                    _batch(batch),
                                                                                                    _fillers(fillers) {}

        void solve();

        Solution& get_solution() { return _s; }

    private:
        std::vector<Bin> _bins;
        Batch _batch;
        Solution _s;
        std::vector<FillerType> _fillers;

        void set_waste(int node_id);

        NextItem next_item();
};

#endif