#include "parser.h"
#include "data.h"
#include "solve.h"
#include "platefiller.h"
#include "statistics.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>

/*
* arguments:
*   dataset: A,B,X followed by id
*   solver type: F, R, PL, PL2, OL or plates number lower bound: PNLB followed by statistics output path
*   ordering: NO, DA, WH, HW, ALL
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
        if(std::string(argv[2]) == "PNLB") // calculate plates number lower bound
        {
            std::string dataset = argv[1];
            std::string statistics_path = argv[3];

            std::stringstream ss;
            std::string batch_path;
            std::string defects_path;

            ss << "dataset_" << dataset[0] << "/" << dataset << "_batch.csv";
            ss >> batch_path;

            Batch batch = read_batch(batch_path);

            plate_number_lower_bound(batch,6000*3210,statistics_path,dataset);

            return 0;
        }

        std::cout << "Argument needed: statistics output path";
        return 1;
    }
    else
    {
        std::string dataset = argv[1];

        std::string method_code = argv[2];
        FillMethod method;

        if(method_code == "F")
        {
            method = FillMethod::NoRotation;
        }
        else if(method_code == "R")
        {
            method = FillMethod::Rotation;
        }
        else if(method_code == "PL")
        {
            method = FillMethod::PlacementLookahead;
        }
        else if(method_code == "PL2")
        {
            method = FillMethod::PlacementLookahead2;
        }
        else if(method_code == "OL")
        {
            method = FillMethod::OrderLookahead;
        }
        else
        {
            std::cout << "Unknown method given.";
            return 1;
        }

        std::string ordering_code = argv[3];
        std::vector<FillerType> fillers;

        if(ordering_code == "NO")
        {
            fillers.push_back(FillerType{
                Ordering::NoOrder,
                method
            });
        }
        else if(ordering_code == "DA")
        {
            fillers.push_back(FillerType{
                Ordering::AreaOrder,
                method
            });
        }
        else if(ordering_code == "WH")
        {
            fillers.push_back(FillerType{
                Ordering::WidthHeight,
                method
            });
        }
        else if(ordering_code == "HW")
        {
            fillers.push_back(FillerType{
                Ordering::HeightWidth,
                method
            });
        }
        else if(ordering_code == "ALL")
        {
            fillers.push_back(FillerType{
                Ordering::NoOrder,
                method
            });

            fillers.push_back(FillerType{
                Ordering::AreaOrder,
                method
            });

            fillers.push_back(FillerType{
                Ordering::HeightWidth,
                method
            });

            fillers.push_back(FillerType{
                Ordering::WidthHeight,
                method
            });
        }
        else
        {
            std::cout << "Unknown ordering code given.";
            return 1;
        }

        std::string statistics_path = argv[4];

        std::stringstream ss;
        std::string batch_path;
        std::string defects_path;

        ss << "dataset_" << dataset[0] << "/" << dataset << "_batch.csv";
        ss >> batch_path;
        ss.clear();
        ss << "dataset_" << dataset[0] << "/" << dataset << "_defects.csv";
        ss >> defects_path;

        Batch batch = read_batch(batch_path);
        std::vector<Bin> bins = read_bins(defects_path);

        Solver solver(bins,batch,fillers);

        std::cout << "Starting on dataset " << dataset << " with algorithm " << method_code << " with ordering set to " << ordering_code << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        solver.solve();
        
        auto end = std::chrono::high_resolution_clock::now();

        double seconds = std::chrono::duration<double>(end - start).count();

        write_solution(solver.get_solution(),"Solution/" + dataset + "_solution.csv");

        create_statistics_file( solver.get_solution(),
                                statistics_path,
                                dataset,
                                seconds);
        
        std::cout << dataset << " done in " << std::to_string(seconds) << " seconds.\n";
    }

    return 0;
}