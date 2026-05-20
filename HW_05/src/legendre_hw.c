#include "legendre_hw.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

void legendre_forward(double x, int lmax, double *P) {
    P[0] = 1.0;
    if (lmax == 0) {
        return;
    }
    P[1] = x;
    for (int ell = 1; ell < lmax; ++ell) {
        P[ell + 1] = ((2.0 * ell + 1.0) * x * P[ell] - (double)ell * P[ell - 1]) / (double)(ell + 1);
    }
}

void legendre_backward(double x, int lmax, int L, double *P) {
    if (L <= lmax) {
        L = lmax + 30;
    }

    double *Q = (double *)malloc(((size_t)L + 2U) * sizeof(double));
    if (Q == NULL) {
        for (int ell = 0; ell <= lmax; ++ell) {
            P[ell] = NAN;
        }
        return;
    }

    Q[L + 1] = 0.0;
    Q[L] = 1.0;

    for (int ell = L; ell >= 1; --ell) {
        Q[ell - 1] = ((2.0 * ell + 1.0) * x * Q[ell] - (double)(ell + 1) * Q[ell + 1]) / (double)ell;
    }

    const double c = 1.0 / Q[0];
    for (int ell = 0; ell <= lmax; ++ell) {
        P[ell] = c * Q[ell];
    }

    free(Q);
}
