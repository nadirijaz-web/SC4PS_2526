#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    ORDER_IJK = 0,
    ORDER_IKJ,
    ORDER_JIK,
    ORDER_JKI,
    ORDER_KIJ,
    ORDER_KJI,
    ORDER_COUNT
} loop_order_t;

static const char *ORDER_NAMES[ORDER_COUNT] = {
    "i-j-k", "i-k-j", "j-i-k", "j-k-i", "k-i-j", "k-j-i"
};

static inline size_t idx(size_t n, size_t i, size_t j) {
    return i * n + j;
}

static double now_seconds(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static int parse_double(const char *text, double *out) {
    char *end = NULL;
    errno = 0;
    const double value = strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0') {
        return 0;
    }
    *out = value;
    return 1;
}

static int parse_size_t(const char *text, size_t *out) {
    char *end = NULL;
    errno = 0;
    const unsigned long long value = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return 0;
    }
    *out = (size_t)value;
    return 1;
}

static void fill_problem_matrices(double *a_mat, double *b_mat, size_t n, double a, double b) {
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            a_mat[idx(n, i, j)] = a;
            b_mat[idx(n, i, j)] = b;
        }
    }
}

static void zero_matrix(double *c_mat, size_t n) {
    memset(c_mat, 0, n * n * sizeof(double));
}

static void matmul_order(const double *a_mat, const double *b_mat, double *c_mat, size_t n, loop_order_t order) {
    zero_matrix(c_mat, n);

    switch (order) {
    case ORDER_IJK:
        for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < n; ++j)
                for (size_t k = 0; k < n; ++k)
                    c_mat[idx(n, i, j)] += a_mat[idx(n, i, k)] * b_mat[idx(n, k, j)];
        break;

    case ORDER_IKJ:
        for (size_t i = 0; i < n; ++i)
            for (size_t k = 0; k < n; ++k) {
                const double aik = a_mat[idx(n, i, k)];
                for (size_t j = 0; j < n; ++j)
                    c_mat[idx(n, i, j)] += aik * b_mat[idx(n, k, j)];
            }
        break;

    case ORDER_JIK:
        for (size_t j = 0; j < n; ++j)
            for (size_t i = 0; i < n; ++i)
                for (size_t k = 0; k < n; ++k)
                    c_mat[idx(n, i, j)] += a_mat[idx(n, i, k)] * b_mat[idx(n, k, j)];
        break;

    case ORDER_JKI:
        for (size_t j = 0; j < n; ++j)
            for (size_t k = 0; k < n; ++k) {
                const double bkj = b_mat[idx(n, k, j)];
                for (size_t i = 0; i < n; ++i)
                    c_mat[idx(n, i, j)] += a_mat[idx(n, i, k)] * bkj;
            }
        break;

    case ORDER_KIJ:
        for (size_t k = 0; k < n; ++k)
            for (size_t i = 0; i < n; ++i) {
                const double aik = a_mat[idx(n, i, k)];
                for (size_t j = 0; j < n; ++j)
                    c_mat[idx(n, i, j)] += aik * b_mat[idx(n, k, j)];
            }
        break;

    case ORDER_KJI:
        for (size_t k = 0; k < n; ++k)
            for (size_t j = 0; j < n; ++j) {
                const double bkj = b_mat[idx(n, k, j)];
                for (size_t i = 0; i < n; ++i)
                    c_mat[idx(n, i, j)] += a_mat[idx(n, i, k)] * bkj;
            }
        break;

    default:
        break;
    }
}

static int quick_verify_constant_matrix(const double *c_mat, size_t n, double expected, double tol) {
    double m1 = 0.0;
    double m2 = 0.0;

    for (size_t i = 0; i < n * n; ++i) {
        m1 += c_mat[i];
        m2 += c_mat[i] * c_mat[i];
    }

    const double n2 = (double)n * (double)n;
    const double m1_expected = n2 * expected;
    const double m2_expected = n2 * expected * expected;

    const int moments_ok = fabs(m1 - m1_expected) <= tol * fmax(1.0, fabs(m1_expected)) &&
                           fabs(m2 - m2_expected) <= tol * fmax(1.0, fabs(m2_expected));
    const int anchor_ok = fabs(c_mat[0] - expected) <= tol * fmax(1.0, fabs(expected));

    return moments_ok && anchor_ok;
}

