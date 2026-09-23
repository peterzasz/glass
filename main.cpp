#include "parser.h"
#include "data.h"
#include "solve.h"
#include <iostream>
#include <vector>
#include <format>

int main()
{
    for(int i = 1; i <= 20; i++)
    {
        Batch batch = read_batch("/home/peterzasz/Documents/opkutgy/dataset_A/A" + std::to_string(i) + "_batch.csv");
        std::vector<Bin> bins = read_bins("/home/peterzasz/Documents/opkutgy/dataset_A/A" + std::to_string(i) + "_defects.csv");

        Solver solver(bins,batch);

        solver.solve();

        write_solution(solver.get_solution(),"/home/peterzasz/Documents/opkutgy/dataset_A/A" + std::to_string(i) + "_solution.csv");
    }
}