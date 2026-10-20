#include "statistics.h"
#include "data.h"

#include <fstream>
#include <filesystem>
#include <iomanip>
#include <string>

void create_statistics_file(
    const Solution& s,
    const std::string& path,
    const std::string& file_name
)
{
    std::filesystem::create_directories(path);

    std::ofstream file(path + "/" + file_name);

    if(!file)
    {
        return;
    }

    if(s.roots.empty())
    {
        return;
    }

    const int plates_used = static_cast<int>(s.roots.size());

    long long total_plate_area = 0;
    long long total_item_area = 0;

    // Total area of all used plates
    for(int root_id : s.roots)
    {
        const Node& root = s.nodes[root_id];

        total_plate_area +=
            static_cast<long long>(root.w) * root.h;
    }

    // Total area occupied by items
    for(const Node& node : s.nodes)
    {
        if(node.type >= 0)
        {
            total_item_area +=
                static_cast<long long>(node.w) * node.h;
        }
    }

    // Find residual on the last plate
    const Node& last_root = s.nodes[s.roots.back()];
    const int last_plate_id = last_root.plate_id;

    int residual_width = 0;

    for(const Node& node : s.nodes)
    {
        if(node.plate_id == last_plate_id &&
           node.type == -3)
        {
            residual_width = node.w;
            break;
        }
    }

    // Challenge objective uses H * residual width
    const long long residual_area =
        static_cast<long long>(last_root.h) *
        residual_width;

    const long long consumed_area =
        total_plate_area - residual_area;

    const long long total_geo_loss =
        consumed_area - total_item_area;

    double total_utilization = 0.0;

    if(consumed_area > 0)
    {
        total_utilization =
            100.0 *
            static_cast<double>(total_item_area) /
            consumed_area;
    }

    // Overall statistics
    file << "platesUsed;" << plates_used << '\n';
    file << "widthResidual;" << residual_width << '\n';
    file << "residualArea;" << residual_area << '\n';
    file << "totalItemArea;" << total_item_area << '\n';
    file << "totalGeoLoss;" << total_geo_loss << '\n';

    file << "totalUtilization;"
         << std::fixed << std::setprecision(2)
         << total_utilization << "%\n";

    file << '\n';

    // Per-plate statistics
    file << "plateId;usedArea;effectiveArea;utilization\n";

    for(int root_id : s.roots)
    {
        const Node& root = s.nodes[root_id];

        long long used_area = 0;

        for(const Node& node : s.nodes)
        {
            if(node.plate_id == root.plate_id &&
               node.type >= 0)
            {
                used_area +=
                    static_cast<long long>(node.w) * node.h;
            }
        }

        long long plate_area =
            static_cast<long long>(root.w) * root.h;

        long long effective_area = plate_area;

        // Only the residual of the final plate is excluded
        if(root.plate_id == last_plate_id)
        {
            effective_area -= residual_area;
        }

        double utilization = 0.0;

        if(effective_area > 0)
        {
            utilization =
                100.0 *
                static_cast<double>(used_area) /
                effective_area;
        }

        file << root.plate_id << ';'
             << used_area << ';'
             << effective_area << ';'
             << std::fixed << std::setprecision(2)
             << utilization << "%\n";
    }
}