#pragma once
#include <DirectXMath.h>

#include <array>
#include <cmath>
#include <vector>

using namespace DirectX;



////////////////////////////////////////////////////
// TODO:
//  * move to rch-utils lib
////////////////////////////////////////////////////

namespace rch {

//--------------------------------------------------------------------------
// Basic Structs
//--------------------------------------------------------------------------

struct Vec3 {
  float x, y, z;

  Vec3() : x(0), y(0), z(0){};
  Vec3(float x1, float y1, float z1) : x(x1), y(y1), z(z1){};
  Vec3(float comVal) : x(comVal), y(comVal), z(comVal){};
  Vec3(Vec3&&) noexcept = default;
  Vec3& operator=(Vec3&& rv) noexcept = default;
  Vec3(const Vec3& orig) = default;
  Vec3& operator=(const Vec3& orig) = default;

  Vec3& operator+=(const Vec3& v2) {
    this->x = this->x + v2.x;
    this->y = this->y + v2.y;
    this->z = this->z + v2.z;
    return *this;
  };

  friend Vec3 operator+(Vec3 v1, const Vec3& v2) {
    Vec3 res;
    res.x = v1.x + v2.x;
    res.y = v1.y + v2.y;
    res.z = v1.z + v2.z;
    return res;
  };

  Vec3& operator-=(const Vec3& v2) {
    this->x = this->x - v2.x;
    this->y = this->y - v2.y;
    this->z = this->z - v2.z;
    return *this;
  };

  friend Vec3 operator-(Vec3 v1, const Vec3& v2) {
    Vec3 res;
    res.x = v1.x - v2.x;
    res.y = v1.y - v2.y;
    res.z = v1.z - v2.z;
    return res;
  };

  Vec3& operator*=(const float scalar) {
    this->x = this->x * scalar;
    this->y = this->y * scalar;
    this->z = this->z * scalar;
    return *this;
  };

  friend Vec3 operator*(const float scalar, Vec3 v1) {
    Vec3 res;
    res.x = v1.x * scalar;
    res.y = v1.y * scalar;
    res.z = v1.z * scalar;
    return res;
  };

  friend Vec3 operator*(Vec3 v1, const float scalar) {
    Vec3 res;
    res.x = v1.x * scalar;
    res.y = v1.y * scalar;
    res.z = v1.z * scalar;
    return res;
  };

  friend Vec3 operator/(Vec3 v1, const float scalar) {
    if (scalar == 0) {
      throw "[Vec3, op: '/'] Division by 0";
    }
    Vec3 res;
    res.x = v1.x / scalar;
    res.y = v1.y / scalar;
    res.z = v1.z / scalar;
    return res;
  };

  
  friend bool operator==(Vec3 v1, const Vec3& v2) {
    if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z) {
      return true;
    }
    return false;
  };

  // TODO: Fairly experimental
  float& operator[](int idx) {
    if (idx == 0) { return this->x; }
    else if (idx == 1) { return this->y; }
    else if (idx == 2) { return this->z; }
    else { throw "Vec3::operator[]: index out of range!"; };
  }

  // TODO: Fairly experimental
  float operator[](int idx) const {
    if (idx == 0) { return this->x; }
    else if (idx == 1) { return this->y; }
    else if (idx == 2) { return this->z; }
    else { throw "Vec3::operator[]: index out of range!"; };
  }

  /************************** General functions *************************/

  void addToXYZ(float scalar) {
    this->x = this->x + scalar;
    this->y = this->y + scalar;
    this->z = this->z + scalar;
  };

  template<typename Func>
  void applyToXYZ(Func fu) {
    this->x = fu(this->x);
    this->y = fu(this->y);
    this->z = fu(this->z);
  }

  /* Which version is better? */
  //static float dotProduct(const Vec3 v1, const Vec3 v2) { 
  //  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z); 
  //}

  static float dotProduct(const Vec3& v1, const Vec3& v2) {
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
  };

