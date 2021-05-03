#pragma once
#include < D3D11.h>
#include <DirectXMath.h>

#include <vector>

#include "Maths3D.h"
#include "dxDevice.h"
#include "dxptr.h"
#include "vertexTypes.h"

using namespace mini;
using namespace DirectX;

namespace rch {

class GraphicsSystem;

class RenTechBasic {
 public:
  RenTechBasic(DxDevice& dev,
               XMFLOAT4 defCol = {0.0f, 0.0f, 0.9f, 1.0f})
      : m_dev(dev),
        m_topoType(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
        m_defaultCol(defCol),
        m_cbModel(m_dev.CreateConstantBuffer<XMFLOAT4X4>()),
        m_cbColour(m_dev.CreateConstantBuffer<XMFLOAT4>()) {
    // Rasterizer
    RasterizerDescription rsDesc;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    m_rastState = m_dev.CreateRasterizerState(rsDesc);

    // Default buffer values
    m_dev.updateBuffer(m_cbColour, defCol);
    setModelCb(rch::gen_id_mtx());

    auto vsCode = m_dev.LoadByteCode(vsPath);
    m_mainVS = m_dev.CreateVertexShader(vsCode);
    m_mainPS = m_dev.CreatePixelShader(m_dev.LoadByteCode(psPath));

    const D3D11_INPUT_ELEMENT_DESC layout[1] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    m_layout = m_dev.CreateInputLayout(layout, vsCode);
  }

  // If the 'topo' parameter is not set (= set to the default value)
  // we used the  member variable 'm_topoType' instead.
  void bind(GraphicsSystem& gx,
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);

  // void unbind((const dx_ptr<ID3D11DeviceContext>& context);
  void unbind() {
    m_dev.context()->VSSetShader(nullptr, nullptr, 0);
    m_dev.context()->PSSetShader(nullptr, nullptr, 0);
  }

  void setTopology(const D3D11_PRIMITIVE_TOPOLOGY topo) { 
    m_dev.context()->IASetPrimitiveTopology(topo);
  }

  void resetTopology() {
    m_dev.context()->IASetPrimitiveTopology(m_topoType);
  }

  void setModelCb(const XMMATRIX mtx) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mtx);
    m_dev.updateBuffer(m_cbModel, temp);
  }

  void setColourCb(const XMFLOAT4 col) { m_dev.updateBuffer(m_cbColour, col); }
  void resetColourCb() { m_dev.updateBuffer(m_cbColour, m_defaultCol); }

 private:
  // ---- Static/Const Vars
  static const wchar_t* vsPath;
  static const wchar_t* psPath;
  const XMFLOAT4 m_defaultCol;

  // ---- Vars
  const DxDevice& m_dev;

  mini::dx_ptr<ID3D11Buffer> m_cbModel, m_cbColour;

  mini::dx_ptr<ID3D11InputLayout> m_layout;
  mini::dx_ptr<ID3D11RasterizerState> m_rastState;

  mini::dx_ptr<ID3D11VertexShader> m_mainVS;
  mini::dx_ptr<ID3D11PixelShader> m_mainPS;

  D3D_PRIMITIVE_TOPOLOGY m_topoType;
};

}  // namespace rch