#pragma once
#include <DirectXMath.h>
#include "Maths3D.h"

using namespace DirectX;

namespace rch {

// TODO: Deprecate?
static XMVECTOR quaternionMul_Mtx(const XMVECTOR& q, const Vec3& vec) {
  XMFLOAT4 auxq;
  XMStoreFloat4(&auxq, q);

  auto qMtx = XMMATRIX(-auxq.x, -auxq.y, -auxq.z, 0, auxq.w, -auxq.z, auxq.y, 0,
                       auxq.z, auxq.w, -auxq.x, 0, -auxq.y, auxq.x, auxq.w, 0);

  XMFLOAT4 auxv = {vec.x, vec.y, vec.z, 0};
  auto vVec = XMLoadFloat4(&auxv);
  auto qMtxT = XMMatrixTranspose(qMtx);

  auto vRes = XMVector4Transform(vVec, qMtx);
  return vRes;
}

// Uses {vec.x, vec.y, vec.z, 0} and XMQuaternionMultiply
static XMVECTOR quaternionMul(const XMVECTOR& q, const Vec3& vec) {
  XMFLOAT4 auxv = {vec.x, vec.y, vec.z, 0};
  auto vVec = XMLoadFloat4(&auxv);
  auto vRes = XMQuaternionMultiply(q, vVec);
  return vRes;
}

// Normalises quaternion 'q' internaly
static XMVECTOR rotateByQ(const XMVECTOR& q, const Vec3& vec) {
  auto normQ = XMQuaternionConjugate(q);
  auto left = quaternionMul(normQ, vec);
  auto res = XMQuaternionMultiply(left, XMQuaternionConjugate(normQ));
  return res;
}

// Normalises quaternion 'q' internaly
static XMVECTOR rotateByQConjugate(const XMVECTOR& q, const Vec3& vec) {
  auto normQ = XMQuaternionConjugate(q);
  auto left = quaternionMul(normQ, vec);
  auto res = XMQuaternionMultiply(left, XMQuaternionConjugate(normQ));
  return res;
}

}  // namespace