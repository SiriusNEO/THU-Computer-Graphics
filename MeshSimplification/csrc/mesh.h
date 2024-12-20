// File: mesh.h
// Author: SiriusNEO

#ifndef MESH_H
#define MESH_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <vector>

#include "element.h"

// A Trimesh-style Mesh Object, storing vertices,
class Mesh {
  private:
    std::vector<Vertex *> vertices;
    std::vector<int> ordered_indices; // used in select pairs
    std::vector<Triangle *> triangles;
    std::priority_queue<VertexPair> pq;

    std::map<std::pair<int, int>, int>
        lastTimestampOfPair; // Record the last timestamp of the pair. Key is
                             // (ordered) indices of vertices

    int triangleCnt = 0; // number of remain triangles
    int globalTime = 0;  // Each state update will tick this time

    void makeVertexPair(Vertex *v0, Vertex *v1) {
        // Make a VertexPair and add it into the heap.

        // std::cout << v0 << " " << v1 << std::endl;

        if (v0 == nullptr || v1 == nullptr) {
            std::cout << "Unexpected nullptr" << std::endl;
        }

        if (v0->idx == v1->idx) {
            std::cout << "[MS] Error: make a pair (v0, v0)" << std::endl;
            exit(0);
        }

        if (v0->isRemoved() || v1->isRemoved()) {
            std::cout << "[MS] Error: make a pair for removed vertices"
                      << std::endl;
            exit(0);
        }

        if (v0->idx > v1->idx)
            std::swap(v0, v1);

        auto hash = std::make_pair(v0->idx, v1->idx);

        // Avoid repeated add
        // Last timestamp equals to current globalTime
        if (lastTimestampOfPair.count(hash) &&
            lastTimestampOfPair[hash] == globalTime) {
            return;
        } else {
            lastTimestampOfPair[hash] = globalTime;
        }

        double contracted_Q[16];
        for (int i = 0; i < 16; ++i) {
            contracted_Q[i] = v0->Q[i] + v1->Q[i];
        }
        Vertex contracted_v = std::move(getContractedV(contracted_Q, v0, v1));
        double error = getQuadricsError(contracted_Q, contracted_v);
        VertexPair pair(v0, v1, contracted_v, error, globalTime);

        // std::cout << "Add VertexPair: " << v0->idx << " " << v1->idx << " "
        //           << pair.timestamp << std::endl;

        // if (v0->idx == 2096 && v1->idx == 2097) {
        //     std::cout << v0->x << " " << v0->y << " " << v0->z << std::endl;
        //     for (int i = 0; i < 16; ++i) {
        //         std::cout << v0->Q[i] << " ";
        //     }
        //     std::cout << "error: " << error << std::endl;
        // }

        v0->paired.insert(v1->idx);
        v1->paired.insert(v0->idx);

        // std::cout << pair << std::endl;

        pq.push(std::move(pair));
    }

    bool contract(VertexPair &pair) {
        // Contract the VertexPair at the top of the heap.
        // Return true/false: whether the triangles are reduced.

        if (pair.v0->isRemoved() || pair.v1->isRemoved()) {
            // Lazy deletion in the heap
            return false;
        }

        auto hash = std::make_pair(pair.v0->idx, pair.v1->idx);
        if (lastTimestampOfPair[hash] > pair.timestamp) {
            // Expired pair
            return false;
        }

        if (pair.v0->idx == pair.v1->idx) {
            std::cout << "[MS] Error: contract a pair (v0, v0): v0="
                      << pair.v0->idx << std::endl;
            exit(-1);
        }

        // std::cout << pair << std::endl;

        ++globalTime; // Tick it

        // Step 1. Remove v1
        for (auto t : pair.v1->triangles) {
            // std::cout << *t << std::endl;
            if (t->contains(pair.v0)) {
                // Remove faces which contain both v0, v1
                triangleCnt--;
                t->removeItself(pair.v1);
            } else {
                // Replaces v1 with v0
                t->replaceVertexWith(pair.v1, pair.v0);
            }
            // Parameters of triangles are not used after we finish calculating
            // Q, so we don't need to update them.
        }

        // Step 2. Update v0 to \overline{v}
        pair.v0->update(pair.contracted_v, pair.v1->Q);

        // Step 3. Replace all pairs related to v1 (v2, v1) with (v2, v0) and
        // re-insert them to heap
        //
        // - Here we use a lazy way:
        //   - We mark v1 as removed (as we already did before), and leave all
        //   pairs with v1 in the heap
        //   - Each time we pop a VertexPair from the heap, we check whether
        //   it's a valid pair by checking its two vertices removed or not.
        //   - So we only need to insert a new pair (x, v0) into the heap.
        for (auto v2_idx : pair.v1->paired) {
            auto v2 = vertices[v2_idx];

            // Remove from v2's side
            v2->paired.erase(pair.v1->idx);

            if (v2->idx == pair.v0->idx) {
                continue;
            }

            // Get another vertex
            makeVertexPair(v2, pair.v0);
        }

        // Mark here to since we need v1 idx above
        pair.v1->markRemoved();

        // Step 4. Update all v0 pairs
        for (auto v2_idx : pair.v0->paired) {
            // std::cout << v2_idx << " ";
            // Get another vertex
            makeVertexPair(vertices[v2_idx], pair.v0);
        }
        // std::cout << std::endl;

        return true;
    }

