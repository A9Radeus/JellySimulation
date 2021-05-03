//#include "SimCube.h"
//#include "Mesh.h"
//#include "vertexTypes.h"
//
//namespace rch {
//
//
//SimCube::SimCube(mini::DxDevice& device) 
//    : SceneObj(device) {
//  auto inds = mini::Mesh::BoxIdxs();
//  m_idxNum = inds.size();
//  m_idxBuff = m_dev.CreateIndexBuffer(inds);
//  m_orientation = rch::get_rotation_between({0, 1, 0}, {1, 1, 1});
//}
//
//void SimCube::update(double dtime) {
//  if (m_meshReady == false) {
//    generateMesh();
//  }
//}
//
//XMMATRIX SimCube::getModelMtx() const { 
//  auto qmtxAround = quaternionToMtx(m_orientation);
//  return qmtxAround;
//}
//
//void SimCube::setTrajLen(std::size_t size) {
//  m_trajBuff.clear();
//  m_trajBuff.reserve(size);
//  m_trajVertNum = 0;
//}
//
//void SimCube::logTrajPos() {
//  m_trajVertNum += 1;
//  auto diagpt = XMFLOAT4{m_sideLen, m_sideLen, m_sideLen, 0};
//  XMVECTOR vPt = XMLoadFloat4(&diagpt);
//  auto resVPt = XMVector4Transform(vPt, getModelMtx());
//
//  XMFLOAT3 aux;
//  XMStoreFloat3(&aux, resVPt);
//  m_trajBuff.push_back(aux);
//
//  m_trajVertBuff.clear();
//  m_trajVertBuff.push_back(m_dev.CreateVertexBuffer(m_trajBuff));
//}
//
//void SimCube::setSideLen(float len) {
//  m_sideLen = len;
//  m_meshReady = false;
//}
//
//void SimCube::setDensity(float density) {
//  m_rho = density;
//  m_meshReady = false;
//}
//
//void SimCube::renderCube() {
//  if (m_meshReady == false || m_showCube == false) {
//    return;
//  }
//  unsigned int strides = sizeof(mini::VertexPositionNormal);
//  unsigned int offset = 0;
//  m_dev.context()->IASetPrimitiveTopology(
//      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//  m_dev.context()->IASetIndexBuffer(m_idxBuff.get(), DXGI_FORMAT_R16_UINT, 0);
//  m_dev.context()->IASetVertexBuffers(0, m_vertBuff.size(), m_vertBuff.data(),
//                                      &strides, &offset);
//  m_dev.context()->DrawIndexed(m_idxNum, 0, 0);
//}
//
//void SimCube::renderDiag() {
//  if (m_meshReady == false || m_showDiag == false) {
//    return;
//  }
//  unsigned int strides = sizeof(mini::VertexPositionNormal);
//  unsigned int offset = 0;
//  m_dev.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
//  m_dev.context()->IASetVertexBuffers(
//      0, m_diagVertBuff.size(), m_diagVertBuff.data(), &strides, &offset);
//  m_dev.context()->Draw(m_diagVertNum, 0);
//  
//}
//
//void SimCube::renderTraj() {
//  if (m_meshReady == false || m_showTraj == false) {
//    return;
//  }
//  unsigned int strides = sizeof(XMFLOAT3);
//  unsigned int offset = 0;
//  m_dev.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
//  m_dev.context()->IASetVertexBuffers(0, m_trajVertBuff.size(),
//                                      m_trajVertBuff.data(), &strides, &offset);
//  m_dev.context()->Draw(m_trajVertNum, 0);
//}
//
//void SimCube::generateMesh() {
//  auto verts =
//      mini::Mesh::ShadedBoxVertsCorner(m_sideLen, m_sideLen, m_sideLen);
//
//  m_vertBuff.clear();
//  m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));
//
//  std::vector<mini::VertexPositionNormal> diagVerts(2);
//  diagVerts[0] = verts[0];
//  diagVerts[1] = verts[7];
//
//  m_diagVertBuff.clear();
//  m_diagVertBuff.push_back(m_dev.CreateVertexBuffer(diagVerts));
//
//  m_meshReady = true;
//}
//
//}  // namespace rch