#include "RenTechBasic.h"
#include "GraphicsSystem.h"

namespace rch {

const wchar_t* RenTechBasic::vsPath = L"defaultVS.cso";
const wchar_t* RenTechBasic::psPath = L"defaultPS.cso";

void RenTechBasic::bind(GraphicsSystem& gx, D3D11_PRIMITIVE_TOPOLOGY topo) {
  m_dev.context()->RSSetState(m_rastState.get());

  m_dev.context()->VSSetShader(m_mainVS.get(), nullptr, 0);
  m_dev.context()->PSSetShader(m_mainPS.get(), nullptr, 0);

  m_dev.context()->IASetInputLayout(m_layout.get());

  if (topo == D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) {
    m_dev.context()->IASetPrimitiveTopology(m_topoType);
  } else {
    m_dev.context()->IASetPrimitiveTopology(topo);
  }

  // Colour has to be set after binding
  resetColourCb();

  ID3D11Buffer* vsBuff[] = {m_cbModel.get(), gx.getViewCb(), gx.getProjCb()};
  m_dev.context()->VSSetConstantBuffers(0, 3, vsBuff);

  ID3D11Buffer* psBuff[] = {m_cbColour.get()};
  m_dev.context()->PSSetConstantBuffers(0, 1, psBuff);
}

}  // namespace rch
