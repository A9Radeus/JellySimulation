#include "CompBezier3PatchC0.h"
#include "Point.h"

namespace rch {

void CompBezier3PatchC0::update(double dtime) {
  // Skip point checking if mesh has to be generated anyway
  bool ptsChanged = false;
  if (m_meshReady != false) {
    ptsChanged = std::any_of(m_cpts.begin(), m_cpts.end(),
                             [](std::shared_ptr<Point> pt) { return pt->getPrsChanged(); });
  }

  if (m_meshReady == false || ptsChanged == true) {
    //updateRequiredVerts();
    genPatchVerts();
    if (m_showPolyline == true) {
      genPolygonVerts();
    }
  }

  // User may enable the polyline without changing the points
  if (m_showPolyline == true && m_polylineReady == false) {
    genPolygonVerts();
  }
}

void CompBezier3PatchC0::render() {
  if (m_initialised == false) {
    throw "[CompBezier3PatchC2::render] m_initialised was false.";
  }

  if (m_meshReady == false) {
    return;
  }

  m_gxSys.m_patchRT.bind(m_gxSys);  // todo
  m_gxSys.m_patchRT.setModelCb(getModelMtx());

  unsigned int strides = sizeof(XMFLOAT3);
  unsigned int offset = 0;
  m_dev.context()->IASetIndexBuffer(m_idxBuff.get(), DXGI_FORMAT_R16_UINT,
                                       0);
  m_dev.context()->IASetVertexBuffers(0, m_vertBuff.size(),
                                         m_vertBuff.data(), &strides, &offset);
  m_dev.context()->DrawIndexed(m_idxNum, 0, 0);

  m_gxSys.m_patchRT.unbind();
}

void CompBezier3PatchC0::init(PointPtrs&& ctrls, Uint across,
                              Uint lenwise, BicubicPatch::WrapMode wrap) {
  m_patAcross = across;
  m_patLenwise = lenwise;
  m_wrMode = wrap;
  initIndexBuffer(wrap);                                 
  
  m_cpts = std::move(ctrls);
  //m_vpSys.clearAssign_Scene(this, std::move(ctrls));
  m_initialised = true;
}

void CompBezier3PatchC0::init(const PointPtrs& ctrls, Uint across,
                              Uint lenwise, BicubicPatch::WrapMode wrap) {
  m_patAcross = across;
  m_patLenwise = lenwise;
  m_wrMode = wrap;
  initIndexBuffer(wrap);                                 
  
  m_cpts = ctrls;
  //m_vpSys.clearAssign_Scene(this, std::move(ctrls));
  m_initialised = true;
}

void CompBezier3PatchC0::renderPolyline() {
  if (m_showPolyline == false) {
    return;
  }

  if (m_initialised == false) {
    throw "[CompBezier3PatchC0::renderPolyline] m_initialised was false.";
  }
  if (m_polylineReady == false || m_polylineVertsNum <= 0) {
    return;
  }
  unsigned int strides = sizeof(XMFLOAT3);
  unsigned int offset = 0;
  m_dev.context()->IASetVertexBuffers(0, m_polylineVertBuff.size(),
                                         m_polylineVertBuff.data(), &strides,
                                         &offset);
  m_dev.context()->Draw(m_polylineVertsNum, 0);
}

void CompBezier3PatchC0::genPatchVerts() {
  auto ptsNum = m_cpts.size();

  // Vertex Buffer
  std::vector<XMFLOAT3> verts(ptsNum);
  for (int i = 0; i < ptsNum; i++) {
    const auto& pt = (m_cpts[i])->getCenterPos();
    verts[i] = XMFLOAT3(pt.x, pt.y, pt.z);
  }
  
  // Update the VERTEX buffer
  // Index buffer should will have been updated by now
  // through the initIndexBuffer() function
  m_vertBuff.clear();
  m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));
  m_vertsNum = verts.size();
  m_meshReady = true;
}

/**
 * Generates a set of positions (Vec3) for future control points
 * of the cylindrical-patch version. "_R" version of this func-
 * tion that uses "right-column wrapping".
 *
 * The height increases in the direction of positive OY-axis.
 * The base circle is described in the XZ-plane.
 * Verts in the resulting container are stored row-major, i.e.:
 *    First "acrNum" of verts in the result are vertices of the
 *    very bottom of the cylinder.
 *
 *    Second "arcNum" of verts are describing the second circle
 *    at "stepY * 1" height.
 */
