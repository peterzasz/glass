#include "cutter.h"

#include <algorithm>
#include <utility>

// -----------------------------------------------------------------------------
// Cutter: fast method
// -----------------------------------------------------------------------------

bool Cutter::fast_try_vertical_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];

    const int x_from = node.children.empty()
        ? node.x
        : _s.nodes[node.children.back()].x + _s.nodes[node.children.back()].w;

    const int x_to = node.x + node.w - item.w;
    const int y_from = node.y;
    const int y_to = node.y + node.h - item.h;

    for(int x = x_from; x <= x_to; ++x)
    {
        for(int y = y_from; y <= y_to; ++y)
        {
            if(!valid_x_cut(x, y, x_from, y_from, node, item) ||
               !can_cut(node, x, y, item))
            {
                continue;
            }

            const std::size_t nodes_size = _s.nodes.size();
            const std::size_t children_size = _s.nodes[node_id].children.size();

            int child_id;

            if(x == x_from)
            {
                child_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    item.w,
                    node.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }
            else
            {
                _s.add_node(
                    node.plate_id,
                    x_from,
                    node.y,
                    x - x_from,
                    node.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);

                child_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    item.w,
                    node.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }

            if(fast_cut(item, prev_item_id, prev_item_visited, child_id))
            {
                return true;
            }

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id].children.resize(children_size);
        }
    }

    return false;
}

bool Cutter::fast_try_horizontal_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];

    const int x_from = node.x;
    const int x_to = node.x + node.w - item.w;

    const int y_from = node.children.empty()
        ? node.y
        : _s.nodes[node.children.back()].y + _s.nodes[node.children.back()].h;

    const int y_to = node.y + node.h - item.h;

    for(int y = y_from; y <= y_to; ++y)
    {
        for(int x = x_from; x <= x_to; ++x)
        {
            if(!valid_y_cut(x, y, x_from, y_from, node, item) ||
               !can_cut(node, x, y, item))
            {
                continue;
            }

            const std::size_t nodes_size = _s.nodes.size();
            const std::size_t children_size = _s.nodes[node_id].children.size();

            int child_id;

            if(y == y_from)
            {
                child_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    item.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }
            else
            {
                _s.add_node(
                    node.plate_id,
                    node.x,
                    y_from,
                    node.w,
                    y - y_from,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);

                child_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    item.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }

            if(fast_cut(item, prev_item_id, prev_item_visited, child_id))
            {
                return true;
            }

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id].children.resize(children_size);
        }
    }

    return false;
}

bool Cutter::fast_try_4_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];

    if(can_cut(node, node.x, node.y, item) && valid_4_cut(node.h, item.h))
    {
        const int child_id = _s.add_node(
            node.plate_id,
            node.x,
            node.y,
            node.w,
            item.h,
            -2,
            4,
            node.node_id,
            false);

        _s.add_node(
            node.plate_id,
            node.x,
            node.y + item.h,
            node.w,
            node.h - item.h,
            -2,
            4,
            node.node_id,
            false);

        return fast_cut(item, prev_item_id, prev_item_visited, child_id);
    }

    const int top_y = node.y + node.h - item.h;

    if(can_cut(node, node.x, top_y, item) && valid_4_cut(node.h, item.h))
    {
        _s.add_node(
            node.plate_id,
            node.x,
            node.y,
            node.w,
            node.h - item.h,
            -2,
            4,
            node.node_id,
            false);

        const int child_id = _s.add_node(
            node.plate_id,
            node.x,
            top_y,
            node.w,
            item.h,
            -2,
            4,
            node.node_id,
            false);

        return fast_cut(item, prev_item_id, prev_item_visited, child_id);
    }

    return false;
}

