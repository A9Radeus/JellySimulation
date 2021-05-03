#pragma once
#include <cassert>
#include <array>
#include <random>

#include "dxApplication.h"
#include "mesh.h"

#include "WiredCube.h"
#include "SceneObj.h"
#include "GuiWidgets.h"
#include "SimulationPopup.h"
#include "Point.h"
#include "CompBezier3PatchC0.h"
#include "LineStrip3D_Points.h"
#include "DeformableMesh.h"

namespace rch { 

class RChApp : public mini::DxApplication {
private:
  struct DisplayOptions {
    bool showCube = true;
    bool showGravity = true;
    float cubeAlpha = 1.0f;

    bool showDiagonal = false;
  };

public:
  using Base = mini::DxApplication;

  explicit RChApp(HINSTANCE appInstance);
  ~RChApp();

  bool HandleCameraInput(double dt, MouseState& mstate);

protected:
  void Update(const mini::Clock& dt) override;
  void Render() override;

private: 
  //std::random_device m_randDev;
  //std::mt19937 m_randGen;

  mini::dx_ptr<ID3D11Buffer> m_cbWorldMtx;                
  mini::dx_ptr<ID3D11Buffer> m_cbViewMtx;  
  mini::dx_ptr<ID3D11Buffer> m_cbSurfaceColor;  
  mini::dx_ptr<ID3D11Buffer> m_cbLightPos;      
  mini::dx_ptr<ID3D11Buffer> m_cbCamPos;

  /*
      Scene objects
  */

  std::vector<std::shared_ptr<rch::SceneObj>> m_sobjs;
  Array3D<std::shared_ptr<rch::Point>, 4, 4, 4> m_springPts;
  std::shared_ptr<rch::WiredCube> m_boundingFrame;
  std::shared_ptr<rch::WiredCube> m_ctrlFrame;
  std::weak_ptr<SceneObj> m_selectedObj;
  std::array<std::shared_ptr<CompBezier3PatchC0>, 6> m_facePatches;
  std::array<std::shared_ptr<LineStrip3D_Points>, 2*4*(4+2)> m_springGrid;

  mini::Mesh m_cyliX;
  mini::Mesh m_cyliY;
  mini::Mesh m_cyliZ;

  //mini::Mesh m_loadedMesh;
  std::shared_ptr<DeformableMesh> m_defomableMesh;

  DirectX::XMFLOAT4X4 m_cyliXMtx;
  DirectX::XMFLOAT4X4 m_cyliYMtx;
  DirectX::XMFLOAT4X4 m_cyliZMtx;
  DirectX::XMFLOAT4X4 m_controlFrMtx;
  DirectX::XMFLOAT4X4 m_edgeCubeMtx;

  mini::dx_ptr<ID3D11InputLayout> m_layoutVertPosNorm;
  mini::dx_ptr<ID3D11InputLayout> m_layoutVertPos;

  mini::dx_ptr<ID3D11VertexShader> m_defaultVS, m_basicPhongVS;
  mini::dx_ptr<ID3D11PixelShader> m_defaultPS, m_basicPhongPS;

  mini::dx_ptr<ID3D11BlendState> m_blendState;

  // Rasterizers
  mini::dx_ptr<ID3D11RasterizerState> m_rsCullFront;
  mini::dx_ptr<ID3D11RasterizerState> m_rsCCW;
  mini::dx_ptr<ID3D11RasterizerState> m_rsWireframe;

  /*
      Settings
  */

  double m_timeAcc = 0;
  float m_fpsTarget = 200;
  int m_spfTarget = 5;
  bool m_frameRdy = false;
  bool m_lockObjAxes = true;
  bool m_displayPts = true;
  bool m_displayPatches = true;

  /*
      Subsystems
  */

  GraphicsSystem m_gxSys;
  AsynchSimulation<JellySim> m_simulation;
  float m_dt = 0;
  
  /*
      GUI 
  */

  std::shared_ptr<TextPopup> m_popupInfo;
  std::shared_ptr<SimulationPopup> m_simPopup;
  DisplayOptions m_dispOpts;

  /*
      Utility functions
  */

  void drawScene();
  void updateGUI();

  // Scene Utils
  void updateParticlesPos(Array3D<SimParticle, 4, 4, 4>& arr);
  void updatePtsPos(const Array3D<SimParticle, 4, 4, 4>& particles);
  void reshapeToCube(Array3D<SimParticle, 4, 4, 4>& particles, float side);
  Array3D<SimParticle, 4, 4, 4> genParticlesCube(float side);

  void initJelly();
  void createSpringPts();
  void createSpringGrid();
  void createJellyPatches();

  void displayGrid(bool val);

  bool handleObjInput(double dt, MouseState& mstate, KeyboardState& kbstate,
                      std::shared_ptr<SceneObj> obj);
  void handleGlobalInput(double dt, MouseState& mstate, KeyboardState& kbstate);

  void UpdateCameraCB(DirectX::XMMATRIX viewMtx);
  //void UpdateCameraCB() { UpdateCameraCB(m_camera.getViewMatrix()); }
  void UpdateCameraCB() { m_gxSys.updateCameraCbs(); }

  void renderSceneCursor();
  void renderLoadedMesh();
  void DrawMesh(const mini::Mesh& m, DirectX::XMFLOAT4X4 worldMtx);

  void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);
  void SetWorldMtx(const DirectX::XMMATRIX& mtx);
  void SetSurfaceColor(DirectX::XMFLOAT4 color);
  void SetShaders(const mini::dx_ptr<ID3D11VertexShader>& vs,
                  const mini::dx_ptr<ID3D11PixelShader>& ps);
  void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
                   const mini::dx_ptr<ID3D11SamplerState>& sampler);

  /*
     Constants
  */

  static const DirectX::XMFLOAT4 LIGHT_POS;
  static constexpr float CAMERA_ZOOM = 12.0f;
  static constexpr float CAMERA_SPEED = 3.0f;
  static constexpr float CAM_T_SPEED = 3.5f;
  static constexpr float TRANSLATION_SPEED = 0.02f;
  
};


}  // namespace rch