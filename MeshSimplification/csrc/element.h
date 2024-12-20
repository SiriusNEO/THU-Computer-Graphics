// File: elements.h
// Author: SiriusNEO

#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <vector>

// Avoid cycle references
class Triangle;
class VertexPair;

// A Vertex in Mesh.
class Vertex {
  private:
  public:
    int idx;
    double x, y, z;
    double Q[16]; // Q is a 4*4 matrix.
    std::vector<Triangle *> triangles;
    std::set<int> paired;

    Vertex(int idx_, double x_, double y_, double z_)
        : idx(idx_), x(x_), y(y_), z(z_) {
        memset(Q, 0, sizeof(Q));
    }

    void markRemoved() { idx = -1; }

    inline bool isRemoved() const { return idx == -1; }

    void removeTriangle(Triangle *t) {
        for (int i = 0; i < triangles.size(); ++i) {
            if (triangles[i] == t) {
                triangles.erase(triangles.begin() + i);
                return;
            }
        }
        std::cout << "[MS] Error: unreachable part" << std::endl;
        exit(-1);
    }

    void update(const Vertex &new_v, double *Q1) {
        // Update v with new \overline{v}
        x = new_v.x;
        y = new_v.y;
        z = new_v.z;
        for (int i = 0; i < 16; ++i)
            Q[i] += Q1[i];
    }

    void calculateQ();

    friend double getDistance(const Vertex *v0, const Vertex *v1);

    friend Vertex getContractedV(double *Q, const Vertex *v0, const Vertex *v1);

    friend double getQuadricsError(double *Q, const Vertex &v);

    friend std::ostream &operator<<(std::ostream &os, const Vertex &v) {
        os << "v " << v.x << " " << v.y << " " << v.z;
        return os;
    }
};

// A Triangle Face in the mesh.
class Triangle {
  private:
    void clockwiseTurn() {
        // v0 -> v1, v1 - > v2, v2 -> v0;
        std::swap(v0, v1); // v1, v0, v2
        std::swap(v0, v2); // v2, v0, v1
        return;
    }

    void counterClockwiseTurn() {
        // v0 -> v2, v1 -> v0, v2 -> v1;
        std::swap(v0, v2); // v1, v0, v2
        std::swap(v0, v1); // v2, v0, v1
        return;
    }

    void regularize() {
        // We want to keep v0 as the vertex with the minimum index number (to
        // ensure the triangle is unique). So we need to transform our triangle.
        int min_idx = std::min(std::min(v0->idx, v1->idx), v2->idx);
        if (min_idx == v1->idx) {
            counterClockwiseTurn();
        } else if (min_idx == v2->idx) {
            clockwiseTurn();
            return;
        }
    }

  public:
    // indices of vertices
    Vertex *v0;
    Vertex *v1;
    Vertex *v2;
    // parameters (p0x + p1y + p2z + p3 = 0)
    double p[4];

    Triangle(Vertex *v0_, Vertex *v1_, Vertex *v2_)
        : v0(v0_), v1(v1_), v2(v2_) {
        regularize();
        initParameters();
    }

    void initParameters() {
        double x1 = v1->x - v0->x;
        double y1 = v1->y - v0->y;
        double z1 = v1->z - v0->z;
        double x2 = v2->x - v0->x;
        double y2 = v2->y - v0->y;
        double z2 = v2->z - v0->z;

        p[0] = y1 * z2 - z1 * y2;
        p[1] = z1 * x2 - x1 * z2;
        p[2] = x1 * y2 - y1 * x2;
        double norm = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
        p[0] /= norm;
        p[1] /= norm;
        p[2] /= norm;
        p[3] = -p[0] * v0->x - p[1] * v0->y - p[2] * v0->z;
    }

    void markRemoved() { v0 = nullptr; }

    inline bool isRemoved() const { return v0 == nullptr; }

    inline bool contains(const Vertex *v) {
        return v0->idx == v->idx || v1->idx == v->idx || v2->idx == v->idx;
    }

    void removeItself(const Vertex *removed_vertex) {
        // Remove a triangle from all its vertex
        // removed_vertex: vertex that is removed and causes this triangle to be
        // removed We don't remove triangle from this vertex since it will cause
        // a concurrent modification in the external loop And since this vertex
        // is also removed, this has no effect on our whole mesh

        if (v0->idx != removed_vertex->idx)
            v0->removeTriangle(this);
        if (v1->idx != removed_vertex->idx)
            v1->removeTriangle(this);
        if (v2->idx != removed_vertex->idx)
            v2->removeTriangle(this);
        markRemoved();
    }

    void replaceVertexWith(Vertex *old_v, Vertex *new_v) {
        if (v0->idx == old_v->idx)
            v0 = new_v;
        else if (v1->idx == old_v->idx)
            v1 = new_v;
        else if (v2->idx == old_v->idx)
            v2 = new_v;
        else {
            std::cout << "Unreachable part" << std::endl;
            exit(-1);
        }
        new_v->triangles.push_back(this);
    }

    // Reload < to be used in the std::set to remove duplicates
    // Sorted by vertex indices
    friend bool operator<(const Triangle &t1, const Triangle &t2) {
        if ((t1.v0->idx < t2.v0->idx) ||
            (t1.v0 == t2.v0 && t1.v1->idx < t2.v1->idx) ||
            (t1.v0 == t2.v0 && t1.v1 == t2.v1 && t1.v2->idx < t2.v2->idx))
            return true;
        return false;
    }

    friend bool operator==(const Triangle &t1, const Triangle &t2) {
        return (t1.v0 == t2.v0 && t1.v1 == t2.v1 && t1.v2 == t2.v2);
    }

    friend std::ostream &operator<<(std::ostream &os, const Triangle &t) {
        os << "f " << t.v0->idx << " " << t.v1->idx << " " << t.v2->idx;
        return os;
    }
};

double getDistance(const Vertex *v0, const Vertex *v1);

// Two Vertex-related tool functions
Vertex getContractedV(double *Q, Vertex *v0, Vertex *v1);

double getQuadricsError(double *Q, const Vertex &v);

class VertexPair {
  public:
    Vertex *v0;
    Vertex *v1;
    Vertex contracted_v;
    double cost;
    int timestamp; // timestamp of the vertex pair when it is added to the queue

    VertexPair(Vertex *v0_, Vertex *v1_, Vertex contracted_v_, double cost_,
               int timestamp_)
        : v0(v0_), v1(v1_), contracted_v(contracted_v_), cost(cost_),
          timestamp(timestamp_) {}

    // Override the < operator to use priority queue
    // We want the top of the priority queue to be the VertexPair with the
    // lowest cost
    inline bool operator<(const VertexPair &pair) const {
        if (cost != pair.cost)
            return cost > pair.cost;
        return v0->idx > pair.v0->idx ||
               (v0->idx == pair.v0->idx && v1->idx > pair.v1->idx);
    }

    friend bool operator==(const VertexPair &p0, const VertexPair &p1) {
        return (p0.v0 == p1.v0 && p0.v1 == p1.v1);
    }

    friend std::ostream &operator<<(std::ostream &os, const VertexPair &p) {
        os << "p(" << p.v0->idx << ", " << p.v1->idx << ") "
           << "cost=" << p.cost << " "
           << "timestamp=" << p.timestamp;
        return os;
    }
};

#endif // ELEMENTS_H
