#pragma once
#include <D3D11.h>
#include <DirectXMath.h>

#include <cstdint>
#include <vector>

//#include "GraphicsSystem.h"
#include "Maths3D.h"
#include "dxDevice.h"
#include "dxptr.h"
#include "vertexTypes.h"

using namespace mini;
using namespace DirectX;

namespace rch {
class GraphicsSystem;

class RenTechB3Patch {
public:
  RenTechB3Patch(DxDevice& dev)
      : m_dev(dev),
        m_topoType(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST),
        m_cbModel(m_dev.CreateConstantBuffer<XMFLOAT4X4>()),
        m_cbColour(m_dev.CreateConstantBuffer<XMFLOAT4>()),
        m_cbTessFacts(m_dev.CreateConstantBuffer<XMFLOAT4>()) {
    // Rasterizer
    RasterizerDescription rsDesc;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    m_rsWired = m_dev.CreateRasterizerState(rsDesc);
    
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    m_rsSolid = m_dev.CreateRasterizerState(rsDesc);

    // Default buffer values
    resetColourCb();
    setModelCb(rch::gen_id_mtx());
    setTessFactorBuff(30.f, 30.f, 30.f);

    auto vsCode = m_dev.LoadByteCode(s_vsPath);
    m_mainVS = m_dev.CreateVertexShader(vsCode);
    m_mainHS = m_dev.CreateHullShader(m_dev.LoadByteCode(s_hsPath));
    m_mainDS = m_dev.CreateDomainShader(m_dev.LoadByteCode(s_dsPath));
    m_mainPS = m_dev.CreatePixelShader(m_dev.LoadByteCode(s_psPath));

    const D3D11_INPUT_ELEMENT_DESC layout[1] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    m_layout = m_dev.CreateInputLayout(layout, vsCode);
  }

  void bind(GraphicsSystem& gx);

  void unbind() {
    m_dev.context()->VSSetShader(nullptr, nullptr, 0);
    m_dev.context()->HSSetShader(nullptr, nullptr, 0);
    m_dev.context()->DSSetShader(nullptr, nullptr, 0);
    m_dev.context()->PSSetShader(nullptr, nullptr, 0);
  }

  // PTODO double precision instead of redundant 0s
  void setTessFactorBuff(float edgesRowSlices, float edgesColSlices,
                         float inside) {
    m_dev.updateBuffer(m_cbTessFacts,
                       XMFLOAT4(edgesRowSlices, edgesColSlices, inside, 0));
  }

  void switchRS() { m_solidFill = !m_solidFill; }

  void setModelCb(const XMMATRIX mtx) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mtx);
    m_dev.updateBuffer(m_cbModel, temp);
  }

  void setColourCb(const XMFLOAT4& col) { 
    m_currColour = col;
    m_dev.updateBuffer(m_cbColour, col);
  }
  void setAlphaColCb(float alpha) {
    setColourCb({m_currColour.x, m_currColour.y, m_currColour.z, alpha});
  }

  void resetColourCb() { m_dev.updateBuffer(m_cbColour, s_defaultCol); }

private:
  // ---- Static/Const Vars
  static const wchar_t* s_vsPath;
  static const wchar_t* s_hsPath;
  static const wchar_t* s_dsPath;
  static const wchar_t* s_psPath;
  //static constexpr XMFLOAT4 s_defaultCol = {0.831f, 0.11f, 0.7f, 0.34f};
  //static constexpr XMFLOAT4 s_defaultCol = {0.831f, 0.11f, 0.7f, 1.0f};
  static constexpr XMFLOAT4 s_defaultCol = {0.847f, 0.32f, 0.71f, 1.0f};

  // ---- Vars
  DxDevice& m_dev;
  bool m_solidFill = false;
  XMFLOAT4 m_currColour = s_defaultCol;

  mini::dx_ptr<ID3D11Buffer> m_cbModel, m_cbColour, m_cbTessFacts;

  mini::dx_ptr<ID3D11InputLayout> m_layout;
  mini::dx_ptr<ID3D11RasterizerState> m_rsWired;
  mini::dx_ptr<ID3D11RasterizerState> m_rsSolid;

  mini::dx_ptr<ID3D11VertexShader> m_mainVS;
  mini::dx_ptr<ID3D11HullShader> m_mainHS;
  mini::dx_ptr<ID3D11DomainShader> m_mainDS;
  mini::dx_ptr<ID3D11PixelShader> m_mainPS;

  D3D_PRIMITIVE_TOPOLOGY m_topoType;
};

}  // namespace rch