bool Cutter::fast_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];

    if(prev_item_id == -1 || node.type == prev_item_id)
    {
        prev_item_visited = true;
    }

    if(node.type != -2)
    {
        return false;
    }

    if(item.w == node.w && item.h == node.h)
    {
        if(!node.children.empty())
        {
            return false;
        }

        if(prev_item_visited && can_cut(node, node.x, node.y, item))
        {
            _s.nodes[node_id].type = item.id;
            return true;
        }

        return false;
    }

    if(node.cut == 4)
    {
        return false;
    }

    if(!node.children.empty())
    {
        for(int child_id : node.children)
        {
            if(fast_cut(item, prev_item_id, prev_item_visited, child_id))
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

    if(node.cut % 2 == 0)
    {
        return fast_try_vertical_cut(item, prev_item_id, prev_item_visited, node_id);
    }

    if(node.cut == 3)
    {
        return fast_try_4_cut(item, prev_item_id, prev_item_visited, node_id);
    }

    return fast_try_horizontal_cut(item, prev_item_id, prev_item_visited, node_id);
}

// -----------------------------------------------------------------------------
// Cutter: rotation method
// -----------------------------------------------------------------------------

bool Cutter::rotation_try_horizontal_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    bool orientation_locked)
{
    const Node node = _s.nodes[node_id];

    const int y_from = node.children.empty()
        ? node.y
        : _s.nodes[node.children.back()].y + _s.nodes[node.children.back()].h;

    const int y_to = node.y + node.h - std::min(item.w, item.h);

    for(int y = y_from; y <= y_to; ++y)
    {
        if(!passes_min_waste_constraint(y, y_from))
        {
            continue;
        }

        auto try_orientation = [&](Item& candidate) -> bool
        {
            if(!rotations_valid_y_cut(node, candidate, y))
            {
                return false;
            }

            const std::size_t nodes_size = _s.nodes.size();
            const std::size_t children_size = _s.nodes[node_id].children.size();

            int child_id;

            if(y == y_from)
            {
                child_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    candidate.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }
            else
            {
                _s.add_node(
                    node.plate_id,
                    node.x,
                    y_from,
                    node.w,
                    y - y_from,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);

                child_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    candidate.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }

            if(rotation_cut(candidate, prev_item_id, prev_item_visited, child_id, true))
            {
                return true;
            }

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id].children.resize(children_size);
            return false;
        };

        if(try_orientation(item))
        {
            return true;
        }

        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w, rotated.h);

            if(try_orientation(rotated))
            {
                return true;
            }
        }
    }

    return false;
}

bool Cutter::rotation_try_vertical_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    bool orientation_locked)
{
    const Node node = _s.nodes[node_id];

    const int x_from = node.children.empty()
        ? node.x
        : _s.nodes[node.children.back()].x + _s.nodes[node.children.back()].w;

    const int x_to = node.x + node.w - std::min(item.w, item.h);

    for(int x = x_from; x <= x_to; ++x)
    {
        if(!passes_min_waste_constraint(x, x_from))
        {
            continue;
        }

        auto try_orientation = [&](Item& candidate) -> bool
        {
            if(!rotations_valid_x_cut(node, candidate, x, x_from))
            {
                return false;
            }

            const std::size_t nodes_size = _s.nodes.size();
            const std::size_t children_size = _s.nodes[node_id].children.size();

            int child_id;

            if(x == x_from)
            {
                child_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    candidate.w,
                    node.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }
            else
            {
                _s.add_node(
                    node.plate_id,
                    x_from,
                    node.y,
                    x - x_from,
                    node.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);

                child_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    candidate.w,
                    node.h,
                    -2,
                    node.cut + 1,
                    node.node_id,
                    false);
            }

            if(rotation_cut(candidate, prev_item_id, prev_item_visited, child_id, true))
            {
                return true;
            }

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id].children.resize(children_size);
            return false;
        };

        if(try_orientation(item))
        {
            return true;
        }

        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w, rotated.h);

            if(try_orientation(rotated))
            {
                return true;
            }
        }
    }

    return false;
}

