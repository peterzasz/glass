#ifndef DATA_H
#define DATA_H

#include <vector>

// Basic structures

struct Item
{
    int w;
    int h;

    int id;
};

struct Stack
{
    std::vector<int> item_ids; // in order

    int id;

    int at = 0; // next item to finish
    int prev_item = -1;
    bool can_cut = true;
};

struct Batch
{
    std::vector<Item> items;
    std::vector<Stack> stacks;
    std::vector<int> order;
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

// Structures for solution
struct Node
{
    int plate_id;
    int node_id;

    int x;
    int y;
    int w;
    int h;

    int type;     // >= 0 item id, -1 waste, -2 branch, -3 residual
    int cut;      // 0, 1, 2, 3, 4
    int parent;   // -1 means no parent/root

    std::vector<int> children; // left to right, bottom to top
};

struct Solution
{
    std::vector<Node> nodes;
    std::vector<int> roots;
};

struct NextItem
{
    int stack_id;
    int sequence;
};

struct Position
{
    int node_id;
    Item it;
    int x;
    int y;
};

#endif