static int full_verify_all_elements(const double *c_mat, size_t n, double expected, double tol) {
    for (size_t i = 0; i < n * n; ++i) {
        if (fabs(c_mat[i] - expected) > tol * fmax(1.0, fabs(expected))) {
            return 0;
        }
    }
    return 1;
}

static int save_matrix_to_file(const char *path, const double *mat, size_t n) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "Could not open output file '%s': %s\n", path, strerror(errno));
        return 0;
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            fprintf(fp, "%.10g%s", mat[idx(n, i, j)], (j + 1 == n) ? "" : " ");
        }
        fputc('\n', fp);
    }

    fclose(fp);
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr,
                "Usage: %s <a> <b> <N> <fileout>\n"
                "A = a 1_N, B = b 1_N, C = AB with C_ij = N*a*b\n",
                argv[0]);
        return 1;
    }

    double a = 0.0;
    double b = 0.0;
    size_t n = 0;
    const char *fileout = argv[4];

    if (!parse_double(argv[1], &a) || !parse_double(argv[2], &b)) {
        fprintf(stderr, "Invalid inputs: a and b must be numeric.\n");
        return 1;
    }

    if (!parse_size_t(argv[3], &n)) {
        fprintf(stderr, "Invalid input: N must be a positive integer.\n");
        return 1;
    }

    if (n == 0) {
        fprintf(stderr, "N must be > 0.\n");
        return 1;
    }

    double *a_mat = (double *)malloc(n * n * sizeof(double));
    double *b_mat = (double *)malloc(n * n * sizeof(double));
    double *c_mat = (double *)malloc(n * n * sizeof(double));

    if (!a_mat || !b_mat || !c_mat) {
        fprintf(stderr, "Memory allocation failed for N=%zu.\n", n);
        free(a_mat);
        free(b_mat);
        free(c_mat);
        return 1;
    }

    fill_problem_matrices(a_mat, b_mat, n, a, b);

    const int repeats = 3;
    double best_time = 1e300;
    loop_order_t best_order = ORDER_IJK;

    printf("Benchmarking %d loop orderings (N=%zu, repeats=%d)...\n", ORDER_COUNT, n, repeats);

    for (int ord = 0; ord < ORDER_COUNT; ++ord) {
        double min_t = 1e300;

        for (int r = 0; r < repeats; ++r) {
            const double t0 = now_seconds();
            matmul_order(a_mat, b_mat, c_mat, n, (loop_order_t)ord);
            const double t1 = now_seconds();
            const double dt = t1 - t0;

            if (dt < min_t) {
                min_t = dt;
            }
        }

        printf("  %-5s : %.6f s\n", ORDER_NAMES[ord], min_t);

        if (min_t < best_time) {
            best_time = min_t;
            best_order = (loop_order_t)ord;
        }
    }

    matmul_order(a_mat, b_mat, c_mat, n, best_order);

    const double expected = (double)n * a * b;
    const double tol = 1e-10;
    const int quick_ok = quick_verify_constant_matrix(c_mat, n, expected, tol);
    const int full_ok = full_verify_all_elements(c_mat, n, expected, tol);

    printf("Fastest ordering: %s (%.6f s)\n", ORDER_NAMES[best_order], best_time);
    printf("Quick verification (moments + anchor) for C_ij ~= N*a*b (%g): %s\n",
           expected, quick_ok ? "PASS" : "FAIL");
    printf("Full verification (all elements) for C_ij ~= N*a*b (%g): %s\n",
           expected, full_ok ? "PASS" : "FAIL");

    if (!save_matrix_to_file(fileout, c_mat, n)) {
        free(a_mat);
        free(b_mat);
        free(c_mat);
        return 1;
    }

    printf("Saved matrix C to %s\n", fileout);

    free(a_mat);
    free(b_mat);
    free(c_mat);

    return (quick_ok && full_ok) ? 0 : 2;
}