bool Cutter::rotation_try_4_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    bool orientation_locked)
{
    const Node node = _s.nodes[node_id];

    if(item.w != node.w || item.h > node.h)
    {
        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w, rotated.h);

            return rotation_try_4_cut(
                rotated,
                prev_item_id,
                prev_item_visited,
                node_id,
                true);
        }

        return false;
    }

    if(!passes_4_cut_min_waste_constraint(node, item))
    {
        return false;
    }

    if(can_do_bottom_4_cut(node, item))
    {
        const int child_id = _s.add_node(
            node.plate_id,
            node.x,
            node.y,
            node.w,
            item.h,
            -2,
            4,
            node.node_id,
            false);

        _s.add_node(
            node.plate_id,
            node.x,
            node.y + item.h,
            node.w,
            node.h - item.h,
            -2,
            4,
            node.node_id,
            false);

        return rotation_cut(
            item,
            prev_item_id,
            prev_item_visited,
            child_id,
            orientation_locked);
    }

    if(can_do_top_4_cut(node, item))
    {
        _s.add_node(
            node.plate_id,
            node.x,
            node.y,
            node.w,
            node.h - item.h,
            -2,
            4,
            node.node_id,
            false);

        const int child_id = _s.add_node(
            node.plate_id,
            node.x,
            node.y + node.h - item.h,
            node.w,
            item.h,
            -2,
            4,
            node.node_id,
            false);

        return rotation_cut(
            item,
            prev_item_id,
            prev_item_visited,
            child_id,
            orientation_locked);
    }

    return false;
}

bool Cutter::rotation_try_subdivide_orientation(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];
    const Node parent = _s.nodes[node.parent];

    int child_index = -1;

    for(int i = 0; i < static_cast<int>(parent.children.size()); ++i)
    {
        if(parent.children[i] == node_id)
        {
            child_index = i;
            break;
        }
    }

    if(child_index == -1)
    {
        return false;
    }

    if(node.cut % 2 == 0)
    {
        const int y_from = node.y;
        const int y_to = node.y + node.h - item.h;

        for(int y = y_from; y <= y_to; ++y)
        {
            if(node.h - item.h < min_waste ||
               !y_cut_avoids_defects(y, parent) ||
               !y_cut_avoids_defects(y + item.h, parent) ||
               !passes_min_waste_constraint(y, y_from) ||
               (y + item.h < node.y + node.h &&
                node.y + node.h - y - item.h < min_waste) ||
               (node.cut == 2 && item.h < min_2_cut))
            {
                continue;
            }

            const std::size_t nodes_size = _s.nodes.size();

            int item_node_id;

            if(y == y_from)
            {
                _s.nodes[node_id].h = item.h;

                _s.add_node(
                    node.plate_id,
                    node.x,
                    y + item.h,
                    node.w,
                    node.h - item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                item_node_id = node_id;
            }
            else if(y + item.h == node.y + node.h)
            {
                _s.nodes[node_id].h = node.h - item.h;

                item_node_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);
            }
            else
            {
                _s.nodes[node_id].h = y - node.y;

                item_node_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                _s.add_node(
                    node.plate_id,
                    node.x,
                    y + item.h,
                    node.w,
                    node.y + node.h - y - item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 2);
            }

            if(rotation_cut(item, prev_item_id, prev_item_visited, item_node_id, true))
            {
                return true;
            }

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id] = node;
            _s.nodes[parent.node_id] = parent;
        }
    }
    else
    {
        const int x_from = node.x;
        const int x_to = node.x + node.w - item.w;

        for(int x = x_from; x <= x_to; ++x)
        {
            if(node.w - item.w < min_waste ||
               !x_cut_avoids_defects(x, parent) ||
               !x_cut_avoids_defects(x + item.w, parent) ||
               !passes_min_waste_constraint(x, x_from) ||
               (x + item.w < node.x + node.w &&
                node.x + node.w - x - item.w < min_waste) ||
               (node.cut == 1 &&
                (item.w < min_1_cut || item.w > max_1_cut)))
            {
                continue;
            }

            const std::size_t nodes_size = _s.nodes.size();

            int item_node_id;

            if(x == x_from)
            {
                _s.nodes[node_id].w = item.w;

                _s.add_node(
                    node.plate_id,
                    x + item.w,
                    node.y,
                    node.w - item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                item_node_id = node_id;
            }
            else if(x + item.w == node.x + node.w)
            {
                _s.nodes[node_id].w = node.w - item.w;

                item_node_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);
            }
            else
            {
                _s.nodes[node_id].w = x - node.x;

                item_node_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                _s.add_node(
                    node.plate_id,
                    x + item.w,
                    node.y,
                    node.x + node.w - x - item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 2);
            }

            if(rotation_cut(item, prev_item_id, prev_item_visited, item_node_id, true))
            {
                return true;
            }

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id] = node;
            _s.nodes[parent.node_id] = parent;
        }
    }

    return false;
}

