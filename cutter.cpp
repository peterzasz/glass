#include "cutter.h"
#include "data.h"

bool Cutter::fast_try_vertical_cut( Item& it, int prev_item_id, bool& prev_item_visited, int node_id)
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
            if(valid_x_cut(x,y,x_from,y_from,node,it) && can_cut(_bin,node,x,y,it))
            {
                int nodes_size = _s.nodes.size();
                int children_size = _s.nodes[node_id].children.size();

                if(x == x_from)
                {
                    int child_id = _s.add_node(node.plate_id,x,node.y,it.w,node.h,-2,node.cut+1,node.node_id,false);

                    if( fast_cut(it,prev_item_id,prev_item_visited,child_id) )
                    {
                        return true;
                    }
                }
                else
                {
                    _s.add_node(node.plate_id,x_from,node.y,x-x_from,node.h,-2,node.cut+1,node.node_id,false);
                    int child_id = _s.add_node(node.plate_id,x,node.y,it.w,node.h,-2,node.cut+1,node.node_id,false);

                    if(cut(it,prev_item_id,prev_item_visited,child_id))
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

bool Cutter::fast_try_4_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id)
{
    Node node = _s.nodes[node_id];

    if(can_cut(_bin,node,node.x,node.y,it) && valid_4_cut(node.h,it.h))
    {
        int child_id = _s.add_node(node.plate_id,node.x,node.y,node.w,it.h,-2,4,node.node_id,false);
        _s.add_node(node.plate_id,node.x,node.y+it.h,node.w,node.h-it.h,-2,4,node.node_id,false);

        return fast_cut(it,prev_item_id,prev_item_visited,child_id);
    }
    else if(can_cut(_bin,node,node.x,node.y+node.h-it.h,it) && valid_4_cut(node.h,it.h))
    {
        _s.add_node(node.plate_id,node.x,node.y,node.w,node.h-it.h,-2,4,node.node_id,false);
        int child_id = _s.add_node(node.plate_id,node.x,node.y+node.h-it.h,node.w,it.h,-2,4,node.node_id,false);

        return fast_cut(it,prev_item_id,prev_item_visited,child_id);
    }

    return false;
}

bool Cutter::fast_try_horizontal_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id)
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
            if(valid_y_cut(x,y,x_from,y_from,node,it) && can_cut(_bin,node,x,y,it))
            {
                int nodes_size = _s.nodes.size();
                int children_size = _s.nodes[node_id].children.size();

                if(y == y_from)
                {
                    int child_id = _s.add_node(node.plate_id, node.x, y, node.w, it.h, -2, node.cut + 1, node.node_id, false);
                    

                    if( fast_cut(it,prev_item_id,prev_item_visited,child_id) )
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
                    
                    if(fast_cut(it,prev_item_id,prev_item_visited,child_id))
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

bool Cutter::fast_cut(Item& it, int prev_item_id, bool& prev_item_visited, int node_id)
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

        if(prev_item_visited && can_cut(_bin,node,node.x,node.y,it))
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
            if(fast_cut(it,prev_item_id, prev_item_visited, ch))
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
        return fast_try_vertical_cut(it, prev_item_id, prev_item_visited, node_id);
    }
    else
    {
        if(node.cut == 3)
        {
            return fast_try_4_cut(it, prev_item_id, prev_item_visited, node_id);
        }
        
        return fast_try_horizontal_cut(it, prev_item_id, prev_item_visited, node_id);
    }

    return false;
}