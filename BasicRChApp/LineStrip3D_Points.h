#pragma once
#include "RenderableVerts.h"
#include "Point.h"

namespace rch {

// TODO: setVerts(std::vector<XMFLOAT3>), etc.
class LineStrip3D_Points : public RenderableVerts {
  using PointsPtrs = std::vector<std::shared_ptr<Point>>;

 public:
  LineStrip3D_Points(GraphicsSystem& gxSys, XMFLOAT4 colour)
      : RenderableVerts(gxSys, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, colour) {}

  
  void update(double dtime) override {
    if (m_ctrlPoints.size() <= 0) {
      return;
    }

    for (const auto& pt : m_ctrlPoints) {
      if (pt->getPrsChanged()) {
        generateVerts();
        return;
      }
    }
  }

  virtual void generateVerts() override {
    if (m_ctrlPoints.size() <= 0) {
      return;
    }

    std::vector<XMFLOAT3> verts(m_ctrlPoints.size());
    size_t ctr = 0;
    for (const auto& pt : m_ctrlPoints) {
      verts[ctr++] = Vec3::XMF3(pt->getCenterPos());
    }

    m_vertBuff.clear();
    m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));
    m_vertsNum = verts.size();
    m_vertsReady = true;
  }

  void setPoints(PointsPtrs&& verts) {
    m_ctrlPoints = std::move(verts);
    m_vertsReady = false;
  }

  void setPoints(const PointsPtrs& verts) {
    m_ctrlPoints = verts;
    m_vertsReady = false;
  }

  // RTTI:
  virtual const char* getTypeName() const { return "LineStrip3D_Points"; }
  virtual SOType getType() const { return SOType::LINESTRIP3D_POINTS; }
  constexpr static SOType getStaticType() { return SOType::LINESTRIP3D_POINTS; }

 private:
  PointsPtrs m_ctrlPoints;
};

}  // namespace rch