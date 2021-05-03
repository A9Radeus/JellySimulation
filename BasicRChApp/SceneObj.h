#pragma once
#include <DirectXMath.h>

#include <functional>

#include "GraphicsSystem.h"
#include "Maths3D.h"
#include "dxDevice.h"
#include "dxptr.h"
//#include "tinyxml2/tinyxml2.h"

using namespace DirectX;

namespace rch {

enum class SOType {
  POINT,
  TORUS,
  SCENE_CURSOR,
  BEZIER3_CURVE_C0,
  BEZIER3_CURVE_C2,
  INTERP_SPLINE3_CD,
  BEZIER3_PATCH_C0,
  BEZIER3_PATCH_C2,
  MILLING_SIMUL,
  LINESTRIP3D_POINTS,
  UNDEFINED
};

class SceneObj {
 public:
  SceneObj(GraphicsSystem& gxSys)
      : m_gxSys(gxSys), m_dev(m_gxSys.getDxDev()), m_pos(0.0) {
    m_angleX = 0.0;
    m_angleY = 0.0;
    m_angleZ = 0.0;
  };
  SceneObj(const SceneObj&) = delete;
  SceneObj& operator=(const SceneObj&) = delete;

  SceneObj(SceneObj&&) = delete;
  SceneObj& operator=(SceneObj&&) = delete;

  virtual ~SceneObj() = default;

  virtual void update(double dtime = 0.0) = 0;
  virtual void render() = 0;
  //virtual void serialise(
  //    tinyxml2::XMLPrinter& pr,
  //    const std::unordered_map<SceneObj*, std::string> lMap){};

  // By default, every rotation, translation (and scaling?)
  // sets the m_prsChanged to TRUE. You can override it in
  // the derived class.

  virtual void setRotation(Vec3 rot) {
    m_angleX = rot.x;
    m_angleY = rot.y;
    m_angleZ = rot.z;
    m_prsChanged = true;  // todo?
  }

  virtual void rotateXAround(double angle, Vec3 rotPoint);
  virtual void rotateYAround(double angle, Vec3 rotPoint);
  virtual void rotateZAround(double angle, Vec3 rotPoint);

  virtual void rotateX(double angle);
  virtual void rotateY(double angle);
  virtual void rotateZ(double angle);

  // TODO ! Scale isn't being handled in 1) modelMtx 2) App 3) setModelParams
  virtual void scale(Vec3 val) {
    m_scaling += val;
    m_prsChanged = true;
  };
  virtual void setScaling(Vec3 val) {
    m_scaling = val;
    m_prsChanged = true;
  };
  virtual void scaleX(float d) {
    m_scaling.x += d;
    m_prsChanged = true;
  };
  virtual void scaleY(float d) {
    m_scaling.y += d;
    m_prsChanged = true;
  };
  virtual void scaleZ(float d) {
    m_scaling.z += d;
    m_prsChanged = true;
  };

  virtual void translate(rch::Vec3 vec);
  virtual void setPos(const rch::Vec3 newPos) {
    m_pos = newPos;
    // doesnt quite when trying to update deBoors in curveC2
    m_prsChanged = true; 
  };

  /* Copies the parameters from another SceneObj */
  virtual void setModelMtxParams(const SceneObj& secObj);
  virtual void setModelMtxParams(double angleX, double angleY, double angleZ,
                                 rch::Vec3 pos);

  // Functions used to inform that position, rotation
  // and/or scale of this object have been changed recently:
  bool getPrsChanged() const { return m_prsChanged; }
  void setPrsChangeHandled() { m_prsChanged = false; }

  // Getters:
  virtual XMMATRIX getModelMtx() const;
  virtual XMMATRIX getInvModelMtx() const;
  virtual rch::Vec3 getCenterPos() const { return m_pos; };
  virtual double getAngleX() const { return m_angleX; };
  virtual double getAngleY() const { return m_angleY; };
  virtual double getAngleZ() const { return m_angleZ; };
  virtual Vec3 getScaling() const { return m_scaling; };

  // RTTI:
  virtual const char* getTypeName() const { return "undefined"; }
  virtual SOType getType() const { return SOType::UNDEFINED; }
  constexpr static SOType getStaticType() { return SOType::UNDEFINED; }

 protected:
  // SceneObjects DO NOT own these:
  GraphicsSystem& m_gxSys;  // init firstly
  const mini::DxDevice& m_dev;

  rch::Vec3 m_pos;
  rch::Vec3 m_scaling{
      1.0f,
      1.0f,
      1.0f,
  };
  bool m_prsChanged = false;
  double m_angleX, m_angleY, m_angleZ;
};

/**
 * TODO: Problems with rotate[X/Y/Z]Around() for ParametricObj if we ve had
 *       another initial rotation on a different axis around the middle
 *       of an object, i.e.: RotY_mid = 180, RotX_curosr = w/e
 */
class ParametricObj : public SceneObj {
 public:
  ParametricObj(GraphicsSystem& gxSys) : SceneObj(gxSys){};
  virtual ~ParametricObj() = default;

  virtual void rotateXAround(double angle, Vec3 rotPoint);
  virtual void rotateYAround(double angle, Vec3 rotPoint);
  virtual void rotateZAround(double angle, Vec3 rotPoint);
};

/**
 * (WIP) For now: curves' abstracition.
 * Abstraction for scene objects consisting of other ScreenObjs
 * usually Points.
 */
class CompoundObj : public SceneObj {
 public:
  CompoundObj(GraphicsSystem& gxSys) : SceneObj(gxSys){};
  virtual ~CompoundObj() = default;

  CompoundObj(const CompoundObj&) = delete;
  CompoundObj& operator=(const CompoundObj&) = delete;
  CompoundObj(CompoundObj&&) = delete;
  CompoundObj& operator=(CompoundObj&&) = delete;

  virtual void update(double dtime = 0.0) = 0;
  virtual void render() = 0;

  virtual void rotateXAround(double angle, Vec3 rotPoint){};
  virtual void rotateYAround(double angle, Vec3 rotPoint){};
  virtual void rotateZAround(double angle, Vec3 rotPoint){};

  virtual void rotateX(double angle){};
  virtual void rotateY(double angle){};
  virtual void rotateZ(double angle){};

  virtual void translate(rch::Vec3 vec){};
  virtual void setPos(rch::Vec3 newPos){};

  virtual void setModelMtxParams(double angleX, double angleY, double angleZ,
                                 rch::Vec3 pos){};
  virtual void setModelMtxParams(SceneObj& secObj){};

  virtual XMMATRIX getModelMtx() const { return rch::gen_id_mtx(); };
  virtual XMMATRIX getInvModelMtx() const { return rch::gen_id_mtx(); };
  virtual rch::Vec3 getCenterPos() const { return m_pos; };
  virtual double getAngleX() const { return m_angleX; };
  virtual double getAngleY() const { return m_angleY; };
  virtual double getAngleZ() const { return m_angleZ; };
};

}  // namespace rch