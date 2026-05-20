#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rng.h"

#define PI 3.14159265358979323846

static int ensure_directory(const char *path) {
    if (mkdir(path, 0777) == 0) {
        return 0;
    }
    if (errno == EEXIST) {
        return 0;
    }

    fprintf(stderr, "Could not create directory '%s': %s\n", path, strerror(errno));
    return -1;
}

static FILE *open_output(const char *path) {
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open '%s' for writing: %s\n", path, strerror(errno));
    }
    return fp;
}

static int compare_double(const void *a, const void *b) {
    const double x = *(const double *)a;
    const double y = *(const double *)b;

    if (x < y) {
        return -1;
    }
    if (x > y) {
        return 1;
    }
    return 0;
}

static void exercise_1_coin_tosses(FILE *report) {
    const int n_tosses = 100000;
    const int checkpoints[] = {10, 100, 1000, 10000, 100000};
    const size_t n_checkpoints = sizeof(checkpoints) / sizeof(checkpoints[0]);
    int heads = 0;
    size_t checkpoint_index = 0;
    Lcg32 rng;
    FILE *data = open_output("output/coin_running.dat");

    if (data == NULL) {
        exit(EXIT_FAILURE);
    }

    lcg32_seed(&rng, 12345u);
    fprintf(report, "Exercise 1: Coin tosses and LLN\n");
    fprintf(report, "N running_fraction_heads\n");
    fprintf(data, "# N running_fraction_heads\n");

    for (int i = 1; i <= n_tosses; i++) {
        if (lcg32_next_double(&rng) < 0.5) {
            heads++;
        }

        const double fraction = (double)heads / (double)i;
        fprintf(data, "%d %.10f\n", i, fraction);

        if (checkpoint_index < n_checkpoints && i == checkpoints[checkpoint_index]) {
            fprintf(report, "%d %.5f\n", i, fraction);
            checkpoint_index++;
        }
    }

    fprintf(report,
            "Comment: the running fraction fluctuates for small N and then "
            "stabilizes near 0.5, as expected from the law of large numbers.\n\n");

    fclose(data);
}

static void exercise_2_pi(FILE *report) {
    const int sample_sizes[] = {100, 1000, 10000, 100000, 1000000};
    const size_t n_sizes = sizeof(sample_sizes) / sizeof(sample_sizes[0]);
    const int max_n = sample_sizes[n_sizes - 1];
    int inside = 0;
    size_t size_index = 0;
    Lcg32 rng_x;
    Lcg32 rng_y;
    FILE *data = open_output("output/pi_results.dat");

    if (data == NULL) {
        exit(EXIT_FAILURE);
    }

    lcg32_seed(&rng_x, 2026u);
    lcg32_seed(&rng_y, 2027u);

    fprintf(report, "Exercise 2: Monte Carlo estimate of pi\n");
    fprintf(report, "N pi_hat absolute_error\n");
    fprintf(data, "# N pi_hat absolute_error\n");

    for (int i = 1; i <= max_n; i++) {
        const double x = lcg32_next_double(&rng_x);
        const double y = lcg32_next_double(&rng_y);

        if (x * x + y * y <= 1.0) {
            inside++;
        }

        if (size_index < n_sizes && i == sample_sizes[size_index]) {
            const double pi_hat = 4.0 * (double)inside / (double)i;
            const double error = fabs(pi_hat - PI);

            fprintf(report, "%d %.6f %.6f\n", i, pi_hat, error);
            fprintf(data, "%d %.12f %.12f\n", i, pi_hat, error);
            size_index++;
        }
    }

    fprintf(report,
            "Comment: the error is random and need not decrease at every listed "
            "N, but its typical scale decreases like 1/sqrt(N).\n\n");

    fclose(data);
}