  static float distance(Vec3 v1, Vec3 v2) {
    Vec3 diff = v2 - v1;
    return std::sqrt(Vec3::dotProduct(diff, diff));
  };

  // Warning! Division by zero is allowed here in order
  // not to produce additional overhead.
  void normalize() {
    float len = Vec3::length(*this);
    assert(len != 0);
    (*this).x = (*this).x / len;
    (*this).y = (*this).y / len;
    (*this).z = (*this).z / len;
  };

  // Warning! Division by zero is allowed here in order 
  // not to produce additional overhead.
  static Vec3 getNormalized(const Vec3& vec){
    float len = Vec3::length(vec);
    assert(len != 0);
    return Vec3(vec.x / len, vec.y / len, vec.z / len);
  };

  static float length(const Vec3& vec) {
    return std::sqrt(Vec3::dotProduct(vec, vec));
  };

  // TODO skretnosc?
  static Vec3 crossProduct(const Vec3& u, const Vec3& v) {
    return {u.y * v.z - u.z * v.y, u.x * v.z - u.z * v.x,
            u.x * v.y - u.y * v.x};
  };

  /************************** DirectX-specific **************************/

  static XMFLOAT3 XMF3(const Vec3& v) {
    return {v.x, v.y, v.z};
  }

  static XMVECTOR XMV4_v0(const Vec3& v) {
    XMFLOAT4 aux = {v.x, v.y, v.z, 0};
    return XMLoadFloat4(&aux);
  }

  static XMVECTOR XMV4_v1(const Vec3& v) {
    XMFLOAT4 aux = {v.x, v.y, v.z, 1};
    return XMLoadFloat4(&aux);
  }
  
  static XMVECTOR XMV4_0v(const Vec3& v) {
    XMFLOAT4 aux = {0, v.x, v.y, v.z};
    return XMLoadFloat4(&aux);
  }

  static Vec3 toVec3(const XMVECTOR& v) {
    XMFLOAT4 aux;
    XMStoreFloat4(&aux, v);
    return {aux.x, aux.y, aux.z};
  }

  void copyValXYZ(const DirectX::XMVECTOR& v) {
    DirectX::XMFLOAT3 aux;
    DirectX::XMStoreFloat3(&aux, v);
    this->x = aux.x;
    this->y = aux.y;
    this->z = aux.z;
  };
};

// PTODO: Come up with a better name...
struct Uint64_2 {
  Uint64_2(uint64_t newX, uint64_t newY) : x(newX), y(newY){};
  uint64_t x;
  uint64_t y;
};

//--------------------------------------------------------------------------
// Trigonometric functions
//--------------------------------------------------------------------------

/**
 * Set of functions that: 
 *  - use ctg(x) = cos(x)/sin(x) to calculate it's value;
 *  - throw when 'angle' = 0 since it's undefined.
 *
 * ctg_fast() uses DirectXMath to obtain sincos() (not implemented on Windows)
 */
double ctg(double angle);
float ctg(float angle);
float ctg_fast(float angle); 


//--------------------------------------------------------------------------
// Functions for common matrices used in 3D graphics.
//  * matrices are in (R^4)x(R^4) and allow for affine transformations
//  * DX is using left handed coordinate system and XMMATRIX is row-major,
//    thus we fall these conventions
//  * angles are in radians
//  
// [DirectX implementation]
//--------------------------------------------------------------------------

XMMATRIX gen_id_mtx();
XMMATRIX gen_translation_mtx(float vx, float vy, float vz);
XMMATRIX gen_scale_mtx(float vx, float vy, float vz);
XMMATRIX gen_rotx_mtx(float angle);
XMMATRIX gen_roty_mtx(float angle);
XMMATRIX gen_rotz_mtx(float angle);
XMMATRIX gen_perspective_fov_mtx(float fov, float aspect, float near, float far);

//--------------------------------------------------------------------------
// Functions related to:
//    [TODO: not sure what... Linear Algebra?]
//
// [DirectX implementation]
//--------------------------------------------------------------------------

