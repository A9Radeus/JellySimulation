#pragma once
#include "SceneObj.h"
#include "Mesh.h"

namespace rch {

class WiredCube : public SceneObj {
public:
  WiredCube(GraphicsSystem& gx, float side = 1.f, XMFLOAT4 col = {0,0,1,1}) 
      : SceneObj(gx), m_side(side), m_colour(col) {
   std::vector<unsigned short> inds = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                       6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};
    m_idxNum = static_cast<unsigned int>(inds.size());
    m_idxBuff = m_dev.CreateIndexBuffer(inds);
  }
  virtual ~WiredCube() = default;

  void update(double dtime = 0.0) override {
    if (m_meshReady == false) {
      generateMesh();
    }
  }
  
  // Disabled: use specific renderXYZ functions
  void render() override {
    if (m_meshReady == false) {
      return;
    }

    m_gxSys.m_basicRT.bind(m_gxSys, D3D11_PRIMITIVE_TOPOLOGY_LINELIST); // TODO
    m_gxSys.m_basicRT.setModelCb(getModelMtx()); 
    m_gxSys.m_basicRT.setColourCb(m_colour); 

    unsigned int strides = sizeof(mini::VertexPosition);
    unsigned int offset = 0;
    //m_dev.context()->IASetPrimitiveTopology(
    //    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_dev.context()->IASetIndexBuffer(m_idxBuff.get(), DXGI_FORMAT_R16_UINT, 0);
    m_dev.context()->IASetVertexBuffers(0, static_cast<UINT>(m_vertBuff.size()),
                                        m_vertBuff.data(),
                                        &strides, &offset);
    m_dev.context()->DrawIndexed(m_idxNum, 0, 0);

    m_gxSys.m_basicRT.unbind();
  };

  void setColour(XMFLOAT4 newCol) { m_colour = newCol; };

  void setSide(float sideLen) {
    m_side = sideLen;
    m_meshReady = false;
  }

  float getSide(float sideLen) const { return m_side; }

  // Usees {0,1}^3 coding of corners. If posx = true,
  // takes the rightmost (in left-handed) corner.
  Vec3 getCorner(bool px, bool py, bool pz) const {
    const float hside = m_side / 2.f;
    const Vec3 localPos = Vec3{bsgn(px) * hside, bsgn(py) * hside, bsgn(pz) * hside};
    if (m_angleX + m_angleY + m_angleZ == 0.0) {
      return m_pos + localPos;
    } else {
      auto rot = XMMatrixIdentity() * XMMatrixRotationX(static_cast<float>(m_angleX)) *
                 XMMatrixRotationY(static_cast<float>(m_angleY)) * XMMatrixRotationZ(static_cast<float>(m_angleZ));
      auto mulRes = XMVector3Transform(Vec3::XMV4_v0(localPos), rot);
      return m_pos + Vec3::toVec3(mulRes);
    }
  }

private:
  // ----------- Functions -----------
  void generateMesh() {
   auto verts = mini::Mesh::BoxVertsUnique(m_side, m_side, m_side);
      //mini::Mesh::ShadedBoxVertsCorner(m_side, m_side, m_side);
   
    m_vertBuff.clear();
    m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));
   
    m_meshReady = true;
  }

  // helper function for corners
  inline static constexpr float bsgn(bool b) { return b ? 1.f : -1.f; };
  
  // GX
  bool m_meshReady = false;
  mini::dx_ptr_vector<ID3D11Buffer> m_vertBuff;
  mini::dx_ptr<ID3D11Buffer> m_idxBuff;
  unsigned int m_idxNum = 0;
  
  // Attributes
  float m_side;
  XMFLOAT4 m_colour;
};

}  // namespace rch