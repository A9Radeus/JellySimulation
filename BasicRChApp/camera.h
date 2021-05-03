#pragma once

#include <DirectXMath.h>

namespace mini {
class Camera {
 public:
  virtual ~Camera() {}
  virtual DirectX::XMMATRIX getViewMatrix() const = 0;
};

class OrbitCamera : public Camera {
 public:
  explicit OrbitCamera(DirectX::XMFLOAT3 target = DirectX::XMFLOAT3(0, 0, 0),
                       float minDistance = 0.0f, float maxDistance = FLT_MAX,
                       float distance = 0.0f);
  explicit OrbitCamera(float minDistance, float maxDistance = FLT_MAX,
                       float distance = 0.0f);

  DirectX::XMMATRIX getViewMatrix() const override;
  DirectX::XMFLOAT4 getCameraPosition() const;

  void MoveTarget(DirectX::XMFLOAT3 v) { MoveTarget(XMLoadFloat3(&v)); }
  void MoveTarget(DirectX::FXMVECTOR v);
  void Rotate(float dx, float dy);
  void setYAngle(float y) {
    m_angleY = DirectX::XMScalarModAngle(y);
  }
  void setXAngle(float x) {
    m_angleX = DirectX::XMScalarModAngle(x);
  }
  void Zoom(float dd);
  void SetDistanceRange(float minDistance, float maxDistance);

  // Returns camera's forward direction parallel to the XZ-plane
  DirectX::XMVECTOR getForwardXZDir() const;
  // Returns camera's right direction parallel to the XZ-plane
  DirectX::XMVECTOR getRightXZDir() const;

  float getXAngle() const { return m_angleX; }
  float getYAngle() const { return m_angleY; }
  float getDistance() const { return m_distance; }
  DirectX::XMFLOAT4 getTarget() const { return m_target; }

  void moveCam(DirectX::FXMVECTOR v) {
    using namespace DirectX;
    auto pos = XMLoadFloat4(&m_target);
    pos = pos + v;
    XMStoreFloat4(&m_target, pos);
  }

 private:
  void ClampDistance();

  float m_angleX, m_angleY;
  float m_distance, m_minDistance, m_maxDistance;
  DirectX::XMFLOAT4 m_target;
};

class FPSCamera : public OrbitCamera {
 public:
  explicit FPSCamera(DirectX::XMFLOAT3 target)
      : OrbitCamera(target, 0.0f, 0.0f) {}

  using OrbitCamera::getTarget;
  using OrbitCamera::getViewMatrix;
  using OrbitCamera::getXAngle;
  using OrbitCamera::getYAngle;
  using OrbitCamera::MoveTarget;
  using OrbitCamera::Rotate;

  /*Returns target's forward direction parallel to ground (XZ) plane*/
  DirectX::XMVECTOR getForwardDir() const;
  /*Returns target's right direction parallel to ground (XZ) plane*/
  DirectX::XMVECTOR getRightDir() const;
};
}  // namespace mini