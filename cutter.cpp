#include "cutter.h"

#include <algorithm>
#include <utility>

// -----------------------------------------------------------------------------
// Cutter
// -----------------------------------------------------------------------------

bool Cutter::rotation_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    bool orientation_locked)
{
    if(prev_item_id == -1 || _s.nodes[node_id].type == prev_item_id)
    {
        prev_item_visited = true;
    }

    if(_s.nodes[node_id].type != -2)
    {
        return false;
    }

    if(item.w == _s.nodes[node_id].w && item.h == _s.nodes[node_id].h ||
       !orientation_locked && item.h == _s.nodes[node_id].w && item.w == _s.nodes[node_id].h)
    {
        if(!_s.nodes[node_id].children.empty())
        {
            return false;
        }

        if(prev_item_visited && no_defect_in_node(_s.nodes[node_id]))
        {
            _s.nodes[node_id].type = item.id;
            return true;
        }

        return false;
    }

    if(_s.nodes[node_id].cut == 4)
    {
        return false;
    }

    if(!_s.nodes[node_id].children.empty())
    {
        for(int child_id : _s.nodes[node_id].children)
        {
            if(rotation_cut(item, prev_item_id, prev_item_visited, child_id, orientation_locked))
            {
                return true;
            }
        }

        if(_s.nodes[node_id].cut == 3)
        {
            return false;
        }
    }

    if(!prev_item_visited)
    {
        return false;
    }

    if(_s.nodes[node_id].children.empty() && _s.nodes[node_id].parent != -1)
    {
        if(rotation_try_subdivide_node(item,prev_item_visited,node_id,orientation_locked))
        {
            return true;
        }
    }

    if(_s.nodes[node_id].cut % 2 == 0)
    {
        return rotation_try_vertical_cut(item,prev_item_visited,node_id,orientation_locked);
    }

    if(_s.nodes[node_id].cut == 3)
    {
        return rotation_try_4_cut(item,prev_item_visited,node_id,orientation_locked);
    }

    return rotation_try_horizontal_cut(item,prev_item_visited,node_id,orientation_locked);
}

bool Cutter::rotation_try_vertical_cut(Item& item, bool& prev_item_visited, int node_id, bool orientation_locked)
{
    int x_from = _s.nodes[node_id].children.empty() ? _s.nodes[node_id].x : _s.nodes[_s.nodes[node_id].children.back()].x +
                                                                            _s.nodes[_s.nodes[node_id].children.back()].w;
    int x_to = orientation_locked ? _s.nodes[node_id].x + _s.nodes[node_id].w - item.w :
                                    _s.nodes[node_id].x + _s.nodes[node_id].w - std::min(item.w,item.h);

    Item rotated = item;
    std::swap(rotated.w,rotated.h);
                
    for(int x = x_from; x <= x_to; ++x)
    {
        if( item_in_node(_s.nodes[node_id],x,_s.nodes[node_id].y, item) && 
            can_cut_vertical_strip(item, _s.nodes[node_id], x, x_from))
        {
            if(x == x_from)
            {
                int child_id = _s.add_node( _bin.id,
                                            x,
                                            _s.nodes[node_id].y,
                                            item.w,
                                            _s.nodes[node_id].h,
                                            -2,
                                            _s.nodes[node_id].cut + 1,
                                            node_id);
                
                return rotation_cut(item,-1,prev_item_visited,child_id,true);
            }
            else
            {
                _s.add_node(_bin.id,
                            x_from,
                            _s.nodes[node_id].y,
                            x-x_from,
                            _s.nodes[node_id].h,
                            -2,
                            _s.nodes[node_id].cut + 1,
                            node_id);
                
                int child_id = _s.add_node( _bin.id,
                                            x,
                                            _s.nodes[node_id].y,
                                            item.w,
                                            _s.nodes[node_id].h,
                                            -2,
                                            _s.nodes[node_id].cut + 1,
                                            node_id);
                    
                return rotation_cut(item,-1,prev_item_visited,child_id,true);
            }
        }

        if(!orientation_locked && item.w != item.h)
        {

            if( item_in_node(_s.nodes[node_id],x,_s.nodes[node_id].y, rotated) && 
                can_cut_vertical_strip(rotated, _s.nodes[node_id], x, x_from))
            {
                if(x == x_from)
                {
                    int child_id = _s.add_node( _bin.id,
                                                x,
                                                _s.nodes[node_id].y,
                                                rotated.w,
                                                _s.nodes[node_id].h,
                                                -2,
                                                _s.nodes[node_id].cut + 1,
                                                node_id);
                    
                    return rotation_cut(rotated,-1,prev_item_visited,child_id,true);
                }
                else
                {
                    _s.add_node(_bin.id,
                                x_from,
                                _s.nodes[node_id].y,
                                x-x_from,
                                _s.nodes[node_id].h,
                                -2,
                                _s.nodes[node_id].cut + 1,
                                node_id);
                    
                    int child_id = _s.add_node( _bin.id,
                                                x,
                                                _s.nodes[node_id].y,
                                                rotated.w,
                                                _s.nodes[node_id].h,
                                                -2,
                                                _s.nodes[node_id].cut + 1,
                                                node_id);
                        
                    return rotation_cut(rotated,-1,prev_item_visited,child_id,true);
                }
            }
        }
    }

    return false;
}

