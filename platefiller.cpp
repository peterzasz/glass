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

void PlateFiller::order_stack_by_area()
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

void PlateFiller::order_stack_by_height_width()
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

            if(item_a.h == item_b.h)
            {
                return item_a.w > item_b.w;
            }
            else
            {
                return item_a.h > item_b.h;
            }
        }
    );
}

void PlateFiller::order_stack_by_width_height()
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

            if(item_a.w == item_b.w)
            {
                return item_a.h > item_b.h;
            }
            else
            {
                return item_a.w > item_b.w;
            }
        }
    );
}

int PlateFiller::cutter_fill(CutMethod method)
{
    int value = 0;

    Cutter cutter = Cutter(_s,_bin,method);

    initialize_stack_order();
    switch(_filler_type.ordering)
    {
        case Ordering::AreaOrder:
            order_stack_by_area();
            break;
        case Ordering::HeightWidth:
            order_stack_by_height_width();
            break;
        case Ordering::WidthHeight:
            order_stack_by_width_height();
            break;
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
            
            if(_items_placed != -1)
            {
                _items_placed++;
                std::cout << _items_placed << "/" << _batch.items.size() << " items placed.\n";
            }
        }

        switch(_filler_type.ordering)
        {
            case Ordering::AreaOrder:
                order_stack_by_area();
                break;
            case Ordering::HeightWidth:
                order_stack_by_height_width();
                break;
            case Ordering::WidthHeight:
                order_stack_by_width_height();
                break;
        }

        next = next_item();
    }

    return value;
}

int PlateFiller::placement_lookahead_fill(FillerType eval_method)
{
    int value = 0;

    initialize_stack_order();
    switch(_filler_type.ordering)
    {
        case Ordering::AreaOrder:
            order_stack_by_area();
            break;
        case Ordering::HeightWidth:
            order_stack_by_height_width();
            break;
        case Ordering::WidthHeight:
            order_stack_by_width_height();
            break;
    }

    NextItem next = next_item();

    while(next.stack_id != -1)
    {
        bool prev_item_visited = false;

        PLCutter pl_cutter(eval_method,_s,_batch,_bin);
        Item item = _batch.items[_batch.stacks[next.stack_id].item_ids[next.sequence]];
        int prev_item_id = _batch.stacks[next.stack_id].prev_item;
        
        _batch.stacks[next.stack_id].prev_item = item.id;

        bool can_cut = pl_cutter.cut(
            item,
            prev_item_id,
            _s.roots.back()
        );

        if(!can_cut)
        {
            _batch.stacks[next.stack_id].at --;
            _batch.stacks[next.stack_id].can_cut = false;
            _batch.stacks[next.stack_id].prev_item = prev_item_id;
        }
        else
        {
            value += item.w*item.h;
            
            if(_items_placed != -1)
            {
                _items_placed++;
                std::cout << _items_placed << "/" << _batch.items.size() << " items placed.\n";
            }
        }


        switch(_filler_type.ordering)
        {
            case Ordering::AreaOrder:
                order_stack_by_area();
                break;
            case Ordering::HeightWidth:
                order_stack_by_height_width();
                break;
            case Ordering::WidthHeight:
                order_stack_by_width_height();
                break;
        }

        next = next_item();
    }

    return value;
}

int PlateFiller::order_lookahead_fill(int block_size, FillerType eval_method)
{
    int value = 0;
    bool can_cut = true;

    while(can_cut)
    {
        Solution best_sol;
        Batch best_batch;
        int best_val = -1;
        int items_val = 0;
        cut_out(block_size,eval_method,best_sol,best_batch,best_val,items_val,0); 

        if(items_val == 0)
        {
            can_cut = false;
        }
        else
        {
            value += items_val;
            _s = best_sol;
            _batch = best_batch;

            std::cout << "Some items are placed.\n";
        }
    }

    return value;
}

void PlateFiller::cut_out(int block_size, FillerType eval_method, Solution& best_sol, Batch& best_batch, int& best_val, int& items_val, int val)
{
    if(block_size == 0)
    {
        if(val == 0)
        {
            return; 
        }

        int items_placed = -1;
        PlateFiller eval_filler(eval_method,_bin,_s,_batch,items_placed);

        Solution save_sol = _s;
        Batch save_batch = _batch;

        int value = val + eval_filler.fill();

        _s = save_sol;
        _batch = save_batch;

        if(value > best_val)
        {
            best_sol = _s;
            best_batch = _batch;
            best_val = value;
            items_val = val;
        }
    }
    else
    {
        bool there_is_avaiable = false;

        for(int i = 0; i < _batch.stacks.size(); ++i)
        {
            if(_batch.stacks[i].can_cut && _batch.stacks[i].at < _batch.stacks[i].item_ids.size())
            {
                Solution save_sol = _s;
                Batch save_batch = _batch;

                Item item = _batch.items[_batch.stacks[i].item_ids[_batch.stacks[i].at]];
                PLCutter cutter(eval_method,_s,_batch,_bin);

                int prev_item_id = _batch.stacks[i].prev_item;

                _batch.stacks[i].prev_item = item.id;
                _batch.stacks[i].at++;

                bool can_cut = cutter.cut(item,prev_item_id,_s.roots.back());

                if(can_cut)
                {
                    cut_out(block_size-1,eval_method,best_sol,best_batch,best_val,items_val,val+item.h*item.w);
                    there_is_avaiable = true;
                }

                _s = save_sol;
                _batch = save_batch;
            }
        }

        if(!there_is_avaiable)
        {
            cut_out(0,eval_method,best_sol,best_batch,best_val,items_val,val);
        }
    }
}


