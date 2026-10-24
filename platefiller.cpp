#include "cutter.h"
#include "data.h"
#include "platefiller.h"
#include <iostream>
#include <algorithm>

NextItem PlateFiller::next_item()
{
    for( int i = 0; i < _batch.stacks.size(); ++i )
    {
        if( _batch.stacks[_batch.order[i]].can_cut && _batch.stacks[_batch.order[i]].at < _batch.stacks[_batch.order[i]].item_ids.size() )
        {
            _batch.stacks[_batch.order[i]].at++;
            return {_batch.order[i], _batch.stacks[_batch.order[i]].at-1};
        }
    }

    return {-1,-1};
}

void PlateFiller::initialize_stack_order()
{
    _batch.order.resize(_batch.stacks.size());
    for(int i = 0; i < _batch.order.size(); ++i)
    {
        _batch.order[i] = i;
    }
}

void PlateFiller::order_stack()
{
    std::sort(
        _batch.order.begin(),
        _batch.order.end(),
        [this](int a, int b)
        {
            bool a_finished = _batch.stacks[a].at >= _batch.stacks[a].item_ids.size();
            bool b_finished = _batch.stacks[b].at >= _batch.stacks[b].item_ids.size();

            if(a_finished != b_finished)
            {
                return !a_finished;
            }

            if(a_finished)
            {
                return false;
            }

            const Item& item_a =
                _batch.items[_batch.stacks[a].item_ids[_batch.stacks[a].at]];

            const Item& item_b =
                _batch.items[_batch.stacks[b].item_ids[_batch.stacks[b].at]];

            return item_a.w * item_a.h > item_b.w * item_b.h;
        }
    );
}

int PlateFiller::cutter_fill(CutMethod method)
{
    int value = 0;

    Cutter cutter = Cutter(_s,_bin,method);

    initialize_stack_order();
    if(_decr_area_order)
    {
        order_stack();
    }

    NextItem next = next_item();

    while(next.stack_id != -1)
    {
        bool prev_item_visited = false;

        Item it = _batch.items[_batch.stacks[next.stack_id].item_ids[next.sequence]];

        bool can_be_cut = cutter.cut(   it, // next item
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
            value += it.w*it.h;
        }

        if(_decr_area_order)
        {
            order_stack();
        }

        next = next_item();
    }

    return value;
}

int PlateFiller::placement_lookahead_fill(FillMethod eval_method)
{
    int value = 0;

    PlacementGenerator placement_generator(_s,_bin);
    PlateFiller eval_filler(eval_method,true,_bin,_s,_batch);

    initialize_stack_order();
    if(_decr_area_order)
    {
        order_stack();
    }

    NextItem next = next_item();

    while(next.stack_id != -1)
    {
        bool prev_item_visited = false;

        Item item = _batch.items[_batch.stacks[next.stack_id].item_ids[next.sequence]];

        std::vector<Change> placements = placement_generator.get_placements(   
            item,                                   // next item
            _batch.stacks[next.stack_id].prev_item, // previous item id
            prev_item_visited,                      // false
            _s.roots.back()                         // starting node id = current root
        );

        if(item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w,rotated.h);

            prev_item_visited = false;

            std::vector<Change> rotated_placements = placement_generator.get_placements(
                rotated,                                // next item rotated
                _batch.stacks[next.stack_id].prev_item, // previous item id
                prev_item_visited,                      // false
                _s.roots.back()                         // starting node id = current root
            );

            placements.insert(placements.end(),rotated_placements.begin(),rotated_placements.end());
        }

        if(placements.empty())
        {
            _batch.stacks[next.stack_id].at --;
            _batch.stacks[next.stack_id].can_cut = false;
        }
        else
        {
            _batch.stacks[next.stack_id].prev_item = _batch.stacks[next.stack_id].item_ids[next.sequence];
            value += item.w*item.h;

            int max_val = -1;
            Change best_change;

            Solution save_solution = _s;
            Batch save_batch = _batch;

            for(const auto& change : placements)
            {
                apply_change(_s,change);

                int val = eval_filler.fill();
                
                if(val > max_val)
                {
                    max_val = val;
                    best_change = change;
                }

                _s = save_solution;
                _batch = save_batch;
            }

            apply_change(_s,best_change);
        }

        if(_decr_area_order)
        {
            order_stack();
        }

        next = next_item();
    }

    return value;
}
