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
    int y_to = node.y + node.h - std::min(it.w,it.h);

    for(int y = y_from; y <= y_to; ++y)
    {
        if(!(y > y_from && y-y_from < min_waste))
        {
            if( item_in_node(node,node.x,y,it) &&
                !(y + it.h < node.y + node.h && node.y + node.h - y - it.h < min_waste) &&
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
                    !(y + rotated.h < node.y + node.h && node.y + node.h - y - rotated.h < min_waste) &&
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
    int x_to = node.x + node.w - std::min(it.w,it.h);

    for(int x = x_from; x <= x_to; ++x)
    {
        if(!(x > x_from && x-x_from < min_waste))
        {
            if( item_in_node(node,x,node.y,it) &&
                !(x + it.w < node.x + node.w && node.x + node.w - x - it.w < min_waste) &&
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
                    !(x + rotated.w < node.x + node.w && node.x + node.w - x - rotated.w < min_waste) &&
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

bool Solver::try_subdivide_node(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked)
{
    Node node = _s.nodes[node_id];
    Node parent = _s.nodes[_s.nodes[node_id].parent];


    int child_index = -1;
    for(int i = 0; i < parent.children.size(); ++i)
    {
        if(parent.children[i] == node.node_id)
        {
            child_index = i;
            break;
        }
    }

    if(node.cut%2 == 0)
    {
        int y_from = node.y;
        int y_to = node.y + node.h - std::min(it.h,it.w);

        for(int y = y_from; y <= y_to; ++y)
        {
            if( node.h - it.h >= min_waste &&
                valid_y_cut(y,parent,bin) &&
                valid_y_cut(y + it.h,parent,bin) &&
                !(y>y_from && y-y_from < min_waste) &&
                !(y+it.h < node.y+node.h && node.y+node.h-y-it.h < min_waste) &&
                y+it.h <= node.y + node.h &&
                !(node.cut == 2 && it.h < min_2_cut))
            {
                int current_nodes_size = _s.nodes.size();
                int current_node_id_at = node_id_at;

                if( y == y_from )
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        node.x,
                        y + it.h,
                        node.w,
                        node.h-it.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                    node_id_at++;

                    _s.nodes[node_id].h = it.h;

                    if(cut(it,prev_item_id,prev_item_visited,node_id,bin,node_id_at,true))
                    {
                        return true;
                    }
                    else
                    {
                        _s.nodes.resize(current_nodes_size);
                        node_id_at = current_node_id_at;
                        _s.nodes[node_id] = node;
                        _s.nodes[parent.node_id] = parent;
                    }
                }
                else if(y+it.h == node.y + node.h)
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        node.x,
                        y,
                        node.w,
                        it.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                    node_id_at++;

                    _s.nodes[node_id].h = node.h-it.h;

                    if(cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                    {
                        return true;
                    }
                    else
                    {
                        _s.nodes.resize(current_nodes_size);
                        node_id_at = current_node_id_at;
                        _s.nodes[node_id] = node;
                        _s.nodes[parent.node_id] = parent;
                    }
                }
                else
                {
                     _s.nodes[node_id].h = y-node.y;

                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        node.x,
                        y,
                        node.w,
                        it.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                    node_id_at++;

                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        node.x,
                        y+it.h,
                        node.w,
                        node.y + node.h - y - it.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+2,node_id_at);

                    node_id_at++;

                    if(cut(it,prev_item_id,prev_item_visited,node_id_at-2,bin,node_id_at,true))
                    {
                        return true;
                    }
                    else
                    {
                        _s.nodes.resize(current_nodes_size);
                        node_id_at = current_node_id_at;
                        _s.nodes[node_id] = node;
                        _s.nodes[parent.node_id] = parent;
                    }
                }
            }
            
            if(!orientation_locked && it.w != it.h)
            {
                
                Item rotated = it;
                std::swap(rotated.w,rotated.h);

                if( node.h - rotated.h >= min_waste && 
                    valid_y_cut(y,parent,bin) &&
                    valid_y_cut(y + rotated.h,parent,bin) &&
                    !(y>y_from && y-y_from < min_waste) &&
                    !(y+rotated.h < node.y+node.h && node.y+node.h-y-rotated.h < min_waste) &&
                    y+rotated.h <= node.y + node.h &&
                    !(node.cut == 2 && rotated.h < min_2_cut))
                {
                    int current_nodes_size = _s.nodes.size();
                    int current_node_id_at = node_id_at;

                    if( y == y_from )
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            node.x,
                            y + rotated.h,
                            node.w,
                            node.h-rotated.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                        node_id_at++;

                        _s.nodes[node_id].h = rotated.h;

                        if(cut(rotated,prev_item_id,prev_item_visited,node_id,bin,node_id_at,true))
                        {
                            return true;
                        }
                        else
                        {
                            _s.nodes.resize(current_nodes_size);
                            node_id_at = current_node_id_at;
                            _s.nodes[node_id] = node;
                            _s.nodes[parent.node_id] = parent;
                        }
                    }
                    else if(y+rotated.h == node.y + node.h)
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            node.x,
                            y,
                            node.w,
                            rotated.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                        node_id_at++;

                        _s.nodes[node_id].h = node.h-rotated.h;

                        if(cut(rotated,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                        {
                            return true;
                        }
                        else
                        {
                            _s.nodes.resize(current_nodes_size);
                            node_id_at = current_node_id_at;
                            _s.nodes[node_id] = node;
                            _s.nodes[parent.node_id] = parent;
                        }
                    }
                    else
                    {
                        _s.nodes[node_id].h = y-node.y;

                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            node.x,
                            y,
                            node.w,
                            rotated.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                        node_id_at++;

                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            node.x,
                            y+rotated.h,
                            node.w,
                            node.y + node.h - y - rotated.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+2,node_id_at);

                        node_id_at++;

                        if(cut(rotated,prev_item_id,prev_item_visited,node_id_at-2,bin,node_id_at,true))
                        {
                            return true;
                        }
                        else
                        {
                            _s.nodes.resize(current_nodes_size);
                            node_id_at = current_node_id_at;
                            _s.nodes[node_id] = node;
                            _s.nodes[parent.node_id] = parent;
                        }
                    }
                }
            }
        }
    }
    else
    {
        int x_from = node.x;
        int x_to = node.x + node.w - std::min(it.w,it.h);

        for(int x = x_from; x <= x_to; ++x)
        {
            if( node.w - it.w >= min_waste &&
                valid_x_cut(x,parent,bin) &&
                valid_x_cut(x + it.w,parent,bin) &&
                !(x>x_from && x-x_from < min_waste) &&
                !(x+it.w < node.x+node.w && node.x+node.w-x-it.w < min_waste) &&
                x + it.w <= node.x + node.w &&
                !(node.cut == 1 && (it.w < min_1_cut || it.w > max_1_cut)))
            {
                int current_nodes_size = _s.nodes.size();
                int current_node_id_at = node_id_at;

                if( x == x_from )
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        x+it.w,
                        node.y,
                        node.w-it.w,
                        node.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                    node_id_at++;

                    _s.nodes[node_id].w = it.w;

                    if(cut(it,prev_item_id,prev_item_visited,node_id,bin,node_id_at,true))
                    {
                        return true;
                    }
                    else
                    {
                        _s.nodes.resize(current_nodes_size);
                        node_id_at = current_node_id_at;
                        _s.nodes[node_id] = node;
                        _s.nodes[parent.node_id] = parent;
                    }
                }
                else if(x+it.w == node.x + node.w)
                {
                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        x,
                        node.y,
                        it.w,
                        node.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                    node_id_at++;

                    _s.nodes[node_id].w = node.w-it.w;

                    if(cut(it,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                    {
                        return true;
                    }
                    else
                    {
                        _s.nodes.resize(current_nodes_size);
                        node_id_at = current_node_id_at;
                        _s.nodes[node_id] = node;
                        _s.nodes[parent.node_id] = parent;
                    }
                }
                else
                {
                     _s.nodes[node_id].w = x-node.x;

                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        x,
                        node.y,
                        it.w,
                        node.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                    node_id_at++;

                    _s.nodes.push_back(Node{
                        bin.id,
                        node_id_at,
                        x+it.w,
                        node.y,
                        node.x + node.w -x- it.w,
                        node.h,
                        -2,
                        node.cut,
                        parent.node_id
                    });

                    _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+2,node_id_at);

                    node_id_at++;

                    if(cut(it,prev_item_id,prev_item_visited,node_id_at-2,bin,node_id_at,true))
                    {
                        return true;
                    }
                    else
                    {
                        _s.nodes.resize(current_nodes_size);
                        node_id_at = current_node_id_at;
                        _s.nodes[node_id] = node;
                        _s.nodes[parent.node_id] = parent;
                    }
                }
            }

            if(!orientation_locked && it.w != it.h)
            {
                Item rotated = it;
                std::swap(rotated.w,rotated.h);

                if( node.w - rotated.w >= min_waste &&
                    valid_x_cut(x,parent,bin) &&
                    valid_x_cut(x + rotated.w,parent,bin) &&
                    !(x>x_from && x-x_from < min_waste) &&
                    !(x+rotated.w < node.x+node.w && node.x+node.w-x-rotated.w < min_waste) &&
                    !(node.cut == 1 && (rotated.w < min_1_cut || rotated.w > max_1_cut)) &&
                    x + rotated.w <= node.x + node.w)
                {
                    int current_nodes_size = _s.nodes.size();
                    int current_node_id_at = node_id_at;

                    if( x == x_from )
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            x+rotated.w,
                            node.y,
                            node.w-rotated.w,
                            node.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                        node_id_at++;

                        _s.nodes[node_id].w = rotated.w;

                        if(cut(rotated,prev_item_id,prev_item_visited,node_id,bin,node_id_at,true))
                        {
                            return true;
                        }
                        else
                        {
                            _s.nodes.resize(current_nodes_size);
                            node_id_at = current_node_id_at;
                            _s.nodes[node_id] = node;
                            _s.nodes[parent.node_id] = parent;
                        }
                    }
                    else if(x+rotated.w == node.x + node.w)
                    {
                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            x,
                            node.y,
                            rotated.w,
                            node.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                        node_id_at++;

                        _s.nodes[node_id].w = node.w-rotated.w;

                        if(cut(rotated,prev_item_id,prev_item_visited,node_id_at-1,bin,node_id_at,true))
                        {
                            return true;
                        }
                        else
                        {
                            _s.nodes.resize(current_nodes_size);
                            node_id_at = current_node_id_at;
                            _s.nodes[node_id] = node;
                            _s.nodes[parent.node_id] = parent;
                        }
                    }
                    else
                    {
                        _s.nodes[node_id].w = x-node.x;

                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            x,
                            node.y,
                            rotated.w,
                            node.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+1,node_id_at);

                        node_id_at++;

                        _s.nodes.push_back(Node{
                            bin.id,
                            node_id_at,
                            x+rotated.w,
                            node.y,
                            node.x + node.w -x- rotated.w,
                            node.h,
                            -2,
                            node.cut,
                            parent.node_id
                        });

                        _s.nodes[parent.node_id].children.insert(_s.nodes[parent.node_id].children.begin()+child_index+2,node_id_at);

                        node_id_at++;

                        if(cut(rotated,prev_item_id,prev_item_visited,node_id_at-2,bin,node_id_at,true))
                        {
                            return true;
                        }
                        else
                        {
                            _s.nodes.resize(current_nodes_size);
                            node_id_at = current_node_id_at;
                            _s.nodes[node_id] = node;
                            _s.nodes[parent.node_id] = parent;
                        }
                    }
                }
            }
        }

        
    }

    return false;
}

bool Solver::cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id, Bin& bin, int& node_id_at, bool orientation_locked)
{
    Node node = _s.nodes[node_id];

    if(prev_item_visited == false && (prev_item_id == -1 || node.type == prev_item_id))
    {
        prev_item_visited = true;
    }

    //check if node is a branch: at this point every node is marked either as branch or as item
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

    if(!prev_item_visited)
    {
        return false;
    }

    // try subdivide node
    if(node.children.empty() && node.parent != -1)
    {
        if(try_subdivide_node(it,prev_item_id,prev_item_visited,node_id,bin,node_id_at,orientation_locked))
        {
            return true;
        }
    }

    // try to cut new part out of this node
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

std::vector<Position> Solver::get_possible_positions()
{
    
    return {};
}

void Solver::cut_out_position(Position& pos, int& node_id_at)
{
    if(_s.nodes[pos.node_id].cut % 2 == 0)
    {
        
    }
    else
    {

    }
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