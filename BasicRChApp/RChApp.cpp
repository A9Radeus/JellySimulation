#include <algorithm>
#include <cmath>
#include <memory>

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
//#include "implot.h"

#include "RchApp.h"

using namespace mini;
using namespace DirectX;

namespace rch {

const XMFLOAT4 RChApp::LIGHT_POS = {-4.0f, 6.0f, -4.0f, 1.0f};

RChApp::~RChApp() {
  // ImGui 
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

RChApp::RChApp(HINSTANCE appInstance)
    : DxApplication(appInstance, 1500, 768, L"RChApp"),
      m_gxSys(m_device, m_camera, m_window),
      // Constant Buffers
      m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
      m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
      m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
      m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4>()),
      m_cbCamPos(m_device.CreateConstantBuffer<XMFLOAT4>()) {
  ///// ImGui Init
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplWin32_Init(m_window.getHandle());
  ImGui_ImplDX11_Init(m_device.get(), m_device.context().get());

  // Camera 
  m_camera.Zoom(CAMERA_ZOOM);
  m_camera.MoveTarget(XMFLOAT3{0.0f, 0.0f, 0.0f});

  // GUI elements
  m_popupInfo = std::make_shared<TextPopup>("Info Popup", "Information not set yet.");
  m_simPopup = std::make_shared<SimulationPopup>("Simulation Params...");

  /* Initialise the scene  */

  m_boundingFrame = std::make_shared<WiredCube>(m_gxSys, JellySimParams::boundingBoxSide);
  m_sobjs.push_back(m_boundingFrame);

  m_ctrlFrame = std::make_shared<WiredCube>(m_gxSys, 1.f, XMFLOAT4{1.f, 0.1f, 0.8f, 1.0f});
  m_sobjs.push_back(m_ctrlFrame);
  
  initJelly();

  m_defomableMesh =
      std::make_shared<DeformableMesh>(m_gxSys, m_springPts, "../Resources/Koltuk.obj", L"BasicPhongVS.cso", L"BasicPhongPS.cso");
  m_defomableMesh->setDisplayOn(false);
  m_sobjs.push_back(m_defomableMesh);

  m_cyliX = mini::Mesh::Cylinder(m_device, 16, 16, 0.5f, 0.01f);
  m_cyliY = mini::Mesh::Cylinder(m_device, 16, 16, 0.5f, 0.01f);
  m_cyliZ = mini::Mesh::Cylinder(m_device, 16, 16, 0.5f, 0.01f);
  XMStoreFloat4x4(&m_controlFrMtx, XMMatrixIdentity());
  XMStoreFloat4x4(&m_edgeCubeMtx,
                    XMMatrixIdentity());
  XMStoreFloat4x4(&m_cyliXMtx,
                    XMMatrixTranslation(0, 0.25f, 0) * XMMatrixRotationZ(-XM_PIDIV2));
  XMStoreFloat4x4(&m_cyliYMtx,
                    XMMatrixTranslation(0, 0.25f, 0));
  XMStoreFloat4x4(&m_cyliZMtx,
                    XMMatrixTranslation(0, 0.25f, 0) * XMMatrixRotationX(XM_PIDIV2));

  // Light CB
  UpdateBuffer(m_cbLightPos, LIGHT_POS);

  // Render states
  RasterizerDescription rsDesc;
  m_rsCullFront = m_device.CreateRasterizerState(rsDesc);

  rsDesc.FrontCounterClockwise = true;
  m_rsCCW = m_device.CreateRasterizerState(rsDesc);
 
  rsDesc.CullMode = D3D11_CULL_NONE;
  rsDesc.FillMode = D3D11_FILL_WIREFRAME;
  m_rsWireframe = m_device.CreateRasterizerState(rsDesc);

  // BlendState
  D3D11_BLEND_DESC bdesc = {};
  auto& brt = bdesc.RenderTarget[0];
  brt.BlendEnable = true;
  brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
  brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  brt.BlendOp = D3D11_BLEND_OP_ADD;
  brt.SrcBlendAlpha = D3D11_BLEND_ZERO;
  brt.DestBlendAlpha = D3D11_BLEND_ZERO;
  brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
  brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  ID3D11BlendState* auxBlendState;
  m_device.get()->CreateBlendState(&bdesc, &auxBlendState);
  m_blendState = mini::dx_ptr<ID3D11BlendState>(auxBlendState);
  m_device.context()->OMSetBlendState(m_blendState.get(), nullptr, 0xFFFFFFFFu);

  // Shaders
  auto vsCode = m_device.LoadByteCode(L"defaultVS.cso");
  auto psCode = m_device.LoadByteCode(L"defaultPS.cso");
  m_defaultVS = m_device.CreateVertexShader(vsCode);
  m_defaultPS = m_device.CreatePixelShader(psCode);
  m_layoutVertPos = m_device.CreateInputLayout(VertexPosition::Layout, vsCode);

  vsCode = m_device.LoadByteCode(L"BasicPhongVS.cso");
  psCode = m_device.LoadByteCode(L"BasicPhongPS.cso");
  m_basicPhongVS = m_device.CreateVertexShader(vsCode);
  m_basicPhongPS = m_device.CreatePixelShader(psCode);
  m_layoutVertPosNorm =
      m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

  ID3D11Buffer* vsb[] = {m_cbWorldMtx.get(), m_gxSys.getViewCb(),
                         m_gxSys.getProjCb()};
  m_device.context()->VSSetConstantBuffers(
      0, 3, vsb);  
  ID3D11Buffer* psb[] = {m_cbSurfaceColor.get(), m_cbLightPos.get(),
                         m_gxSys.getViewCb()};
  m_device.context()->PSSetConstantBuffers(
      0, 3, psb); 
}

void RChApp::Update(const Clock& c) {
  double dt = c.getFrameTime();

  // Handle FPS cap
  m_timeAcc += dt;
  if (m_timeAcc < (1.0 / m_fpsTarget)) {
    m_frameRdy = false;
    return; 
  } else {
    m_timeAcc = 0;
    m_frameRdy = true;
  }

  // Perform simulation steps
  bool simUpdated = false;
  for (int i = 0; i < m_spfTarget; i++) {
    if (m_simulation.step()) {
      simUpdated = true;
    } else {
      break;
    }
  }

  // Update the Jelly
  if (simUpdated) {
    const auto& simState = m_simulation.getState();
    updatePtsPos(simState);
  }

  // Update GUI
  updateGUI();

  // Handle Inputs
  KeyboardState kbState;
  bool kbStateReady = false;
  if (m_keyboard.GetState(kbState)) {
    kbStateReady = true;
  }
  MouseState moState;
  bool moStateReady = false;
  if (m_mouse.GetState(moState)) {
    moStateReady = true;
  }

  if (auto igio = ImGui::GetIO(); igio.WantTextInput == false &&
                                  igio.WantCaptureMouse == false &&
                                  moStateReady && kbStateReady) {
    handleGlobalInput(dt, moState, kbState);

    if (auto sptr = m_selectedObj.lock()) {
      handleObjInput(dt, moState, kbState, sptr);
    } else {
      HandleCameraInput(dt, moState); 
    }
  }

  // Update all Scene Objects
  for (const auto& obj : m_sobjs) {
    obj->update(dt);
  }
  for (std::size_t i = 0; i < m_facePatches.size(); ++i) {
    m_facePatches[i]->update();
  }
  for (std::size_t i = 0; i < m_springPts.size(); ++i) {
    m_springPts.valLin(i)->update();
  }
}

void RChApp::drawScene() {
  if (m_displayPts) {
    for (std::size_t i = 0; i < m_springPts.size(); ++i) {
      m_springPts.valLin(i)->render(); 
    }
  }

  for (const auto& obj : m_sobjs) {
    obj->render();
  }

  renderSceneCursor();

  // Has to be render lastly to apply the alpha
  if (m_displayPatches) {
    for (std::size_t i = 0; i < m_facePatches.size(); ++i) {
      m_facePatches[i]->render();
    }
  }
}

void RChApp::Render() {
  if (m_frameRdy == false) {
    return;
  }

  Base::Render();

  UpdateCameraCB();
  drawScene();

  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void RChApp::updateGUI() {
  // Start a new frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Settings");
  if (ImGui::TreeNode("Display")) {

    ImGui::SliderFloat("FPS", &m_fpsTarget, 1.f, 600.f);

    ImGui::SliderInt("SPF", &m_spfTarget, 1, 100);

    static bool dispObjMesh = false;
    if (ImGui::Checkbox("Display deformable mesh", &dispObjMesh)) {
      m_defomableMesh->setDisplayOn(dispObjMesh);
    }

    static bool dispGrid = true;
    if (ImGui::Checkbox("Display grid", &dispGrid)) {
      displayGrid(dispGrid);
    }
    ImGui::Checkbox("Display points", &m_displayPts);
    ImGui::Checkbox("Display patches", &m_displayPatches);
    if (ImGui::Button("Solid/Wired patches")) {
      m_gxSys.m_patchRT.switchRS();
    }

    static float alpha = 1.f;
    if (ImGui::SliderFloat("Jelly's alpha", &alpha, 0.f, 1.f)) {
      m_gxSys.m_patchRT.setAlphaColCb(alpha);
    }


    ImGui::TreePop();
  }
  
  if (ImGui::TreeNode("Simulation")) {
    auto collMode = m_simulation.core().getCollisionMode();
    ImGui::Text("Collision Mode [");
    ImGui::SameLine();
    if (collMode == JellySim::CollisionMode::ALL_AXES) {
      ImGui::TextColored({0.1f, 0.9f, 0.1f, 1.f}, "All Axes");
    } else {
      ImGui::TextColored({ 0.1f, 0.9f, 0.1f, 1.f } , "One Axis");
    }
    ImGui::SameLine();
    ImGui::Text("]");

    if (ImGui::Button("Switch Collision Mode")) {
      m_simulation.core().switchCollisionMode();
    }

    float restitution = m_simulation.core().getRestiution(); 
    if (inputFloatClamped("Restiution", &restitution, 0.f, 1.f)) {
      m_simulation.core().setRestiution(restitution);
    }

    if (ImGui::Button("Start a new simulation", ImVec2(300, 0))) {
      m_simPopup->open();
    }

    if (ImGui::Button("Pause", ImVec2(95, 0))) {
      m_simulation.pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Resume", ImVec2(95, 0))) {
      m_simulation.resume();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(95, 0))) {
      m_simulation.stop();
    }

    ImGui::TreePop();
  } 

  if (ImGui::TreeNode("Control")) {

    if (ImGui::Button("Select Control-Frame")) {
      m_selectedObj = m_ctrlFrame;
    }

    if (ImGui::Button("Activate Control-Frame")) {
      m_simulation.core().activateFrame();
    }
    ImGui::SameLine();
    if (ImGui::Button("Deactivate Control-Frame")) {
      m_simulation.core().deactivateFrame();
    }

    ImGui::Separator();

    if (ImGui::Button("Deselect")) {
      m_selectedObj.reset();
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Statistics")) {
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    
    ImGui::TreePop();
  }
  ImGui::End();

  /* 
      Popups 
  */

  m_popupInfo->update();

  m_simPopup->update();
  if (m_simPopup->chosenReady()) {
    rch::SimulationPopup::Params p;
    m_simPopup->getChosen(&p);

    m_dt = p.dt;
    m_ctrlFrame->setSide(p.cubeSide);

    m_simulation.stop();
    m_simulation.start(genParticlesCube(p.cubeSide), p, m_ctrlFrame);
  }
}

// PTODO: separate class
#pragma region Scene Utils

void RChApp::renderSceneCursor() {
  // 3D Cursor (TODO: make a separate SceneObj)
  m_device.context()->RSSetState(nullptr);
  SetShaders(m_basicPhongVS, m_basicPhongPS);
  m_device.context()->IASetInputLayout(m_layoutVertPosNorm.get());
  ID3D11Buffer* vsb[] = {m_cbWorldMtx.get(), m_gxSys.getViewCb(),
                         m_gxSys.getProjCb()};
  m_device.context()->VSSetConstantBuffers(0, 3, vsb);
  ID3D11Buffer* psb[] = {m_cbSurfaceColor.get(), m_cbLightPos.get(),
                         m_gxSys.getViewCb()};
  m_device.context()->PSSetConstantBuffers(0, 3, psb);

  UpdateBuffer(m_cbSurfaceColor, XMFLOAT4{1.0, 0.0, 0.0, 1});
  DrawMesh(m_cyliX, m_cyliXMtx);
  UpdateBuffer(m_cbSurfaceColor, XMFLOAT4{0.0, 1.0, 0.0, 1});
  DrawMesh(m_cyliY, m_cyliYMtx);
  UpdateBuffer(m_cbSurfaceColor, XMFLOAT4{0.0, 0.0, 1.0, 1});
  DrawMesh(m_cyliZ, m_cyliZMtx);
}

void RChApp::updateParticlesPos(Array3D<SimParticle, 4, 4, 4>& arr) {
  for (std::size_t i = 0; i < m_springPts.dim3(); ++i) {
    for (std::size_t j = 0; j < m_springPts.dim2(); ++j) {
      for (std::size_t k = 0; k < m_springPts.dim1(); ++k) {
        const auto& pt = m_springPts.val(i, j, k);
        arr.assign({pt->getCenterPos(), {0, 0, 0}}, i, j, k);
      }
    }
  }
}

void RChApp::updatePtsPos(const Array3D<SimParticle, 4, 4, 4>& particles) {
  for (std::size_t i = 0; i < m_springPts.size(); ++i) {
    const auto& simParticle = particles.valLin(i);
    auto& pt = m_springPts.objLin(i);
    pt->setPos(simParticle.pos);
  }
}

void RChApp::reshapeToCube(Array3D<SimParticle, 4, 4, 4>& particles, float side = 1.f) {
  for (std::size_t i = 0; i < particles.dim3(); ++i) {
    for (std::size_t j = 0; j < particles.dim2(); ++j) {
      for (std::size_t k = 0; k < particles.dim1(); ++k) {
        auto newPos =
            side * Vec3{(i / 3.f - 0.5f), (j / 3.f - 0.5f), (k / 3.f - 0.5f)};
        particles.assign({newPos, {0, 0, 0}}, i, j, k);
      }
    }
  }
}

Array3D<SimParticle, 4, 4, 4> RChApp::genParticlesCube(float side = 1.f) {
  Array3D<SimParticle, 4, 4, 4> ps; 
  for (std::size_t i = 0; i < ps.dim3(); ++i) {
    for (std::size_t j = 0; j < ps.dim2(); ++j) {
      for (std::size_t k = 0; k < ps.dim1(); ++k) {
        auto newPos =
            side * Vec3{(i / 3.f - 0.5f), (j / 3.f - 0.5f), (k / 3.f - 0.5f)};
        ps.assign({newPos, {0, 0, 0}}, i, j, k);
      }
    }
  }
  return ps;
}

void RChApp::initJelly() {
  createSpringPts();
  createSpringGrid();
  createJellyPatches();
}

void RChApp::createSpringPts() {
  float param = 2.0f;
  for (std::size_t i = 0; i < m_springPts.dim3(); ++i) {
    for (std::size_t j = 0; j < m_springPts.dim2(); ++j) {
      for (std::size_t k = 0; k < m_springPts.dim1(); ++k) {
        param -= 0.01f;

        auto pt = std::make_shared<Point>(m_gxSys);
        pt->setPos(param *
                   Vec3{(i / 3.f - 0.1f), (j / 3.f - 0.2f), (k / 3.f - 0.3f)});

        m_springPts.assign(pt, i, j, k);
      }
    }
  }
}

void RChApp::createSpringGrid() {
  std::size_t ctr = 0;

  // Horizontal (over first dim)
  for (std::size_t i = 0; i < 4; i++) {
    for (std::size_t j = 0; j < 4; j++) {
      auto v1 = subsetOf(m_springPts, {i, j, 0}, {i, j, 3});
      m_springGrid[ctr] = std::make_shared<LineStrip3D_Points>(m_gxSys, XMFLOAT4{0.0f, 1.f, 2.f, 1.f});
      m_springGrid[ctr]->setPoints(std::move(v1));
      m_sobjs.push_back(m_springGrid[ctr]);

      ctr++;
    }
  }

  // Vertical (over first dim)
  for (std::size_t i = 0; i < 4; i++) {
    for (std::size_t j = 0; j < 4; j++) {
      std::vector<std::shared_ptr<Point>> vec = {
          m_springPts.val(i, 0, j),
          m_springPts.val(i, 1, j),
          m_springPts.val(i, 2, j),
          m_springPts.val(i, 3, j),
      };

      m_springGrid[ctr] = std::make_shared<LineStrip3D_Points>(
          m_gxSys, XMFLOAT4{0.0f, 1.f, 2.f, 1.f});
      m_springGrid[ctr]->setPoints(std::move(vec));
      m_sobjs.push_back(m_springGrid[ctr]);

      ctr++;
    }
  }

  // Horizontal (over second dim)
  for (std::size_t i = 0; i < 4; i++) {
    for (std::size_t j = 0; j < 4; j++) {
      std::vector<std::shared_ptr<Point>> vec = {
          m_springPts.val(0, i, j),
          m_springPts.val(1, i, j),
          m_springPts.val(2, i, j),
          m_springPts.val(3, i, j),
      };

      m_springGrid[ctr] = std::make_shared<LineStrip3D_Points>(
          m_gxSys, XMFLOAT4{0.0f, 1.f, 2.f, 1.f});
      m_springGrid[ctr]->setPoints(std::move(vec));
      m_sobjs.push_back(m_springGrid[ctr]);

      ctr++;
    }
  }
}

// PTODO: one vector
void RChApp::createJellyPatches() {
  // LEFT
  m_facePatches[0] = std::make_shared<CompBezier3PatchC0>(m_gxSys);
  std::vector<std::shared_ptr<Point>> left;
  left.reserve(16);
  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = 0; j < 4; ++j) {
      left.emplace_back(m_springPts.val(0, j, i));
    }
  }
  m_facePatches[0]->init(left, 1, 1, BicubicPatch::WrapMode::None);

