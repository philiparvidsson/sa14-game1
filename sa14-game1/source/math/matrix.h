/*------------------------------------------------------------------------------
 * File: matrix.h
 * Created: June 17, 2015
 * Last changed: July 16, 2015
 *
 * Author(s): Philip Arvidsson (contact@philiparvidsson.com)
 *
 * Description:
 *   Matrix types, macros, functions etc. The matrices are stored in row-major
 *   order, i.e. ((mat4x4)mat).m[1][3] referes to the fourth element on the
 *   second row.
 *----------------------------------------------------------------------------*/

// @To-do: Implement using SSE/SIMD.

#ifndef matrix_h_
#define matrix_h_

/*------------------------------------------------
 * INCLUDES
 *----------------------------------------------*/

#include "base/common.h"
#include "math/vector.h"

#include <math.h>
#include <string.h>

/*------------------------------------------------
 * MACROS
 *----------------------------------------------*/

#define mat_n(v) (((vec_n(v))==16) ? ( 4) \
               : (((vec_n(v))== 9) ? ( 3) \
               : (((vec_n(v))== 4) ? ( 2) \
               :                     (-1))))

#define mat_identity(r) mat_identity_((float*)r, mat_n(*r))

#define mat_transpose(m) mat_transpose_((float*)m, mat_n(*m))

#define mat_transl_xy(x, y, r) mat_transl_xy_(x, y, (float*)r, mat_n(*r))
#define mat_transl_xyz(x, y, z, r) mat_transl_xyz_(x, y, z, (float*)r, mat_n(*r))

#define mat_rot(u, a, r) mat_rot_((float*)u, a, (float*)r, mat_n(*r))
#define mat_rot_x(a, r) mat_rot_x_(a, (float*)r, mat_n(*r))
#define mat_rot_y(a, r) mat_rot_y_(a, (float*)r, mat_n(*r))
#define mat_rot_z(a, r) mat_rot_z_(a, (float*)r, mat_n(*r))

#define mat_scale(f, r) mat_scale_(f, (float*)r, mat_n(*r))
#define mat_scale_xy(f, r) mat_scale_xy_(x, y, (float*)r, mat_n(*r))
#define mat_scale_xyz(f, r) mat_scale_xyz_(x, y, (float*)r, mat_n(*r))

#define mat_add(a, b, r) \
    mat_add_((float*)a, (float*)b, (float*)r, mat_n(*a), mat_n(*b), mat_n(*r))

#define mat_sub(a, b, r) \
    mat_sub_((float*)a, (float*)b, (float*)r, mat_n(*a), mat_n(*b), mat_n(*r))

#define mat_mul(a, b, r) \
    mat_mul_((float*)a, (float*)b, (float*)r, mat_n(*a), mat_n(*b), mat_n(*r))

#define mat_div(a, b, r) \
    mat_div_((float*)a, (float*)b, (float*)r, mat_n(*a), mat_n(*b), mat_n(*r))

#define vec_mat_mul(v, m, r) \
    vec_mat_mul_((float*)v, (float*)m, (float*)r, vec_n(*v), mat_n(*m), vec_n(*r))


/*------------------------------------------------
 * TYPES
 *----------------------------------------------*/

typedef union {
    float value[4];
    float m[2][2];
    vec2 row[2];
    struct { vec2 x; vec2 y; };
} mat2x2;

typedef union {
    float value[9];
    float m[3][3];
    vec3 row[3];
    struct { vec3 x; vec3 y; vec3 z; };
} mat3x3;

typedef union {
    float value[16];
    float m[4][4];
    vec4 row[4];
    struct { vec4 x; vec4 y; vec4 z; vec4 w; };
} mat4x4;

/*------------------------------------------------
 * FUNCTIONS
 *----------------------------------------------*/

static inline void mat_identity_(float* r, int n) {
    memset(r, 0, sizeof(float)*n*n);
    for (int i = 0; i < n; i++)
        r[i*n+i] = 1.0f;
}

