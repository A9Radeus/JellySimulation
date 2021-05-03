#pragma once
#include "SceneObj.h"
#include "Point.h"
//#include "VirtualPointsSys.h"

namespace rch {

/**
 * Example for sizeX = 3, sizeY = 4:
 *  
 *  ^
 *  |  9 10 11
 *  |  6  7  8
 *  |  3  4  5
 *  Y  0  1  2 
 *  X ------->
 *  
 *  .at(0, 0) = 0
 *  .at(1, 0) = 1
 *  .at(1, 1) = 4
 *  .at(2, 2) = 8
 */
template <typename DataT>
class ContainerXY {
  using Uint = unsigned int;
public:
  ContainerXY(std::size_t sizeX, std::size_t sizeY)
      : m_sizeX(sizeX),
        m_sizeY(sizeY),
        m_maxIdx(sizeX * sizeY - 1), 
        m_vec(sizeX * sizeY) {};

  DataT& refAt(Uint x, Uint y) {
    return m_vec.at(x + y * (m_sizeX));
  }

  DataT at(Uint x, Uint y) const {
    return m_vec.at(x + y * (m_sizeX));
  }

  bool set(Uint x, Uint y, const DataT& data) {
    if (x + y * (m_sizeX) > m_maxIdx) {
      //throw std::string("OORange: ") + std:to_string(x) + " ; " + std::to_string(y);
      return false;
    }
    m_vec[x + y * (m_sizeX)] = data;
    return true;
  }

  bool set(Uint x, Uint y, DataT&& data) {
    if (x + y * (m_sizeX) > m_maxIdx) {
      return false;
    }
    m_vec[x + y * (m_sizeX)] = std::move(data);
    return true;
  }

  // PTODO: Additional safety? Move c-tor?
  std::vector<DataT>&& extractInnerBuf() { return std::move(m_vec); }

 private:
  const std::size_t m_sizeX;
  const std::size_t m_sizeY;
  std::vector<DataT> m_vec;
  const Uint m_maxIdx;
};

/**
 * Abstraction for Bezier Bicubic Patches.
 * User has to call init() after creating 
 * a patch for the first time.
 */
class BicubicPatch : public CompoundObj {
  using Uint = unsigned int;
public:
  enum class WrapMode { Left, Right, Top, Bottom, None };
  using PointPtrs = std::vector<std::shared_ptr<Point>>;

public:
  BicubicPatch(GraphicsSystem& gxSys)
      : CompoundObj(gxSys) {
    //m_vpSys.registerOwner(this);
  };
  //virtual ~BicubicPatch() { m_vpSys.unregisterOwner(this); };

  BicubicPatch(const BicubicPatch&) = delete;
  BicubicPatch& operator=(const BicubicPatch&) = delete;

  BicubicPatch(BicubicPatch&&) = delete;
  BicubicPatch& operator=(BicubicPatch&&) = delete;

  /******************** Overrides/Virtuals ********************/
  virtual void update(double dtime = 0.0) = 0;
  virtual void render() = 0;
  //virtual void init() = 0; TODO

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

  /********************* BicubicPatch-specific *********************/
  virtual void renderPolyline() = 0;
  void switchShowPolyline() { m_showPolyline ^= true; };
  bool getShowPolyline() const { return m_showPolyline; };

  //virtual void init(std::vector<Point*>&& ctrls, Uint across, Uint lenwise,
  //                  BicubicPatch::WrapMode wrap) = 0;

    virtual void init(PointPtrs&& ctrls, Uint across, Uint lenwise,
                    BicubicPatch::WrapMode wrap) = 0;
  virtual void init(const PointPtrs& ctrls, Uint across, Uint lenwise,
                    BicubicPatch::WrapMode wrap) = 0;

  //virtual const std::vector<Point*>& getConcreteControlPoints() = 0;
  virtual const PointPtrs& getConcreteControlPoints() = 0;

  void setInsideTessFtr(float inside);
  void setEdgeTessFtrs(float rowSlices, float colSlices);
  std::array<float, 3> getTessFtrs();

protected:
  WrapMode m_wrMode = WrapMode::None;
  Uint m_patAcross = 0;
  Uint m_patLenwise = 0;

  bool m_showPolyline = false;
  bool m_initialised  = false;

  float m_insideTessFtr = 4.0f;
  float m_edgeRowTessFtr = 4.0f; // row-slices
  float m_edgeColTessFtr = 4.0f; // column-slices

private:
  const float s_tessFtrMin = 0.0f;
  const float s_tessFtrMax = 128.0f;
};

// TODO test wether column & row ~= right & top
// column prawie na pewno tak, gorzej z row
BicubicPatch::WrapMode toWrapDir(const char* name);

}  // namespace rch