bool Cutter::rotation_try_subdivide_node(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    bool orientation_locked)
{
    if(rotation_try_subdivide_orientation(
        item,
        prev_item_id,
        prev_item_visited,
        node_id))
    {
        return true;
    }

    if(!orientation_locked && item.w != item.h)
    {
        Item rotated = item;
        std::swap(rotated.w, rotated.h);

        return rotation_try_subdivide_orientation(
            rotated,
            prev_item_id,
            prev_item_visited,
            node_id);
    }

    return false;
}

bool Cutter::rotation_cut(
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    bool orientation_locked)
{
    const Node node = _s.nodes[node_id];

    if(!prev_item_visited && (prev_item_id == -1 || node.type == prev_item_id))
    {
        prev_item_visited = true;
    }

    if(node.type != -2)
    {
        return false;
    }

    const bool exact_original = item.w == node.w && item.h == node.h;
    const bool exact_rotated =
        !orientation_locked && item.h == node.w && item.w == node.h;

    if(exact_original || exact_rotated)
    {
        if(!node.children.empty())
        {
            return false;
        }

        if(prev_item_visited && no_defect_in_node(node))
        {
            _s.nodes[node_id].type = item.id;
            return true;
        }

        return false;
    }

    if(node.cut == 4)
    {
        return false;
    }

    if(!node.children.empty())
    {
        for(int child_id : node.children)
        {
            if(rotation_cut(
                item,
                prev_item_id,
                prev_item_visited,
                child_id,
                orientation_locked))
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

    if(node.children.empty() && node.parent != -1)
    {
        if(rotation_try_subdivide_node(
            item,
            prev_item_id,
            prev_item_visited,
            node_id,
            orientation_locked))
        {
            return true;
        }
    }

    if(node.cut % 2 == 0)
    {
        return rotation_try_vertical_cut(
            item,
            prev_item_id,
            prev_item_visited,
            node_id,
            orientation_locked);
    }

    if(node.cut == 3)
    {
        return rotation_try_4_cut(
            item,
            prev_item_id,
            prev_item_visited,
            node_id,
            orientation_locked);
    }

    return rotation_try_horizontal_cut(
        item,
        prev_item_id,
        prev_item_visited,
        node_id,
        orientation_locked);
}

// -----------------------------------------------------------------------------
// CutPlacementGenerator
// -----------------------------------------------------------------------------

bool CutPlacementGenerator::same_node(const Node& a, const Node& b)
{
    return a.plate_id == b.plate_id &&
           a.node_id == b.node_id &&
           a.x == b.x &&
           a.y == b.y &&
           a.w == b.w &&
           a.h == b.h &&
           a.type == b.type &&
           a.cut == b.cut &&
           a.parent == b.parent &&
           a.children == b.children;
}

void CutPlacementGenerator::record_placement(std::vector<Change>& placements) const
{
    Change change;

    const std::size_t common_size = std::min(
        _base_solution.nodes.size(),
        _s.nodes.size());

    for(std::size_t i = 0; i < common_size; ++i)
    {
        if(!same_node(_base_solution.nodes[i], _s.nodes[i]))
        {
            change.updates.push_back(NodeChange{
                static_cast<int>(i),
                _s.nodes[i]
            });
        }
    }

    for(std::size_t i = common_size; i < _s.nodes.size(); ++i)
    {
        change.updates.push_back(NodeChange{
            static_cast<int>(i),
            _s.nodes[i]
        });
    }

    placements.push_back(std::move(change));
}

std::vector<int> CutPlacementGenerator::possible_x_cut_positions(
    const Item& item,
    const Node& node) const
{
    std::vector<int> positions;

    const int x_from = node.children.empty()
        ? node.x
        : _s.nodes[node.children.back()].x + _s.nodes[node.children.back()].w;

    const int x_to = node.x + node.w - item.w;

    for(int x = x_from; x <= x_to; ++x)
    {
        if(x_cut_avoids_defects(x, node) &&
           x_cut_avoids_defects(x + item.w, node))
        {
            positions.push_back(x);
            break;
        }
    }

    for(int x = x_to; x >= x_from; --x)
    {
        if(x_cut_avoids_defects(x, node) &&
           x_cut_avoids_defects(x + item.w, node))
        {
            if(positions.empty() || positions.back() != x)
            {
                positions.push_back(x);
            }
            break;
        }
    }

    for(const Defect& defect : _bin.defects)
    {
        if(!defect_in_node(node, defect))
        {
            continue;
        }

        const int before = defect.x - item.w;
        const int after = defect.x + defect.w;

        if(before >= x_from && before <= x_to)
        {
            positions.push_back(before);
        }

        if(after >= x_from && after <= x_to)
        {
            positions.push_back(after);
        }
    }

    std::sort(positions.begin(), positions.end());
    positions.erase(
        std::unique(positions.begin(), positions.end()),
        positions.end());

    return positions;
}

std::vector<int> CutPlacementGenerator::possible_y_cut_positions(
    const Item& item,
    const Node& node) const
{
    std::vector<int> positions;

    const int y_from = node.children.empty()
        ? node.y
        : _s.nodes[node.children.back()].y + _s.nodes[node.children.back()].h;

    const int y_to = node.y + node.h - item.h;

    for(int y = y_from; y <= y_to; ++y)
    {
        if(y_cut_avoids_defects(y, node) &&
           y_cut_avoids_defects(y + item.h, node))
        {
            positions.push_back(y);
            break;
        }
    }

    for(int y = y_to; y >= y_from; --y)
    {
        if(y_cut_avoids_defects(y, node) &&
           y_cut_avoids_defects(y + item.h, node))
        {
            if(positions.empty() || positions.back() != y)
            {
                positions.push_back(y);
            }
            break;
        }
    }

    for(const Defect& defect : _bin.defects)
    {
        if(!defect_in_node(node, defect))
        {
            continue;
        }

        const int before = defect.y - item.h;
        const int after = defect.y + defect.h;

        if(before >= y_from && before <= y_to)
        {
            positions.push_back(before);
        }

        if(after >= y_from && after <= y_to)
        {
            positions.push_back(after);
        }
    }

    std::sort(positions.begin(), positions.end());
    positions.erase(
        std::unique(positions.begin(), positions.end()),
        positions.end());

    return positions;
}

std::vector<int> CutPlacementGenerator::possible_x_subdivision_positions(
    const Item& item,
    const Node& node) const
{
    std::vector<int> positions;
    const int x_from = node.x;
    const int x_to = node.x + node.w - item.w;

    for(int x = x_from; x <= x_to; ++x)
    {
        if(x_cut_avoids_defects(x, node) &&
           x_cut_avoids_defects(x + item.w, node))
        {
            positions.push_back(x);
            break;
        }
    }

    for(int x = x_to; x >= x_from; --x)
    {
        if(x_cut_avoids_defects(x, node) &&
           x_cut_avoids_defects(x + item.w, node))
        {
            if(positions.empty() || positions.back() != x)
            {
                positions.push_back(x);
            }
            break;
        }
    }

    for(const Defect& defect : _bin.defects)
    {
        if(!defect_in_node(node, defect))
        {
            continue;
        }

        const int before = defect.x - item.w;
        const int after = defect.x + defect.w;

        if(before >= x_from && before <= x_to)
        {
            positions.push_back(before);
        }

        if(after >= x_from && after <= x_to)
        {
            positions.push_back(after);
        }
    }

    std::sort(positions.begin(), positions.end());
    positions.erase(
        std::unique(positions.begin(), positions.end()),
        positions.end());

    return positions;
}

std::vector<int> CutPlacementGenerator::possible_y_subdivision_positions(
    const Item& item,
    const Node& node) const
{
    std::vector<int> positions;
    const int y_from = node.y;
    const int y_to = node.y + node.h - item.h;

    for(int y = y_from; y <= y_to; ++y)
    {
        if(y_cut_avoids_defects(y, node) &&
           y_cut_avoids_defects(y + item.h, node))
        {
            positions.push_back(y);
            break;
        }
    }

    for(int y = y_to; y >= y_from; --y)
    {
        if(y_cut_avoids_defects(y, node) &&
           y_cut_avoids_defects(y + item.h, node))
        {
            if(positions.empty() || positions.back() != y)
            {
                positions.push_back(y);
            }
            break;
        }
    }

    for(const Defect& defect : _bin.defects)
    {
        if(!defect_in_node(node, defect))
        {
            continue;
        }

        const int before = defect.y - item.h;
        const int after = defect.y + defect.h;

        if(before >= y_from && before <= y_to)
        {
            positions.push_back(before);
        }

        if(after >= y_from && after <= y_to)
        {
            positions.push_back(after);
        }
    }

    std::sort(positions.begin(), positions.end());
    positions.erase(
        std::unique(positions.begin(), positions.end()),
        positions.end());

    return positions;
}

void CutPlacementGenerator::try_vertical_cut(
    std::vector<Change>& placements,
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];

    const int x_from = node.children.empty()
        ? node.x
        : _s.nodes[node.children.back()].x + _s.nodes[node.children.back()].w;

    for(int x : possible_x_cut_positions(item, node))
    {
        if(!passes_min_waste_constraint(x, x_from) ||
           !rotations_valid_x_cut(node, item, x, x_from))
        {
            continue;
        }

        const std::size_t nodes_size = _s.nodes.size();
        const std::size_t children_size = _s.nodes[node_id].children.size();

        int child_id;

        if(x == x_from)
        {
            child_id = _s.add_node(
                node.plate_id,
                x,
                node.y,
                item.w,
                node.h,
                -2,
                node.cut + 1,
                node.node_id,
                false);
        }
        else
        {
            _s.add_node(
                node.plate_id,
                x_from,
                node.y,
                x - x_from,
                node.h,
                -2,
                node.cut + 1,
                node.node_id,
                false);

            child_id = _s.add_node(
                node.plate_id,
                x,
                node.y,
                item.w,
                node.h,
                -2,
                node.cut + 1,
                node.node_id,
                false);
        }

        generate(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            child_id,
            true);

        _s.nodes.resize(nodes_size);
        _s.nodes[node_id].children.resize(children_size);
    }
}

void CutPlacementGenerator::try_horizontal_cut(
    std::vector<Change>& placements,
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];

    const int y_from = node.children.empty()
        ? node.y
        : _s.nodes[node.children.back()].y + _s.nodes[node.children.back()].h;

    for(int y : possible_y_cut_positions(item, node))
    {
        if(!passes_min_waste_constraint(y, y_from) ||
           !rotations_valid_y_cut(node, item, y))
        {
            continue;
        }

        const std::size_t nodes_size = _s.nodes.size();
        const std::size_t children_size = _s.nodes[node_id].children.size();

        int child_id;

        if(y == y_from)
        {
            child_id = _s.add_node(
                node.plate_id,
                node.x,
                y,
                node.w,
                item.h,
                -2,
                node.cut + 1,
                node.node_id,
                false);
        }
        else
        {
            _s.add_node(
                node.plate_id,
                node.x,
                y_from,
                node.w,
                y - y_from,
                -2,
                node.cut + 1,
                node.node_id,
                false);

            child_id = _s.add_node(
                node.plate_id,
                node.x,
                y,
                node.w,
                item.h,
                -2,
                node.cut + 1,
                node.node_id,
                false);
        }

        generate(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            child_id,
            true);

        _s.nodes.resize(nodes_size);
        _s.nodes[node_id].children.resize(children_size);
    }
}

