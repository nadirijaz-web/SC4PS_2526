#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <hdf5.h>

#define MAX_LINE 512
#define MAX_PATH 512

typedef struct {
    unsigned long long n;
    unsigned long long chunk_size;
    double a;
    double x_value;
    double y_value;
    char output_file[MAX_PATH];
    bool has_n;
    bool has_chunk_size;
    bool has_a;
    bool has_x_value;
    bool has_y_value;
    bool has_output_file;
} Config;

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) {
        s++;
    }
    if (*s == '\0') {
        return s;
    }
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return s;
}

static void lower_key(char *s) {
    for (; *s; s++) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static void remove_quotes(char *s) {
    size_t len = strlen(s);
    if (len >= 2) {
        if ((s[0] == '"' && s[len - 1] == '"') || (s[0] == '\'' && s[len - 1] == '\'')) {
            memmove(s, s + 1, len - 2);
            s[len - 2] = '\0';
        }
    }
}

static bool read_ull(const char *text, unsigned long long *out) {
    errno = 0;
    char *end = NULL;
    unsigned long long value = strtoull(text, &end, 10);
    if (errno != 0 || end == text) {
        return false;
    }
    while (*end) {
        if (!isspace((unsigned char)*end)) {
            return false;
        }
        end++;
    }
    *out = value;
    return true;
}

static bool read_double(const char *text, double *out) {
    errno = 0;
    char *end = NULL;
    double value = strtod(text, &end);
    if (errno != 0 || end == text) {
        return false;
    }
    while (*end) {
        if (!isspace((unsigned char)*end)) {
            return false;
        }
        end++;
    }
    *out = value;
    return true;
}

static int read_config(const char *filename, Config *cfg) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open input file: %s\n", filename);
        return 1;
    }

    char line[MAX_LINE];
    int line_no = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;
        char *comment = strchr(line, '#');
        if (comment != NULL) {
            *comment = '\0';
        }

        char *clean = trim(line);
        if (*clean == '\0') {
            continue;
        }

        char *eq = strchr(clean, '=');
        if (eq == NULL) {
            printf("Invalid input line %d: expected Variable = Value\n", line_no);
            fclose(fp);
            return 1;
        }

        *eq = '\0';
        char *key = trim(clean);
        char *value = trim(eq + 1);
        lower_key(key);

        if (strcmp(key, "n") == 0) {
            if (!read_ull(value, &cfg->n)) {
                printf("Invalid value for n on line %d\n", line_no);
                fclose(fp);
                return 1;
            }
            cfg->has_n = true;
        } else if (strcmp(key, "chunk_size") == 0 || strcmp(key, "chunksize") == 0) {
            if (!read_ull(value, &cfg->chunk_size)) {
                printf("Invalid value for chunk_size on line %d\n", line_no);
                fclose(fp);
                return 1;
            }
            cfg->has_chunk_size = true;
        } else if (strcmp(key, "a") == 0) {
            if (!read_double(value, &cfg->a)) {
                printf("Invalid value for a on line %d\n", line_no);
                fclose(fp);
                return 1;
            }
            cfg->has_a = true;
        } else if (strcmp(key, "x_value") == 0 || strcmp(key, "x") == 0) {
            if (!read_double(value, &cfg->x_value)) {
                printf("Invalid value for x_value on line %d\n", line_no);
                fclose(fp);
                return 1;
            }
            cfg->has_x_value = true;
        } else if (strcmp(key, "y_value") == 0 || strcmp(key, "y") == 0) {
            if (!read_double(value, &cfg->y_value)) {
                printf("Invalid value for y_value on line %d\n", line_no);
                fclose(fp);
                return 1;
            }
            cfg->has_y_value = true;
        } else if (strcmp(key, "output_file") == 0 || strcmp(key, "output") == 0) {
            remove_quotes(value);
            if (strlen(value) >= sizeof(cfg->output_file)) {
                printf("output_file is too long on line %d\n", line_no);
                fclose(fp);
                return 1;
            }
            strcpy(cfg->output_file, value);
            cfg->has_output_file = true;
        } else {
            printf("Unknown input variable on line %d: %s\n", line_no, key);
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);

    if (!cfg->has_n || !cfg->has_a || !cfg->has_x_value || !cfg->has_y_value ||
        !cfg->has_chunk_size || !cfg->has_output_file) {
        printf("Input file must contain n, a, x_value, y_value, chunk_size, and output_file.\n");
        return 1;
    }

    if (cfg->n == 0 || cfg->chunk_size == 0) {
        printf("n and chunk_size must both be positive.\n");
        return 1;
    }

    return 0;
}

static int write_scalar_double(hid_t group, const char *name, double value) {
    hid_t space = H5Screate(H5S_SCALAR);
    if (space < 0) {
        return 1;
    }
    hid_t dset = H5Dcreate2(group, name, H5T_NATIVE_DOUBLE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dset < 0) {
        H5Sclose(space);
        return 1;
    }
    int status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value) < 0;
    H5Dclose(dset);
    H5Sclose(space);
    return status;
}