bool Cutter::rotation_try_horizontal_cut(Item& item, bool& prev_item_visited, int node_id, bool orientation_locked)
{
    int y_from = _s.nodes[node_id].children.empty() ? _s.nodes[node_id].y : _s.nodes[_s.nodes[node_id].children.back()].y +
                                                                            _s.nodes[_s.nodes[node_id].children.back()].h;

    int y_to = orientation_locked ? _s.nodes[node_id].y + _s.nodes[node_id].h - item.h :
                                    _s.nodes[node_id].y + _s.nodes[node_id].h - std::min(item.h,item.w);

    Item rotated = item;
    std::swap(rotated.w,rotated.h);

    for(int y = y_from; y <= y_to; ++y)
    {
        if( item_in_node(_s.nodes[node_id],_s.nodes[node_id].x,y,item) && 
            can_cut_horizontal_strip(item, _s.nodes[node_id], y, y_from))
        {
            if(y == y_from)
            {
                int child_id = _s.add_node( _bin.id,
                                            _s.nodes[node_id].x,
                                            y,
                                            _s.nodes[node_id].w,
                                            item.h,
                                            -2,
                                            _s.nodes[node_id].cut + 1,
                                            node_id);
                
                return rotation_cut(item,-1,prev_item_visited,child_id,true);
            }
            else
            {
                _s.add_node(_bin.id,
                            _s.nodes[node_id].x,
                            y_from,
                            _s.nodes[node_id].w,
                            y-y_from,
                            -2,
                            _s.nodes[node_id].cut + 1,
                            node_id);
                
                int child_id = _s.add_node( _bin.id,
                                            _s.nodes[node_id].x,
                                            y,
                                            _s.nodes[node_id].w,
                                            item.h,
                                            -2,
                                            _s.nodes[node_id].cut + 1,
                                            node_id);
                    
                return rotation_cut(item,-1,prev_item_visited,child_id,true);
            }
        }

        if(!orientation_locked && item.w != item.h)
        {
            if( item_in_node(_s.nodes[node_id],_s.nodes[node_id].x,y,rotated) && 
                can_cut_horizontal_strip(rotated, _s.nodes[node_id], y, y_from))
            {
                if(y == y_from)
                {
                    int child_id = _s.add_node( _bin.id,
                                                _s.nodes[node_id].x,
                                                y,
                                                _s.nodes[node_id].w,
                                                rotated.h,
                                                -2,
                                                _s.nodes[node_id].cut + 1,
                                                node_id);
                    
                    return rotation_cut(rotated,-1,prev_item_visited,child_id,true);
                }
                else
                {
                    _s.add_node(_bin.id,
                                _s.nodes[node_id].x,
                                y_from,
                                _s.nodes[node_id].w,
                                y-y_from,
                                -2,
                                _s.nodes[node_id].cut + 1,
                                node_id);
                    
                    int child_id = _s.add_node( _bin.id,
                                                _s.nodes[node_id].x,
                                                y,
                                                _s.nodes[node_id].w,
                                                rotated.h,
                                                -2,
                                                _s.nodes[node_id].cut + 1,
                                                node_id);
                        
                    return rotation_cut(rotated,-1,prev_item_visited,child_id,true);
                }
            }
        }
    }

    return false;
}

bool Cutter::rotation_try_4_cut(Item& item, bool& prev_item_visited, int node_id, bool orientation_locked)
{
    if(item.w != _s.nodes[node_id].w)
    {
        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w,rotated.h);

            return rotation_try_4_cut(rotated,prev_item_visited,node_id,true);
        }

        return false;
    }

    if(can_do_bottom_4_cut(_s.nodes[node_id],item))
    {
        int child_id = _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y,
            item.w,
            item.h,
            -2,
            4,
            node_id
        );

        _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y + item.h,
            item.w,
            _s.nodes[node_id].h - item.h,
            -2,
            4,
            node_id
        );

        return rotation_cut(item,-1,prev_item_visited,child_id,true);
    }
    else if(can_do_top_4_cut(_s.nodes[node_id],item))
    {
        _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y,
            item.w,
            _s.nodes[node_id].h - item.h,
            -2,
            4,
            node_id
        );

        int child_id = _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y + _s.nodes[node_id].h - item.h,
            item.w,
            item.h,
            -2,
            4,
            node_id
        );

        return rotation_cut(item,-1,prev_item_visited,child_id,true);
    }
    
    return false;
}

