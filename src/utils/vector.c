#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

long double vec_len(Vector v)
{
    return sqrtl(v.x * v.x + v.y * v.y);
}

long double vec_dot(Vector v1, Vector v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}

Vector vec_add(Vector v1, Vector v2)
{
    return (Vector){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
    };
}

Vector vec_sub(Vector v1, Vector v2)
{
    return (Vector){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
    };
}

Vector vec_mul_scalar(long double k, Vector v)
{
    return (Vector){
        .x = k * v.x,
        .y = k * v.y,
    };
}

Vector vec_normalize(Vector v)
{
    // Handle zero vectors.
    if (vec_len(v) == 0) return vec_mul_scalar(0, v);

    return vec_mul_scalar(1 / vec_len(v), v);
}
