#include "solve.h"
#include "data.h"
#include <iostream>
#include <utility>
#include <algorithm>

constexpr int min_1_cut = 100;
constexpr int max_1_cut = 3500;
constexpr int min_2_cut = 100;
constexpr int min_waste = 20;

Solver::Solver(const std::vector<Bin>& bins, const Batch& batch) : _bins(bins), _batch(batch) {}

bool Solver::defect_in_node(const Node& node, const Defect& defect) const
{
    return node.x <= defect.x &&
           node.y <= defect.y && 
           defect.x + defect.w <= node.x + node.w &&
           defect.y + defect.h <= node.y + node.h;
}

bool Solver::item_in_node(const Node& node, int x, int y, const Item& it) const
{
    return node.x <= x &&
           node.y <= y &&
           x + it.w <= node.x + node.w &&
           y + it.h <= node.y + node.h;
}

bool Solver::item_touches_defect(int x, int y, const Item& it, const Defect& defect) const
{
    return x < defect.x + defect.w &&
           y < defect.y + defect.h &&
           defect.x < x + it.w &&
           defect.y < y + it.h;
}

bool Solver::no_defect_in_node(const Node& node, const Bin& bin) const
{
    for(auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            return false;
        }
    }

    return true;
}

bool Solver::x_cut_touches_defect(int x, const Defect& defect) const
{
    return defect.x < x && x < defect.x + defect.w;
}

bool Solver::y_cut_touches_defect(int y, const Defect& defect) const
{
    return defect.y < y && y < defect.y + defect.h;
}

bool Solver::valid_x_cut(int x, const Node& node, const Bin& bin) const
{
    for(auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect) && x_cut_touches_defect(x,defect))
        {
            return false;
        }
    }

    return true;
}

bool Solver::valid_y_cut(int y, const Node& node, const Bin& bin) const
{
    for(auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect) && y_cut_touches_defect(y,defect))
        {
            return false;
        }
    }

    return true;
}

bool Solver::try_horizontal_cut( Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked )
{
    Node node = _s.nodes[node_id];
    
    int y_from = node.children.empty() ? node.y : _s.nodes[node.children.back()].y + _s.nodes[node.children.back()].h;
    int y_to = node.y + node.h;

    for(int y = y_from; y <= y_to; ++y)
    {
        if(!(y > y_from && y-y_from < min_waste))
        {
            if( item_in_node(node,node.x,y,it) &&
                !(y + it.h < y_to && y_to - y - it.h < min_waste) &&
                !(node.cut == 1 && it.h < min_2_cut) && 
                valid_y_cut(y,node,bin) && 
                valid_y_cut(y+it.h,node,bin) )
            {
                int nodes_size = _s.nodes.size();
                int node_children_size = node.children.size();
                int node_id_at_current = node_id_at;

                if(y == y_from)
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        node.x,
                        y,
                        node.w,
                        it.h,
                        -2,
                        node.cut + 1,
                        node.node_id
                    });

                    _s.nodes[node_id].children.push_back(node_id_at);

                    node_id_at++;
                }
                else
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        node.x,
                        y_from,
                        node.w,
                        y-y_from,
                        -2,
                        node.cut + 1,
                        node.node_id
                    });

                    _s.nodes[node_id].children.push_back(node_id_at);

                    node_id_at++;

                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        node.x,
                        y,
                        node.w,
                        it.h,
                        -2,
                        node.cut + 1,
                        node.node_id
                    });

                    _s.nodes[node_id].children.push_back(node_id_at);

                    node_id_at++;
                }

                if(cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                {
                    return true;
                }

                _s.nodes.resize(nodes_size);
                _s.nodes[node_id].children.resize(node_children_size);
                node_id_at = node_id_at_current;
            }

            if( !orientation_locked && it.w != it.h )
            {
                Item rotated = it;
                std::swap(rotated.w,rotated.h);

                if( item_in_node(node,node.x,y,rotated) &&
                    !(y + rotated.h < y_to && y_to - y - rotated.h < min_waste) &&
                    !(node.cut == 1 && (rotated.h < min_2_cut)) && 
                    valid_y_cut(y,node,bin) && 
                    valid_y_cut(y+rotated.h,node,bin) )
                {
                    int nodes_size = _s.nodes.size();
                    int node_children_size = node.children.size();
                    int node_id_at_current = node_id_at;

                    if(y == y_from)
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            node.x,
                            y,
                            node.w,
                            rotated.h,
                            -2,
                            node.cut + 1,
                            node.node_id
                        });

                        _s.nodes[node_id].children.push_back(node_id_at);

                        node_id_at++;
                    }
                    else
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            node.x,
                            y_from,
                            node.w,
                            y-y_from,
                            -2,
                            node.cut + 1,
                            node.node_id
                        });

                        _s.nodes[node_id].children.push_back(node_id_at);

                        node_id_at++;

                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            node.x,
                            y,
                            node.w,
                            rotated.h,
                            -2,
                            node.cut + 1,
                            node.node_id
                        });

                        _s.nodes[node_id].children.push_back(node_id_at);

                        node_id_at++;
                    }

                    if(cut(rotated,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                    {
                        return true;
                    }

                    _s.nodes.resize(nodes_size);
                    _s.nodes[node_id].children.resize(node_children_size);
                    node_id_at = node_id_at_current;
                }
            }
        }
    }

    return false;
}