bool Cutter::rotation_try_subdivide_node(Item& item, bool& prev_item_visited, int node_id, bool orientation_locked)
{
    Item rotated = item;
    std::swap(rotated.w,rotated.h);

    if(_s.nodes[node_id].cut % 2 == 1)
    {
        int x_from = _s.nodes[node_id].x;
        int x_to = orientation_locked ? _s.nodes[node_id].x + _s.nodes[node_id].w - item.w : 
                                        _s.nodes[node_id].x + _s.nodes[node_id].w - std::min(item.w,item.h);

        if(orientation_locked && x_from == x_to)
        {
            return false;
        }

        for(int x = x_from; x <= x_to; ++x)
        {
            if( x_from != _s.nodes[node_id].x + _s.nodes[node_id].w - item.w &&
                item_in_node(_s.nodes[node_id],x,_s.nodes[node_id].y,item) && 
                can_subdivide_by_vertical_strip(item,_s.nodes[node_id],x,x_from,_s.nodes[node_id].x + _s.nodes[node_id].w - item.w))
            {   
                if(x == x_from)
                {
                    _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x + item.w,
                        _s.nodes[node_id].y,
                        _s.nodes[node_id].w - item.w,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id)+1
                    );

                    _s.nodes[node_id].w = item.w;

                    return rotation_cut(item,-1,prev_item_visited,node_id,true);
                }
                else if(x == _s.nodes[node_id].x + _s.nodes[node_id].w - item.w)
                {

                    int child_id = _s.add_node(
                        _bin.id,
                        x,
                        _s.nodes[node_id].y,
                        item.w,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id) + 1
                    );

                    _s.nodes[node_id].w = _s.nodes[node_id].w - item.w;

                    return rotation_cut(item,-1,prev_item_visited,child_id,true);
                }
                else
                {
                    int node_child_id = which_child(node_id);

                    int child_id = _s.add_node(
                        _bin.id,
                        x,
                        _s.nodes[node_id].y,
                        item.w,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 1
                    );

                    _s.add_node(
                        _bin.id,
                        x+item.w,
                        _s.nodes[node_id].y,
                        _s.nodes[node_id].x + _s.nodes[node_id].w - item.w - x,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 2
                    );

                    _s.nodes[node_id].w = x - x_from;

                    return rotation_cut(item,-1,prev_item_visited,child_id,true);
                }  
            }

            if( !orientation_locked && item.w != item.h &&
                x_from != _s.nodes[node_id].x + _s.nodes[node_id].w - rotated.w &&
                item_in_node(_s.nodes[node_id],x,_s.nodes[node_id].y,rotated) && 
                can_subdivide_by_vertical_strip(rotated,_s.nodes[node_id],x,x_from,_s.nodes[node_id].x + _s.nodes[node_id].w - rotated.w))
            {   
                if(x == x_from)
                {
                    _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x + rotated.w,
                        _s.nodes[node_id].y,
                        _s.nodes[node_id].w - rotated.w,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id)+1
                    );

                    _s.nodes[node_id].w = rotated.w;

                    return rotation_cut(rotated,-1,prev_item_visited,node_id,true);
                }
                else if(x == _s.nodes[node_id].x + _s.nodes[node_id].w - rotated.w)
                {

                    int child_id = _s.add_node(
                        _bin.id,
                        x,
                        _s.nodes[node_id].y,
                        rotated.w,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id) + 1
                    );

                    _s.nodes[node_id].w = _s.nodes[node_id].w - rotated.w;

                    return rotation_cut(rotated,-1,prev_item_visited,child_id, true);
                }
                else
                {
                    int node_child_id = which_child(node_id);

                    int child_id = _s.add_node(
                        _bin.id,
                        x,
                        _s.nodes[node_id].y,
                        rotated.w,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 1
                    );

                    _s.add_node(
                        _bin.id,
                        x+rotated.w,
                        _s.nodes[node_id].y,
                        _s.nodes[node_id].x + _s.nodes[node_id].w - rotated.w - x,
                        _s.nodes[node_id].h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 2
                    );

                    _s.nodes[node_id].w = x - x_from;

                    return rotation_cut(rotated,-1,prev_item_visited,child_id,true);
                }
            }
        }
    }
    else
    {
        int y_from = _s.nodes[node_id].y;
        int y_to = orientation_locked ? _s.nodes[node_id].y + _s.nodes[node_id].h - item.h:
                                        _s.nodes[node_id].y + _s.nodes[node_id].h - std::min(item.w,item.h);

        if(orientation_locked && y_from == y_to)
        {
            return false;
        }

        for(int y = y_from; y <= y_to; ++y)
        {
            if( y_from != _s.nodes[node_id].y + _s.nodes[node_id].h - item.h &&
                item_in_node(_s.nodes[node_id],_s.nodes[node_id].x,y,item) &&
                can_subdivide_by_horizontal_strip(item,_s.nodes[node_id],y,y_from,_s.nodes[node_id].y + _s.nodes[node_id].h - item.h))
            {
                if(y == y_from)
                {
                    _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        _s.nodes[node_id].y + item.h,
                        _s.nodes[node_id].w,
                        _s.nodes[node_id].h - item.h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id)+1
                    );

                    _s.nodes[node_id].h = item.h;

                    return rotation_cut(item,-1,prev_item_visited,node_id,true);
                }
                else if(y == _s.nodes[node_id].y + _s.nodes[node_id].h - item.h)
                {

                    int child_id = _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        y,
                        _s.nodes[node_id].w,
                        item.h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id) + 1
                    );

                    _s.nodes[node_id].h = _s.nodes[node_id].h - item.h;

                    return rotation_cut(item,-1,prev_item_visited,child_id,true);
                }
                else
                {
                    int node_child_id = which_child(node_id);

                    int child_id = _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        y,
                        _s.nodes[node_id].w,
                        item.h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 1
                    );

                    _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        y+item.h,
                        _s.nodes[node_id].w,
                        _s.nodes[node_id].y + _s.nodes[node_id].h - item.h - y,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 2
                    );

                    _s.nodes[node_id].h = y - y_from;

                    return rotation_cut(item,-1,prev_item_visited,child_id,true);
                }
            }

            if( !orientation_locked && item.w != item.h &&
                y_from != _s.nodes[node_id].y + _s.nodes[node_id].h - rotated.h &&
                item_in_node(_s.nodes[node_id],_s.nodes[node_id].x,y,rotated) &&
                can_subdivide_by_horizontal_strip(rotated,_s.nodes[node_id],y,y_from,_s.nodes[node_id].y + _s.nodes[node_id].h - rotated.h))
            {
                if(y == y_from)
                {
                    _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        _s.nodes[node_id].y + rotated.h,
                        _s.nodes[node_id].w,
                        _s.nodes[node_id].h - rotated.h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id)+1
                    );

                    _s.nodes[node_id].h = rotated.h;

                    return rotation_cut(rotated,-1,prev_item_visited,node_id,true);
                }
                else if(y == _s.nodes[node_id].y + _s.nodes[node_id].h - rotated.h)
                {

                    int child_id = _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        y,
                        _s.nodes[node_id].w,
                        rotated.h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        which_child(node_id) + 1
                    );

                    _s.nodes[node_id].h = _s.nodes[node_id].h - rotated.h;

                    return rotation_cut(rotated,-1,prev_item_visited,child_id,true);
                }
                else
                {
                    int node_child_id = which_child(node_id);

                    int child_id = _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        y,
                        _s.nodes[node_id].w,
                        rotated.h,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 1
                    );

                    _s.add_node(
                        _bin.id,
                        _s.nodes[node_id].x,
                        y+rotated.h,
                        _s.nodes[node_id].w,
                        _s.nodes[node_id].y + _s.nodes[node_id].h - rotated.h - y,
                        -2,
                        _s.nodes[node_id].cut,
                        _s.nodes[node_id].parent,
                        false,
                        node_child_id + 2
                    );

                    _s.nodes[node_id].h = y - y_from;

                    return rotation_cut(rotated,-1,prev_item_visited,child_id,true);
                }
            }
        }
    }

    return false;
}

