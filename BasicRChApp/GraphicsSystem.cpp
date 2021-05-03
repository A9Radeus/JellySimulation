#include "GraphicsSystem.h"

#include "exceptions.h"

using namespace DirectX;

namespace rch {

GraphicsSystem::GraphicsSystem(mini::DxDevice& dev, const mini::OrbitCamera& camera,
                               const mini::Window& window)
    : m_dev(dev),
      m_camera(camera),
      m_cbProj(m_dev.CreateConstantBuffer<XMFLOAT4X4>()),
      m_cbView(m_dev.CreateConstantBuffer<XMFLOAT4X4, 2>()),
      m_windowSize(window.getClientSize()),
      m_pointRT(m_dev),
      m_basicRT(m_dev),
      m_patchRT(m_dev) { 
  // Projection Mtx
  XMFLOAT4X4 tmpMtx;
  XMStoreFloat4x4(&tmpMtx, getProjMtx());
  dev.updateBuffer(m_cbProj, tmpMtx);

  // Camera Buffers
  updateCameraCbs();
}

/**
 * Creates a ray (origin, direction) at the passed screen-coords and 
 * transforms it to object-space (coord-sys with the owner of modelMtx being
 * at the (0, 0, 0) point) OR world-space if the modelMtx is = Identity.
 *
 * Returns: {ray origin point, ray direction vector}
 *
 * Parameters:
 *  - modelMtx: Matrix that transforms an object to the world-space.
 *      If modelMtx = Id, the ray will be in world-space.
 *  - xPosS, yPosS: (x,y) position on the screen in "screen-space",
 *      thus (0,0) is the top-left corner and (W,H) -- the bottom-right.
 */
std::pair<XMVECTOR, XMVECTOR> 
GraphicsSystem::unprojectScreenRay(XMMATRIX modelMtx, float xPosS, float yPosS) {
  float viewportW = m_windowSize.cx;
  float viewportH = m_windowSize.cy;

  auto vRayOrigS = XMVECTOR({xPosS, yPosS, 0.0f});
  auto vRayDestS = XMVECTOR({xPosS, yPosS, 1.0f});
  auto vRayOrigO =
      XMVector3Unproject(vRayOrigS, 0, 0, viewportW, viewportH, 0, 1,
                         getProjMtx(), getCameraViewMtx(), modelMtx);
  auto vRayDestO =
      XMVector3Unproject(vRayDestS, 0, 0, viewportW, viewportH, 0, 1,
                         getProjMtx(), getCameraViewMtx(), modelMtx);
  auto vRayDirO = XMVector3Normalize({vRayDestO - vRayOrigO});

  return {vRayOrigO, vRayDirO};
}

}  // namespace rch