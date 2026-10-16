#include "parser.h"
#include "data.h"
#include "solve.h"
#include <iostream>
#include <vector>
#include <chrono>

int main()
{
    for(int i = 1; i <= 15; i++)
    {
        Batch batch = read_batch("/home/peterzasz/Documents/opkutgy/dataset_B/B" + std::to_string(i) + "_batch.csv");
        std::vector<Bin> bins = read_bins("/home/peterzasz/Documents/opkutgy/dataset_B/B" + std::to_string(i) + "_defects.csv");

        Solver solver(bins,batch);


        auto start = std::chrono::high_resolution_clock::now();

        solver.solve();

        write_solution(solver.get_solution(),"/home/peterzasz/Documents/opkutgy/dataset_B/B" + std::to_string(i) + "_solution.csv");

        auto end = std::chrono::high_resolution_clock::now();

        double seconds = std::chrono::duration<double>(end - start).count();
        
        std::cout << "Dataset " << std::to_string(i) << " done in " << std::to_string(seconds) << " seconds.\n";
    }
}