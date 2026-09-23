#include "solve.h"
#include "data.h"
#include <iostream>

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
    return x <= defect.x + defect.w &&
           y <= defect.y + defect.h &&
           defect.x <= x + it.w &&
           defect.y <= y + it.h;
}

bool Solver::x_cut_touches_defect(int x, const Defect& defect) const
{
    return defect.x < x && x < defect.x + defect.w;
}

bool Solver::y_cut_touches_defect(int y, const Defect& defect) const
{
    return defect.y < y && y < defect.y + defect.h;
}

bool Solver::can_cut(const Bin& bin, const Node& node, int x, int y, const Item& it)
{

    if(!item_in_node(node,x,y,it))
    {
        return false;
    }

    std::vector<Defect> defects;

    for(const auto& defect : bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            defects.push_back(defect);
        }
    }

    for(const auto& defect : defects)
    {
        if(item_touches_defect(x,y,it,defect))
        {
            return false;
        }
    }

    if(node.cut == 4)
    {
        if(it.w == node.w && it.h == node.h)
        {
            return true;
        }

        return false;
    }

    if( node.cut%2 == 0 )
    {
        for(const auto& defect : defects)
        {
            if(x_cut_touches_defect( x, defect ) || x_cut_touches_defect( x + it.w, defect ))
            {
                return false;
            }
        }
    }
    else
    {
        for(const auto& defect : defects)
        {
            if(y_cut_touches_defect( y, defect ) || y_cut_touches_defect( y + it.h, defect ))
            {
                return false;
            }
        }
    }

    if( node.cut == 3 )
    {
        if( node.x != x || node.x + node.w != x + it.w )
        {
            return false;
        }

        if( node.y != y && node.y + node.h != y + it.h )
        {
            return false;
        }
    }

    return true;
}

bool Solver::cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at)
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
            if(cut(it,prev_item_id, prev_item_visited, ch, bin, node_id_at))
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
        int x_from, y_from, x_to, y_to;

        if(node.children.empty())
        {
            x_from = node.x;
            x_to = node.x + node.w - it.w;
            y_from = node.y;
            y_to = node.y + node.h - it.h;
        }
        else
        {
            x_from = _s.nodes[node.children.back()].x+_s.nodes[node.children.back()].w;
            x_to = node.x + node.w - it.w;
            y_from = node.y;
            y_to = node.y + node.h - it.h;
        }

        for(int x = x_from; x <= x_to; ++x)
        {
            for(int y = y_from; y <= y_to; ++y)
            {
                if(can_cut(bin,node,x,y,it))
                {
                    if(x == x_from)
                    {
                        _s.nodes.push_back(Node{
                            node.plate_id,
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

                        if( cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at) ) return true;
                    }
                    else
                    {
                        _s.nodes.push_back(Node{
                            node.plate_id,
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
                            node.plate_id,
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

                        if(cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at)) return true;
                    }
                }
            }
        }
    }
    else
    {
        if(node.cut == 3)
        {
            if(can_cut(bin,node,node.x,node.y,it))
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

                return cut(it,prev_item_id,prev_item_visited,node_id_at-2,bin,node_id_at);
            }
            else if(can_cut(bin,node,node.x,node.y+node.h-it.h,it))
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

                return cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at);
            }

            return false;
        }

        int x_from, y_from, x_to, y_to;

        if(node.children.empty())
        {
            x_from = node.x;
            x_to = node.x + node.w - it.w;
            y_from = node.y;
            y_to = node.y + node.h - it.h;
        }
        else
        {
            x_from = node.x;
            x_to = node.x + node.w - it.w;
            y_from = _s.nodes[node.children.back()].y+_s.nodes[node.children.back()].h;
            y_to = node.y + node.h - it.h;
        }

        for(int y = y_from; y <= y_to; ++y)
        {
            for(int x = x_from; x <= x_to; ++x)
            {
                if(can_cut(bin,node,x,y,it))
                {
                    if(y == y_from)
                    {
                        _s.nodes.push_back(Node{
                            node.plate_id,
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

                        if( cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at) ) return true;
                    }
                    else
                    {
                        _s.nodes.push_back(Node{
                            node.plate_id,
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
                            node.plate_id,
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
                        
                        if(cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at)) return true;
                    }
                }
            }
        }
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

        NextItem next = next_item();

        while(next.stack_id != -1)
        {
            bool prev_item_visited = false;

            bool can_be_cut = cut( _batch.items[_batch.stacks[next.stack_id].item_ids[next.sequence]], // next item
                                   _batch.stacks[next.stack_id].prev_item, // previous item id
                                   prev_item_visited, // false
                                   _s.roots.back(), // starting node id = current root
                                   bin, // current bin used
                                   node_id_at
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

    for(auto root : _s.roots) set_waste(root,node_id_at);

    if( _s.nodes[_s.nodes[_s.roots.back()].children.back()].type == -1 )
    {
        _s.nodes[_s.nodes[_s.roots.back()].children.back()].type = -3;
    }
}