static void exercise_3_change_of_variables(FILE *report) {
    const int n = 200000;
    Lcg32 rng;
    FILE *data = open_output("output/y_square_sample.dat");

    if (data == NULL) {
        exit(EXIT_FAILURE);
    }

    lcg32_seed(&rng, 13579u);
    fprintf(data, "# y = u^2\n");

    for (int i = 0; i < n; i++) {
        const double u = lcg32_next_double(&rng);
        fprintf(data, "%.12f\n", u * u);
    }

    fprintf(report, "Exercise 3: Change of variables Y = U^2\n");
    fprintf(report,
            "Analytic density: f_Y(y) = 1/(2*sqrt(y)) for 0 < y < 1.\n");
    fprintf(report,
            "Comment: the generated data should produce a histogram that is "
            "largest near zero and follows the analytic density.\n\n");

    fclose(data);
}

static double *exercise_4_inverse_exponential(FILE *report, int *n_out, double *lambda_out) {
    const int n = 200000;
    const double lambda = 1.5;
    double *sample = malloc((size_t)n * sizeof(*sample));
    double sum = 0.0;
    Lcg32 rng;
    FILE *data = open_output("output/exponential_sample.dat");

    if (sample == NULL) {
        fprintf(stderr, "Could not allocate exponential sample.\n");
        exit(EXIT_FAILURE);
    }
    if (data == NULL) {
        free(sample);
        exit(EXIT_FAILURE);
    }

    lcg32_seed(&rng, 246813579u);
    fprintf(data, "# y = -log(1-u)/lambda, lambda = %.6f\n", lambda);

    for (int i = 0; i < n; i++) {
        const double u = lcg32_next_double(&rng);
        const double y = -log(1.0 - u) / lambda;

        sample[i] = y;
        sum += y;
        fprintf(data, "%.12f\n", y);
    }

    fprintf(report, "Exercise 4: Inverse transform exponential\n");
    fprintf(report, "lambda = %.6f\n", lambda);
    fprintf(report, "sample_mean = %.5f\n", sum / (double)n);
    fprintf(report, "exact_mean = %.5f\n", 1.0 / lambda);
    fprintf(report,
            "Comment: the inverse transform sample should match the exponential "
            "PDF lambda*exp(-lambda*y), and the mean is close to 1/lambda.\n\n");

    fclose(data);
    *n_out = n;
    *lambda_out = lambda;
    return sample;
}

static void exercise_5_empirical_cdf(FILE *report, double *sample, int n, double lambda) {
    double max_diff = 0.0;
    FILE *data = open_output("output/empirical_cdf.dat");

    if (data == NULL) {
        exit(EXIT_FAILURE);
    }

    qsort(sample, (size_t)n, sizeof(*sample), compare_double);
    fprintf(data, "# y empirical_cdf exact_cdf\n");

    for (int i = 0; i < n; i++) {
        const double empirical = (double)(i + 1) / (double)n;
        const double exact = 1.0 - exp(-lambda * sample[i]);
        const double diff = fabs(empirical - exact);

        if (diff > max_diff) {
            max_diff = diff;
        }
        fprintf(data, "%.12f %.12f %.12f\n", sample[i], empirical, exact);
    }

    fprintf(report, "Exercise 5: Empirical CDF\n");
    fprintf(report, "maximum_absolute_cdf_difference = %.5f\n", max_diff);
    fprintf(report,
            "Comment: the empirical CDF is a step function and lies close to "
            "the exact CDF F(y)=1-exp(-lambda*y).\n\n");

    fclose(data);
}

int main(void) {
    FILE *report;
    double *exponential_sample;
    int exponential_n;
    double lambda;

    if (ensure_directory("output") != 0) {
        return EXIT_FAILURE;
    }

    report = open_output("output/report.txt");
    if (report == NULL) {
        return EXIT_FAILURE;
    }

    fprintf(report, "Homework 07 - Random numbers\n");
    fprintf(report,
            "Generator used for the exercises: LCG x_{n+1} = 1664525*x_n + "
            "1013904223 mod 2^32, as in the lesson notes.\n\n");

    exercise_1_coin_tosses(report);
    exercise_2_pi(report);
    exercise_3_change_of_variables(report);
    exponential_sample = exercise_4_inverse_exponential(report, &exponential_n, &lambda);
    exercise_5_empirical_cdf(report, exponential_sample, exponential_n, lambda);

    fclose(report);
    free(exponential_sample);

    printf("Wrote homework results to output/report.txt and data files to output/.\n");
    return EXIT_SUCCESS;
}