static inline void mat_transpose_(float* m, int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = (i+1); j < n; j++) {
            float tmp      = m[i*n+j];
                  m[i*n+j] = m[i+j*n];
                  m[i+j*n] = tmp;
        }
    }
}

static inline void mat_transl_xy_(float x, float y, float* m, int n) {
    if (n < 3) error("matrix is too small");

    mat_identity_(m, n);

    m[  n-1] = x;
    m[2*n-1] = y;
}

static inline void mat_transl_xyz_(float x, float y, float z, float* m, int n) {
    if (n < 4) error("matrix is too small");

    mat_identity_(m, n);

    m[  n-1] = x;
    m[2*n-1] = y;
    m[3*n-1] = z;
}

static inline void mat_rot_(const vec3* u, float a, float* m, int n) {
    if (n < 3) error("matrix is too small");

    mat_identity_(m, n);

    float cos_a = cosf(a), sin_a = sinf(a);
    float one_minus_cos_a = 1.0f - cos_a;
    float x = u->x, y = u->y, z = u->z;

    m[0    ] = cos_a + x*x*one_minus_cos_a;
    m[1    ] = x*y*one_minus_cos_a - z*sin_a;
    m[2    ] = x*z*one_minus_cos_a + y*sin_a;
    m[n    ] = y*x*one_minus_cos_a + z*sin_a;
    m[n+1  ] = cos_a + y*y*one_minus_cos_a;
    m[n+2  ] = y*z*one_minus_cos_a - x*sin_a;
    m[2*n  ] = z*x*one_minus_cos_a - y*sin_a;
    m[2*n+1] = z*y*one_minus_cos_a + x*sin_a;
    m[2*n+2] = cos_a + z*z*one_minus_cos_a;
}

static inline void mat_rot_x_(float a, float* m, int n) {
    if (n < 3) error("matrix is too small");

    mat_identity_(m, n);

    float cos_a = cosf(a), sin_a = sinf(a);

    m[n  +1] =  cos_a;
    m[n  +2] = -sin_a;
    m[2*n+1] =  sin_a;
    m[2*n+2] =  cos_a;
}

static inline void mat_rot_y_(float a, float* m, int n) {
    if (n < 3) error("matrix is too small");

    mat_identity_(m, n);

    float cos_a = cosf(a), sin_a = sinf(a);

    m[    0] =  cos_a;
    m[    2] =  sin_a;
    m[2*n  ] = -sin_a;
    m[2*n+2] =  cos_a;
}

static inline void mat_rot_z_(float a, float* m, int n) {
    mat_identity_(m, n);

    float cos_a = cosf(a);
    float sin_a = sinf(a);

    m[  0] =  cos_a;
    m[  1] = -sin_a;
    m[n  ] =  sin_a;
    m[n+1] =  cos_a;
}

static inline void mat_scale_(float f, float* m, int n) {
    mat_identity_(m, n);

    for (int i = 0; i < n; i++)
        m[i+i*n] = f;
}

static inline void mat_scale_xy_(float x, float y, float* m, int n) {
    mat_identity_(m, n);

    m[0  ] = x;
    m[n+1] = y;
}

static inline void mat_scale_xyz_(float x, float y, float z, float* m, int n) {
    mat_identity_(m, n);

    m[0    ] = x;
    m[n+1  ] = y;
    m[2*n+2] = z;
}

static inline void mat_add_(const float* a, const float* b, float* r, int a_n, int b_n, int r_n) {
    for (int i = 0; i < min(a_n, min(b_n, r_n)); i++) {
        for (int j = 0; j < min(a_n, min(b_n, r_n)); j++)
            r[i*r_n+j] = a[i*a_n+j] + b[i*b_n+j];
    }
}

static inline void mat_sub_(const float* a, const float* b, float* r, int a_n, int b_n, int r_n) {
    for (int i = 0; i < min(a_n, min(b_n, r_n)); i++) {
        for (int j = 0; j < min(a_n, min(b_n, r_n)); j++)
            r[i*r_n+j] = a[i*a_n+j] - b[i*b_n+j];
    }
}

