#include "statistics.h"
#include "data.h"

#include <fstream>
#include <filesystem>
#include <iomanip>
#include <string>

void create_statistics_file(
    const Solution& s,
    const std::string& path,
    const std::string& dataset
)
{
    std::filesystem::create_directories(path);

    std::ofstream file(path + "/" + dataset + "_statistics.csv");

    if(!file)
    {
        return;
    }

    file << "instanceId;validSolution;nPlates;totalGeoLoss;widthResidual\n";

    int nPlates = s.roots.size();
    
    const Node& last_1_cut = s.nodes[s.nodes[s.roots.back()].children.back()];

    int widthResidual = last_1_cut.type == -3 ? last_1_cut.w : 0;

    long long totalPlatesArea = 0;

    for(auto& root: s.roots)
    {
        totalPlatesArea += s.nodes[root].w*s.nodes[root].h;
    }

    totalPlatesArea -= widthResidual*last_1_cut.h;

    long long totalItemsArea = 0;

    for(auto& node : s.nodes)
    {
        if(node.type >= 0)
        {
            totalItemsArea += node.w*node.h;
        }
    }

    double totalGeoLoss = (1 - static_cast<double>(totalItemsArea)/totalPlatesArea)*100;

    file << dataset << ";" << 1 << ";" << nPlates << ";" << totalGeoLoss << "%" << ";" << widthResidual;
}