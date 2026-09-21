#include "parser.h"
#include "data.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

Batch read_batch(std::string path)
{
    std::ifstream file(path);

    if(!file.is_open())
    {
        std::cerr << "Could not open file\n";
        return {};
    }

    std::string line;

    // skip header
    std::getline(file, line);

    Batch b{};

    while(std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string field;

        std::getline(ss, field, ';');
        int item_id = std::stoi(field);

        std::getline(ss, field, ';');
        int height = std::stoi(field);

        std::getline(ss, field, ';');
        int width = std::stoi(field);

        std::getline(ss, field, ';');
        int stack = std::stoi(field);

        std::getline(ss, field, ';');
        int sequence = std::stoi(field); 
        
        Item i{width,height,item_id};
        b.items.push_back(i);
        
        if(stack >= b.stacks.size())
        {
            b.stacks.resize(stack + 1);
        }

        b.stacks[stack].id = stack;
        if( b.stacks[stack].item_ids.size() < sequence )
        {
            b.stacks[stack].item_ids.resize(sequence);
        }
        b.stacks[stack].item_ids[sequence-1] = item_id;
    }

    return b;
}

std::vector<Bin> read_bins(std::string path)
{
    std::ifstream file(path);

    if(!file.is_open())
    {
        std::cerr << "Could not open file\n";
        return {};
    }

    std::string line;

    // skip header
    std::getline(file, line);

    std::vector<Bin> bins(100,{6000,3210,{},0});

    for( int i = 0; i < bins.size(); i++ )
    {
        bins[i].id = i;
    }

    while(std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string field;

        std::getline(ss, field, ';');
        int defect_id = std::stoi(field);

        std::getline(ss, field, ';');
        int plate_id = std::stoi(field);

        std::getline(ss, field, ';');
        int x = std::stoi(field);

        std::getline(ss, field, ';');
        int y = std::stoi(field);

        std::getline(ss, field, ';');
        int w = std::stoi(field);
        
        std::getline(ss, field, ';');
        int h = std::stoi(field); 

        bins[plate_id].defects.push_back({x,y,w,h,defect_id,plate_id});
    }

    return bins;
}

void write(const Node& node, int& node_id_at, int parent_id, const Solution& s, std::ofstream& file)
{
    file << node.plate_id << ';'
             << node_id_at << ';'
             << node.x << ';'
             << node.y << ';'
             << node.w << ';'
             << node.h << ';'
             << node.type << ';'
             << node.cut << ';';

    if(node.parent != -1)
    {
        file << parent_id;
    }

    file << '\n';

    int new_parent_id = node_id_at;

    node_id_at++;

    for( auto ch : node.children )
    {
        write(s.nodes[ch],node_id_at,new_parent_id,s,file);
    }
}

void write_solution(const Solution& s, const std::string& path)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        std::cerr << "Could not open output file\n";
        return;
    }

    // header
    file << "PLATE_ID;NODE_ID;X;Y;WIDTH;HEIGHT;TYPE;CUT;PARENT\n";

    int node_id_at = 0;

    for(auto root : s.roots)
    {
        write(s.nodes[root],node_id_at,-1,s,file);
    }
}