static inline void mat_mul_(const float* a, const float* b, float* r, int a_n, int b_n, int r_n) {
    int const  n = min(a_n, min(b_n, r_n));
    mat4x4     t;
    float     *m = (r==a || r==b) ? (float*)&t : r;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float d = 0.0f;

            for (int k = 0; k < n; k++)
                d += a[i*a_n+k] * b[j+k*b_n];

            m[i*r_n+j] = d;
        }
    }

    if (m == (float*)&t) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++)
                r[i*r_n+j] = m[i*r_n+j];
        }
    }
}

static inline void mat_div_(const float* a, const float* b, float* r, int a_n, int b_n, int r_n) {
    for (int i = 0; i < min(a_n, min(b_n, r_n)); i++) {
        for (int j = 0; j < min(a_n, min(b_n, r_n)); j++)
            r[i*r_n+j] = a[i*a_n+j] / b[i*b_n+j];
    }
}

static inline void vec_mat_mul_(const float* v, const float* m, float* r, int v_n, int m_n, int r_n) {
    for (int i = 0; i < min(v_n, min(m_n, r_n)); i++) {
        r[i] = 0.0f;
        for (int j = 0; j < min(v_n, min(m_n, r_n)); j++)
            r[i] += v[j] * m[i*m_n+j];
    }
}

static inline void mat4x4_look_at(const vec3* pos, const vec3* at,
                                  const vec3* up, mat4x4 *r)
{
    vec3 x_axis, y_axis, z_axis;

    // First, we create a z-axis from the target to the camera position.
    vec_sub(pos, at, &z_axis);
    vec_normalize(&z_axis, &z_axis);

    // The x-axis is the cross product of the up-axis and the z-axis.
    vec3_cross(up, &z_axis, &x_axis);
    vec_normalize(&x_axis, &x_axis);

    // The y-axis is the cross product of the x- and z-axes.
    vec3_cross(&z_axis, &x_axis, &y_axis);

    // The rotation matrix.
    mat4x4 rot = {
         x_axis.x, x_axis.y, x_axis.z, 0.0f,
         y_axis.x, y_axis.y, y_axis.z, 0.0f,
         z_axis.x, z_axis.y, z_axis.z, 0.0f,
             0.0f,     0.0f,     0.0f, 1.0f
    };

    // The translation matrix.
    mat4x4 transl;
    mat_transl_xyz(-pos->x, -pos->y, -pos->z, &transl);

    // We want to rotate around the camera position, so we translate before
    // rotating here.
    mat_identity(r);
    mat_mul(&transl, r, r);
    mat_mul(&rot,    r, r);
}

static inline void mat4x4_ortho(float left, float right, float bottom,
                                float top, float far, float near, mat4x4 *res)
{
    float l=left, r=right, b=bottom, t=top, n=near, f=far;

    *res = (mat4x4) {
        2.0f/(r-l), 0.0f,       0.0f,       -(r+l)/(r-l),
        0.0f,       2.0f/(t-b), 0.0f,       -(t+b)/(t-b),
        0.0f,       0.0f,       2.0f/(n-f), -(n+f)/(n-f),
        0.0f,       0.0f,       0.0f,        1.0f
    };
}

static inline void mat4x4_persp(float left, float right, float bottom,
                                        float top, float far, float near, mat4x4 *res)
{
    float l=left, r=right, b=bottom, t=top, n=near, f=far;

    *res = (mat4x4) {
        -(2.0f*f)/(r-l),  0.0f,            (r+l)/(r-l), 0.0f,
         0.0f,           -(2.0f*f)/(t-b),  (t+b)/(t-b), 0.0f,
         0.0f,            0.0f,            (f+n)/(f-n), (-2.0f*f*n)/(f-n),
         0.0f,            0.0f,           -1.0f,        0.0f
    };
}

#endif // matrix_h_
