#pragma once
//#include <d3d11.h>
//
//#include "dxDevice.h"
//#include "dxptr.h"
//
//#include "SceneObj.h"
//
//namespace rch {
//
//class SimCube : public SceneObj {
// public:
//  SimCube(mini::DxDevice& device);
//  virtual ~SimCube() = default;
//
//  void update(double dtime = 0.0) override;
//
//  // Disabled: use specific renderXYZ functions 
//  void render() override{};
//
//  void renderCube();
//  void renderDiag();
//  void renderTraj();
//
//  XMMATRIX getModelMtx() const override;
//
//  XMVECTOR getOrientation() const { return m_orientation; };
//  void setOrientation(XMVECTOR qnion) { m_orientation = qnion; };
//
//  void showTraj(bool val) { m_showTraj = val; }
//  void setTrajLen(std::size_t size);
//  void logTrajPos();
//
//  float getSideLen() const { return m_sideLen; };
//  void  setSideLen(float len);
//
//  float getDensity() const { return m_rho; };
//  void setDensity(float density);
//
//  void showDiagonal(bool value) { m_showDiag = value; };
//  void showCube(bool value) { m_showCube = value; };
//
// private:
//  // ----------- Functions -----------
//  void generateMesh();
//
//  // ----------- Variables -----------
//  bool m_meshReady = false;
//
//  // Cube Gx
//  bool m_showCube = true;
//  mini::dx_ptr_vector<ID3D11Buffer> m_vertBuff;
//  mini::dx_ptr<ID3D11Buffer> m_idxBuff;
//  unsigned int m_idxNum = 0;
//
//  // The main diagonal Gx
//  bool m_showDiag = false;
//  mini::dx_ptr_vector<ID3D11Buffer> m_diagVertBuff;
//  static constexpr unsigned int m_diagVertNum = 2;
//
//  // SimCube attributes
//  float m_sideLen = 1.0f;
//  float m_rho = 1.0f;
//  XMVECTOR m_orientation;
//
//  // Trajectory
//  bool m_showTraj = false;
//  std::vector<XMFLOAT3> m_trajBuff;
//  mini::dx_ptr_vector<ID3D11Buffer> m_trajVertBuff;
//  std::size_t m_trajVertNum = 0;
//};
//
//}  // namespace rch