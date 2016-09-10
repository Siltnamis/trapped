#ifndef AV_MATH_HEADER
#define AV_MATH_HEADER

#include <math.h>

#define PI 3.14159265358979323846264338327950288f

union vec2
{
    struct { float x, y; };
    struct { float s, t; };
    float e[2];
};

union vec3
{
    struct { float x, y, z; };
    struct { float r, g, b; };
    struct { float s, t, p; };
    vec2 xy;
    float e[3];
};

union vec4
{
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    struct { float s, t, p, q; };
    struct { vec2 xy, zw; };
    vec3 xyz;
    vec3 rgb;
    float e[4];
};

union mat2
{
    struct { vec2 x, y; };
    struct { float m00, m01, m10, m11; };
    vec2 col[2];
    float e[4];
};
inline mat2 mat2_identity()
{
    return {1.f, 0.f, 0.f, 1.f};
}

union mat3
{
    struct {vec3 x, y, z; };
    struct {
            float m00, m01, m02; // 1st column
            float m10, m11, m12; // 2nd column
            float m20, m21, m22; // 3rd column
            };
    vec3 col[3];
    float e[9];
};
inline mat3 mat3_identity()
{
    return {1, 0, 0, 0, 1, 0, 0, 0, 1};
}

union mat4
{
    struct { vec4 x, y, z, w; };
    struct {
            float m00, m01, m02, m03; // 1st column
            float m10, m11, m12, m13; // 2nd column
            float m20, m21, m22, m23; // 3rd column
            float m30, m31, m32, m33; // 4th column 
           };

    vec4 col[4];
    float e[16];
};
inline mat4 mat4_identity()
{
    mat4 result = {};
    result.m00 = 1;
    result.m11 = 1;
    result.m22 = 1;
    result.m33 = 1;
    return result;
}

//==========util==========
inline float radians(float angle)
{
    return angle*PI/180.f;
}

//==========Vec2 stuff==========
inline vec2 operator+(vec2 A, vec2 B)
{
    vec2 result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    return result;
}
inline vec2& operator+=(vec2& A, vec2 B)
{
    return (A = A + B);
}
inline vec2 operator-(vec2 A, vec2 B)
{
    vec2 result;
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    return result;
}
inline vec2& operator-=(vec2& A, vec2 B)
{
    return (A = A - B);
}
inline vec2 operator-(vec2 A)
{
    vec2 result;
    result.x = -A.x;
    result.y = -A.y;
    return result;
}
inline vec2 operator*(vec2 v, float scalar)
{
    vec2 result;
    result.x = v.x*scalar;
    result.y = v.y*scalar;
    return result;
}
inline vec2 operator*(float scalar, vec2 v)
{
    vec2 result;
    result.x = v.x*scalar;
    result.y = v.y*scalar;
    return result;
}
inline vec2& operator*=(vec2& v, float scalar)
{
    return ( v = v*scalar);
}
inline vec2 operator/(vec2 v, float scalar)
{
    vec2 result;
    result.x = v.x/scalar;
    result.y = v.y/scalar;
    return result;
}
inline vec2& operator/=(vec2& v, float scalar)
{
    return (v = v/scalar);
}

inline float dot(vec2 A, vec2 B)
{
    return A.x*B.x + A.y*B.y;
}

inline float length(vec2 v)
{
    return sqrtf(v.x*v.x + v.y*v.y);
}

inline vec2 normalize(vec2 v)
{
    float l = length(v);
    if(l > 0)
        return v/l;
    else
        return {0.f, 0.f};
}

//==========Vec3 stuff==========
inline vec3 operator+(vec3 A, vec3 B)
{
    vec3 result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    return result;
}
inline vec3& operator+=(vec3& A, vec3 B)
{
    return (A = A + B);
}
inline vec3 operator-(vec3 A, vec3 B)
{
    vec3 result;
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    return result;
}
inline vec3& operator-=(vec3& A, vec3 B)
{
    return (A = A - B);
}
inline vec3 operator-(vec3 v)
{
    vec3 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return result;
}
inline vec3 operator*(vec3 v, float scalar)
{
    vec3 result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    return result;
}
inline vec3 operator*(float scalar, vec3 v)
{
    vec3 result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    return result;
}
inline vec3& operator*=(vec3& v, float scalar)
{
    return (v = v*scalar);
}
inline vec3 operator/(vec3 v, float scalar)
{
    vec3 result;
    result.x = v.x / scalar;
    result.y = v.y / scalar;
    result.z = v.z / scalar;
    return result;
}
inline vec3& operator/=(vec3& v, float scalar)
{
    return (v = v/scalar);
}