  // RIGHT
  m_facePatches[1] = std::make_shared<CompBezier3PatchC0>(m_gxSys);
  auto right = subsetOf(m_springPts, {3, 0, 0}, {3, 3, 3});
  m_facePatches[1]->init(right, 1, 1, BicubicPatch::WrapMode::None);


  // BACK (OZ+)
  m_facePatches[2] = std::make_shared<CompBezier3PatchC0>(m_gxSys);
  std::vector<std::shared_ptr<Point>> back;
  back.reserve(16);
  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = 0; j < 4; ++j) {
      back.emplace_back(m_springPts.val(i, j, 3));
    }
  }
  m_facePatches[2]->init(back, 1, 1, BicubicPatch::WrapMode::None);

  // FRONT
  m_facePatches[3] = std::make_shared<CompBezier3PatchC0>(m_gxSys);
  std::vector<std::shared_ptr<Point>> front;
  front.reserve(16);
  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = 0; j < 4; ++j) {
      front.emplace_back(m_springPts.val(j, i, 0));
    }
  }
  m_facePatches[3]->init(front, 1, 1, BicubicPatch::WrapMode::None);

  // DOWN
  m_facePatches[4] = std::make_shared<CompBezier3PatchC0>(m_gxSys);
  std::vector<std::shared_ptr<Point>> up;
  up.reserve(16);
  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = 0; j < 4; ++j) {
      up.emplace_back(m_springPts.val(i, 0, j));
    }
  }
  m_facePatches[4]->init(up, 1, 1, BicubicPatch::WrapMode::None);

  // UP
  m_facePatches[5] = std::make_shared<CompBezier3PatchC0>(m_gxSys);
  std::vector<std::shared_ptr<Point>> down;
  down.reserve(16);
  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = 0; j < 4; ++j) {
      down.emplace_back(m_springPts.val(3-i, 3, j));
    }
  }
  m_facePatches[5]->init(down, 1, 1, BicubicPatch::WrapMode::None);
}

