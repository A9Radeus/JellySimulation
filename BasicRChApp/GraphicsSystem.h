#pragma once 
#include "DirectXMath.h"

#include "dxptr.h" 
#include "windowApplication.h"
#include "dxDevice.h" 
#include "camera.h" 

#include "Maths3D.h"
#include "RenTechPoint.h"
#include "RenTechB3Patch.h"
#include "RenTechBasic.h"


using namespace DirectX;

namespace rch {

class GraphicsSystem {
public:
  // PTODO just Window Size?
 GraphicsSystem(mini::DxDevice& dev, const mini::OrbitCamera& camera,
                 const mini::Window& window);

  XMMATRIX getCameraViewMtx() const { return m_camera.getViewMatrix(); };
  XMFLOAT4 getCameraPos() const { return m_camera.getCameraPosition(); };
  XMFLOAT4 getCameraRightDir() const { 
    XMFLOAT4 temp;
    XMStoreFloat4(&temp, m_camera.getRightXZDir());
    return temp;
  };
  /* PTODO: store XMMATIRX instead? */
  XMMATRIX getProjMtx() const { 
    auto ratio = static_cast<float>(m_windowSize.cx) / m_windowSize.cy;
    return gen_perspective_fov_mtx(XM_PIDIV4, ratio, 0.01f, 150.0f);
  };

  ID3D11Buffer* getViewCb() { return m_cbView.get(); };
  ID3D11Buffer* getProjCb() { return m_cbProj.get(); };

  const mini::DxDevice& getDxDev() { return m_dev; }
  ID3D11Device* getDevice() const { return m_dev.get(); }
  const mini::dx_ptr<ID3D11DeviceContext>& getContext() const {
    return m_dev.context();
  }
  const mini::dx_ptr<IDXGISwapChain>& getSwapChain() const {
    return m_dev.swapChain();
  }

  void updateCameraCbs() {
    XMMATRIX view = m_camera.getViewMatrix();
    XMMATRIX invView = XMMatrixInverse(nullptr, view);
    XMFLOAT4X4 viewMats[2]; 
    XMStoreFloat4x4(&viewMats[0], view);
    XMStoreFloat4x4(&viewMats[1], invView);
    m_dev.updateBuffer(m_cbView, viewMats);
  }

  /********************* General Utility-functions *********************/

  /**
   * TODO move
   * Multiplies the 'ptW' in world coordinates by view matrix
   * and projetion matrix afterwards. To convert the result to
   * the [-1,1]x[-1,1] coordinates, XYZ components have to be
   * divided by the W component
   */
  Vec3 projWorldPtToScreen(Vec3 ptW) {
    auto viewMtx = getCameraViewMtx();
    auto projMtx = getProjMtx();
    auto vPosW = XMVECTOR({(float)ptW.x, (float)ptW.y, (float)ptW.z, 1.0f});
    auto vPosView = XMVector4Transform(vPosW, viewMtx);
    auto vPosProj = XMVector4Transform(vPosView, projMtx);

    XMFLOAT4 xmPosS;
    XMStoreFloat4(&xmPosS, vPosProj);

    return {xmPosS.x, xmPosS.y, xmPosS.z};
  }

  std::pair<XMVECTOR, XMVECTOR> unprojectScreenRay(XMMATRIX modelMtx,
                                                    float xPosS, float yPosS);

private:
  mini::DxDevice& m_dev;
  const mini::OrbitCamera& m_camera;
  const SIZE m_windowSize;

  const mini::dx_ptr<ID3D11Buffer> m_cbProj;
  const mini::dx_ptr<ID3D11Buffer> m_cbView; // Holds both viewMtx and invViewMtx

  /* todo */
public:
  RenTechBasic m_basicRT;
  RenTechPoint m_pointRT;
  RenTechB3Patch m_patchRT;
};

}  // namespace rch
