#include "solve.h"
#include "data.h"
#include "cutter.h"
#include <iostream>

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
        _s.add_node(
                    bin.id,
                    0,
                    0,
                    bin.w,
                    bin.h,
                    -2,
                    0,
                    -1,
                    true
                );

        //create cutter
        Cutter cutter = Cutter(_s,bin,CutMethod::Rotation);

        NextItem next = next_item();

        while(next.stack_id != -1)
        {
            bool prev_item_visited = false;

            bool can_be_cut = cutter.cut(   _batch.items[_batch.stacks[next.stack_id].item_ids[next.sequence]], // next item
                                            _batch.stacks[next.stack_id].prev_item, // previous item id
                                            prev_item_visited, // false
                                            _s.roots.back() // starting node id = current root
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