void RChApp::displayGrid(bool val) {
  for (auto obj : m_sobjs) {
    if (obj->getType() == LineStrip3D_Points::getStaticType()) {
      auto ptr = std::static_pointer_cast<LineStrip3D_Points>(obj);
      ptr->setDisplayOn(val);
    }
  }
}

#pragma endregion

#pragma region Input Handling

bool RChApp::handleObjInput(double dt, MouseState& mstate,
                            KeyboardState& kbstate, std::shared_ptr<SceneObj> obj) {
  auto mDelt = mstate.getMousePositionChange();
  bool objChanged = false;

  if (mstate.isButtonDown(0)) {
    if (m_lockObjAxes) {
      if (kbstate.isKeyDown(DIK_LCONTROL)) {
        obj->translate(Vec3(0, 0, mDelt.x * TRANSLATION_SPEED));
      } else if (kbstate.isKeyDown(DIK_LALT)) {
        obj->translate(Vec3(0, mDelt.x * TRANSLATION_SPEED, 0));
      } else {
        obj->translate(Vec3(mDelt.x * TRANSLATION_SPEED, 0, 0));
      }
    } else {
      if (kbstate.isKeyDown(DIK_LCONTROL)) {
        obj->translate(Vec3(0, 0, mDelt.x * TRANSLATION_SPEED));
      } else {
        obj->translate(
            Vec3(mDelt.x * TRANSLATION_SPEED, -mDelt.y * TRANSLATION_SPEED, 0));
      }
    }

    objChanged = true;
  } else if (mstate.isButtonDown(1)) {
    if (kbstate.isKeyDown(DIK_LCONTROL)) {
      obj->rotateZ(((double)-mDelt.x) * ROTATION_SPEED);
    } else if (kbstate.isKeyDown(DIK_LALT)) {
      obj->rotateY(((double)-mDelt.x) * ROTATION_SPEED);
    } else {
      obj->rotateX(((double)-mDelt.x) * ROTATION_SPEED);
    }
    objChanged = true;
  }

  return objChanged;
}