bool Solver::can_do_bottom_4_cut(const Bin& bin, const Node& node, const Item& it) const
{
    for(auto& defect : bin.defects)
    {
        if( defect_in_node(node,defect) && item_touches_defect(node.x,node.y,it,defect))
        {
            return false;
        }
    }

    return true;
}

bool Solver::can_do_top_4_cut(const Bin& bin, const Node& node, const Item& it) const
{
    for(auto& defect : bin.defects)
    {
        if( defect_in_node(node,defect) && item_touches_defect(node.x,node.y+node.h-it.h,it,defect))
        {
            return false;
        }
    }

    return true;
}

bool Solver::try_4_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked)
{
    Node node = _s.nodes[node_id];

    if(it.w != node.w || it.h > node.h)
    {
        if(!orientation_locked)
        {
            Item rotated = it;
            std::swap(rotated.h,rotated.w);

            if(try_4_cut(rotated, prev_item_id, prev_item_visited,node_id,bin,node_id_at,true))
            {
                return true;
            }
        }

        return false;
    }

    if(can_do_bottom_4_cut(bin,node,it))
    {
        if(node.h-it.h >= min_waste)
        {
            _s.nodes.push_back(Node{
                node.plate_id,
                node_id_at,
                node.x,
                node.y,
                node.w,
                it.h,
                -2,
                4,
                node.node_id
            });

            _s.nodes[node_id].children.push_back(node_id_at);

            node_id_at++;

            _s.nodes.push_back(Node{
                node.plate_id,
                node_id_at,
                node.x,
                node.y+it.h,
                node.w,
                node.h-it.h,
                -2,
                4,
                node.node_id
            });

            _s.nodes[node_id].children.push_back(node_id_at);

            node_id_at++;

            return cut(it,prev_item_id,prev_item_visited,node_id_at-2,bin,node_id_at,orientation_locked);
        }
    }
    
    if(can_do_top_4_cut(bin,node,it))
    {
        if(node.h-it.h >= min_waste)
        {
            _s.nodes.push_back(Node{
                node.plate_id,
                node_id_at,
                node.x,
                node.y,
                node.w,
                node.h-it.h,
                -2,
                4,
                node.node_id
            });

            _s.nodes[node_id].children.push_back(node_id_at);

            node_id_at++;

            _s.nodes.push_back(Node{
                node.plate_id,
                node_id_at,
                node.x,
                node.y+node.h-it.h,
                node.w,
                it.h,
                -2,
                4,
                node.node_id
            });

            _s.nodes[node_id].children.push_back(node_id_at);

            node_id_at++;

            return cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,orientation_locked);
        }
    }

    return false;
}

