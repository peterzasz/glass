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
        case Ordering::HeightWidth:
            order_stack_by_height_width();
        case Ordering::WidthHeight:
            order_stack_by_width_height();
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
            case Ordering::HeightWidth:
                order_stack_by_height_width();
            case Ordering::WidthHeight:
                order_stack_by_width_height();
        }

        next = next_item();
    }

    return value;
}

int PlateFiller::placement_lookahead_fill(FillerType eval_method)
{
    int value = 0;

    PlacementGenerator placement_generator(_s,_bin);
    
    int items_placed = -1;
    PlateFiller eval_filler = PlateFiller(eval_method,_bin,_s,_batch,items_placed);

    

    initialize_stack_order();
    switch(_filler_type.ordering)
    {
        case Ordering::AreaOrder:
            order_stack_by_area();
        case Ordering::HeightWidth:
            order_stack_by_height_width();
        case Ordering::WidthHeight:
            order_stack_by_width_height();
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
            case Ordering::HeightWidth:
                order_stack_by_height_width();
            case Ordering::WidthHeight:
                order_stack_by_width_height();
        }

        next = next_item();
    }

    return value;
}

int PlateFiller::dynamic_order_fill(FillerType eval_method)
{
    int value = 0;

    PlacementGenerator placement_generator(_s,_bin);
    int items_placed = -1;
    PlateFiller eval_filler =  PlateFiller(eval_method,_bin,_s,_batch,items_placed);

    while(true)
    {
        bool can_place = false;

        int max_val = -1;
        Change best_change;
        int best_stack;

        for(int i = 0; i < _batch.stacks.size(); ++i)
        {
            if(_batch.stacks[i].can_cut && _batch.stacks[i].at < _batch.stacks[i].item_ids.size())
            {
                Solution save_solution = _s;
                Batch save_batch = _batch;

                Item item = _batch.items[_batch.stacks[i].item_ids[_batch.stacks[i].at]];

                bool prev_item_visited = false;

                std::vector<Change> placements = placement_generator.get_placements(
                    item,
                    _batch.stacks[i].prev_item,
                    prev_item_visited,
                    _s.roots.back()
                );

                if(item.w != item.h)
                {
                    Item rotated = item;
                    std::swap(rotated.w,rotated.h);

                    prev_item_visited = false;

                    std::vector<Change> rotated_placements = placement_generator.get_placements(
                        rotated,
                        _batch.stacks[i].prev_item,
                        prev_item_visited,
                        _s.roots.back()
                    );

                    placements.insert(placements.end(),rotated_placements.begin(),rotated_placements.end());
                }

                if(!placements.empty())
                {
                    can_place = true;

                    for(const auto& change : placements)
                    {
                        int val = item.w*item.h;

                        apply_change(_s,change);
                        _batch.stacks[i].at++;
                        _batch.stacks[i].prev_item = item.id;

                        val += eval_filler.fill();

                        if(val > max_val)
                        {
                            max_val = val;
                            best_change = change;
                            best_stack = i;
                        }

                        _s = save_solution;
                        _batch = save_batch;
                    }
                }
                else
                {
                    _batch.stacks[i].can_cut = false;
                }
            }
        }

        if(!can_place)
        {
            break;
        }
        else
        {
            int best_item_id = _batch.stacks[best_stack].item_ids[_batch.stacks[best_stack].at];
            apply_change(_s,best_change);
            _batch.stacks[best_stack].prev_item = best_item_id;
            _batch.stacks[best_stack].at++;

            value += _batch.items[best_item_id].w*_batch.items[best_item_id].h;

            if(_items_placed != -1)
            {
                _items_placed++;
                std::cout << _items_placed << "/" << _batch.items.size() << " items placed.\n";
            }
        }
    }

    return value;
}
