#pragma once

#pragma once
#include <D3D11.h>
#include <DirectXMath.h>

#include <vector>

#include "SceneObj.h"
#include "dxDevice.h"
#include "dxptr.h"
#include "vertexTypes.h"

using namespace mini;
using namespace DirectX;

namespace rch {

// TODO: everything...
class RenderableVerts : public SceneObj {
 public:
  RenderableVerts(GraphicsSystem& gxSys, D3D_PRIMITIVE_TOPOLOGY topo,
                  XMFLOAT4 col = {1, 1, 1, 1});
  virtual void update(double dtime = 0.0);
  virtual void render();
  virtual void setDisplayOn(bool val) { m_displayOn = val; };

  void changeColour(const XMFLOAT4& col) {
    // m_col = col;
    m_dev.updateBuffer(m_cbColour, col);
  }

 protected:
  virtual void generateVerts() = 0;
  virtual void bind();
  virtual void unbind();

  void setModelCb(const XMMATRIX& mtx) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mtx);
    m_dev.updateBuffer(m_cbModel, temp);
  }
  // XMFLOAT4 getColour() const { return m_col; }
  void changeRasterizer(RasterizerDescription rs) {
    m_rastState = m_dev.CreateRasterizerState(rs);
  }

  // DirectX
  mini::dx_ptr_vector<ID3D11Buffer> m_vertBuff;
  size_t m_vertsNum = 0;

  mini::dx_ptr<ID3D11Buffer> m_cbModel, m_cbCamPos, m_cbColour;

  mini::dx_ptr<ID3D11InputLayout> m_layout;
  mini::dx_ptr<ID3D11RasterizerState> m_rastState;

  mini::dx_ptr<ID3D11VertexShader> m_mainVS;
  mini::dx_ptr<ID3D11PixelShader> m_mainPS;

  D3D_PRIMITIVE_TOPOLOGY m_topo;

  bool m_vertsReady = false;
  bool m_displayOn = true;
  // XMFLOAT4 m_col;

  const wchar_t* m_vsPath = L"defaultVS.cso";
  const wchar_t* m_psPath = L"defaultPS.cso";
};

}  // namespace rch