#pragma once 
#include < D3D11.h>
#include <DirectXMath.h>

#include <vector>

#include "dxDevice.h"
#include "dxptr.h"
#include "vertexTypes.h"

#include "Maths3D.h"


using namespace mini;
using namespace DirectX;

namespace rch {

class GraphicsSystem;

class RenTechPoint {
public:
  RenTechPoint(DxDevice& dev)
     : m_dev(dev),
       m_topoType(D3D_PRIMITIVE_TOPOLOGY_POINTLIST),
       m_cbModel(m_dev.CreateConstantBuffer<XMFLOAT4X4>()),
       m_cbColour(m_dev.CreateConstantBuffer<XMFLOAT4>()),
       m_cbCamRightDir(m_dev.CreateConstantBuffer<XMFLOAT4>()) {

    // Rasterizer
    RasterizerDescription rsDesc;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    m_rastState = m_dev.CreateRasterizerState(rsDesc);

    //// Default buffer values
    //resetColourCb();
    //setModelCb(rch::gen_id_mtx());
    //m_dev.updateBuffer(m_cbCamRightDir, m_gxSys.getCameraRightDir());

    auto vsCode = m_dev.LoadByteCode(s_vsPath);
    m_mainVS = m_dev.CreateVertexShader(vsCode);
    m_mainGS = m_dev.CreateGeometryShader(m_dev.LoadByteCode(s_gsPath));
    m_mainPS = m_dev.CreatePixelShader(m_dev.LoadByteCode(s_psPath));

    const D3D11_INPUT_ELEMENT_DESC layout[1] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
        D3D11_INPUT_PER_VERTEX_DATA, 0}};
    m_layout = m_dev.CreateInputLayout(layout, vsCode); 
  }

  //void bind((const dx_ptr<ID3D11DeviceContext>& context);
  //void bind(const mini::DxDevice& dev) { 
  void bind(GraphicsSystem& gx);

  // void bind((const dx_ptr<ID3D11DeviceContext>& context);
  //void unbind(const mini::DxDevice& dev) {
  void unbind() {
    m_dev.context()->VSSetShader(nullptr, nullptr, 0);
    m_dev.context()->GSSetShader(nullptr, nullptr, 0);
    m_dev.context()->PSSetShader(nullptr, nullptr, 0);
  }

  void setModelCb(const XMMATRIX mtx) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mtx);
    m_dev.updateBuffer(m_cbModel, temp);
  }

  void setColourCb(const XMFLOAT4 col) { 
    m_dev.updateBuffer(m_cbColour, col);
  }

  void resetColourCb() { 
    m_dev.updateBuffer(m_cbColour, s_defaultCol);
  }

private:
  // ---- Static/Const Vars
  static const wchar_t* s_vsPath;
  static const wchar_t* s_gsPath;
  static const wchar_t* s_psPath;
  static const XMFLOAT4 s_defaultCol;

  // ---- Vars
  DxDevice& m_dev;

  mini::dx_ptr<ID3D11Buffer> m_cbModel, m_cbColour, m_cbCamRightDir;
  
  mini::dx_ptr<ID3D11InputLayout> m_layout;
  mini::dx_ptr<ID3D11RasterizerState> m_rastState;

  mini::dx_ptr<ID3D11VertexShader> m_mainVS;
  mini::dx_ptr<ID3D11GeometryShader> m_mainGS;
  mini::dx_ptr<ID3D11PixelShader> m_mainPS;

  D3D_PRIMITIVE_TOPOLOGY m_topoType;
};

}  // namespace rch