void solve3DiagLinEqs_Reuse(std::vector<float>& a, std::vector<float>& b,
                      std::vector<float>& c, std::vector<Vec3>& d, int n);

std::vector<Vec3> solve3DiagLinEqs(const std::vector<float>& a,
                                   const std::vector<float>& b,
                                   const std::vector<float>& c,
                                   const std::vector<Vec3>& d);

//--------------------------------------------------------------------------
// General Functions (WIP)
// TODO: sort these functions into specific sections
//
// [DirectX implementation]
//--------------------------------------------------------------------------

// Returns std::powf(base, 2)
auto fpow2 = [](float base) -> float { return std::powf(base, 2); };
// Returns std::powf(base, 3)
auto fpow3 = [](float base) -> float { return std::powf(base, 3); };

/**
 * Linear Interpolation between "a" and "b"
 * using parameter "t" which has to be in [0, 1]
 */
template <typename T, typename RealNum>
T lerp(const T& a, const T& b, const RealNum& t) {
  assert(t >= 0.0f && t <= 1.0f);
  return a + ((b - a) * t);
}

/**
 * Calculates what fraction of segment [a, b] is "val".
 * Returns ((val - a) / (b - a)) which should be in [0, 1]
 * (by presumption Num is a type with Reals' arithmetics)
 */
template <typename Num>
Num fracOfSegm(Num a, Num b, Num val) {
  assert(a <= b && val >= a && val <= b);
  // if ((a <= b && val >= a && val <= b) == false) {
  //  int tst = 1;
  //}
  if (a == b) {
    return 1;
  }
  return ((val - a) / (b - a));
}

/**
 * Version with clamping that allows for "val" to be
 * outside of the [a, b]. Returns:
 *    0, val < a
 *    1, val > b
 *    fracOfSegm, OTH
 */
template <typename Num>
Num fracOfSegmCl(Num a, Num b, Num val) {
  assert(a <= b);
  if (a == b || val > b) {
    return 1;
  } else if (val < a) {
    return 0;
  }

  return ((val - a) / (b - a));
}

bool checkRaySphereIntersection(DirectX::XMFLOAT3 rayOrigin,
                                DirectX::XMFLOAT3 rayDirection, float radius);

bool solveRaySphereIntersection(DirectX::XMFLOAT3 rayOrigin,
                                DirectX::XMFLOAT3 rayDirection, float radius,
                                float* sol1, float* sol2);

double calcSimplePolygonArea(std::vector<Vec3>& pts);

XMMATRIX quaternionToMtx(XMVECTOR qnion);

// TODO move
// TODO mention the author or make your own version
static XMVECTOR get_rotation_between(const Vec3& u, const Vec3& v) {
  // It is important that the inputs are of equal length when
  // calculating the half-way vector.
  auto uN = Vec3::getNormalized(u);
  auto vN = Vec3::getNormalized(v);

      //assert(uN == -1.0f * vN);
  //// Unfortunately, we have to check for when u == -v, as u + v
  //// in this case will be (0, 0, 0), which cannot be normalized.
  if (uN == -1.0f * vN) {
    // TODO
    // 180 degree rotation around any orthogonal vector
    //return Quaternion(0, normalized(orthogonal(u)));
    throw "[get_rotation_between] Corner case";
  }

  Vec3 half = Vec3::getNormalized(uN + vN);
  Vec3 vecPart = Vec3::crossProduct(uN, half);
  XMFLOAT4 resF4 = {vecPart.x, vecPart.y, vecPart.z,
                    Vec3::dotProduct(uN, half)};
  return XMLoadFloat4(&resF4);
}

//--------------------------------------------------------------------------
// Functions related to:
//    Geometry (WIP)
//
// [Generic implementation]
//--------------------------------------------------------------------------

template <typename T>
inline bool inCircle(T x, T y, T radius, T middleX = 0, T middleY = 0) {
  return (rch::fpow2(x - middleX) + rch::fpow2(y - middleY) <=
          rch::fpow2(radius));
}