inline float dot(vec3 A, vec3 B)
{
    return A.x*B.x + A.y*B.y + A.z*B.z; 
}

inline vec3 cross(vec3 A, vec3 B)
{
    vec3 result;
    result.x = A.y*B.z - A.z*B.y;
    result.y = A.z*B.x - A.x*B.z;
    result.z = A.x*B.y - A.y*B.x;
    return result;
}

inline float length(vec3 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline vec3 normalize(vec3 v)
{
    float l = length(v);
    if(l > 0)
        return v/l;
    else 
        return {0.f, 0.f, 0.f};
}

//==========Vec4 stuff==========
inline vec4 operator+(vec4 A, vec4 B)
{
    vec4 result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    result.w = A.w + B.w;
    return result;
}
inline vec4& operator+=(vec4& A, vec4 B)
{
    return (A = A + B);
}
inline vec4 operator-(vec4 A, vec4 B)
{
    vec4 result;
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    result.w = A.w - B.w;
    return result;
}
inline vec4& operator-=(vec4& A, vec4 B)
{
    return (A = A - B);
}
inline vec4 operator-(vec4 v)
{
    vec4 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    result.w = -v.w;
    return result;
}
inline vec4 operator*(vec4 v, float scalar)
{
    vec4 result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    result.w = v.w * scalar;
    return result;
}
inline vec4 operator*(float scalar, vec4 v)
{
    vec4 result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    result.w = v.w * scalar;
    return result;
}
inline vec4& operator*=(vec4& v, float scalar)
{
    return (v = v*scalar);
}
inline vec4 operator/(vec4 v, float scalar)
{
    vec4 result;
    result.x = v.x / scalar;
    result.y = v.y / scalar;
    result.z = v.z / scalar;
    result.w = v.w / scalar;
    return result;
}
inline vec4& operator/=(vec4& v, float scalar)
{
    return (v = v/scalar);
}

inline float dot(vec4 A, vec4 B)
{
    return A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
}

inline float length(vec4 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

inline vec4 normalize(vec4 v)
{
    float l = length(v);
    if(l > 0)
        return v/l;
    else 
        return {0.f, 0.f, 0.f, 0.f};
}

//==========mat2stuff==========
inline mat2 operator+(mat2 ml, mat2 mr)
{
    return  {
                ml.m00 + mr.m00, ml.m01 + mr.m01,
                ml.m10 + mr.m10, ml.m11 + mr.m11
            };
            
}
inline mat2& operator+=(mat2& ml, mat2 mr)
{
    return (ml = ml + mr);
}
inline mat2 operator-(mat2 ml, mat2 mr)
{
    return  {
                ml.m00 - mr.m00, ml.m01 - mr.m01,
                ml.m10 - mr.m10, ml.m11 - mr.m11
            };
}
inline mat2& operator-=(mat2& ml, mat2 mr)
{
    return (ml = ml - mr);
}

inline mat2 operator*(mat2 ml, mat2 mr)
{
#if 1
    mat2 result;
    for(int j = 0; j < 2; ++j){
        for(int i = 0; i < 2; ++i){
            result.e[j*2 + i] = ml.e[2*0 + i]*mr.e[j*2 + 0] + ml.e[2*1 +i]*mr.e[j*2 + 1];
        }
    }
    return result;
#else
    
#endif
}

inline mat2& operator*=(mat2& ml, mat2 mr)
{
    return (ml = ml * mr);
}

inline mat2 operator*(mat2 m, float scalar)
{
    return {m.e[0]*scalar, m.e[1]*scalar, m.e[2]*scalar, m.e[3]*scalar};
}
inline mat2& operator*=(mat2& m, float scalar)
{
    return (m = m*scalar);
}

inline vec2 operator*(mat2 m, vec2 v)
{
    return {m.m00*v.x + m.m10*v.y, m.m01*v.x + m.m11*v.y};
}

//==========mat3stuff==========
inline mat3 operator*(mat3 ml, mat3 mr)
{
    mat3 result;
    for(int j = 0; j < 3; ++j){
        for(int i = 0; i < 3; ++i){
            result.e[j*3 + i] =     ml.e[3*0 + i] * mr.e[j*3 + 0]
                                +   ml.e[3*1 + i] * mr.e[j*3 + 1]  
                                +   ml.e[3*2 + i] * mr.e[j*3 + 2];
        }
    }
    return result;
}

inline mat3& operator*=(mat3& ml, mat3 mr)
{
    return (ml = ml * mr);
}

inline vec3 operator*(mat3 m, vec3 v)
{
    return {
            m.m00*v.x + m.m10*v.y + m.m20*v.z, 
            m.m01*v.x + m.m11*v.y + m.m21*v.z,
            m.m02*v.x + m.m12*v.y + m.m22*v.z
            };
}

//==========mat4stuff==========
inline mat4 operator*(mat4 ml, mat4 mr)
{
    mat4 result;
    for(int j = 0; j < 4; ++j){
        for(int i = 0; i < 4; ++i){
            result.e[j*4 + i] =     ml.e[4*0 + i] * mr.e[j*4 + 0]
                                +   ml.e[4*1 + i] * mr.e[j*4 + 1]
                                +   ml.e[4*2 + i] * mr.e[j*4 + 2]
                                +   ml.e[4*3 + i] * mr.e[j*4 + 3];
        }
    }
    return result;
}
inline mat4& operator*=(mat4& ml, mat4 mr)
{
    return (ml = ml * mr);
}
inline vec4 operator*(mat4 m, vec4 v)
{
    vec4 result;
    result.x = m.m00*v.x + m.m01*v.y + m.m02*v.z + m.m03*v.w;
    result.y = m.m10*v.x + m.m11*v.y + m.m12*v.z + m.m13*v.w;
    result.z = m.m20*v.x + m.m21*v.y + m.m22*v.z + m.m23*v.w;
    result.w = m.m30*v.x + m.m31*v.y + m.m32*v.z + m.m33*v.w;
    return result;
}

//==========gl related mat stuff==========
inline mat4 look_at(vec3 eye, vec3 center, vec3 up)
{
    vec3 f, s, u;
    f = center - eye;
    f = normalize(f);
    s = cross(f, up);
    s = normalize(s);
    u = cross(s, f);

    mat4 result = mat4_identity();

    result.m00 = +s.x;
    result.m10 = +s.y;
    result.m20 = +s.z;

    result.m01 = +u.x;
    result.m11 = +u.y;
    result.m21 = +u.z;

    result.m02 = -f.x;
    result.m12 = -f.y;
    result.m22 = -f.z;

    result.m30 = -dot(s, eye);
    result.m31 = -dot(u, eye);
    result.m32 =  dot(f, eye);

    return result;
}

inline mat4 perspective(float fov, float aspect_ratio, float z_near, float z_far)
{
    mat4 result = {};
    float tan_half_fov = tanf(fov/2.f);
    
    result.m00 = 1.f / (aspect_ratio*tan_half_fov);
    result.m11 = 1.f / (tan_half_fov);
    result.m22 = -(z_far + z_near)/(z_far - z_near);
    result.m23 = -1.f;
    result.m32 = -2.f*z_far*z_near/(z_far - z_near);

    return result;
}

inline mat3 mat3_ortho(float left, float right, float bottom, float top)
{
    mat3 result = {};

    result.m00 = 2.f/(right - left);
    result.m11 = 2.f/(top - bottom);
    result.m22 = 1.f;

    result.m20 = -(right + left)/(right - left);
    result.m21 = -(top + bottom)/(top - bottom);
    //result.m20 = -0.5;

    return result;
}

inline mat4 ortho(float left, float right, float bottom, float top, float near, float far)
{
    mat4 result = {};
    result.m00 = 2.f/(right - left);
    result.m11 = 2.f/(top - bottom);
    result.m22 = -2.f/(far - near);
    result.m30 = -(right + left)/(right - left);
    result.m31 = -(top + bottom)/(top - bottom);
    result.m32 = -(far + near)/(far - near);
    result.m33 = 1.f;
    return result;
}
#endif //AV_MATH_HEADER