std::vector<Vec3> CompBezier3PatchC0::genCyliVertsY_R(float radius,
                                                            float height,
                                                            Uint across,
                                                            Uint lenwise){
  Uint cols = s_vsPerSide + (s_vsPerSide - 1) * (across - 1);
  Uint rows = s_vsPerSide + (s_vsPerSide - 1) * (lenwise - 1);

  const auto calcXZ = [&radius](float t) {
    return Vec3(radius * std::cosf(t), 0.0f, radius * std::sinf(t));
  };

  // Reserve cols-1 -- 1 column omitted
  std::vector<Vec3> verts((cols-1)*rows);
  float stepXZ = (XM_2PI) / (float)(cols - 1);
  float stepY = height / (float)(rows - 1);

  for (float i = 0, ctr = 0; i < rows; i += 1) {
    // The last vertex is omitted since it's
    // equal to the first one
    for (float j = 0; j < cols - 1; j += 1) {
      verts[ctr++] = calcXZ(stepXZ * j) + Vec3(0, stepY * i, 0);
    }
  }

  return verts;
};

// @See genCyliVertsY_R
std::vector<Vec3> CompBezier3PatchC0::genCyliVertsX_R(float radius,
                                                      float height, Uint across,
                                                      Uint lenwise) {
  Uint cols = s_vsPerSide + (s_vsPerSide - 1) * (across - 1);
  Uint rows = s_vsPerSide + (s_vsPerSide - 1) * (lenwise - 1);

  const auto calcYZ = [&radius](float t) {
    return Vec3(0.0f, radius * std::cosf(t), radius * std::sinf(t));
  };

  // Reserve cols-1 -- 1 column omitted
  std::vector<Vec3> verts(static_cast<std::size_t>(cols - 1) * rows);
  float stepXZ = (XM_2PI) / (float)(cols - 1);
  float stepX = height / (float)(rows - 1);

  for (float i = 0, ctr = 0; i < rows; i += 1) {
    // The last vertex is omitted since it's
    // equal to the first one
    for (float j = 0; j < cols - 1; j += 1) {
      verts[ctr++] = calcYZ(stepXZ * j) + Vec3(stepX * i, 0, 0);
    }
  }

  return verts;
};

/**
 * Generates a set of positions (Vec3) for future control points
 * of the rectangular-patch version.
 *
 * The verts are generated on the XZ-plane and are row-major
 * across the ENTIRE patch, i.e. for across = 2, lenwise = 1:
 *    verts[7] = the LAST vert of the FIRST row
 *               of the SECOND atomic patch.
 *    verts[8] = the FIRST vert of the SECOND row
 *               of the FIRST atomic patch.
 */
std::vector<Vec3> CompBezier3PatchC0::genRectVerts(float width, float length,
                                                   Uint across, Uint lenwise) {
  Uint acrNum = s_vsPerSide + (s_vsPerSide - 1) * (across - 1);
  Uint lenwNum = s_vsPerSide + (s_vsPerSide - 1) * (lenwise - 1);

  std::vector<Vec3> verts(static_cast<std::size_t>(acrNum) * lenwNum);
  float stepX = width / (float)(acrNum - 1);
  float stepZ = length / (float)(lenwNum - 1);
  for (int i = 0, ctr = 0; i < lenwNum; i++) {  // OZ
    for (int j = 0; j < acrNum; j++) {          // OX (first)
      verts[ctr] = {(float)j * stepX, 0.0f, (float)i * stepZ};
      ctr++;
    }
  }

  return verts;
};