void CutPlacementGenerator::try_4_cut(
    std::vector<Change>& placements,
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];

    if(item.w != node.w || item.h > node.h ||
       !passes_4_cut_min_waste_constraint(node, item))
    {
        return;
    }

    if(can_do_bottom_4_cut(node, item))
    {
        const std::size_t nodes_size = _s.nodes.size();
        const std::size_t children_size = _s.nodes[node_id].children.size();

        const int child_id = _s.add_node(
            node.plate_id,
            node.x,
            node.y,
            node.w,
            item.h,
            -2,
            4,
            node.node_id,
            false);

        _s.add_node(
            node.plate_id,
            node.x,
            node.y + item.h,
            node.w,
            node.h - item.h,
            -2,
            4,
            node.node_id,
            false);

        generate(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            child_id,
            true);

        _s.nodes.resize(nodes_size);
        _s.nodes[node_id].children.resize(children_size);
    }

    if(can_do_top_4_cut(node, item))
    {
        const std::size_t nodes_size = _s.nodes.size();
        const std::size_t children_size = _s.nodes[node_id].children.size();

        _s.add_node(
            node.plate_id,
            node.x,
            node.y,
            node.w,
            node.h - item.h,
            -2,
            4,
            node.node_id,
            false);

        const int child_id = _s.add_node(
            node.plate_id,
            node.x,
            node.y + node.h - item.h,
            node.w,
            item.h,
            -2,
            4,
            node.node_id,
            false);

        generate(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            child_id,
            true);

        _s.nodes.resize(nodes_size);
        _s.nodes[node_id].children.resize(children_size);
    }
}