//--------------------------------------------------------------------------
// Functions related to:
//    3D Curves, Bernstein Basis, etc (WIP)
//
// [DirectX implementation]
//--------------------------------------------------------------------------

/**
 * Converts coefficients a_i, i in [0, 3] of a monomial:
 *    P(t) = a_i * t^i,
 * to coefficients of P(t) in Bernsteins basis:
 *    P(t) = b^i * B^3_i(t)
 * 
 * Order of a_i -s is important, i.e. passing a_(3-i) 
 * will NOT result in returning b_(3-i).
 * The algorithm is more efficient than convertion using 
 * inversed Mtx (M_e(B) ^-1).
 * (Based on K. Marciniak's MG1 Lecture 2, p. 6/22)
 */
template<typename CoordT>
std::array<CoordT, 4> monomToBernBasis3(const std::array<CoordT, 4> a) {
  std::array<CoordT, 4> res;
  const CoordT common = (a[1] / 3) + (a[2] / 3);
  res[0] = a[0];
  res[1] = res[0] + (a[1] / 3);
  res[2] = res[1] + common;
  res[3] = res[2] + common + (a[2] / 3) + a[3];
  return res; // RVO
};

/**
 * Calculates a value of a polynomial in Bernstein basis (degree = 3)
 * for parameter "t", i.e. returns w(t) (= b[i] * BernBasis3^i).
 * 
 * "t" - in principle should be in [0, 1]
 * "TVec" - vector type, @see lerp<>() for requirements
 * "TRealNum" - Any real number
 */
template <typename TVec, typename TRealNum>
TVec deCasteljauBern3(const std::array<TVec, 4>& b, TRealNum t) {
  const TVec common = lerp(b[1], b[2], t);

  const TVec leftQuadratic = lerp(lerp(b[0], b[1], t), common, t);
  const TVec rightQuadratic = lerp(common, lerp(b[2], b[3], t), t);

  return lerp(leftQuadratic, rightQuadratic, t);
}

/* 
     PTODO: use std::array for calcBernsteinXValue() params.
*/

std::array<float, 4> calcBernstein3Basis(float t);
XMFLOAT4 calcBernstein3Basis_DX(float t);
XMFLOAT3 calcBernstein3Value(Vec3 pt1, Vec3 pt2, Vec3 pt3, Vec3 pt4, float t);
XMFLOAT3 calcBernstein2Basis(float t);
XMFLOAT3 calcBernstein2Value(Vec3 pt1, Vec3 pt2, Vec3 pt3, float t);


//--------------------------------------------------------------------------
// General Classes (WIP)
// TODO: sort these into specific sections
//
// [DirectX implementation]
//--------------------------------------------------------------------------

// TODO: naming conventions, functions' access,
//       templeted getConvexHull<algo>(.), etc.
struct ConvexHull2dCalc {
  ConvexHull2dCalc()  = default;
  ~ConvexHull2dCalc() = default;

 
  // 'Less' comparator that creates order: bottom-left to top-right
  // If point1.x == point2.x, compare Y coord.
  static bool lessX_Y(const Vec3& a, const Vec3& b) {
    return ((a.x < b.x) || (a.x == b.x && a.y < b.y));
  }

  /**
   * Set of utility functions for checking whether given three 2D points
   * are: making counter-/clockwise turn OR are collinear.
   * 
   * Since these points are in XY-plane, the Z-value of their ((a,b) 
   * & (a,c) -vectors) cross product is being used to determine the turn.
   */
  static bool ccw(const Vec3& a, const Vec3& b, const Vec3& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0;
  }
  static bool cw(const Vec3& a, const Vec3& b, const Vec3& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) < 0;
  }
  static bool collinear(const Vec3& a, const Vec3& b, const Vec3& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) == 0;
  }

  // The main convex hull functions
  static bool toConvexHull(std::vector<rch::Vec3>& pts);
  //std::vector<rch::Vec3> convexHullOf(std::vector<rch::Vec3>& pts);
};

} // end namespace rch