// -----------------------------------------------------------------------------
// PlacementGenerator
// -----------------------------------------------------------------------------

void PlacementGenerator::generate_placements(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    Change change,
    std::vector<Change>& placements)
{
    if(prev_item_id == -1 || _s.nodes[node_id].type == prev_item_id)
    {
        prev_item_visited = true;
    }

    if(_s.nodes[node_id].type != -2)
    {
        return;
    }

    if(item.w == _s.nodes[node_id].w && item.h == _s.nodes[node_id].h)
    {
        if(!_s.nodes[node_id].children.empty())
        {
            return;
        }

        if(prev_item_visited && no_defect_in_node(_s.nodes[node_id]))
        {
            _s.nodes[node_id].type = item.id;
            change.updates.push_back(NodeChange{node_id,_s.nodes[node_id]});
            placements.push_back(change);

            _s.nodes[node_id].type = -2;

            return;
        }

        return;
    }

    if(_s.nodes[node_id].cut == 4)
    {
        return;
    }

    if(!_s.nodes[node_id].children.empty())
    {
        std::vector<int> children = _s.nodes[node_id].children;

        for(int child_id : children)
        {
            generate_placements(item, prev_item_id, prev_item_visited, child_id,change,placements);
        }

        if(_s.nodes[node_id].cut == 3)
        {
            return;
        }
    }

    if(!prev_item_visited)
    {
        return;
    }

    if(_s.nodes[node_id].children.empty() && _s.nodes[node_id].parent != -1)
    {
        try_subdivide_node(item,prev_item_visited,node_id,change,placements);
    }

    if(_s.nodes[node_id].cut % 2 == 0)
    {
        try_vertical_cut(item,prev_item_visited,node_id,change,placements);
    }
    else if(_s.nodes[node_id].cut == 3)
    {
        try_4_cut(item,prev_item_visited,node_id,change,placements);
    }
    else
    {
        try_horizontal_cut(item,prev_item_visited,node_id,change,placements);
    }
}