void CutPlacementGenerator::try_subdivide_node(
    std::vector<Change>& placements,
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id)
{
    const Node node = _s.nodes[node_id];
    const Node parent = _s.nodes[node.parent];

    int child_index = -1;

    for(int i = 0; i < static_cast<int>(parent.children.size()); ++i)
    {
        if(parent.children[i] == node_id)
        {
            child_index = i;
            break;
        }
    }

    if(child_index == -1)
    {
        return;
    }

    if(node.cut % 2 == 0)
    {
        const int y_from = node.y;

        for(int y : possible_y_subdivision_positions(item, node))
        {
            if(node.h - item.h < min_waste ||
               !y_cut_avoids_defects(y, parent) ||
               !y_cut_avoids_defects(y + item.h, parent) ||
               !passes_min_waste_constraint(y, y_from) ||
               (y + item.h < node.y + node.h &&
                node.y + node.h - y - item.h < min_waste) ||
               (node.cut == 2 && item.h < min_2_cut))
            {
                continue;
            }

            const std::size_t nodes_size = _s.nodes.size();
            int item_node_id;

            if(y == node.y)
            {
                _s.nodes[node_id].h = item.h;

                _s.add_node(
                    node.plate_id,
                    node.x,
                    y + item.h,
                    node.w,
                    node.h - item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                item_node_id = node_id;
            }
            else if(y + item.h == node.y + node.h)
            {
                _s.nodes[node_id].h = node.h - item.h;

                item_node_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);
            }
            else
            {
                _s.nodes[node_id].h = y - node.y;

                item_node_id = _s.add_node(
                    node.plate_id,
                    node.x,
                    y,
                    node.w,
                    item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                _s.add_node(
                    node.plate_id,
                    node.x,
                    y + item.h,
                    node.w,
                    node.y + node.h - y - item.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 2);
            }

            generate(
                placements,
                item,
                prev_item_id,
                prev_item_visited,
                item_node_id,
                true);

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id] = node;
            _s.nodes[parent.node_id] = parent;
        }
    }
    else
    {
        const int x_from = node.x;

        for(int x : possible_x_subdivision_positions(item, node))
        {
            if(node.w - item.w < min_waste ||
               !x_cut_avoids_defects(x, parent) ||
               !x_cut_avoids_defects(x + item.w, parent) ||
               !passes_min_waste_constraint(x, x_from) ||
               (x + item.w < node.x + node.w &&
                node.x + node.w - x - item.w < min_waste) ||
               (node.cut == 1 &&
                (item.w < min_1_cut || item.w > max_1_cut)))
            {
                continue;
            }

            const std::size_t nodes_size = _s.nodes.size();
            int item_node_id;

            if(x == node.x)
            {
                _s.nodes[node_id].w = item.w;

                _s.add_node(
                    node.plate_id,
                    x + item.w,
                    node.y,
                    node.w - item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                item_node_id = node_id;
            }
            else if(x + item.w == node.x + node.w)
            {
                _s.nodes[node_id].w = node.w - item.w;

                item_node_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);
            }
            else
            {
                _s.nodes[node_id].w = x - node.x;

                item_node_id = _s.add_node(
                    node.plate_id,
                    x,
                    node.y,
                    item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 1);

                _s.add_node(
                    node.plate_id,
                    x + item.w,
                    node.y,
                    node.x + node.w - x - item.w,
                    node.h,
                    -2,
                    node.cut,
                    parent.node_id,
                    false,
                    child_index + 2);
            }

            generate(
                placements,
                item,
                prev_item_id,
                prev_item_visited,
                item_node_id,
                true);

            _s.nodes.resize(nodes_size);
            _s.nodes[node_id] = node;
            _s.nodes[parent.node_id] = parent;
        }
    }
}

