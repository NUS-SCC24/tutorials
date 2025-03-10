/**
 *
 * Sparse Matrix-Vector Product (SpMV)
 *
 * TKR
 *
 **/
#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>
#include "timer.hpp"

size_t size;

int sparseness;

typedef float* vector;

typedef struct
{
    float** element;
} matrix;

typedef struct
{
    float* v;
    size_t* col_idx;
    size_t* row_idx;
    size_t nnz;
} csr_matrix;


void allocate_vector(vector* v)
{
    // allocate array for all the rows
    *v = (float*)malloc(sizeof(float) * size);
    if (v == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
}


void allocate_matrix(matrix* m)
{
    // allocate array for all the rows
    m->element = (float**)malloc(sizeof(float*) * size);
    if (m->element == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    // allocate an array for each row of the matrix
    for (size_t i = 0; i < size; i++) {
        m->element[i] = (float*)malloc(sizeof(float) * size);
        if (m->element[i] == NULL) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
    }
}

void free_vector(vector v)
{
    free(v);
}

void free_matrix(matrix* m)
{
    for (size_t i = 0; i < size; i++) {
        free(m->element[i]);
    }
    free(m->element);
}

void free_csr_matrix(csr_matrix* m)
{
    free(m->v);
    free(m->row_idx);
    free(m->col_idx);
}

void init_vector(vector* v)
{
    for (size_t i = 0; i < size; i++)
        (*v)[i] = (float)rand()/(float)(RAND_MAX/9);
}

void init_matrix_sparse(matrix* m)
{
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            if (rand() % sparseness)
                m->element[i][j] = 0;
            else
                m->element[i][j] = (float)rand()/(float)(RAND_MAX/8) + 1;
        }
    }
}

void compress(matrix* m, csr_matrix* c)
{
    size_t nnz = 0;
    for (size_t i = 0; i < size; i++)
        for (size_t j = 0; j < size; j++)
            if (m->element[i][j] != 0.0)
                nnz++;
    
    c->v       = (float*)malloc(sizeof(float) * nnz); 
    c->col_idx = (size_t*)malloc(sizeof(size_t) * nnz); 
    c->row_idx = (size_t*)malloc(sizeof(size_t) * (size + 1));
    c->nnz     = nnz;

    nnz = 0;
    c->row_idx[0] = 0;
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            if (m->element[i][j] != 0.0) {
                c->v[nnz] = m->element[i][j];
                c->col_idx[nnz] = j; 
                nnz++;
            }
        }
        c->row_idx[i+1] = nnz;
    }
}

/**
 * Multiplies sparse matrix @a with vector @b storing
 * the result in vector @result
 */
void spmv(csr_matrix a, vector b, vector result)
{
    for (size_t i = 0; i < size; i++) {
        size_t row_start = a.row_idx[i];
        size_t row_end   = a.row_idx[i+1]; 
        float sum = 0;
        for(size_t j = row_start; j < row_end; j++) {
            size_t a_col = a.col_idx[j];
            float a_coef = a.v[j];
            float b_coef = b[a_col];
            sum += a_coef * b_coef;
        }
        result[i] = sum;
    }
}

double check_vector(vector* v)
{
    double t = 0.0;

    for (size_t i = 0; i < size; i++)
        t += (*v)[i];
 
    return t;
}

void print_vector(vector* m)
{
    printf("          ");
    for (size_t i = 0; i < size; i++) {
        printf("%6.2f  ", (*m)[i]);
    }
    printf("\n");
}

void print_matrix(matrix* m)
{
    for (size_t i = 0; i < size; i++) {
        printf("row %4lu: ", i);
        for (size_t j = 0; j < size; j++)
            printf("%6.2f  ", m->element[i][j]);
        printf("\n");
    }
}

void work()
{
    matrix a;
    csr_matrix a_c;
    vector b, result;
    long long before, after;

    allocate_matrix(&a);
    allocate_vector(&b);
    allocate_vector(&result);

    init_matrix_sparse(&a);
    init_vector(&b);
    init_vector(&result);

    compress(&a, &a_c);

    printf("Compressed matrix with %lu non-zero elements\n", a_c.nnz);   
    
    before = wall_clock_time();
    spmv(a_c, b, result);
    after = wall_clock_time();

    fprintf(stderr, "Multiplication took %1.2f ms\n", ((float)(after - before)) / 1000000);

    printf("Check value: %16.18f\n", check_vector(&result));   
   
    if (size <= 8) {
        print_matrix(&a);
        printf("            *\n");
        print_vector(&b);
        printf("            =\n");
        print_vector(&result);
    }

    free_matrix(&a);
    free_csr_matrix(&a_c);
    free_vector(b);
    free_vector(result);
}

int main(int argc, char** argv)
{
    srand(0);

    printf("Usage: %s <size> <sparseness>\n", argv[0]);

    size = 1024;
    sparseness = 1000;

    if (argc >= 2)
        size = atoi(argv[1]);
    if (argc >= 3)
        sparseness = atoi(argv[2]);

    printf("Sparse Matrix-Vector Multiplication of size %'ld, sparseness %3.6f%%\n",
        size * size,
        (float)(sparseness - 1)/(float)sparseness * 100.0
    );

    work();

    return 0;
}
