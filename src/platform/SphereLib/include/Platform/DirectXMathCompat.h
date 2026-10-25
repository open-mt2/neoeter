#pragma once

#include "Platform.h"
#include <cmath>

#if defined(_WIN32) && !defined(USE_PLATFORM_D3D_COMPAT)
    #include <d3d9.h>
    #include <directxsdk/d3dx9.h>
#else

// -------------------------------------------------------------
// D3D Basic Types
// -------------------------------------------------------------
typedef DWORD D3DCOLOR;

#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

#ifndef D3DCOLOR_RGBA
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#endif

#ifndef D3DCOLOR_XRGB
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)
#endif

struct D3DVECTOR {
    float x;
    float y;
    float z;
};

struct D3DMATRIX {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

// -------------------------------------------------------------
// D3DX Math Structures
// -------------------------------------------------------------
struct D3DXVECTOR2 {
    float x, y;

    D3DXVECTOR2() : x(0.0f), y(0.0f) {}
    D3DXVECTOR2(float _x, float _y) : x(_x), y(_y) {}
};

struct D3DXVECTOR3 : public D3DVECTOR {
    D3DXVECTOR3() { x = 0.0f; y = 0.0f; z = 0.0f; }
    D3DXVECTOR3(const float* pf) { x = pf[0]; y = pf[1]; z = pf[2]; }
    D3DXVECTOR3(const D3DVECTOR& v) { x = v.x; y = v.y; z = v.z; }
    D3DXVECTOR3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

    operator float* () { return (float*)&x; }
    operator const float* () const { return (const float*)&x; }

    D3DXVECTOR3& operator += (const D3DXVECTOR3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    D3DXVECTOR3& operator -= (const D3DXVECTOR3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    D3DXVECTOR3& operator *= (float f) { x *= f; y *= f; z *= f; return *this; }
    D3DXVECTOR3& operator /= (float f) { float inv = 1.0f / f; x *= inv; y *= inv; z *= inv; return *this; }

    D3DXVECTOR3 operator + () const { return *this; }
    D3DXVECTOR3 operator - () const { return D3DXVECTOR3(-x, -y, -z); }

    D3DXVECTOR3 operator + (const D3DXVECTOR3& v) const { return D3DXVECTOR3(x + v.x, y + v.y, z + v.z); }
    D3DXVECTOR3 operator - (const D3DXVECTOR3& v) const { return D3DXVECTOR3(x - v.x, y - v.y, z - v.z); }
    D3DXVECTOR3 operator * (float f) const { return D3DXVECTOR3(x * f, y * f, z * f); }
    D3DXVECTOR3 operator / (float f) const { float inv = 1.0f / f; return D3DXVECTOR3(x * inv, y * inv, z * inv); }

    friend D3DXVECTOR3 operator * (float f, const D3DXVECTOR3& v) { return D3DXVECTOR3(f * v.x, f * v.y, f * v.z); }

    bool operator == (const D3DXVECTOR3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator != (const D3DXVECTOR3& v) const { return x != v.x || y != v.y || z != v.z; }
};

struct D3DXVECTOR4 {
    float x, y, z, w;

    D3DXVECTOR4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    D3DXVECTOR4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

struct D3DXMATRIX : public D3DMATRIX {
    D3DXMATRIX() {
        _11 = 1.0f; _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
        _21 = 0.0f; _22 = 1.0f; _23 = 0.0f; _24 = 0.0f;
        _31 = 0.0f; _32 = 0.0f; _33 = 1.0f; _34 = 0.0f;
        _41 = 0.0f; _42 = 0.0f; _43 = 0.0f; _44 = 1.0f;
    }
    D3DXMATRIX(const D3DMATRIX& m) { *static_cast<D3DMATRIX*>(this) = m; }
    D3DXMATRIX(float f11, float f12, float f13, float f14,
               float f21, float f22, float f23, float f24,
               float f31, float f32, float f33, float f34,
               float f41, float f42, float f43, float f44) {
        _11 = f11; _12 = f12; _13 = f13; _14 = f14;
        _21 = f21; _22 = f22; _23 = f23; _24 = f24;
        _31 = f31; _32 = f32; _33 = f33; _34 = f34;
        _41 = f41; _42 = f42; _43 = f43; _44 = f44;
    }

    float& operator () (unsigned int Row, unsigned int Col) { return m[Row][Col]; }
    float  operator () (unsigned int Row, unsigned int Col) const { return m[Row][Col]; }
};

struct D3DXPLANE {
    float a, b, c, d;

    D3DXPLANE() : a(0.0f), b(0.0f), c(0.0f), d(0.0f) {}
    D3DXPLANE(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) {}
};

// -------------------------------------------------------------
// D3DX Math Inlines
// -------------------------------------------------------------
inline float D3DXVec3LengthSq(const D3DXVECTOR3* pV) {
    if (!pV) return 0.0f;
    return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
}

inline float D3DXVec3Length(const D3DXVECTOR3* pV) {
    return std::sqrt(D3DXVec3LengthSq(pV));
}

inline float D3DXPlaneDotCoord(const D3DXPLANE* pP, const D3DXVECTOR3* pV) {
    if (!pP || !pV) return 0.0f;
    return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z + pP->d;
}

inline D3DXPLANE* D3DXPlaneNormalize(D3DXPLANE* pOut, const D3DXPLANE* pP) {
    if (!pOut || !pP) return nullptr;
    float norm = std::sqrt(pP->a * pP->a + pP->b * pP->b + pP->c * pP->c);
    if (norm > 0.0f) {
        float inv = 1.0f / norm;
        pOut->a = pP->a * inv;
        pOut->b = pP->b * inv;
        pOut->c = pP->c * inv;
        pOut->d = pP->d * inv;
    } else {
        pOut->a = 0.0f;
        pOut->b = 0.0f;
        pOut->c = 0.0f;
        pOut->d = 0.0f;
    }
    return pOut;
}

#endif
