#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

bool check_vector(double d[], double expected, int n) {
    double tol = 1e-12;

    for (int i = 0; i < n; i++) {
        if (fabs(d[i] - expected) > tol) {
            printf("Error at index %d: got %.1f, expected %.15f\n",
                   i, d[i], expected);
            return false;
        }
    }

    return true;
}

void fill_vector(double v[], int n, double value) {
    for (int i = 0; i < n; i++) {
        v[i] = value;
    }
}

void compute_sum(double d[], double x[], double y[], double a, int n) {
    for (int i = 0; i < n; i++) {
        d[i] = a * x[i] + y[i];
    }
}

int run_case(int n, double a, double x_value, double y_value) {
    double *X = malloc(n * sizeof(double));
    double *Y = malloc(n * sizeof(double));
    double *D = malloc(n * sizeof(double));

    if (X == NULL || Y == NULL || D == NULL) {
        printf("Memory allocation failed for N = %d\n", n);
        free(X);
        free(Y);
        free(D);
        return 1;
    }

    fill_vector(X, n, x_value);
    fill_vector(Y, n, y_value);
    compute_sum(D, X, Y, a, n);

    double expected = a * x_value + y_value;

    printf("For N = %d:\n", n);
    printf("a = %.2f, x = %.2f, y = %.2f\n", a, x_value, y_value);
    printf("Expected value a*x + y = %.1f\n", expected);
    printf("First computed value D[0] = %.1f\n", D[0]);

    if (check_vector(D, expected, n)) {
        printf("Check passed for N = %d\n\n", n);
    } else {
        printf("Check failed for N = %d\n\n", n);
    }

    free(X);
    free(Y);
    free(D);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s a x y\n", argv[0]);
        return 1;
    }

    double a = strtod(argv[1], NULL);
    double x = strtod(argv[2], NULL);
    double y = strtod(argv[3], NULL);

    printf("Special example check:\n");
    printf("a = %.2f, x = %.2f, y = %.2f\n", a, x, y);
    printf("a*x + y = %.1f\n\n", a * x + y);

    int sizes[3] = {10, 1000000, 100000000};

    for (int i = 0; i < 3; i++) {
        run_case(sizes[i], a, x, y);
    }

    return 0;
}