std::vector<int> PlacementGenerator::possible_x_coords_for_vertical_cut(const Item& item, const Node& node, int x_from, int x_to) const
{
    std::vector<int> x_coords;

    for(int x = x_from; x <= x_to; ++x)
    {
        if(can_cut_vertical_strip(item,node,x,x_from))
        {
            x_coords.push_back(x);
            break;
        }
    }

    if(x_coords.empty())
    {
        return x_coords;
    }

    for(int x = x_to; x>x_coords[0]; --x)
    {
        if(can_cut_vertical_strip(item,node,x,x_from))
        {
            x_coords.push_back(x);
            break;
        }
    }

    if(x_coords.size() == 1)
    {
        return x_coords;
    }

    for(const auto& defect: _bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            int back = defect.x-item.w;
            int front = defect.x+defect.w;

            if(back > x_coords[0] && back < x_coords[1] && can_cut_vertical_strip(item,node,back,x_from))
            {
                x_coords.push_back(back);
            }


            if(front > x_coords[0] && front < x_coords[1] && can_cut_vertical_strip(item,node,front,x_from))
            {
                x_coords.push_back(front);
            }
        }
    }

    std::sort(x_coords.begin(), x_coords.end());
    x_coords.erase(
        std::unique(x_coords.begin(), x_coords.end()),
        x_coords.end()
    );

    return x_coords;
}

void PlacementGenerator::try_vertical_cut(Item& item, bool& prev_item_visited, int node_id, Change change, std::vector<Change>& placements)
{
    int x_from = _s.nodes[node_id].children.empty() ? _s.nodes[node_id].x : _s.nodes[_s.nodes[node_id].children.back()].x +
                                                                            _s.nodes[_s.nodes[node_id].children.back()].w;
    int x_to = _s.nodes[node_id].x + _s.nodes[node_id].w - item.w;

                
    for(int x : possible_x_coords_for_vertical_cut(item,_s.nodes[node_id],x_from,x_to))
    {
        int save_nodes_size = _s.nodes.size();
        int save_children_size = _s.nodes[node_id].children.size();

        Change candidate_change = change;

        if(x == x_from)
        {

            int child_id = _s.add_node( _bin.id,
                                        x,
                                        _s.nodes[node_id].y,
                                        item.w,
                                        _s.nodes[node_id].h,
                                        -2,
                                        _s.nodes[node_id].cut + 1,
                                        node_id);

            candidate_change.updates.push_back(NodeChange{
                child_id,
                _s.nodes[child_id]
            });

            candidate_change.updates.push_back(NodeChange{
                node_id,
                _s.nodes[node_id]
            });
            
            generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
        }
        else
        {
            int before_child_id = _s.add_node(
                _bin.id,
                x_from,
                _s.nodes[node_id].y,
                x-x_from,
                _s.nodes[node_id].h,
                -2,
                _s.nodes[node_id].cut + 1,
                node_id);
            
            int child_id = _s.add_node( 
                _bin.id,
                x,
                _s.nodes[node_id].y,
                item.w,
                _s.nodes[node_id].h,
                -2,
                _s.nodes[node_id].cut + 1,
                node_id);

            candidate_change.updates.push_back(NodeChange{
                before_child_id,
                _s.nodes[before_child_id]
            });

            candidate_change.updates.push_back(NodeChange{
                child_id,
                _s.nodes[child_id]
            });

            candidate_change.updates.push_back(NodeChange{
                node_id,
                _s.nodes[node_id]
            });
                
            generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
        }

        _s.nodes.resize(save_nodes_size);
        _s.nodes[node_id].children.resize(save_children_size);

    }

    return;
}

std::vector<int> PlacementGenerator::possible_y_coords_for_horizontal_cut(const Item& item, const Node& node, int y_from, int y_to) const
{
    std::vector<int> y_coords;

    for(int y = y_from; y <= y_to; ++y)
    {
        if(can_cut_horizontal_strip(item,node,y,y_from))
        {
            y_coords.push_back(y);
            break;
        }
    }

    if(y_coords.empty())
    {
        return y_coords;
    }

    for(int y = y_to; y>y_coords[0]; --y)
    {
        if(can_cut_horizontal_strip(item,node,y,y_from))
        {
            y_coords.push_back(y);
            break;
        }
    }

    if(y_coords.size() == 1)
    {
        return y_coords;
    }

    for(const auto& defect: _bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            int bottom = defect.y-item.h;
            int top = defect.y+defect.h;

            if(bottom > y_coords[0] && bottom < y_coords[1] && can_cut_horizontal_strip(item,node,bottom,y_from))
            {
                y_coords.push_back(bottom);
            }


            if(top > y_coords[0] && top < y_coords[1] && can_cut_horizontal_strip(item,node,top,y_from))
            {
                y_coords.push_back(top);
            }
        }
    }

    std::sort(y_coords.begin(), y_coords.end());
    y_coords.erase(
        std::unique(y_coords.begin(), y_coords.end()),
        y_coords.end()
    );

    return y_coords;
}

