#include "solve.h"
#include "data.h"
#include <iostream>

Solver::Solver(const std::vector<Bin>& bins, const Batch& batch) : _bins(bins), _batch(batch) {}

bool Solver::try_vertical_cut( Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin )
{
    Node node = _s.nodes[node_id];

    int x_from = node.children.empty() ? node.x : _s.nodes[node.children.back()].x+_s.nodes[node.children.back()].w;
    int x_to = node.x + node.w - it.w;
    int y_from = node.y;
    int y_to = node.y + node.h - it.h;

    for(int x = x_from; x <= x_to; ++x)
    {
        for(int y = y_from; y <= y_to; ++y)
        {
            if(valid_x_cut(x,y,x_from,y_from,node,it) && can_cut(bin,node,x,y,it))
            {
                int nodes_size = _s.nodes.size();
                int children_size = _s.nodes[node_id].children.size();

                if(x == x_from)
                {
                    int child_id = _s.add_node(node.plate_id,x,node.y,it.w,node.h,-2,node.cut+1,node.node_id,false);

                    if( cut(it,prev_item_id,prev_item_visited,child_id,bin) )
                    {
                        return true;
                    }
                }
                else
                {
                    _s.add_node(node.plate_id,x_from,node.y,x-x_from,node.h,-2,node.cut+1,node.node_id,false);
                    int child_id = _s.add_node(node.plate_id,x,node.y,it.w,node.h,-2,node.cut+1,node.node_id,false);

                    if(cut(it,prev_item_id,prev_item_visited,child_id,bin))
                    {
                        return true;
                    }
                }

                _s.nodes.resize(nodes_size);
                _s.nodes[node_id].children.resize(children_size);
            }
        }
    }

    return false;
}

bool Solver::try_4_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin)
{
    Node node = _s.nodes[node_id];

    if(can_cut(bin,node,node.x,node.y,it) && valid_4_cut(node.h,it.h))
    {
        int child_id = _s.add_node(node.plate_id,node.x,node.y,node.w,it.h,-2,4,node.node_id,false);
        _s.add_node(node.plate_id,node.x,node.y+it.h,node.w,node.h-it.h,-2,4,node.node_id,false);

        return cut(it,prev_item_id,prev_item_visited,child_id,bin);
    }
    else if(can_cut(bin,node,node.x,node.y+node.h-it.h,it) && valid_4_cut(node.h,it.h))
    {
        _s.add_node(node.plate_id,node.x,node.y,node.w,node.h-it.h,-2,4,node.node_id,false);
        int child_id = _s.add_node(node.plate_id,node.x,node.y+node.h-it.h,node.w,it.h,-2,4,node.node_id,false);

        return cut(it,prev_item_id,prev_item_visited,child_id,bin);
    }

    return false;
}

bool Solver::try_horizontal_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin)
{
    Node node = _s.nodes[node_id];

    int x_from = node.x;
    int x_to = node.x + node.w - it.w;
    int y_from = node.children.empty() ? node.y : _s.nodes[node.children.back()].y+_s.nodes[node.children.back()].h;
    int y_to = node.y + node.h - it.h;

    for(int y = y_from; y <= y_to; ++y)
    {
        for(int x = x_from; x <= x_to; ++x)
        {
            if(valid_y_cut(x,y,x_from,y_from,node,it) && can_cut(bin,node,x,y,it))
            {
                int nodes_size = _s.nodes.size();
                int children_size = _s.nodes[node_id].children.size();

                if(y == y_from)
                {
                    int child_id = _s.add_node(node.plate_id, node.x, y, node.w, it.h, -2, node.cut + 1, node.node_id, false);
                    

                    if( cut(it,prev_item_id,prev_item_visited,child_id,bin) )
                    {
                        return true;
                    }
                }
                else
                {
                    int nodes_size = _s.nodes.size();
                    int children_size = _s.nodes[node_id].children.size();

                    _s.add_node(node.plate_id,node.x,y_from,node.w,y-y_from,-2,node.cut+1,node.node_id,false);
                    int child_id = _s.add_node(node.plate_id,node.x,y,node.w,it.h,-2,node.cut+1,node.node_id,false);
                    
                    if(cut(it,prev_item_id,prev_item_visited,child_id,bin))
                    {
                        return true;
                    }
                }

                _s.nodes.resize(nodes_size);
                _s.nodes[node_id].children.resize(children_size);
            }
        }
    }

    return false;
}

