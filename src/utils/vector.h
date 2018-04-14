#ifndef VECTOR_H
#define VECTOR_H

typedef struct vec_t {
    long double x;
    long double y;
} Vector;

#endif

long double vec_len(Vector v);
long double vec_dot(Vector v1, Vector v2);
Vector vec_add(Vector v1, Vector v2);
Vector vec_sub(Vector v1, Vector v2);
Vector vec_mul_scalar(long double k, Vector v);
Vector vec_normalize(Vector v);
