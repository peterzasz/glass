#include "parser.h"
#include "data.h"
#include "solve.h"
#include <iostream>
#include <vector>

int main()
{
    Batch batch = read_batch("/home/peterzasz/Documents/opkutgy/dataset_A/A1_batch.csv");
    std::vector<Bin> bins = read_bins("/home/peterzasz/Documents/opkutgy/dataset_A/A1_defects.csv");

    Solver solver(bins,batch);

    solver.solve();

    write_solution(solver.get_solution(),"/home/peterzasz/Documents/opkutgy/dataset_A/A1_solution.csv");
}