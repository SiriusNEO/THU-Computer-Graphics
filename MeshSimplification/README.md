# Mesh Simplification

Course Project 1, Computer Graphics, Tsinghua University

Our task is to implement a classical Mesh Simplification algorithm "Surface Simplification Using Quadric Error Metrics", SIGGRAPH 97.

## Run My Code

I implement the algorithm in C++ for efficiency and use `Makefile` as the build tool. Make sure your environment has a C
++ compiler (`g++` or `clang++`). Then run the following command in the root directory. The binary `ms` (short for `mesh simplify`) will be compiled and built.

```bash
bash build.sh
```

Run examples under `obj/`, using:

```bash
./ms obj/Input/Dragon.obj obj/MyOutput/Dragon_0.1.obj 0.05
```

Here `0.05` is the simplification ratio (the number of faces of the result / the original faces number). The threshold used in pair selection is hardcoded as `0.01` in `main.cpp`, which you can also modify.


## Result

Note: `threshold=0.01` in all following settings.

### Visualization of Some Examples

<img src="assets/dragon.png" alt="" width="600"/>

Example 1: Dragon.obj, ratio=0.05, Vertices: 1047, Faces: 10460

<img src="assets/dragon1.png" alt="" width="600"/>

Example 2: Dragon.obj, ratio=0.01, Vertices: 1047, Faces: 2091

<img src="assets/horse.png" alt="" width="600"/>

Example 3: Horse.obj, ratio=0.01, Vertices: 486, Faces: 968

### Efficiency



## Implementation

First I try to implement in Python using `trimesh` library to load/store Mesh, but soon I found Python is too slow for this task (Although we can use Torch/Triton to parallelize some computation in GPU, the bottleneck of this algorithm (Select a pair from the top of heap, contract them, repeatedly) is hard to parallelize. So finally I embrace C++.

### Load/Store Mesh

The format of the mesh is not complicated. Each line represents a Vertex (start with `v`) or a Face (start with `f`). For a Vertex, the following three numbers are the coordinates (`x`, `y`, `z`); For a Face, the following three integers represent the indices of three endpoints of this triangle (Each face is a triangle).

### Data Structures

I implement several basic data structures which are necessary for the algorithm: `class Vertex`, `class Triangle` and `class VertexPair`. And the whole `Mesh` is wrapped as a `class Mesh` so that we can operate it easily.

We allocates memory for each `Vertex` and `Triangle` (a.k.a `Face`) when loading them, and reference them using C++ pointers to avoid necessary copy.

### Algorithm Implementation

Most part follows the original paper. There are some points worth mentioning:
- For the heap, I use `std::priority_queue` in C++ STL by overriding the `<` operator of `class VertexPair`. And since it's hard to perform `delete` and `update` operations in `priority_queue`, I do this in a **lazy manner**:
  - For `delete`, I mark the corresponding vertex as removed by setting its index to `-1`. And each time we pop a pair from the heap, we will check whether the pair contains deleted vertex. If so, discard it and pop another pair.
  - For `update`, I maintain a `timestamp` in each pair and record the newest timestamp for each `vertex_id` pair. If I found the pair we pop is not the newest, a.k.a it's expired, we will discard it too.
- For calculating `\overline{v}` from `v1` and `v2`, we need to calculate the determinant and inverse of a 4th order matrix. I calculate it directly by violently expanding to achieve a better performance.


### Optimization


