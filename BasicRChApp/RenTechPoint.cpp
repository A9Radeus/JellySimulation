#include "RenTechPoint.h"
#include "GraphicsSystem.h"

namespace rch {

const wchar_t* RenTechPoint::s_vsPath = L"worldVS.cso";
const wchar_t* RenTechPoint::s_gsPath = L"scPointGS.cso";
const wchar_t* RenTechPoint::s_psPath = L"defaultPS.cso";
const XMFLOAT4 RenTechPoint::s_defaultCol = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

void RenTechPoint::bind(GraphicsSystem& gx) {
  m_dev.context()->RSSetState(m_rastState.get());

  m_dev.context()->VSSetShader(m_mainVS.get(), nullptr, 0);
  m_dev.context()->GSSetShader(m_mainGS.get(), nullptr, 0);
  m_dev.context()->PSSetShader(m_mainPS.get(), nullptr, 0);

  m_dev.context()->IASetInputLayout(m_layout.get());
  m_dev.context()->IASetPrimitiveTopology(m_topoType);

  // Different colour, if needed, has to be set after binding
  resetColourCb();

  ID3D11Buffer* vsBuff[] = {m_cbModel.get()};
  m_dev.context()->VSSetConstantBuffers(0, 1, vsBuff);

  m_dev.updateBuffer(m_cbCamRightDir, gx.getCameraRightDir());
  ID3D11Buffer* gsBuff[] = {gx.getViewCb(), gx.getProjCb(),
                            m_cbCamRightDir.get()};
  m_dev.context()->GSSetConstantBuffers(0, 3, gsBuff);

  ID3D11Buffer* psBuff[] = {m_cbColour.get()};
  m_dev.context()->PSSetConstantBuffers(0, 1, psBuff);
}

}  // namespace rch