void PlacementGenerator::try_horizontal_cut(Item& item, bool& prev_item_visited, int node_id, Change change, std::vector<Change>& placements)
{
    int y_from = _s.nodes[node_id].children.empty() ? _s.nodes[node_id].y : _s.nodes[_s.nodes[node_id].children.back()].y +
                                                                            _s.nodes[_s.nodes[node_id].children.back()].h;

    int y_to = _s.nodes[node_id].y + _s.nodes[node_id].h - item.h;

    for(int y: possible_y_coords_for_horizontal_cut(item,_s.nodes[node_id],y_from,y_to))
    {
        int save_nodes_size = _s.nodes.size();
        int save_children_size = _s.nodes[node_id].children.size();

        Change candidate_change = change;

        if(y == y_from)
        {
            int child_id = _s.add_node( 
                _bin.id,
                _s.nodes[node_id].x,
                y,
                _s.nodes[node_id].w,
                item.h,
                -2,
                _s.nodes[node_id].cut + 1,
                node_id);

            candidate_change.updates.push_back(NodeChange{
                child_id,
                _s.nodes[child_id]
            });

            candidate_change.updates.push_back(NodeChange{
                node_id,
                _s.nodes[node_id]
            });
            
            generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
        }
        else
        {
            int before_child_id = _s.add_node(
                _bin.id,
                _s.nodes[node_id].x,
                y_from,
                _s.nodes[node_id].w,
                y-y_from,
                -2,
                _s.nodes[node_id].cut + 1,
                node_id);
            
            int child_id = _s.add_node( _bin.id,
                                        _s.nodes[node_id].x,
                                        y,
                                        _s.nodes[node_id].w,
                                        item.h,
                                        -2,
                                        _s.nodes[node_id].cut + 1,
                                        node_id);

            candidate_change.updates.push_back(NodeChange{
                before_child_id,
                _s.nodes[before_child_id]
            });

            candidate_change.updates.push_back(NodeChange{
                child_id,
                _s.nodes[child_id]
            });

            candidate_change.updates.push_back(NodeChange{
                node_id,
                _s.nodes[node_id]
            });
                
            generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
        }

        _s.nodes.resize(save_nodes_size);
        _s.nodes[node_id].children.resize(save_children_size);
    }

    return;
}

void PlacementGenerator::try_4_cut(Item& item, bool& prev_item_visited, int node_id, Change change, std::vector<Change>& placements)
{
    if(item.w != _s.nodes[node_id].w)
    {
        return;
    }

    int save_nodes_size = _s.nodes.size();
    int save_children_size = _s.nodes[node_id].children.size();

    if(can_do_bottom_4_cut(_s.nodes[node_id],item))
    {
        int child_id = _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y,
            item.w,
            item.h,
            -2,
            4,
            node_id
        );

        int other_child_id = _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y + item.h,
            item.w,
            _s.nodes[node_id].h - item.h,
            -2,
            4,
            node_id
        );

        change.updates.push_back(NodeChange{
            child_id,
            _s.nodes[child_id]
        });

        change.updates.push_back(NodeChange{
            other_child_id,
            _s.nodes[other_child_id]
        });

        change.updates.push_back(NodeChange{
            node_id,
            _s.nodes[node_id]
        });

        generate_placements(item,-1,prev_item_visited,child_id,change,placements);
    }
    else if(can_do_top_4_cut(_s.nodes[node_id],item))
    {
        int other_child_id = _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y,
            item.w,
            _s.nodes[node_id].h - item.h,
            -2,
            4,
            node_id
        );

        int child_id = _s.add_node(
            _bin.id,
            _s.nodes[node_id].x,
            _s.nodes[node_id].y + _s.nodes[node_id].h - item.h,
            item.w,
            item.h,
            -2,
            4,
            node_id
        );

        change.updates.push_back(NodeChange{
            child_id,
            _s.nodes[child_id]
        });

        change.updates.push_back(NodeChange{
            other_child_id,
            _s.nodes[other_child_id]
        });

        change.updates.push_back(NodeChange{
            node_id,
            _s.nodes[node_id]
        });

        generate_placements(item,-1,prev_item_visited,child_id,change,placements);
    }
    
    _s.nodes.resize(save_nodes_size);
    _s.nodes[node_id].children.resize(save_children_size);

    return;
}