// TODO: Top + Bottom wrap
// PTODO More thorough description
// Iterates over rows within every patch. Afterwards, iterates over all patches
// of the same row (i.e. row of ALL the atomic patches).
void CompBezier3PatchC0::initIndexBuffer(WrapMode wrap) {
  Uint indsNum = m_patAcross * m_patLenwise * s_patchSize;  
  // Vertices across (in one row of the entire vert array).
  Uint vsAcrossNum =
      s_vsPerSide + (s_vsPerSide - 1) * (m_patAcross - 1);
  // These wrap-modes imply that one column last (Right)
  // or first (Left) is going to be reused, thus "-1".
  if (wrap == WrapMode::Left || wrap == WrapMode::Right) {
    vsAcrossNum -= 1;
  }

  const int firstColIdx = 0;
  const int lastColIdx = m_patAcross - 1;
  std::vector<unsigned short> inds(indsNum);
  for (int i = 0, ctr = 0; i < m_patLenwise; i++) {
    for (int j = 0; j < m_patAcross; j++) {
      // this "for" iterates over rows (lengthwise) of a single patch:
      for (int k = 0; k < s_vsPerSide; k++) {
        if ((j == firstColIdx) && (wrap == WrapMode::Left)) {
          // last vertex of the last patch on THIS row:
          inds[ctr++] = ((k + (i * (s_vsPerSide - 1))) * vsAcrossNum) +
                        (lastColIdx * (s_vsPerSide - 1)) + 3;
        } else {
          inds[ctr++] = ((k + (i * (s_vsPerSide - 1))) * vsAcrossNum) +
                        (j * (s_vsPerSide - 1)) + 0;
        }

        inds[ctr++] = ((k + (i * (s_vsPerSide - 1))) * vsAcrossNum) +
                      (j * (s_vsPerSide - 1)) + 1;
        inds[ctr++] = ((k + (i * (s_vsPerSide - 1))) * vsAcrossNum) +
                      (j * (s_vsPerSide - 1)) + 2;

        if ((j == lastColIdx) && (wrap == WrapMode::Right)) {
          // first vertex of the first patch on THIS row:
          inds[ctr++] = ((k + (i * (s_vsPerSide - 1))) * vsAcrossNum);
        } else {
          inds[ctr++] = ((k + (i * (s_vsPerSide - 1))) * vsAcrossNum) +
                        (j * (s_vsPerSide - 1)) + 3;
        }
      }
    }
  }
  m_idxBuff = m_dev.CreateIndexBuffer(inds);
  m_idxNum = inds.size();
}

void CompBezier3PatchC0::genPolygonVerts() {
  if (m_initialised == false) {
    return;
  }
  
  int ptsNum = m_cpts.size();
  Uint columns = s_vsPerSide + (s_vsPerSide - 1) * (m_patAcross - 1);
  Uint rows = s_vsPerSide + (s_vsPerSide - 1) * (m_patLenwise - 1);
  if (m_wrMode == WrapMode::Left || m_wrMode == WrapMode::Right) {
    columns -= 1;
  }
  // PTODO:
  //if (m_wrMode == WrapMode::Top || m_wrMode == WrapMode::Bottom) {
  //  rows -= 1;
  //}

  // 2 sets of lines and x2 since LINELIST is being used
  // For "- 2*(rows + columns)" @see inner loops' iterator range
  std::vector<XMFLOAT3> verts(2 * 2 * columns * rows - 2*(rows + columns));
  Uint ctr = 0;

  // Lines parallel to OX-axis
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < (columns - 1); j++) {
      int idx1 = (i * columns) + j + 0;
      int idx2 = (i * columns) + j + 1;
      auto pt1 = (m_cpts.at(idx1))->getCenterPos();
      auto pt2 = (m_cpts.at(idx2))->getCenterPos();
      verts[ctr++] = {(float)pt1.x, (float)pt1.y, (float)pt1.z};
      verts[ctr++] = {(float)pt2.x, (float)pt2.y, (float)pt2.z};
    }
  }
  
  // Lines parallel to OZ-axis
  for (int i = 0; i < columns; i++) {
    for (int j = 0; j < (rows - 1); j++) {
      int idx1 = i + (j + 0)*columns;
      int idx2 = i + (j + 1)*columns;
      auto pt1 = (m_cpts.at(idx1))->getCenterPos();
      auto pt2 = (m_cpts.at(idx2))->getCenterPos();
      verts[ctr++] = {(float)pt1.x, (float)pt1.y, (float)pt1.z};
      verts[ctr++] = {(float)pt2.x, (float)pt2.y, (float)pt2.z};
    }
  }

  m_polylineVertBuff.clear();
  m_polylineVertBuff.push_back(m_dev.CreateVertexBuffer(verts));
  m_polylineVertsNum = verts.size();
  m_polylineReady = true;
}

}  // namespace rch