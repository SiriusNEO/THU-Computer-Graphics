// File: element.cpp
// Author: SiriusNEO

#include "element.h"

void Vertex::calculateQ() {
    for (auto t : triangles) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                Q[i * 4 + j] += t->p[i] * t->p[j];
            }
        }
    }
}

double getDistance(const Vertex *v0, const Vertex *v1) {
    return sqrt((v0->x - v1->x) * (v0->x - v1->x) +
                (v0->y - v1->y) * (v0->y - v1->y) +
                (v0->z - v1->z) * (v0->z - v1->z));
}

Vertex getContractedV(double *Q, Vertex *v0, Vertex *v1) {
    static const double eps = 1e-12;
    // det of
    // [q11, q12, q13]
    // [q12, q22, q23]
    // [q13, q23, q33]

    // [q0, q1, q2]
    // [q1, q5, q6]
    // [q2, q6, q10]
    double detQ = Q[0] * Q[5] * Q[10] + Q[1] * Q[6] * Q[2] +
                  Q[2] * Q[1] * Q[6] - Q[2] * Q[5] * Q[2] - Q[6] * Q[6] * Q[0] -
                  Q[1] * Q[1] * Q[10];

    // \overline{v}
    Vertex ret(-1, 0, 0, 0);

    if (true) {
        // not invertible
        ret.x = (v0->x + v1->x) / 2.0;
        ret.y = (v0->y + v1->y) / 2.0;
        ret.z = (v0->z + v1->z) / 2.0;
    } else {
        // Solving the equation
        double x = Q[3] * Q[5] * Q[10] + Q[2] * Q[6] * Q[7] +
                   Q[1] * Q[6] * Q[11] - Q[3] * Q[6] * Q[6] -
                   Q[1] * Q[7] * Q[10] - Q[2] * Q[5] * Q[11];

        double y = Q[0] * Q[7] * Q[10] + Q[1] * Q[2] * Q[11] +
                   Q[2] * Q[3] * Q[6] - Q[0] * Q[6] * Q[11] -
                   Q[1] * Q[3] * Q[10] - Q[2] * Q[2] * Q[7];

        double z = Q[0] * Q[5] * Q[11] + Q[1] * Q[3] * Q[6] +
                   Q[1] * Q[2] * Q[7] - Q[2] * Q[3] * Q[5] -
                   Q[0] * Q[6] * Q[7] - Q[1] * Q[1] * Q[11];

        ret.x = -x / detQ;
        ret.y = -y / detQ;
        ret.z = -z / detQ;
    }
    return ret;
}

double getQuadricsError(double *Q, const Vertex &v) {
    // v^T Q v
    double error = 0;

    for (int i = 0; i < 4; ++i) {
        double reduceSum = 0;

        reduceSum += v.x * Q[0 * 4 + i];
        reduceSum += v.y * Q[1 * 4 + i];
        reduceSum += v.z * Q[2 * 4 + i];
        reduceSum += 1 * Q[3 * 4 + i];

        double coeff = 1;
        switch (i) {
        case 0:
            coeff = v.x;
            break;
        case 1:
            coeff = v.y;
            break;
        case 2:
            coeff = v.z;
            break;
        case 3:
            coeff = 1;
            break;
        }

        // std::cout << reduceSum << std::endl;

        error += coeff * reduceSum;
    }

    return error;
}