static int write_scalar_ull(hid_t group, const char *name, unsigned long long value) {
    hid_t space = H5Screate(H5S_SCALAR);
    if (space < 0) {
        return 1;
    }
    hid_t dset = H5Dcreate2(group, name, H5T_NATIVE_ULLONG, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dset < 0) {
        H5Sclose(space);
        return 1;
    }
    int status = H5Dwrite(dset, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value) < 0;
    H5Dclose(dset);
    H5Sclose(space);
    return status;
}

static int write_string(hid_t group, const char *name, const char *value) {
    hid_t type = H5Tcopy(H5T_C_S1);
    if (type < 0) {
        return 1;
    }
    H5Tset_size(type, strlen(value) + 1);
    H5Tset_strpad(type, H5T_STR_NULLTERM);

    hid_t space = H5Screate(H5S_SCALAR);
    if (space < 0) {
        H5Tclose(type);
        return 1;
    }
    hid_t dset = H5Dcreate2(group, name, type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dset < 0) {
        H5Sclose(space);
        H5Tclose(type);
        return 1;
    }
    int status = H5Dwrite(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, value) < 0;
    H5Dclose(dset);
    H5Sclose(space);
    H5Tclose(type);
    return status;
}

static int create_hw06_output(const Config *cfg) {
    const double expected_value = cfg->a * cfg->x_value + cfg->y_value;
    const unsigned long long num_chunks = (cfg->n + cfg->chunk_size - 1) / cfg->chunk_size;

    double *buffer = malloc((size_t)cfg->chunk_size * sizeof(double));
    if (buffer == NULL) {
        printf("Memory allocation failed for chunk_size = %llu\n", cfg->chunk_size);
        return 1;
    }

    hsize_t d_dims[1] = {(hsize_t)cfg->n};
    hsize_t c_dims[1] = {(hsize_t)num_chunks};
    hsize_t d_chunk[1] = {(hsize_t)((cfg->chunk_size < cfg->n) ? cfg->chunk_size : cfg->n)};

    hid_t file = H5Fcreate(cfg->output_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file < 0) {
        printf("Could not create HDF5 output file: %s\n", cfg->output_file);
        free(buffer);
        return 1;
    }

    hid_t d_space = H5Screate_simple(1, d_dims, NULL);
    hid_t d_plist = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(d_plist, 1, d_chunk);
    hid_t d_set = H5Dcreate2(file, "D", H5T_NATIVE_DOUBLE, d_space, H5P_DEFAULT, d_plist, H5P_DEFAULT);

    hid_t c_space = H5Screate_simple(1, c_dims, NULL);
    hid_t c_set = H5Dcreate2(file, "chunk_sums", H5T_NATIVE_DOUBLE, c_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    if (d_space < 0 || d_plist < 0 || d_set < 0 || c_space < 0 || c_set < 0) {
        printf("Could not create one or more HDF5 datasets.\n");
        if (c_set >= 0) H5Dclose(c_set);
        if (c_space >= 0) H5Sclose(c_space);
        if (d_set >= 0) H5Dclose(d_set);
        if (d_plist >= 0) H5Pclose(d_plist);
        if (d_space >= 0) H5Sclose(d_space);
        H5Fclose(file);
        free(buffer);
        return 1;
    }

    double global_sum = 0.0;
    double max_abs_error = 0.0;
    double first_value = 0.0;

    for (unsigned long long chunk = 0; chunk < num_chunks; chunk++) {
        unsigned long long offset = chunk * cfg->chunk_size;
        unsigned long long remaining = cfg->n - offset;
        unsigned long long this_n = (remaining < cfg->chunk_size) ? remaining : cfg->chunk_size;

        double chunk_sum = 0.0;
        for (unsigned long long i = 0; i < this_n; i++) {
            buffer[i] = cfg->a * cfg->x_value + cfg->y_value;
            chunk_sum += buffer[i];
            double error = fabs(buffer[i] - expected_value);
            if (error > max_abs_error) {
                max_abs_error = error;
            }
        }
        if (chunk == 0) {
            first_value = buffer[0];
        }
        global_sum += chunk_sum;

        hsize_t start[1] = {(hsize_t)offset};
        hsize_t count[1] = {(hsize_t)this_n};
        hid_t mem_space = H5Screate_simple(1, count, NULL);
        if (mem_space < 0) {
            printf("Could not create memory space for chunk %llu\n", chunk);
            H5Dclose(c_set);
            H5Sclose(c_space);
            H5Dclose(d_set);
            H5Pclose(d_plist);
            H5Sclose(d_space);
            H5Fclose(file);
            free(buffer);
            return 1;
        }
        H5Sselect_hyperslab(d_space, H5S_SELECT_SET, start, NULL, count, NULL);
        if (H5Dwrite(d_set, H5T_NATIVE_DOUBLE, mem_space, d_space, H5P_DEFAULT, buffer) < 0) {
            printf("Could not write chunk %llu to dataset D\n", chunk);
            H5Sclose(mem_space);
            H5Dclose(c_set);
            H5Sclose(c_space);
            H5Dclose(d_set);
            H5Pclose(d_plist);
            H5Sclose(d_space);
            H5Fclose(file);
            free(buffer);
            return 1;
        }
        H5Sclose(mem_space);

        hsize_t c_start[1] = {(hsize_t)chunk};
        hsize_t c_count[1] = {1};
        hid_t c_mem_space = H5Screate_simple(1, c_count, NULL);
        H5Sselect_hyperslab(c_space, H5S_SELECT_SET, c_start, NULL, c_count, NULL);
        if (H5Dwrite(c_set, H5T_NATIVE_DOUBLE, c_mem_space, c_space, H5P_DEFAULT, &chunk_sum) < 0) {
            printf("Could not write chunk sum %llu\n", chunk);
            H5Sclose(c_mem_space);
            H5Dclose(c_set);
            H5Sclose(c_space);
            H5Dclose(d_set);
            H5Pclose(d_plist);
            H5Sclose(d_space);
            H5Fclose(file);
            free(buffer);
            return 1;
        }
        H5Sclose(c_mem_space);

        if (num_chunks <= 20 || chunk < 3 || chunk + 3 >= num_chunks) {
            printf("Chunk %llu: elements %llu to %llu, partial sum = %.15g\n",
                   chunk, offset, offset + this_n - 1, chunk_sum);
        } else if (chunk == 3) {
            printf("...\n");
        }
    }

    hid_t param = H5Gcreate2(file, "parameters", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    hid_t checks = H5Gcreate2(file, "checks", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    int write_error = 0;
    if (param < 0 || checks < 0) {
        write_error = 1;
    } else {
        write_error |= write_scalar_ull(param, "n", cfg->n);
        write_error |= write_scalar_ull(param, "chunk_size", cfg->chunk_size);
        write_error |= write_scalar_ull(param, "num_chunks", num_chunks);
        write_error |= write_scalar_double(param, "a", cfg->a);
        write_error |= write_scalar_double(param, "x_value", cfg->x_value);
        write_error |= write_scalar_double(param, "y_value", cfg->y_value);
        write_error |= write_string(param, "operation", "D = a * X + Y");
        write_error |= write_scalar_double(checks, "expected_value", expected_value);
        write_error |= write_scalar_double(checks, "first_value", first_value);
        write_error |= write_scalar_double(checks, "global_sum", global_sum);
        write_error |= write_scalar_double(checks, "expected_global_sum", expected_value * (double)cfg->n);
        write_error |= write_scalar_double(checks, "max_abs_error", max_abs_error);
    }

    if (checks >= 0) H5Gclose(checks);
    if (param >= 0) H5Gclose(param);
    H5Dclose(c_set);
    H5Sclose(c_space);
    H5Dclose(d_set);
    H5Pclose(d_plist);
    H5Sclose(d_space);
    H5Fclose(file);
    free(buffer);

    if (write_error) {
        printf("The main data were written, but metadata/check datasets could not be completed.\n");
        return 1;
    }

    double expected_global_sum = expected_value * (double)cfg->n;
    double global_error = fabs(global_sum - expected_global_sum);
    double tolerance = 1e-10 * fmax(1.0, fabs(expected_global_sum));

    printf("\nHW06 output written to %s\n", cfg->output_file);
    printf("First computed value D[0] = %.15g\n", first_value);
    printf("Expected value a*x + y = %.15g\n", expected_value);
    printf("Global sum = %.15g\n", global_sum);
    printf("Expected global sum = %.15g\n", expected_global_sum);
    printf("Max absolute element error = %.15g\n", max_abs_error);

    if (max_abs_error <= 1e-12 && global_error <= tolerance) {
        printf("Check passed. Output matches D[i] = a*x + y for all elements.\n");
        return 0;
    }

    printf("Check failed.\n");
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s input.txt\n", argv[0]);
        return 1;
    }

    Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    if (read_config(argv[1], &cfg) != 0) {
        return 1;
    }

    printf("HW06 chunked DAXPY calculation\n");
    printf("Input file: %s\n", argv[1]);
    printf("n = %llu\n", cfg.n);
    printf("a = %.15g\n", cfg.a);
    printf("x_value = %.15g\n", cfg.x_value);
    printf("y_value = %.15g\n", cfg.y_value);
    printf("chunk_size = %llu\n", cfg.chunk_size);
    printf("output_file = %s\n\n", cfg.output_file);

    return create_hw06_output(&cfg);
}
