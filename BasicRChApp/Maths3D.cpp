#include <cmath>
//#include <vector>
#include <algorithm>
#include "Maths3D.h"

namespace rch {

double ctg(double angle) { 
	if (angle == 0) {
		throw "[rch::ctg] ctg(0) is undefined";
	}
	double ctg_val = cos(angle) / sin(angle);
	return ctg_val;
}

float ctg(float angle) {
  if (angle == 0) {
    throw "[rch::ctg] ctg(0) is undefined";
  }
  float ctg_val = cos(angle) / sin(angle);
  return ctg_val;
}

// Uses DirectX's sincos() implementation to speed up
// ctg(x) = cos(x)/sin(x) calculations
float ctg_fast(float angle) { 
	if (angle == 0) {
		throw "[rch::ctg] ctg(0) is undefined";
	}
  float sin_val = 0.0, cos_val = 0.0;
  DirectX::XMScalarSinCos(&sin_val, &cos_val, angle);
	return (cos(angle) / sin(angle));
}

DirectX::XMMATRIX gen_id_mtx() { 
		return DirectX::XMMATRIX(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
}

DirectX::XMMATRIX gen_translation_mtx(float vx, float vy, float vz) {
	return DirectX::XMMATRIX(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		vx,   vy,   vz,   1.0f
	);
}

DirectX::XMMATRIX gen_scale_mtx(float vx, float vy, float vz) {
	return DirectX::XMMATRIX(
		vx,    0.0f,  0.0f,  0.0f,
		0.0f,  vy,    0.0f,  0.0f,
		0.0f,  0.0f,  vz,    0.0f,
		0.0f,  0.0f,  0.0f,  1.0f
	);
}

DirectX::XMMATRIX gen_rotx_mtx(float angle) { 
	return DirectX::XMMATRIX(
		1.0f,  0.0f,        0.0f,        0.0f,
		0.0f,  cos(angle),  sin(angle),  0.0f,
		0.0f,  -sin(angle), cos(angle),  0.0f,
		0.0f,  0.0f,        0.0f,        1.0f
	);
}

DirectX::XMMATRIX gen_roty_mtx(float angle) { 
	return DirectX::XMMATRIX(
		cos(angle), 0.0f,  -sin(angle), 0.0f,
		0.0f,       1.0f,  0.0f,        0.0f,
		sin(angle), 0.0f,  cos(angle),  0.0f,
		0.0f,       0.0f,  0.0f,        1.0f
	);
}

DirectX::XMMATRIX gen_rotz_mtx(float angle) { 
	return DirectX::XMMATRIX(
		cos(angle),   sin(angle), 0.0f,  0.0f,
		-sin(angle),  cos(angle), 0.0f,  0.0f,
		0.0f,         0.0f,       1.0f,  0.0f,
		0.0f,         0.0f,       0.0f,  1.0f
	);
}

// Returns "perspectiveFOV" matrix for left-handed system
XMMATRIX gen_perspective_fov_mtx(float fov, float aspect,
                                               float near, float far) { 
	float ctg_val = ctg(fov * 0.5f); 
	return DirectX::XMMATRIX(
			(ctg_val/aspect),  0.0f,          0.0f,                          0.0f,
			0.0f,              ctg_val,       0.0f,													 0.0f,
			0.0f,              0.0f,          ((far+near)/(far-near)),       1.0f,
			0.0f,              0.0f,          ((-2.0f*far*near)/(far-near)), 0.0f
	);
}

/**
 *	Returns whether the ray (origin, direction) intersects
 *  the given sphere, which is assumed to be at the (0,0,0),
 *	this the function requires the ray to be in object-space 
 *  or object being at (0,0,0).
 *
 *
 *	Raycasting is done by solving line-sphere intersection:
 *    Sphere(*):
 *      ||P - C||^2 = r^2 = dot(P-C, P-C) =(**) dot(P, P)
 *    Ray:
 *      Pt(t) = orig + (t * direction)
 *    Intersection:
 *      dot(orig + (t * direction), orig + (t * direction)) = r^2
 * =>
 *      t^2 * dot(dir, dir) 
 *				+ 2*t * dot(dir, org-C)
 *				+ dot(orig - C, orig - C) - r^2 = 0
 *
 *  (*) P it a point on the sphere (x, y, z)
 *  (**) cause the center point 'C' should be in (0,0,0)
 */
bool checkRaySphereIntersection(XMFLOAT3 rayOrigin,
                                XMFLOAT3 rayDirection, float radius) {
  /*** Get coefficients for the quadratic equation: ***/
  // t^2 * dot(dir, dir)
  float a = (rayDirection.x * rayDirection.x) +
            (rayDirection.y * rayDirection.y) +
            (rayDirection.z * rayDirection.z);

  // 2*t * dot(dir, org - C)
  float b =
      2.0f * ((rayDirection.x * rayOrigin.x) + (rayDirection.y * rayOrigin.y) +
              (rayDirection.z * rayOrigin.z));

  // dot(orig - C, orig - C) - r^2
  float c = ((rayOrigin.x * rayOrigin.x) + (rayOrigin.y * rayOrigin.y) +
             (rayOrigin.z * rayOrigin.z)) -
            (radius * radius);

  /** If the discriminant is negative there are no solutions for 't' ***/
  float discriminant = (b * b) - (4 * a * c);
  if (discriminant < 0.0f) {
    return false;
  }
  return true;
}

/**
 * Version of checkRaySphereIntersection(.) that:
 *   1) Returns false if the intersection didnt occur.
 *   2) Returns true OTH, and writes out the roots to 'sol1' and 'sol2'.
 *      If the discriminant = 0, then sol1 = sol2.
 *
 * PTODO: What if the a factor is = 0?
 */
bool solveRaySphereIntersection(XMFLOAT3 rayOrigin,
                                XMFLOAT3 rayDirection,
                                float radius, float* sol1, float* sol2) {
  /*** Get coefficients for the quadratic equation: ***/
  // t^2 * dot(dir, dir)
  float a = (rayDirection.x * rayDirection.x) +
            (rayDirection.y * rayDirection.y) +
            (rayDirection.z * rayDirection.z);

  // 2*t * dot(dir, org - C)
  float b =
      2.0f * ((rayDirection.x * rayOrigin.x) + (rayDirection.y * rayOrigin.y) +
              (rayDirection.z * rayOrigin.z));

  // dot(orig - C, orig - C) - r^2
  float c = ((rayOrigin.x * rayOrigin.x) + (rayOrigin.y * rayOrigin.y) +
             (rayOrigin.z * rayOrigin.z)) -
            (radius * radius);

  /****** Find the intersection points *******/
  float delta = (b * b) - (4 * a * c);
  if (delta < 0.0f) {
    return false;
  }
	
	const double sqrtDelta = std::sqrt(delta);
	if (delta == 0.0f) {
    *sol1 = static_cast<float>(-(b / (2.0 * a)));
    *sol2 = *sol1;
	} else {
    *sol1 = static_cast<float>((-b + sqrtDelta) / (2.0 * a));
    *sol2 = static_cast<float>((-b - sqrtDelta) / (2.0 * a));
	}

  return true;
}

// Calculates a SIGNED area assuming the pts define a simple polygon.
double calcSimplePolygonArea(std::vector<Vec3>& pts) { 
  auto ptsNum = pts.size();
  double sum = 0.0;
  for (auto i = 0; i < ptsNum; i++) {
    auto pt1 = pts.at(i);
    auto pt2 = pts.at((i + 1) % ptsNum);
    sum += ((pt1.x * pt2.y) - (pt2.x * pt1.y));
  }
  return 0.5 * (sum);
}

// z = {b, c, d, a}
XMMATRIX quaternionToMtx(XMVECTOR qnion) { 
  XMFLOAT4 aux;
  XMStoreFloat4(&aux, qnion);
  auto a = aux.w;
  auto b = aux.x;
  auto c = aux.y;
  auto d = aux.z;

  return DirectX::XMMATRIX(
		a*a + b*b - c*c - d*d, 2*b*c - 2*a*d,          2*b*d + 2*a*c, 0.0f,
		2*b*c+2*a*d,           a*a - b*b + c*c - d*d, 2*c*d-2*a*b, 0.0f,
		2*b*d - 2*a*c,  2*c*d + 2*a*b, a*a-b*b-c*c + d*d,  0.0f,
		0.0f,  0.0f,        0.0f,        1.0f
	);

}

std::array<float, 4> calcBernstein3Basis(float t) {
  const float invT = (1.0f - t);
  const float B3_0 = invT * invT * invT;
  const float B3_1 = 3.0f * invT * invT * t;
  const float B3_2 = 3.0f * invT * t * t;
  const float B3_3 = t * t * t;
  return {B3_0, B3_1, B3_2, B3_3};
}

// Overload that return XMFLOAT4 instead
XMFLOAT4 calcBernstein3Basis_DX(float t) {
  auto res = calcBernstein3Basis(t);
  return {res[0], res[1], res[2], res[3]};
}

XMFLOAT3 calcBernstein3Value(Vec3 pt1, Vec3 pt2, Vec3 pt3, Vec3 pt4, float t) {
  auto basis = calcBernstein3Basis_DX(t);

  float xValue =
      basis.x * pt1.x + basis.y * pt2.x + basis.z * pt3.x + basis.w * pt4.x;
  float yValue =
      basis.x * pt1.y + basis.y * pt2.y + basis.z * pt3.y + basis.w * pt4.y;
  float zValue =
      basis.x * pt1.z + basis.y * pt2.z + basis.z * pt3.z + basis.w * pt4.z;

  return XMFLOAT3(xValue, yValue, zValue);
}

XMFLOAT3 calcBernstein2Basis(float t) {
  const float invT = (1.0f - t);
  const float B3_0 = invT * invT;
  const float B3_1 = 2.0f * invT * t;
  const float B3_2 = t * t;
  return {B3_0, B3_1, B3_2};
}

XMFLOAT3 calcBernstein2Value(Vec3 pt1, Vec3 pt2, Vec3 pt3, float t) {
  auto basis = calcBernstein2Basis(t);

  float xValue = basis.x * pt1.x + basis.y * pt2.x + basis.z * pt3.x;
  float yValue = basis.x * pt1.y + basis.y * pt2.y + basis.z * pt3.y;
  float zValue = basis.x * pt1.z + basis.y * pt2.z + basis.z * pt3.z;

  return XMFLOAT3(xValue, yValue, zValue);
}

/**
 * Solves a set of "n" linear equation of tridiagonal matrix "n" x "n":
 *    |b0 c0 0 ||x0| |d0|
 *    |a1 b1 c1||x1|=|d1|
 *    |0  a2 b2||x2| |d2|
 *
 * This implementation reuses(!) both "c[]" and "d[]".
 * The result is returned through the vector "d[]".
 * 
 * Necessary conditions (not checked by the function):
 *  1) || bi || > || ai || + || ci ||
 *  2) Size of all vectors passed to the function is == "n"
 *
 * Complexity: O(n)
 * 
 * Based on Keivan Moradi's implementation from:
 * https://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm
 * -- viewed at 13.06.2020
 */
void solve3DiagLinEqs_Reuse(std::vector<float>& a, std::vector<float>& b,
                            std::vector<float>& c, std::vector<Vec3>& d, int n) {
  n = n - 1;  // since we start from x0 (not x1)
  c[0] = c[0] / b[0];
  d[0] = d[0] / b[0];

  for (int i = 1; i < n; i++) {
    c[i] /= b[i] - a[i] * c[i - 1];
    d[i] = (d[i] - a[i] * d[i - 1]) / (b[i] - a[i] * c[i - 1]);
  }

  d[n] = (d[n] - a[n] * d[n - 1]) / (b[n] - a[n] * c[n - 1]);

  for (int i = n; i-- > 0;) {
    d[i] = d[i] - (c[i] * d[i + 1]);
  }
}

// A version of solve3DiagLinEqs(.) that does NOT reuse the given matrices.
// See @solve3DiagLinEqs_Reuse() for details.
std::vector<Vec3> solve3DiagLinEqs(const std::vector<float>& a,
                                   const std::vector<float>& b,
                                   const std::vector<float>& c,
                                   const std::vector<Vec3>& d) {
  int n = d.size();
  assert((a.size() == n) && (b.size() == n) && (c.size() == n));
  
  std::vector<float> acc(n); // accumulator
  std::vector<Vec3> sol(n);  // stores the solution

  n = n - 1;  
  acc[0] = c[0] / b[0];
  sol[0] = d[0] / b[0];

  for (int i = 1; i < n; i++) {
    acc[i] = c[i] / (b[i] - a[i] * acc[i - 1]);
    sol[i] = (d[i] - a[i] * sol[i - 1]) / (b[i] - a[i] * acc[i - 1]);
  }

  sol[n] = (sol[n] - a[n] * sol[n - 1]) / (b[n] - a[n] * acc[n - 1]);

  for (int i = n; i-- > 0;) {
    sol[i] = sol[i] - (acc[i] * sol[i + 1]);
  }

  return sol;
}


// Uses Graham's Scan to get a convex hull of 'pts'.
// The resulting set is written out to the 'pts'.
// Function returns false if getting the convex hull failed.
// The pts.size() has to be at least 3.
//
// The implementation is based on en.wikipedia.org/wiki/Graham_scan
// and cp-algorithms.com/geometry/grahams-scan-convex-hull.html (TODO)
bool ConvexHull2dCalc::toConvexHull(std::vector<rch::Vec3>& pts) {
  if (pts.size() < 3) {
    return false;
  }

  // Sort the pts from bottom-left to top-right
  // to use them as pivots
  std::sort(pts.begin(), pts.end(), &lessX_Y);
  rch::Vec3 botL = pts[0];     // bottom-left
  rch::Vec3 topR = pts.back(); // top-right
  
  // Init the lower and upper hull
  std::vector<rch::Vec3> upper, lower;
  upper.push_back(botL);
  lower.push_back(botL);


  for (int i = 1; i < pts.size(); i++) {
    // Handle the uppper hull
    if (i == pts.size() - 1 || cw(botL, pts[i], topR)) {
      while (upper.size() >= 2 &&
             !cw(upper[upper.size() - 2], upper[upper.size() - 1], pts[i])) {
        upper.pop_back();
      }
      upper.push_back(pts[i]);
    }
    // Handle the lower hull
    if (i == pts.size() - 1 || ccw(botL, pts[i], topR)) {
      while (lower.size() >= 2 &&
             !ccw(lower[lower.size() - 2], lower[lower.size() - 1], pts[i])) {
        lower.pop_back();
      }
      lower.push_back(pts[i]);
    }
  }

  // Merge the result
  pts.resize(upper.size() + lower.size() - 2);
  auto ctr = 0;
  for (auto i = 0; i < upper.size(); i++) {
    pts[ctr++] = upper[i];
  }
  for (auto i = lower.size() - 2; i > 0; i--) {
    pts[ctr++] = lower[i];
  }
  return true;
}

}  // namespace rch