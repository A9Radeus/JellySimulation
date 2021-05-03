#include "Point.h"

namespace rch {

void Point::update(double dtime) { }

void Point::render() {
  m_gxSys.m_pointRT.bind(m_gxSys); // todo
  m_gxSys.m_pointRT.setModelCb(getModelMtx());

  constexpr unsigned int strides = sizeof(XMFLOAT3);
  constexpr unsigned int offset = 0;
  m_dev.context()->IASetVertexBuffers(0, m_vertBuff.size(),
                                         m_vertBuff.data(), &strides, &offset);
  m_dev.context()->Draw(m_vertsNum, 0);

  m_gxSys.m_pointRT.unbind();
}

DirectX::XMMATRIX Point::getModelMtx() const {
  //return rch::gen_translation_mtx(m_pos.x, m_pos.y, m_pos.z) *
  //       rch::gen_scale_mtx(m_scaling.x, m_scaling.y, m_scaling.z);
  return rch::gen_translation_mtx(m_pos.x, m_pos.y, m_pos.z);
  
}

}  // namespace rch