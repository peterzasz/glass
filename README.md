# ROADEF/EURO 2018 Cutting Optimization Solver — `main`

C++ heuristic solver for the **ROADEF/EURO 2018 Cutting Optimization Challenge** proposed by Saint-Gobain.

This branch contains the **baseline greedy solver**. It is the reference implementation used for comparing later heuristics such as item rotation, area-based stack ordering, and placement lookahead.

## Problem

The task is to cut rectangular customer items from defective glass plates while respecting the technological constraints of the challenge.

The solver handles the main cutting restrictions:

- guillotine cuts only;
- 1-cuts are vertical;
- 2-cuts are horizontal;
- 3-cuts are vertical;
- restricted 4-cuts are used only for trimming after a 3-cut;
- minimum distance between consecutive 1-cuts: `100`;
- maximum distance between consecutive 1-cuts: `3500`, except for the residual;
- minimum distance between consecutive 2-cuts: `100`;
- minimum waste size: `20`;
- items may not overlap defects;
- cuts may not pass through defects;
- items belonging to the same stack must be produced in the prescribed order;
- glass plates are used in the given order.

## Branch strategy

The `main` branch uses a simple constructive greedy strategy.

For each plate, the solver repeatedly:

1. takes the next available item from the first stack that can currently be processed;
2. traverses the current cutting tree in depth-first order;
3. searches for the first feasible placement of the item;
4. commits that placement immediately;
5. temporarily disables a stack if its next item cannot be placed on the current plate;
6. opens the next plate once no available stack can place another item.

This branch does **not** use the later experimental heuristics for rotating items, dynamically ordering stacks by area, or evaluating several alternative placements before choosing one.

## Cutting-tree representation

A cutting pattern is represented as a tree of rectangular `Node`s. A node stores information such as:

- plate ID;
- node ID;
- position `(x, y)`;
- width and height;
- node type;
- cut level;
- parent node;
- child nodes.

The tree structure represents the sequence of guillotine cuts. The required order of items inside each stack is checked through the depth-first order of the tree.

## Greedy placement

The central recursive routine tries to place an item into the current tree. Depending on the cut level, it attempts the appropriate next cut direction while checking:

- item containment;
- minimum and maximum cut dimensions;
- minimum waste requirements;
- defect-free item regions;
- cuts that do not cross defects;
- stack-order feasibility.

The first successful placement is kept.

## Purpose of this branch

`main` serves as the baseline for measuring whether later heuristics improve the solution. Useful comparison metrics include:

- number of plates used;
- total geometric loss;
- residual width;
- runtime.

## Results

|   basic |   rotation |   decreasing order |   evaluate |   evaluate 2 |
|--------:|-----------:|-------------------:|-----------:|-------------:|
|       6 |          6 |                  5 |          5 |            5 |
|       6 |          5 |                  5 |          5 |            5 |
|       4 |          3 |                  2 |          3 |            2 |
|      15 |         14 |                 14 |         14 |           13 |
|      19 |         17 |                 15 |         14 |           14 |
|      22 |         18 |                 16 |         15 |           15 |
|       3 |          3 |                  3 |          3 |            3 |
|       3 |          2 |                  2 |          2 |            2 |
|       6 |          6 |                  6 |          4 |            4 |
|       5 |          4 |                  3 |          3 |            3 |
|       1 |          1 |                  1 |          1 |            1 |
|       2 |          2 |                  2 |          2 |            2 |
|       8 |          6 |                  6 |          6 |            6 |
|       4 |          4 |                  3 |          3 |            3 |
|       4 |          4 |                  3 |          3 |            3 |
|       7 |          5 |                  4 |          4 |            4 |
|       4 |          4 |                  4 |          3 |            3 |
|       7 |          6 |                  5 |          5 |            5 |
|      13 |         11 |                 10 |          9 |            9 |
|       4 |          4 |                  4 |          4 |            3 |
|      27 |         26 |                 24 |         22 |           22 |
|      30 |         27 |                 23 |         22 |           21 |
|      22 |         19 |                 18 |         16 |           16 |
|      39 |         35 |                 32 |         30 |           30 |
|      14 |         13 |                 12 |         11 |           11 |
|      35 |         33 |                 29 |         27 |           27 |
|       7 |          6 |                  5 |          5 |            5 |
|      25 |         23 |                 20 |         19 |           19 |
|      30 |         28 |                 23 |         22 |           22 |
|      12 |         11 |                 10 |         10 |           10 |
|      32 |         28 |                 28 |         26 |           26 |
|      15 |         14 |                 13 |         13 |           12 |
|      14 |         13 |                 12 |         11 |           11 |
|      28 |         24 |                 22 |         21 |           20 |
|      23 |         21 |                 19 |         18 |           18 |

## Source

The main solver logic is implemented in `solve.cpp`, with the corresponding data structures and declarations in the project headers such as `solve.h` and `data.h`.

## Reference

This project is based on the **ROADEF/EURO 2018 Cutting Optimization Challenge** by Saint-Gobain Datalab.