bool Solver::try_vertical_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked)
{
    Node node = _s.nodes[node_id];

    int x_from =  node.children.empty() ? node.x : _s.nodes[node.children.back()].x + _s.nodes[node.children.back()].w;
    int x_to = node.x + node.w;

    for(int x = x_from; x <= x_to; ++x)
    {
        if(!(x > x_from && x-x_from < min_waste))
        {
            if( item_in_node(node,x,node.y,it) &&
                !(x + it.w < x_to && x_to - x - it.w < min_waste) &&
                !(node.cut == 0 && (it.w < min_1_cut || it.w > max_1_cut)) && 
                valid_x_cut(x,node,bin) && 
                valid_x_cut(x+it.w,node,bin) )
            {
                int nodes_size = _s.nodes.size();
                int node_children_size = node.children.size();
                int node_id_at_current = node_id_at;

                if(x == x_from)
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        x,
                        node.y,
                        it.w,
                        node.h,
                        -2,
                        node.cut + 1,
                        node.node_id
                    });

                    _s.nodes[node_id].children.push_back(node_id_at);

                    node_id_at++;
                }
                else
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        x_from,
                        node.y,
                        x-x_from,
                        node.h,
                        -2,
                        node.cut + 1,
                        node.node_id
                    });

                    _s.nodes[node_id].children.push_back(node_id_at);

                    node_id_at++;

                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        x,
                        node.y,
                        it.w,
                        node.h,
                        -2,
                        node.cut + 1,
                        node.node_id
                    });

                    _s.nodes[node_id].children.push_back(node_id_at);

                    node_id_at++;
                }

                if(cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                {
                    return true;
                }

                _s.nodes.resize(nodes_size);
                _s.nodes[node_id].children.resize(node_children_size);
                node_id_at = node_id_at_current;
            }

            if( !orientation_locked && it.w != it.h )
            {
                Item rotated = it;
                std::swap(rotated.w,rotated.h);

                if( item_in_node(node,x,node.y,rotated) &&
                    !(x + rotated.w < x_to && x_to - x - rotated.w < min_waste) &&
                    !(node.cut == 0 && (rotated.w < min_1_cut || rotated.w > max_1_cut)) && 
                    valid_x_cut(x,node,bin) && 
                    valid_x_cut(x+rotated.w,node,bin) )
                {
                    int nodes_size = _s.nodes.size();
                    int node_children_size = node.children.size();
                    int node_id_at_current = node_id_at;

                    if(x == x_from)
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            x,
                            node.y,
                            rotated.w,
                            node.h,
                            -2,
                            node.cut + 1,
                            node.node_id
                        });

                        _s.nodes[node_id].children.push_back(node_id_at);

                        node_id_at++;
                    }
                    else
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            x_from,
                            node.y,
                            x-x_from,
                            node.h,
                            -2,
                            node.cut + 1,
                            node.node_id
                        });

                        _s.nodes[node_id].children.push_back(node_id_at);

                        node_id_at++;

                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            x,
                            node.y,
                            rotated.w,
                            node.h,
                            -2,
                            node.cut + 1,
                            node.node_id
                        });

                        _s.nodes[node_id].children.push_back(node_id_at);

                        node_id_at++;
                    }

                    if(cut(rotated,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                    {
                        return true;
                    }

                    _s.nodes.resize(nodes_size);
                    _s.nodes[node_id].children.resize(node_children_size);
                    node_id_at = node_id_at_current;
                }
            }
        }
    }

    return false;
}