void RChApp::handleGlobalInput(double dt, MouseState& mstate,
                               KeyboardState& kbstate) {

  if (kbstate.isKeyDown(DIK_J)) {
    if (m_selectedObj.expired()) {
      m_selectedObj = m_ctrlFrame;
    } else {
      m_selectedObj.reset();
    }
  }
  if (kbstate.isKeyDown(DIK_K)) {
    m_simulation.core().switchFrameActive();
  }
  if (kbstate.isKeyDown(DIK_L)) {
    m_lockObjAxes = !m_lockObjAxes;
  }

  if (kbstate.isKeyDown(DIK_W)) {
    m_camera.moveCam(dt * CAM_T_SPEED * m_camera.getForwardXZDir());
  }
  if (kbstate.isKeyDown(DIK_S)) {
    m_camera.moveCam(dt * CAM_T_SPEED * ((-1.0f) * m_camera.getForwardXZDir()));
  }
  if (kbstate.isKeyDown(DIK_A)) {
    m_camera.moveCam(dt * CAM_T_SPEED * ((-1.0f) * m_camera.getRightXZDir()));
  }
  if (kbstate.isKeyDown(DIK_D)) {
    m_camera.moveCam(dt * CAM_T_SPEED * m_camera.getRightXZDir());
  }
  if (kbstate.isKeyDown(DIK_1)) {
    m_camera.moveCam(dt * CAM_T_SPEED * XMVECTOR({0.0f, -1.0f, 0.0f}));
  }
  if (kbstate.isKeyDown(DIK_3)) {
    m_camera.moveCam(dt * CAM_T_SPEED * XMVECTOR({0.0f, 1.0f, 0.0f}));
  }
}

