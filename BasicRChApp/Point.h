#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <cmath>

#include "SceneObj.h"
#include "GraphicsSystem.h"
#include "dxDevice.h"
#include "dxptr.h"


using namespace DirectX;

namespace rch {

// PTODO PointsManager/System that renders them at once (no modelMtx needed) 
class Point : public SceneObj {
public:
  enum class Mode { NON_V, PURE_V, SCENE_V };

public:
  Point(GraphicsSystem& gxSys, Mode mode = Mode::NON_V)
      : SceneObj(gxSys) {
    m_mode = mode;

    std::vector<XMFLOAT3> verts(m_vertsNum);
    verts[0] = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));
  };

  Point(const Point& rhs) : SceneObj(rhs.m_gxSys) {
    m_mode = rhs.m_mode;

    std::vector<XMFLOAT3> verts(m_vertsNum);
    verts[0] = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));

    m_angleX = rhs.m_angleX;
    m_angleY = rhs.m_angleY;
    m_angleZ = rhs.m_angleZ;
    m_pos = rhs.m_pos;
    m_scaling = rhs.m_scaling;
    //setModelMtxParams(static_cast<const SceneObj&>(rhs));
  };

  Point(Point&& rhs) noexcept
      : SceneObj(rhs.m_gxSys), m_vertBuff(std::move(rhs.m_vertBuff)) {
    m_mode = rhs.m_mode;

    m_angleX = rhs.m_angleX;
    m_angleY = rhs.m_angleY;
    m_angleZ = rhs.m_angleZ;
    m_pos = std::move(rhs.m_pos);
    m_scaling = std::move(rhs.m_scaling);
  };

  Point& operator=(const Point& rhs) = delete;
  Point& operator=(Point&& rhs) = delete;
  
  virtual ~Point() = default;

  virtual void update(double dtime = 0.0) override;
  virtual void render() override;

  void setMode(Mode mode) { m_mode = mode; };
  Mode getMode() const { return m_mode; };

  // RTTI --
  virtual const char* getTypeName() const override { return "Point"; }
  virtual SOType getType() const override { return SOType::POINT; }
  constexpr static SOType getStaticType() { return SOType::POINT; }

  // Disable rotations around the point's center --
  virtual void rotateX(double angle) override {};
  virtual void rotateY(double angle) override {};
  virtual void rotateZ(double angle) override {};

  /*virtual void scale(Vec3 val) { m_scaling += val; };
  virtual void setScaling(Vec3 val) { m_scaling = val; };
  virtual void scaleX(float d) { m_scaling.x += d; };
  virtual void scaleY(float d) { m_scaling.y += d; };
  virtual void scaleZ(float d) { m_scaling.z += d; };*/

  // Scaling for points is rather a tool to achieve symetry
  // thus the "copysignf" and constant scaling delta "scaleDt" 
  // is merely to provide a more predictible usage for a user.
  virtual void scale(Vec3 val) {
    scaleX(val.x);
    scaleY(val.y);
    scaleZ(val.z);
    m_prsChanged = true;
  };
  virtual void setScaling(Vec3 val) { scale(val); m_prsChanged = true; };
  virtual void scaleX(float d) {
    m_pos.x += copysignf(1, m_pos.x) * d * scaleDt;
    m_prsChanged = true;
  };
  virtual void scaleY(float d) {
    m_pos.y += copysignf(1, m_pos.y) * d * scaleDt;
    m_prsChanged = true;
  };
  virtual void scaleZ(float d) {
    m_pos.z += copysignf(1, m_pos.z) * d * scaleDt;
    m_prsChanged = true;
  };

  virtual DirectX::XMMATRIX getModelMtx() const override;

  // PTODO getCenterPos() through Mtx mul
  //virtual Vec3 getCenterPos() override {  };

 private:
  // ----------- Functions -----------

  // ----------- Variables -----------
  mini::dx_ptr_vector<ID3D11Buffer> m_vertBuff;
  static constexpr short m_vertsNum = 1;
  Mode m_mode = Mode::NON_V;
  static constexpr float scaleDt = 0.2f;
};


}  // namespace rch