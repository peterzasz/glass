# ROADEF/EURO 2018 Cutting Optimization Solver — `rotations`

C++ heuristic solver for the **ROADEF/EURO 2018 Cutting Optimization Challenge** proposed by Saint-Gobain.

This branch extends the baseline greedy solver by allowing the algorithm to consider an item in **both possible orientations**.

## Problem

The task is to cut rectangular customer items from defective glass plates using guillotine cuts while respecting the challenge constraints.

Important restrictions handled by the solver include:

- guillotine cuts only;
- 1-cuts are vertical;
- 2-cuts are horizontal;
- 3-cuts are vertical;
- restricted 4-cuts are allowed as trimming cuts;
- minimum distance between consecutive 1-cuts: `100`;
- maximum distance between consecutive 1-cuts: `3500`, except for the residual;
- minimum distance between consecutive 2-cuts: `100`;
- minimum waste size: `20`;
- items may not overlap defects;
- cuts may not pass through defects;
- items in the same stack must respect their prescribed order;
- plates are used in the given order.

## Branch strategy

The solver is still greedy: for each selected item, it searches the cutting tree and commits the first feasible placement it finds.

The main difference from `main` is **90° item rotation**. When an item cannot be placed in its original orientation, or when a recursive cutting step allows another orientation to be tested, the solver also considers

```text
(w, h) -> (h, w)
```

Rotation is handled locally during the search so that testing one orientation does not permanently modify the stored item dimensions.

Square items are equivalent under rotation and therefore do not need to be tested twice.

## Placement process

For each plate, the solver repeatedly:

1. chooses the next available item according to the stack order;
2. traverses the current cutting tree;
3. tries to place the item in its original orientation;
4. where appropriate, tries the rotated orientation;
5. commits the first feasible placement;
6. disables the current stack for the plate if its next item cannot be placed;
7. continues with another stack or opens the next plate.

## Cutting-tree representation

A cutting pattern is represented as a tree of rectangular `Node`s containing information such as:

- plate ID and node ID;
- position `(x, y)`;
- dimensions;
- node type;
- cut level;
- parent and children.

The recursive search follows the tree and respects the required depth-first ordering of items belonging to the same stack.

## Why rotation helps

Allowing both orientations gives the heuristic more freedom when fitting items into narrow strips or around defects. It can therefore produce a different number of plates, geometric loss, and residual width than the baseline solver while keeping the same general greedy construction method.

## Purpose of this branch

The `rotations` branch is intended to isolate the effect of **item rotation** relative to `main`.

Useful comparison metrics include:

- number of plates used;
- total geometric loss;
- residual width;
- runtime.

This branch does not use the later area-based stack-priority or multi-placement rollout heuristic.

## Source

The core solver logic is implemented in `solve.cpp`, with declarations and data structures in headers such as `solve.h` and `data.h`.

## Reference

This project is based on the **ROADEF/EURO 2018 Cutting Optimization Challenge** by Saint-Gobain Datalab.