void CutPlacementGenerator::generate(
    std::vector<Change>& placements,
    Item& item,
    int prev_item_id,
    bool& prev_item_visited,
    int node_id,
    bool orientation_locked)
{
    const Node node = _s.nodes[node_id];

    if(!prev_item_visited && (prev_item_id == -1 || node.type == prev_item_id))
    {
        prev_item_visited = true;
    }

    if(node.type != -2)
    {
        return;
    }

    const bool exact_original = item.w == node.w && item.h == node.h;
    const bool exact_rotated =
        !orientation_locked && item.h == node.w && item.w == node.h;

    if(exact_original || exact_rotated)
    {
        if(node.children.empty() && prev_item_visited && no_defect_in_node(node))
        {
            const int old_type = _s.nodes[node_id].type;
            _s.nodes[node_id].type = item.id;
            record_placement(placements);
            _s.nodes[node_id].type = old_type;
        }

        return;
    }

    if(node.cut == 4)
    {
        return;
    }

    if(!node.children.empty())
    {
        for(int child_id : node.children)
        {
            generate(
                placements,
                item,
                prev_item_id,
                prev_item_visited,
                child_id,
                orientation_locked);
        }

        if(node.cut == 3)
        {
            return;
        }
    }

    if(!prev_item_visited)
    {
        return;
    }

    if(node.children.empty() && node.parent != -1)
    {
        try_subdivide_node(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            node_id);

        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w, rotated.h);

            try_subdivide_node(
                placements,
                rotated,
                prev_item_id,
                prev_item_visited,
                node_id);
        }
    }

    if(node.cut % 2 == 0)
    {
        try_vertical_cut(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            node_id);

        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w, rotated.h);

            try_vertical_cut(
                placements,
                rotated,
                prev_item_id,
                prev_item_visited,
                node_id);
        }
    }
    else if(node.cut == 3)
    {
        try_4_cut(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            node_id);

        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w, rotated.h);

            try_4_cut(
                placements,
                rotated,
                prev_item_id,
                prev_item_visited,
                node_id);
        }
    }
    else
    {
        try_horizontal_cut(
            placements,
            item,
            prev_item_id,
            prev_item_visited,
            node_id);

        if(!orientation_locked && item.w != item.h)
        {
            Item rotated = item;
            std::swap(rotated.w, rotated.h);

            try_horizontal_cut(
                placements,
                rotated,
                prev_item_id,
                prev_item_visited,
                node_id);
        }
    }
}

std::vector<Change> CutPlacementGenerator::get_placements(
    Item& item,
    int prev_item_id,
    int node_id,
    bool allow_rotation)
{
    _base_solution = _s;

    std::vector<Change> placements;
    bool prev_item_visited = false;

    generate(
        placements,
        item,
        prev_item_id,
        prev_item_visited,
        node_id,
        !allow_rotation);

    _s = _base_solution;
    return placements;
}

void apply_change(Solution& solution, const Change& change)
{
    for(const NodeChange& update : change.updates)
    {
        if(solution.nodes.size() <= static_cast<std::size_t>(update.node_id))
        {
            solution.nodes.resize(static_cast<std::size_t>(update.node_id) + 1);
        }

        solution.nodes[update.node_id] = update.new_node;
    }
}