bool RChApp::HandleCameraInput(double dt, MouseState& mstate) {
  bool res = false;

  // Handle mouse inputs
  auto mousePos = mstate.getMousePositionChange();

  if (mstate.isButtonDown(0)) {
    m_camera.Rotate(mousePos.y * ROTATION_SPEED, mousePos.x * ROTATION_SPEED);
    res = true;
  } else if (mstate.isButtonDown(1)) {
    m_camera.Zoom(mousePos.y * ZOOM_SPEED);
  }
  
  return res;
}

#pragma endregion

#pragma region DX11 Utils

void RChApp::SetSurfaceColor(DirectX::XMFLOAT4 color) {
  UpdateBuffer(m_cbSurfaceColor, color);
}

void RChApp::UpdateCameraCB(XMMATRIX viewMtx) {
  XMVECTOR det;
  XMMATRIX invViewMtx = XMMatrixInverse(&det, viewMtx);
  XMFLOAT4X4 view[2];
  XMStoreFloat4x4(view, viewMtx);
  XMStoreFloat4x4(view + 1, invViewMtx);
  UpdateBuffer(m_cbViewMtx, view);
  UpdateBuffer(m_cbCamPos, m_camera.getCameraPosition());
}

void RChApp::SetWorldMtx(DirectX::XMFLOAT4X4 mtx) {
  UpdateBuffer(m_cbWorldMtx, mtx);
}

void RChApp::SetWorldMtx(const XMMATRIX& mtx) {
  DirectX::XMFLOAT4X4 aux;
  XMStoreFloat4x4(&aux, mtx);
  UpdateBuffer(m_cbWorldMtx, aux);
}

void RChApp::SetShaders(const dx_ptr<ID3D11VertexShader>& vs,
                        const dx_ptr<ID3D11PixelShader>& ps) {
  m_device.context()->VSSetShader(vs.get(), nullptr, 0);
  m_device.context()->PSSetShader(ps.get(), nullptr, 0);
}

void RChApp::SetTextures(
    std::initializer_list<ID3D11ShaderResourceView*> resList,
    const dx_ptr<ID3D11SamplerState>& sampler) {
  m_device.context()->PSSetShaderResources(0, static_cast<UINT>(resList.size()), resList.begin());
  auto s_ptr = sampler.get();
  m_device.context()->PSSetSamplers(0, 1, &s_ptr);
}

void RChApp::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx) {
  SetWorldMtx(worldMtx);
  m.Render(m_device.context());
}

#pragma endregion

}  // namespace rch