#ifndef LEGENDRE_HW_H
#define LEGENDRE_HW_H

void legendre_forward(double x, int lmax, double *P);
void legendre_backward(double x, int lmax, int L, double *P);

#endif
