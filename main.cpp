#include "parser.h"
#include "data.h"
#include "solve.h"
#include <iostream>
#include <vector>
#include <chrono>

int main(int argc,char **argv)
{
    if(argc == 1)
    {
        std::cout << "Argument needed: Which dataset to process?";
        return 1;
    }
    else
    {
        std::string dataset = argv[1];

        int dataset_size;

        if(dataset == "A")
        {
            dataset_size = 20;
        }
        else if(dataset == "B")
        {
            dataset_size = 15;
        }
        else if(dataset == "X")
        {
            dataset_size = 15;
        }
        else
        {
            std::cout << "Unknown dataset given.";
            return 1;
        }

        for(int i = 1; i <= dataset_size; i++)
        {
            Batch batch = read_batch("dataset_" + dataset + "/" + dataset + std::to_string(i) + "_batch.csv");
            std::vector<Bin> bins = read_bins("dataset_" + dataset + "/" + dataset + std::to_string(i) + "_defects.csv");

            Solver solver(bins,batch);


            auto start = std::chrono::high_resolution_clock::now();

            solver.solve();

            write_solution(solver.get_solution(),"Solution/" + dataset + std::to_string(i) + "_solution.csv");

            auto end = std::chrono::high_resolution_clock::now();

            double seconds = std::chrono::duration<double>(end - start).count();
            
            std::cout << "Dataset " << dataset << std::to_string(i) << " done in " << std::to_string(seconds) << " seconds.\n";
        }
    }

    return 0;
}