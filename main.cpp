#include "parser.h"
#include "data.h"
#include "solve.h"
#include "platefiller.h"
#include "statistics.h"
#include <iostream>
#include <vector>
#include <chrono>

/*
* arguments:
*   dataset: A,B,X
*   solver type: F, R, PLF, PLR
*   ordering: 0,1
*   statistics output path
*/ 
int main(int argc,char **argv)
{
    if(argc == 1)
    {
        std::cout << "Argument needed: dataset, solver type, ordering and statistics output path";
        return 1;
    }
    else if(argc == 2)
    {
        std::cout << "Argument needed: solver type, ordering, and statistics output path";
        return 1;
    }
    else if(argc == 3)
    {
        std::cout << "Argument needed: ordering and statistics output path";
        return 1;
    }
    else if(argc == 4)
    {
        std::cout << "Argument needed: statistics output path";
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

        std::string method_code = argv[2];
        FillMethod method;

        if(method_code == "F")
        {
            method = FillMethod::Fast;
        }
        else if(method_code == "R")
        {
            method = FillMethod::Rotation;
        }
        else if(method_code == "PLR")
        {
            method = FillMethod::PlacementLookaheadRotationEval;
        }
        else if(method_code == "PLF")
        {
            method = FillMethod::PlacementLookaheadFastEval;
        }
        else
        {
            std::cout << "Unknown method given.";
            return 1;
        }

        bool area_order = argv[3] != std::string("0");

        std::string statistics_path = argv[4];

        std::cout << "Starting on dataset " << dataset << " with algorithm " << method_code << " with area ordering set to: " << (area_order ? true : false) << std::endl;

        for(int i = 1; i <= dataset_size; i++)
        {
            Batch batch = read_batch("dataset_" + dataset + "/" + dataset + std::to_string(i) + "_batch.csv");
            std::vector<Bin> bins = read_bins("dataset_" + dataset + "/" + dataset + std::to_string(i) + "_defects.csv");

            Solver solver(bins,batch,method,area_order);


            auto start = std::chrono::high_resolution_clock::now();

            solver.solve();
            
            auto end = std::chrono::high_resolution_clock::now();

            double seconds = std::chrono::duration<double>(end - start).count();

            write_solution(solver.get_solution(),"Solution/" + dataset + std::to_string(i) + "_solution.csv");

            create_statistics_file( solver.get_solution(),
                                    statistics_path,
                                    dataset + std::to_string(i));

            
            
            std::cout << dataset << std::to_string(i) << " done in " << std::to_string(seconds) << " seconds.\n";
        }
    }

    return 0;
}