bool Solver::cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked)
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
    if( (it.w == node.w && it.h == node.h) || (!orientation_locked && it.h == node.w && it.w == node.h) )
    {
        if(!node.children.empty())
        {
            return false;
        }

        if(prev_item_visited && no_defect_in_node(node,bin))
        {
            _s.nodes[node_id].type = it.id;

            return true;
        }

        return false;
    }

    if(node.cut == 4)
    {
        return false;
    }

    // try to cut in descendant of node
    if(!node.children.empty())
    {
        for(auto ch : node.children)
        {
            if(cut(it,prev_item_id, prev_item_visited, ch, bin, node_id_at, orientation_locked))
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
        return try_vertical_cut(it, prev_item_id, prev_item_visited, node_id, bin, node_id_at,orientation_locked);
    }
    else
    {
        if(node.cut == 3)
        {
            return try_4_cut(it, prev_item_id, prev_item_visited, node_id, bin, node_id_at, orientation_locked);
        }
        
        return try_horizontal_cut(it, prev_item_id, prev_item_visited, node_id, bin, node_id_at,orientation_locked);
    }

    return false;
}

void Solver::set_waste(int node_id, int& node_id_at)
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
            set_waste(n_id,node_id_at);
        }

        if(node.cut%2 == 0)
        {
            int begin = _s.nodes[node.children.back()].x+_s.nodes[node.children.back()].w;
            int end = node.x + node.w;

            if(begin < end)
            {
                _s.nodes.push_back(Node{
                    node.plate_id,
                    node_id_at,
                    begin,
                    node.y,
                    end-begin,
                    node.h,
                    -1,
                    node.cut + 1,
                    node.node_id
                });

                _s.nodes[node_id].children.push_back(node_id_at);

                node_id_at++;
            }
        }
        else
        {
            int begin = _s.nodes[node.children.back()].y+_s.nodes[node.children.back()].h;
            int end = node.y + node.h;

            if(begin < end)
            {
                _s.nodes.push_back(Node{
                    node.plate_id,
                    node_id_at,
                    node.x,
                    begin,
                    node.w,
                    end-begin,
                    -1,
                    node.cut + 1,
                    node.node_id
                });

                _s.nodes[node_id].children.push_back(node_id_at);

                node_id_at++;
            }
        }
    }
}

NextItem Solver::next_item()
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

void Solver::initialize_stack_order()
{
    _batch.order.resize(_batch.stacks.size());
    for(int i = 0; i < _batch.order.size(); ++i)
    {
        _batch.order[i] = i;
    }
}

void Solver::order_stack()
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

void Solver::solve()
{
    int node_id_at = 0;

    for(auto& bin: _bins)
    {
        // add root node
        _s.nodes.push_back(Node{
            bin.id,         // bin id
            node_id_at,     // node id
            0,              // x
            0,              // y
            bin.w,          // w
            bin.h,          // h
            -2,             // branch
            0,              // cut type
            -1              // no parent
        });

        _s.roots.push_back(node_id_at);

        node_id_at++;

        initialize_stack_order();
        order_stack();

        NextItem next = next_item();

        while(next.stack_id != -1)
        {
            bool prev_item_visited = false;

            bool can_be_cut = cut( _batch.items[_batch.stacks[next.stack_id].item_ids[next.sequence]], // next item
                                   _batch.stacks[next.stack_id].prev_item, // previous item id
                                   prev_item_visited, // false
                                   _s.roots.back(), // starting node id = current root
                                   bin, // current bin used
                                   node_id_at,
                                   false
            );

            if(!can_be_cut)
            {
                _batch.stacks[next.stack_id].at --;
                _batch.stacks[next.stack_id].can_cut = false;
            }
            else
            {
                _batch.stacks[next.stack_id].prev_item = _batch.stacks[next.stack_id].item_ids[next.sequence];
                order_stack();
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

    for(auto root : _s.roots) set_waste(root,node_id_at);

    if( _s.nodes[_s.nodes[_s.roots.back()].children.back()].type == -1 )
    {
        _s.nodes[_s.nodes[_s.roots.back()].children.back()].type = -3;
    }
}