bool Solver::cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin)
{
    Node node = _s.nodes[node_id];

    if(prev_item_id == -1 || node.type == prev_item_id)
    {
        prev_item_visited = true;
    }

    //check if node is a branch
    if(node.type != -2)
    {
        return false;
    }

    //check if node is the same size as item
    if(it.w == node.w && it.h == node.h)
    {
        if(!node.children.empty())
        {
            return false;
        }

        if(prev_item_visited && can_cut(bin,node,node.x,node.y,it))
        {
            _s.nodes[node_id].type = it.id;

            return true;
        }

        return false;
    }

    // try to cut in descendant of node
    if(!node.children.empty())
    {
        for(auto ch : node.children)
        {
            if(cut(it,prev_item_id, prev_item_visited, ch, bin))
            {
                return true;
            }
        }

        if(node.cut == 3)
        {
            return false;
        }
    }

    
    // try to cut new part out of this node
    if(!prev_item_visited)
    {
        return false;
    }

    if(node.cut % 2 == 0)
    {
        return try_vertical_cut(it, prev_item_id, prev_item_visited, node_id, bin);
    }
    else
    {
        if(node.cut == 3)
        {
            return try_4_cut(it, prev_item_id, prev_item_visited, node_id, bin);
        }
        
        return try_horizontal_cut(it, prev_item_id, prev_item_visited, node_id, bin);
    }

    return false;
}

void Solver::set_waste(int node_id)
{
    Node node = _s.nodes[node_id];

    if(node.children.empty())
    {
        if(node.type == -2)
        {
            _s.nodes[node_id].type = -1;
        }
    }
    else
    {
        for( auto n_id : node.children )
        {
            set_waste(n_id);
        }

        if(node.cut%2 == 0)
        {
            int begin = _s.nodes[node.children.back()].x+_s.nodes[node.children.back()].w;
            int end = node.x + node.w;

            if(begin < end)
            {
                _s.add_node(node.plate_id,begin,node.y,end-begin,node.h,-1,node.cut+1,node.node_id,false);
            }
        }
        else
        {
            int begin = _s.nodes[node.children.back()].y+_s.nodes[node.children.back()].h;
            int end = node.y + node.h;

            if(begin < end)
            {
                _s.add_node(node.plate_id,node.x,begin,node.w,end-begin,-1,node.cut+1,node.node_id,false);
            }
        }
    }
}

NextItem Solver::next_item()
{
    for( int i = 0; i < _batch.stacks.size(); ++i )
    {
        if( _batch.stacks[i].can_cut && _batch.stacks[i].at < _batch.stacks[i].item_ids.size() )
        {
            _batch.stacks[i].at++;
            return {i, _batch.stacks[i].at-1};
        }
    }

    return {-1,-1};
}

void Solver::solve()
{
    for(auto& bin: _bins)
    {
        // add root node
        _s.add_node(bin.id,0,0,bin.w,bin.h,-2,0,-1,true);

        NextItem next = next_item();

        while(next.stack_id != -1)
        {
            bool prev_item_visited = false;

            bool can_be_cut = cut( _batch.items[_batch.stacks[next.stack_id].item_ids[next.sequence]], // next item
                                   _batch.stacks[next.stack_id].prev_item, // previous item id
                                   prev_item_visited, // false
                                   _s.roots.back(), // starting node id = current root
                                   bin // current bin used
            );

            if(!can_be_cut)
            {
                _batch.stacks[next.stack_id].at --;
                _batch.stacks[next.stack_id].can_cut = false;
            }
            else
            {
                _batch.stacks[next.stack_id].prev_item = _batch.stacks[next.stack_id].item_ids[next.sequence];
            }

            next = next_item();
        }

        bool finished = true;

        for(auto& stack : _batch.stacks)
        {
            if(stack.at < stack.item_ids.size())
            {
                finished = false;
            }

            stack.can_cut = true;
            stack.prev_item = -1;
        }

        if(finished)
        {
            break;
        }
    }

    for(auto root : _s.roots) set_waste(root);

    if( _s.nodes[_s.nodes[_s.roots.back()].children.back()].type == -1 )
    {
        _s.nodes[_s.nodes[_s.roots.back()].children.back()].type = -3;
    }
}