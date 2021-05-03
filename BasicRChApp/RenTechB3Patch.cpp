#include "RenTechB3Patch.h"
#include "GraphicsSystem.h"

namespace rch {

const wchar_t* RenTechB3Patch::s_vsPath = L"worldVS.cso";
const wchar_t* RenTechB3Patch::s_hsPath = L"b3PatchC0_HS.cso";
const wchar_t* RenTechB3Patch::s_dsPath = L"b3PatchC0_DS.cso";
const wchar_t* RenTechB3Patch::s_psPath = L"BasicPhongPS.cso";
//const wchar_t* RenTechB3Patch::s_psPath = L"defaultPS.cso";

void RenTechB3Patch::bind(GraphicsSystem& gx) {
  if (m_solidFill) {
    m_dev.context()->RSSetState(m_rsSolid.get());
  } else {
    m_dev.context()->RSSetState(m_rsWired.get());
  }

  m_dev.context()->VSSetShader(m_mainVS.get(), nullptr, 0);
  m_dev.context()->HSSetShader(m_mainHS.get(), nullptr, 0);
  m_dev.context()->DSSetShader(m_mainDS.get(), nullptr, 0);
  m_dev.context()->PSSetShader(m_mainPS.get(), nullptr, 0);

  m_dev.context()->IASetInputLayout(m_layout.get());
  m_dev.context()->IASetPrimitiveTopology(m_topoType);

  resetColourCb();

  ID3D11Buffer* vsBuff[] = {m_cbModel.get()};
  m_dev.context()->VSSetConstantBuffers(0, 1, vsBuff);

  ID3D11Buffer* hsBuff[] = {gx.getViewCb(), m_cbTessFacts.get()};
  m_dev.context()->HSSetConstantBuffers(0, 2, hsBuff);

  ID3D11Buffer* dsBuff[] = {gx.getProjCb()};
  m_dev.context()->DSSetConstantBuffers(0, 1, dsBuff);

  ID3D11Buffer* psBuff[] = {m_cbColour.get()};
  m_dev.context()->PSSetConstantBuffers(0, 1, psBuff);
}

}  // namespace rch
