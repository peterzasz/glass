#ifndef DATA_H
#define DATA_H

#include <vector>

struct Item
{
    int w;
    int h;
    int id;
};

struct Stack
{
    std::vector<int> item_ids;
    int id;
    int at = 0;
    int prev_item = -1;
    bool can_cut = true;
};

struct Batch
{
    std::vector<Item> items;
    std::vector<Stack> stacks;
};

struct Defect
{
    int x;
    int y;
    int w;
    int h;
    int id;
    int plate_id;
};

struct Bin
{
    int w;
    int h;
    std::vector<Defect> defects;
    int id;
};

struct Node
{
    int plate_id;
    int node_id;
    int x;
    int y;
    int w;
    int h;
    int type;   // >= 0 item id, -1 waste, -2 branch, -3 residual
    int cut;    // 0, 1, 2, 3, 4
    int parent; // -1 means root
    std::vector<int> children;
};

struct Solution
{
    std::vector<Node> nodes;
    std::vector<int> roots;

    int add_node(
        int plate_id,
        int x,
        int y,
        int w,
        int h,
        int type,
        int cut_level,
        int parent_id,
        bool root,
        int child_position = -1)
    {
        const int node_id = static_cast<int>(nodes.size());

        nodes.push_back(Node{
            plate_id,
            node_id,
            x,
            y,
            w,
            h,
            type,
            cut_level,
            parent_id,
            {}
        });

        if(parent_id != -1)
        {
            auto& children = nodes[parent_id].children;

            if(child_position >= 0 && child_position <= static_cast<int>(children.size()))
            {
                children.insert(children.begin() + child_position, node_id);
            }
            else
            {
                children.push_back(node_id);
            }
        }

        if(root)
        {
            roots.push_back(node_id);
        }

        return node_id;
    }
};

struct NextItem
{
    int stack_id;
    int sequence;
};

struct NodeChange
{
    int node_id;
    Node new_node;
};

struct Change
{
    std::vector<NodeChange> updates;
};

#endif