  public:
    ~Mesh() {
        for (auto v : vertices) {
            delete v;
        }
        for (auto t : triangles) {
            delete t;
        }
    }

    void load(std::string path) {
        std::cout << "[MS] Load obj from " + path + " ..." << std::endl;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "[MS] Failed to load obj: " << path << std::endl;
            exit(-1);
        }

        std::string line;
        while (getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                double x, y, z;
                iss >> x >> y >> z;
                Vertex *v = new Vertex(vertices.size(), x, y, z);
                vertices.push_back(v);
                ordered_indices.push_back(v->idx);
                // std::cout << "Vertex " << v->idx << " " << v << std::endl;
            } else if (prefix == "f") {
                int v0, v1, v2;
                iss >> v0 >> v1 >> v2;
                // Note: shift by 1!!!
                Triangle *t = new Triangle(vertices[v0 - 1], vertices[v1 - 1],
                                           vertices[v2 - 1]);
                triangles.push_back(t);

                // Note: here we should use address in the global memory pool!
                t->v0->triangles.push_back(t);
                t->v1->triangles.push_back(t);
                t->v2->triangles.push_back(t);
            }
        }
        triangleCnt = triangles.size();

        file.close();
        std::cout << "[MS] Load finished. "
                  << "Vertices: " << vertices.size() << " "
                  << "Triangles: " << triangles.size() << std::endl;
    }

    void store(std::string path) {
        std::cout << "[MS] Store obj to " + path + " ..." << std::endl;
        std::ofstream os(path);

        // Skip removed vertices
        int newVertexId = 0;
        for (auto &v : vertices) {
            if (!v->isRemoved()) {
                v->idx = ++newVertexId;
                os << *v << std::endl;
            }
        }

        for (auto &t : triangles) {
            if (!t->isRemoved()) {
                os << *t << std::endl;
            }
        }
    }

    void calculateQ() {
        std::cout << "[MS] Calculating Q matrices for each vertex" << std::endl;

        for (auto &v : vertices) {
            v->calculateQ();
        }
    }

    void selectValidPairs(double threshold) {
        std::cout << "[MS] Selecting valid pairs with threshold = " << threshold
                  << std::endl;

        // Add all edge contractions
        for (auto &t : triangles) {
            makeVertexPair(t->v0, t->v1);
            makeVertexPair(t->v0, t->v2);
            makeVertexPair(t->v1, t->v2);
        }

        // Sort indices
        auto cmp = [this](int idx0, int idx1) -> bool {
            return vertices[idx0]->x < vertices[idx1]->x;
        };
        std::sort(ordered_indices.begin(), ordered_indices.end(), cmp);

        for (int i = 0; i < vertices.size(); ++i) {
            int idx0 = ordered_indices[i];
            for (int j = i + 1; j < vertices.size(); ++j) {
                int idx1 = ordered_indices[j];
                if (getDistance(vertices[idx0], vertices[idx1]) < threshold) {
                    makeVertexPair(vertices[idx0], vertices[idx1]);
                }

                if (vertices[idx0]->x - vertices[idx1]->x > threshold) {
                    break;
                }
            }
        }

        std::cout << "[MS] Selection finished. Total pairs: " << pq.size()
                  << std::endl;
    }

    void simplify(double ratio) {
        std::cout << "[MS] Start simplifying. Ratio: " << ratio << std::endl;

        // Number of triangles the simplified mesh should have
        const int origTriangleCnt = triangleCnt,
                  simplifiedTriangleCnt = triangleCnt * ratio;
        while (triangleCnt > simplifiedTriangleCnt) {
            bool flag = false;
            while (!flag) {
                VertexPair pair = std::move(pq.top());
                pq.pop();
                flag = contract(pair);
            };

            std::cout << "[MS] Current triangles: " << triangleCnt << "/"
                      << origTriangleCnt << std::endl;
        }
    }
};

#endif // MESH_H
