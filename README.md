# ROADEF/EURO 2018 Cutting Optimization Solver

Experimental C++ solver for the **ROADEF/EURO 2018 Cutting Optimization Challenge** proposed by Saint-Gobain.

The problem is a constrained two-dimensional glass cutting problem. Rectangular items must be cut from glass plates using guillotine cuts while respecting defects, stack order, cutting-level restrictions, and waste constraints.

## Problem overview

The solver works with rectangular glass plates containing defects and tries to place all requested items while using as few plates and as little unusable material as possible.

Important constraints handled by the solver include:

- items may be rotated by 90°;
- items may not overlap defects;
- cuts may not pass through defects;
- cutting is guillotine-based;
- 1-cuts are vertical;
- 2-cuts are horizontal;
- 3-cuts are vertical;
- limited 4-cuts are allowed as trimming cuts;
- minimum distance between 1-cuts: `100`;
- maximum distance between 1-cuts: `3500` (except for the residual);
- minimum distance between 2-cuts: `100`;
- minimum waste size: `20`;
- items belonging to the same stack must respect their prescribed order;
- plates are used in the given order.

## Representation

A cutting pattern is represented as a tree.

Each `Node` stores information such as:

- plate ID;
- node ID;
- rectangle position `(x, y)`;
- width and height;
- node type;
- cut level;
- parent node;
- child nodes.

The tree structure follows the sequence of guillotine cuts. Item order inside a stack is checked through the depth-first order of the resulting cutting tree.

## Solver strategy

The original solver uses a greedy constructive heuristic:

1. Choose the next available item from the stacks.
2. Traverse the current cutting tree.
3. Search for a feasible position for the item.
4. Try the item in both orientations when rotation is allowed.
5. Respect defects and all cut-size restrictions.
6. If the item cannot be placed on the current plate, temporarily disable its stack and try another stack.
7. Open the next plate when no more items fit.

The stack order can also be dynamically reordered according to the area of the next available item.

## Reusing unused regions

Unused branch nodes are not immediately discarded as waste. The solver can subdivide an unused region at the same cut level and reuse part of it for a later item.

This is implemented by splitting a free region into one of the following forms:

```text
[item][remainder]

[remainder][item]

[remainder][item][remainder]
```

with the analogous vertical arrangement for horizontal cuts.

## Placement lookahead

The current experimental branch extends the greedy solver by considering **multiple possible positions for the current item**.

For an item, the solver:

1. generates feasible placements;
2. stores each placement as a `Change`;
3. applies one candidate temporarily;
4. greedily fills the remaining space on the current plate;
5. evaluates the candidate by the total additional item area packed;
6. restores the original state;
7. chooses and applies the candidate with the best rollout value.

Conceptually:

```text
current item
    |
    +-- placement 1 -> greedy rollout -> score
    |
    +-- placement 2 -> greedy rollout -> score
    |
    +-- placement 3 -> greedy rollout -> score
    |
    `-- choose best placement
```

A `Change` stores the nodes that must be created or modified in order to reproduce a candidate placement.

## Main placement routines

The placement search is divided into several routines:

- `placement_try_vertical_cut(...)`
- `placement_try_horizontal_cut(...)`
- `placement_try_4_cut(...)`
- `placement_try_subdivide_node(...)`
- `get_placements(...)`
- `apply_change(...)`
- `evaluate_change(...)`

Temporary modifications to the solution tree are rolled back after each speculative branch.

## Project status

Implemented:

- guillotine cutting tree;
- item rotation;
- defect avoidance;
- stack-order handling;
- dynamic stack ordering by item area;
- 1-, 2-, 3-, and restricted 4-cut handling;
- reuse/subdivision of unused regions;
- temporary mutation and rollback during search;
- representation and application of candidate changes;
- greedy rollout evaluation of candidate placements.

## Source structure

The core solving logic is currently implemented in `solve.cpp` and uses the project data structures declared in the corresponding headers, including `solve.h` and `data.h`.

The implementation is heuristic and under active development rather than a finished exact optimizer.

## Reference

This project is based on the **ROADEF/EURO 2018 Cutting Optimization Challenge** by Saint-Gobain Datalab.