std::vector<int> PlacementGenerator::possible_x_coords_for_subdivision(const Item& item, const Node& node, int x_from, int x_to) const
{
    std::vector<int> x_coords;

    for(int x = x_from; x <= x_to; ++x)
    {
        if(can_subdivide_by_vertical_strip(item,node,x,x_from,x_to))
        {
            x_coords.push_back(x);
            break;
        }
    }

    if(x_coords.empty())
    {
        return x_coords;
    }

    for(int x = x_to; x>x_coords[0]; --x)
    {
        if(can_subdivide_by_vertical_strip(item,node,x,x_from,x_to))
        {
            x_coords.push_back(x);
            break;
        }
    }

    if(x_coords.size() == 1)
    {
        return x_coords;
    }

    for(const auto& defect: _bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            int back = defect.x-item.w;
            int front = defect.x+defect.w;

            if(back > x_coords[0] && back < x_coords[1] && can_subdivide_by_vertical_strip(item,node,back,x_from,x_to))
            {
                x_coords.push_back(back);
            }


            if(front > x_coords[0] && front < x_coords[1] && can_subdivide_by_vertical_strip(item,node,front,x_from,x_to))
            {
                x_coords.push_back(front);
            }
        }
    }

    std::sort(x_coords.begin(), x_coords.end());
    x_coords.erase(
        std::unique(x_coords.begin(), x_coords.end()),
        x_coords.end()
    );

    return x_coords;
}

std::vector<int> PlacementGenerator::possible_y_coords_for_subdivision(const Item& item, const Node& node, int y_from, int y_to) const
{
    std::vector<int> y_coords;

    for(int y = y_from; y <= y_to; ++y)
    {
        if(can_subdivide_by_horizontal_strip(item,node,y,y_from,y_to))
        {
            y_coords.push_back(y);
            break;
        }
    }

    if(y_coords.empty())
    {
        return y_coords;
    }

    for(int y = y_to; y>y_coords[0]; --y)
    {
        if(can_subdivide_by_horizontal_strip(item,node,y,y_from,y_to))
        {
            y_coords.push_back(y);
            break;
        }
    }

    if(y_coords.size() == 1)
    {
        return y_coords;
    }

    for(const auto& defect: _bin.defects)
    {
        if(defect_in_node(node,defect))
        {
            int bottom = defect.y-item.h;
            int top = defect.y+defect.h;

            if(bottom > y_coords[0] && bottom < y_coords[1] && can_subdivide_by_horizontal_strip(item,node,bottom,y_from,y_to))
            {
                y_coords.push_back(bottom);
            }


            if(top > y_coords[0] && top < y_coords[1] && can_subdivide_by_horizontal_strip(item,node,top,y_from,y_to))
            {
                y_coords.push_back(top);
            }
        }
    }

    std::sort(y_coords.begin(), y_coords.end());
    y_coords.erase(
        std::unique(y_coords.begin(), y_coords.end()),
        y_coords.end()
    );

    return y_coords;
}

