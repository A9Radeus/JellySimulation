#pragma once
#include "Array3D.h"
#include "GraphicsSystem.h"
#include "Mesh.h"
#include "Point.h"
#include "RenderableVerts.h"

namespace rch {

// PTODO: since we re using inds, this shouldnt really derive from
// RenderableVerts
class DeformableMesh : public RenderableVerts {
 public:
  DeformableMesh(GraphicsSystem& gx,
                 Array3D<std::shared_ptr<rch::Point>, 4, 4, 4>& massPts,
                 std::string meshFilePath,
                 const wchar_t* vsPath, const wchar_t* psPath)
      : RenderableVerts(gx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
        m_meshFN(meshFilePath),
        m_massPts(massPts), m_vsPath(vsPath), m_psPath(psPath) {
    auto buffs = Mesh::LoadObjBuffs(m_dev, m_meshFN);
    m_meshPosOffests = getBoundValues(buffs.first);

    m_vertsNum = buffs.first.size();
    m_meshVerts = buffs.first;
    m_idxBuff = m_dev.CreateIndexBuffer(buffs.second);
    m_idxNum = buffs.second.size();

    auto vsCode = m_dev.LoadByteCode(m_vsPath);
    m_mainVS = m_dev.CreateVertexShader(vsCode);
    m_mainPS = m_dev.CreatePixelShader(m_dev.LoadByteCode(m_psPath));

    const D3D11_INPUT_ELEMENT_DESC layout[2] = {
        VertexPositionNormal::Layout[0], VertexPositionNormal::Layout[1]};
    m_layout = m_dev.CreateInputLayout(layout, vsCode);

      // Rasterizer
    RasterizerDescription rsDesc;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    m_rastState = m_dev.CreateRasterizerState(rsDesc);
  }

  void update(double dt = 0.0) override {
    if (/*m_vertsReady == false ||*/ m_displayOn == false) {
      return;
    }
    generateVerts();
  };

  virtual void render() {
    if (m_vertsReady == false || m_displayOn == false) {
      return;
    }

    bind();

    unsigned int strides = sizeof(VertexPositionNormal);
    unsigned int offset = 0;
    m_gxSys.getContext()->IASetIndexBuffer(m_idxBuff.get(),
                                           DXGI_FORMAT_R16_UINT, 0);
    m_gxSys.getContext()->IASetVertexBuffers(
        0, m_vertBuff.size(), m_vertBuff.data(), &strides, &offset);
    m_gxSys.getContext()->DrawIndexed(m_idxNum, 0, 0);

    unbind();
  };

  // TODO
  // RTTI --
  virtual const char* getTypeName() const override { return "Deformable Mesh"; }
  virtual SOType getType() const override { return SOType::UNDEFINED; }
  constexpr static SOType getStaticType() { return SOType::UNDEFINED; }

 protected:

  void generateVerts() override {
    auto verts = m_meshVerts;
    auto scaledVerts = m_meshVerts;

    for (auto& vert : verts) {
      auto ncoord = toNormMeshCoord(vert.position);
      auto BernU = calcBernstein3Basis(ncoord.x);
      auto BernV = calcBernstein3Basis(ncoord.y);
      auto BernW = calcBernstein3Basis(ncoord.z);

      vert.position = {0, 0, 0};

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          for (int k = 0; k < 4; k++) {
            const auto& mpt = m_massPts.val(i, j, k);
            auto coord = mpt->getCenterPos() * BernU[i] * BernV[j] * BernW[k];
            vert.position.x += coord.x;
            vert.position.y += coord.y;
            vert.position.z += coord.z;
          }
        }
      }
    }

    for (auto& vert : scaledVerts) {
      XMFLOAT3 offset = xm_mul(s_offset, vert.normal);

      vert.position = xm_subt(vert.position, offset);

      auto ncoord = toNormMeshCoord(vert.position);
      auto BernU = calcBernstein3Basis(ncoord.x);
      auto BernV = calcBernstein3Basis(ncoord.y);
      auto BernW = calcBernstein3Basis(ncoord.z);

      vert.position = {0, 0, 0};

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          for (int k = 0; k < 4; k++) {
            const auto& mpt = m_massPts.val(i, j, k);
            auto coord = mpt->getCenterPos() * BernU[i] * BernV[j] * BernW[k];
            vert.position.x += coord.x;
            vert.position.y += coord.y;
            vert.position.z += coord.z;
          }
        }
      }
    }

    for (std::size_t i = 0; i < verts.size(); ++i) {
      verts[0].normal = xm_subt(verts[i].position, scaledVerts[i].position);
    }


    m_vertBuff.clear();
    m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));
    m_vertsReady = true;
  }

 private:
  const wchar_t* m_vsPath;
  const wchar_t* m_psPath;

  std::string m_meshFN;
  std::array<Vec3, 2> m_meshPosOffests;

  Array3D<std::shared_ptr<rch::Point>, 4, 4, 4>& m_massPts;

  unsigned int m_idxNum = 0;
  mini::dx_ptr<ID3D11Buffer> m_idxBuff;

  std::vector<VertexPositionNormal> m_meshVerts;

  static constexpr float s_offset = 0.002f;

  std::array<Vec3, 2> getBoundValues(const std::vector<VertexPositionNormal>& vs) {
    Vec3 cmax = {FLT_MIN, FLT_MIN, FLT_MIN};
    Vec3 cmin = {FLT_MAX, FLT_MAX, FLT_MAX};
    for (const auto& v : vs) {
      auto vx = v.position.x;
      auto vy = v.position.y;
      auto vz = v.position.z;

      cmax.x = max(cmax.x, vx);
      cmin.x = min(cmin.x, vx);

      cmax.y = max(cmax.y, vy);
      cmin.y = min(cmin.y, vy);

      cmax.z = max(cmax.z, vz);
      cmin.z = min(cmin.z, vz);
    }

    return {cmin, cmax};
  }

  Vec3 toNormMeshCoord(const Vec3& vert) {
    auto x = rch::fracOfSegm(m_meshPosOffests[0].x, m_meshPosOffests[1].x, vert.x);
    auto y = rch::fracOfSegm(m_meshPosOffests[0].y, m_meshPosOffests[1].y, vert.y);
    auto z = rch::fracOfSegm(m_meshPosOffests[0].z, m_meshPosOffests[1].z, vert.z);
    return {x, y, z};
  }

  Vec3 toNormMeshCoord(const XMFLOAT3& vert) {
    auto x = rch::fracOfSegm(m_meshPosOffests[0].x, m_meshPosOffests[1].x, vert.x);
    auto y = rch::fracOfSegm(m_meshPosOffests[0].y, m_meshPosOffests[1].y, vert.y);
    auto z = rch::fracOfSegm(m_meshPosOffests[0].z, m_meshPosOffests[1].z, vert.z);
    return {x, y, z};
  }

  XMFLOAT3 xm_subt(const XMFLOAT3& left, const XMFLOAT3& right) {
    return {left.x - right.x, left.y - right.y, left.z - right.z};
  }

  XMFLOAT3 xm_add(const XMFLOAT3& left, const XMFLOAT3& right) {
    return {left.x + right.x, left.y + right.y, left.z + right.z};
  }

  XMFLOAT3 xm_mul(float scalar, const XMFLOAT3& xmf) {
    return {scalar * xmf.x, scalar * xmf.y, scalar * xmf.z};
  }

};

}  // namespace rch
