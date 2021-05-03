#include "RenderableVerts.h"

namespace rch {

RenderableVerts::RenderableVerts(GraphicsSystem& gxSys,
                                 D3D_PRIMITIVE_TOPOLOGY topo, XMFLOAT4 col)
    : SceneObj(gxSys),
      m_topo(topo),
      m_cbModel(m_dev.CreateConstantBuffer<XMFLOAT4X4>()),
      m_cbColour(m_dev.CreateConstantBuffer<XMFLOAT4>()) {
  // Rasterizer
  RasterizerDescription rsDesc;
  rsDesc.CullMode = D3D11_CULL_NONE;
  rsDesc.FillMode = D3D11_FILL_WIREFRAME;
  m_rastState = m_dev.CreateRasterizerState(rsDesc);

  // Default buffer values
  setModelCb(rch::gen_id_mtx());
  changeColour(col);

  auto vsCode = m_dev.LoadByteCode(m_vsPath);
  m_mainVS = m_dev.CreateVertexShader(vsCode);
  m_mainPS = m_dev.CreatePixelShader(m_dev.LoadByteCode(m_psPath));

  const D3D11_INPUT_ELEMENT_DESC layout[1] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0}};
  m_layout = m_dev.CreateInputLayout(layout, vsCode);
}

void RenderableVerts::update(double dtime) {
  if (m_vertsReady == false && m_displayOn == true) {
    generateVerts();
  }
}

void RenderableVerts::render() {
  if (m_vertsReady == false || m_displayOn == false) {
    return;
  }

  bind();

  unsigned int strides = sizeof(XMFLOAT3);
  unsigned int offset = 0;
  m_gxSys.getContext()->IASetVertexBuffers(
      0, m_vertBuff.size(), m_vertBuff.data(), &strides, &offset);
  m_gxSys.getContext()->Draw(m_vertsNum, 0);

  unbind();
}

void RenderableVerts::bind() {
  m_dev.context()->RSSetState(m_rastState.get());

  m_dev.context()->VSSetShader(m_mainVS.get(), nullptr, 0);
  m_dev.context()->PSSetShader(m_mainPS.get(), nullptr, 0);

  m_dev.context()->IASetInputLayout(m_layout.get());

  m_dev.context()->IASetPrimitiveTopology(m_topo);

  ID3D11Buffer* vsBuff[] = {m_cbModel.get(), m_gxSys.getViewCb(),
                            m_gxSys.getProjCb()};
  m_dev.context()->VSSetConstantBuffers(0, 3, vsBuff);

  ID3D11Buffer* psBuff[] = {m_cbColour.get()};
  m_dev.context()->PSSetConstantBuffers(0, 1, psBuff);
}

void RenderableVerts::unbind() {
  m_dev.context()->VSSetShader(nullptr, nullptr, 0);
  m_dev.context()->PSSetShader(nullptr, nullptr, 0);
}

}  // namespace rch