void PlacementGenerator::try_subdivide_node(Item& item, bool& prev_item_visited, int node_id, Change change, std::vector<Change>& placements)
{
    if(_s.nodes[node_id].cut % 2 == 1)
    {
        int x_from = _s.nodes[node_id].x;
        int x_to = _s.nodes[node_id].x + _s.nodes[node_id].w - item.w;

        if(x_from == x_to)
        {
            return;
        }

        for(int x : possible_x_coords_for_subdivision(item,_s.nodes[node_id],x_from,x_to))
        {
            std::vector<int> save_children = _s.nodes[_s.nodes[node_id].parent].children;
            int save_nodes_size = _s.nodes.size();
            Node save_node = _s.nodes[node_id];

            Change candidate_change = change;
            
            if(x == x_from)
            {
                int child_id = _s.add_node(
                    _bin.id,
                    _s.nodes[node_id].x + item.w,
                    _s.nodes[node_id].y,
                    _s.nodes[node_id].w - item.w,
                    _s.nodes[node_id].h,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    which_child(node_id)+1
                );

                _s.nodes[node_id].w = item.w;

                candidate_change.updates.push_back(NodeChange{
                    child_id,
                    _s.nodes[child_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    node_id,
                    _s.nodes[node_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    _s.nodes[node_id].parent,
                    _s.nodes[_s.nodes[node_id].parent]
                });

                generate_placements(item,-1,prev_item_visited,node_id,candidate_change,placements);
            }
            else if(x == x_to)
            {
                int child_id = _s.add_node(
                    _bin.id,
                    x,
                    _s.nodes[node_id].y,
                    item.w,
                    _s.nodes[node_id].h,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    which_child(node_id) + 1
                );

                _s.nodes[node_id].w = _s.nodes[node_id].w - item.w;

                candidate_change.updates.push_back(NodeChange{
                    child_id,
                    _s.nodes[child_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    node_id,
                    _s.nodes[node_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    _s.nodes[node_id].parent,
                    _s.nodes[_s.nodes[node_id].parent]
                });

                generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
            }
            else
            {
                int node_child_id = which_child(node_id);

                int child_id = _s.add_node(
                    _bin.id,
                    x,
                    _s.nodes[node_id].y,
                    item.w,
                    _s.nodes[node_id].h,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    node_child_id + 1
                );

                int other_child_id = _s.add_node(
                    _bin.id,
                    x+item.w,
                    _s.nodes[node_id].y,
                    _s.nodes[node_id].x + _s.nodes[node_id].w - item.w - x,
                    _s.nodes[node_id].h,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    node_child_id + 2
                );

                _s.nodes[node_id].w = x - x_from;

                candidate_change.updates.push_back(NodeChange{
                    child_id,
                    _s.nodes[child_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    other_child_id,
                    _s.nodes[other_child_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    node_id,
                    _s.nodes[node_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    _s.nodes[node_id].parent,
                    _s.nodes[_s.nodes[node_id].parent]
                });

                generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
            }

            _s.nodes.resize(save_nodes_size);
            _s.nodes[_s.nodes[node_id].parent].children = save_children;
            _s.nodes[node_id] = save_node;
            
        }
    }
    else
    {
        int y_from = _s.nodes[node_id].y;
        int y_to =  _s.nodes[node_id].y + _s.nodes[node_id].h - item.h;

        if(y_from == y_to)
        {
            return;
        }

        for(int y : possible_y_coords_for_subdivision(item,_s.nodes[node_id],y_from,y_to))
        {
            std::vector<int> save_children = _s.nodes[_s.nodes[node_id].parent].children;
            int save_nodes_size = _s.nodes.size();
            Node save_node = _s.nodes[node_id];

            Change candidate_change = change;

            if(y == y_from)
            {
                int child_id = _s.add_node(
                    _bin.id,
                    _s.nodes[node_id].x,
                    _s.nodes[node_id].y + item.h,
                    _s.nodes[node_id].w,
                    _s.nodes[node_id].h - item.h,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    which_child(node_id)+1
                );

                _s.nodes[node_id].h = item.h;

                candidate_change.updates.push_back(NodeChange{
                    child_id,
                    _s.nodes[child_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    node_id,
                    _s.nodes[node_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    _s.nodes[node_id].parent,
                    _s.nodes[_s.nodes[node_id].parent]
                });

                generate_placements(item,-1,prev_item_visited,node_id,candidate_change,placements);
            }
            else if(y == y_to)
            {

                int child_id = _s.add_node(
                    _bin.id,
                    _s.nodes[node_id].x,
                    y,
                    _s.nodes[node_id].w,
                    item.h,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    which_child(node_id) + 1
                );

                _s.nodes[node_id].h = _s.nodes[node_id].h - item.h;

                candidate_change.updates.push_back(NodeChange{
                    child_id,
                    _s.nodes[child_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    node_id,
                    _s.nodes[node_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    _s.nodes[node_id].parent,
                    _s.nodes[_s.nodes[node_id].parent]
                });

                generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
            }
            else
            {
                int node_child_id = which_child(node_id);

                int child_id = _s.add_node(
                    _bin.id,
                    _s.nodes[node_id].x,
                    y,
                    _s.nodes[node_id].w,
                    item.h,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    node_child_id + 1
                );

                int other_child = _s.add_node(
                    _bin.id,
                    _s.nodes[node_id].x,
                    y+item.h,
                    _s.nodes[node_id].w,
                    _s.nodes[node_id].y + _s.nodes[node_id].h - item.h - y,
                    -2,
                    _s.nodes[node_id].cut,
                    _s.nodes[node_id].parent,
                    false,
                    node_child_id + 2
                );

                _s.nodes[node_id].h = y - y_from;

                candidate_change.updates.push_back(NodeChange{
                    child_id,
                    _s.nodes[child_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    other_child,
                    _s.nodes[other_child]
                });

                candidate_change.updates.push_back(NodeChange{
                    node_id,
                    _s.nodes[node_id]
                });

                candidate_change.updates.push_back(NodeChange{
                    _s.nodes[node_id].parent,
                    _s.nodes[_s.nodes[node_id].parent]
                });

                generate_placements(item,-1,prev_item_visited,child_id,candidate_change,placements);
            }

            _s.nodes.resize(save_nodes_size);
            _s.nodes[_s.nodes[node_id].parent].children = save_children;
            _s.nodes[node_id] = save_node;
        }
    }

    return;
}

int Cutter::which_child(int node_id)
{
    if(_s.nodes[node_id].parent == -1)
    {
        return -1;
    }

    const Node& parent = _s.nodes[_s.nodes[node_id].parent];

    for(size_t i = 0; i < parent.children.size(); ++i)
    {
        if(parent.children[i] == node_id)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

int PlacementGenerator::which_child(int node_id)
{
    if(_s.nodes[node_id].parent == -1)
    {
        return -1;
    }

    const Node& parent = _s.nodes[_s.nodes[node_id].parent];

    for(size_t i = 0; i < parent.children.size(); ++i)
    {
        if(parent.children[i] == node_id)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void apply_change(Solution& s, const Change& c)
{
    for(const auto& node_change : c.updates )
    {
        if(node_change.node_id >= s.nodes.size())
        {
            s.nodes.resize(node_change.node_id + 1);
        }

        s.nodes[node_change.node_id] = node_change.node;
    }
}