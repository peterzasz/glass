# ROADEF/EURO 2018 Cutting Optimization Solver — `area-order`

C++ heuristic solver for the **ROADEF/EURO 2018 Cutting Optimization Challenge** proposed by Saint-Gobain.

This branch extends the greedy solver with **dynamic stack ordering based on the area of the next available item**. It also retains the item-rotation search used in the `rotations` branch.

## Problem

The task is to cut rectangular customer items from defective glass plates while respecting stack order and the technological rules of guillotine glass cutting.

The solver handles the main constraints of the challenge:

- guillotine cuts only;
- 1-cuts are vertical;
- 2-cuts are horizontal;
- 3-cuts are vertical;
- restricted 4-cuts are used for trimming;
- minimum distance between consecutive 1-cuts: `100`;
- maximum distance between consecutive 1-cuts: `3500`, except for the residual;
- minimum distance between consecutive 2-cuts: `100`;
- minimum waste size: `20`;
- items may not overlap defects;
- cuts may not pass through defects;
- items from the same stack must remain in their prescribed order;
- plates are used in the given order;
- items may be rotated by 90°.

## Area-order heuristic

Each stack has exactly one next item that is currently eligible according to that stack's internal order. Instead of always trying stacks in their original order, this branch sorts the active stacks according to the area of their next item:

```text
area = width * height
```

Stacks whose next item has a larger area are considered first.

Conceptually:

```text
stack A -> next item area 800000
stack B -> next item area 250000
stack C -> next item area 600000

priority: A, C, B
```

After an item is successfully placed, the corresponding stack advances to its next item and the stack order is recomputed. The priority therefore changes dynamically during construction.

Finished or temporarily unavailable stacks are not allowed to block active stacks.

## Solver strategy

For each plate, the solver:

1. initializes the list of stacks;
2. orders active stacks by decreasing area of their next item;
3. takes the highest-priority available item;
4. searches for the first feasible placement in the cutting tree;
5. considers both the original and rotated orientation where applicable;
6. commits the first feasible placement;
7. reorders the stacks after a successful placement;
8. temporarily disables a stack if its next item cannot fit on the current plate;
9. opens the next plate when no active stack can place another item.

This remains a greedy heuristic: it changes **which item is tried next**, rather than evaluating many possible placements globally.

## Motivation

Large items are generally more difficult to fit after a plate has already been fragmented by many cuts. Trying larger available items first is therefore a natural heuristic for reducing fragmentation and preserving useful space for the rest of the batch.

The effectiveness of this rule is instance-dependent, which is why this branch is kept separately for benchmarking against `main` and `rotations`.

## Cutting-tree representation

Cutting patterns are represented as trees of rectangular `Node`s. Each node stores the rectangle geometry, cut level, type, parent, and children. Recursive placement follows the guillotine-cut hierarchy while enforcing defects and stack order.

## Purpose of this branch

The `area-order` branch isolates the effect of the **decreasing-area stack-priority heuristic** on top of the greedy placement framework.

Useful comparison metrics include:

- number of plates used;
- total geometric loss;
- residual width;
- runtime.

This branch does not use the later multi-placement rollout strategy that generates and evaluates many alternative positions for the same item.

## Source

The core solving logic is implemented in `solve.cpp`, with project data structures and declarations in headers such as `solve.h` and `data.h`.

## Reference

This project is based on the **ROADEF/EURO 2018 Cutting Optimization Challenge** by Saint-Gobain Datalab.
