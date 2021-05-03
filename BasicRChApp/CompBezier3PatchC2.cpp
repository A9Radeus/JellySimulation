#include "CompBezier3PatchC2.h"

namespace rch {

//TODO: move this and the curveC2 version to maths 3D @see curveC2
// Ptodo:
// std::vector<Vec3> deBoorToBernstein3(const std::vector<Point>& ctrlPts) {}
std::vector<Vec3> deBoorToBernstein3(const std::vector<Vec3>& ctrlPts) {
  static constexpr short s_segmSize = 4;
  assert(ctrlPts.size() >= s_segmSize);
  const short segmNum = (ctrlPts.size() - s_segmSize) + 1;

  // It's 4 Bernstein pts per segment, but the knot-point
  // between every two segments is always shared
  std::vector<Vec3> res(segmNum * 3 + 1);

  // Uniform approach, thus Greville abscisae's
  // values are identical to knots' values
  float knots[4] = {0, 1, 2, 3};
  float grevAbsci[4] = {0, 1, 2, 3};

  Vec3 deb0 = (ctrlPts.at(0));
  Vec3 deb1 = (ctrlPts.at(0 + 1));
  Vec3 deb2 = (ctrlPts.at(0 + 2));
  Vec3 deb3;

  // Required Bernstein Internal Abscissae
  float bernAbsci[4];
  bernAbsci[0] = knots[0] + 2.0f * ((knots[1] - knots[0]) / 3.0f);
  bernAbsci[1] = knots[1] + ((knots[2] - knots[1]) / 3.0f);
  bernAbsci[3] = knots[2] + ((knots[3] - knots[2]) / 3.0f);

  // The first bernstein coordinate (b0) of the first segment
  Vec3 innerBern[4];
  Vec3 outerBern1, outerBern2;
  innerBern[0] = rch::lerp(
      deb0, deb1, fracOfSegmCl(grevAbsci[0], grevAbsci[1], bernAbsci[0])),
  innerBern[1] = rch::lerp(
      deb1, deb2, fracOfSegmCl(grevAbsci[1], grevAbsci[2], bernAbsci[1]));
  outerBern1 =
      rch::lerp(innerBern[0], innerBern[1],
                fracOfSegmCl(bernAbsci[0], bernAbsci[1], grevAbsci[1]));
  res[0] = (outerBern1);

  // Calculating {b1, b2, b3} coordinates of every succeeding
  // segment. Coord b0 is taken from the most recent segment.
  for (int i = 0; i < segmNum; i++) {
    // Since "outerBern1" coordinate is already in the vector
    // deb0 and grevAbsci[0] is not required for further calculations
    deb1 = (ctrlPts.at(i + 1));
    deb2 = (ctrlPts.at(i + 2));
    deb3 = (ctrlPts.at(i + 3));
    grevAbsci[1] = knots[1] = i + 1;
    grevAbsci[2] = knots[2] = i + 2;
    grevAbsci[3] = knots[3] = i + 3;

    // Bernstein Internal Abscissae
    bernAbsci[2] = knots[1] + (2.0f * ((knots[2] - knots[1]) / 3.0f));
    bernAbsci[3] = knots[2] + ((knots[3] - knots[2]) / 3.0f);

    // Remaining inner bernstein coordinates
    innerBern[2] = rch::lerp(
        deb1, deb2, fracOfSegmCl(grevAbsci[1], grevAbsci[2], bernAbsci[2]));
    innerBern[3] = rch::lerp(
        deb2, deb3, fracOfSegmCl(grevAbsci[2], grevAbsci[3], bernAbsci[3]));

    // Remaining outer bernstein coordinate. The first "outerBern"
    // is already in the "res" vector
    outerBern2 =
        rch::lerp(innerBern[2], innerBern[3],
                  fracOfSegmCl(bernAbsci[2], bernAbsci[3], grevAbsci[2]));

    res[3 * i + 1] = (innerBern[1]);
    res[3 * i + 2] = (innerBern[2]);
    res[3 * i + 3] = (outerBern2);

    // Update innerBern[3] of this segment
    // is the innerBern[1] of the next segment
    innerBern[1] = innerBern[3];
  }

  return res;
}

// TODO: shared_ptr
//void CompBezier3PatchC2::init(std::vector<Point*>&& ctrls, Uint across,
//                              Uint lenwise, WrapMode wrap) {
//  m_patAcross = across;
//  m_patLenwise = lenwise;
//  m_wrMode = wrap;
//  initIndexBuffer(wrap);
//
//  initCtrlPtsView(ctrls, wrap);
//  m_concretePts = std::move(ctrls);
//
//  m_initialised = true;
//}

// TODO Handle the other wrap modes
void CompBezier3PatchC2::initCtrlPtsView(std::vector<Point*> pts, WrapMode mode) {
  Uint columns = s_vsPerSide + m_patAcross - 1;
  Uint rows = s_vsPerSide + m_patLenwise - 1;
  Uint cptsNum = columns * rows;
  Uint vcols = columns;
  //Uint vrows = rows;
  switch (mode) {
    case WrapMode::Left:
    case WrapMode::Right:
      vcols -= 3;
      break;
    case WrapMode::Top:
    case WrapMode::Bottom:
      //vrows -= 3;
      break;
    default:
      break;
  }

  const Uint lastColIdx = vcols - 1;
  m_cpts.reserve(cptsNum);
  for (int i = 0; i < rows; i++) {
    if (mode == WrapMode::Left) {
      // Three last points of this row
      m_cpts.emplace_back(pts[lastColIdx + i * vcols]);
      m_cpts.emplace_back(pts[(lastColIdx - 1) + i * vcols]);
      m_cpts.emplace_back(pts[(lastColIdx - 2) + i * vcols]);
    } 
    for (int j = 0; j < vcols; j++) {
      m_cpts.emplace_back(pts[j + i * vcols]);
    }
    if (mode == WrapMode::Right) {
      // three first points of this row
      m_cpts.emplace_back(pts[0 + i * vcols]);
      m_cpts.emplace_back(pts[1 + i * vcols]);
      m_cpts.emplace_back(pts[2 + i * vcols]);
    }
  }
}

/**
 * Converts the control points (DeBoor points) to Bernstein coords
 * and stores these in the vertex buffer (row major).
 *
 * NOTE: Convertion in shaders TODO why was a bad option
 */
void CompBezier3PatchC2::genPatchVerts() {
  auto ptsNum = m_cpts.size();

  Uint columns = s_vsPerSide + m_patAcross - 1;
  Uint rows = s_vsPerSide + m_patLenwise - 1;

  static constexpr auto numOfBezi3 = [](Uint deBoors) {
    return 4 + (deBoors - 4) * 3;
  };

  ////////////////////////////////////////////////////////////////////
  // Generate Bezier control points (bernstein coordinates) from
  // the DeBoor points (b-spline coordinates).
  //
  // Firstly convertes every column of DeBoor coordinates to Bernstein-
  // intermediate coordinates (grouped in columns. Afterwards, from
  // the resulting columns rows are created and to these Bernstein-inte-
  // rmediate coords (grouped in rows) the same algoritm is applied,
  // i.e. these intermediate Bernstein coords are interpreted as DeBoor
  // points for the second dimension).
  /////////////////////////////////////////////////////////////////////

  std::vector<std::vector<Vec3>> intermBeziCols(numOfBezi3(columns));
  std::vector<std::vector<Vec3>> intermBeziRows(numOfBezi3(rows));
  std::vector<std::vector<Vec3>> deBoorCols(columns);

  // Gather column vectors
  for (int i = 0; i < columns; i++) {
    deBoorCols[i].reserve(rows);
    for (int j = 0; j < rows; j++) {
      int idx1 = i + j * columns;  // ~ j * vertsPerRow
      Vec3 pt = m_cpts.at(idx1)->getCenterPos();
      auto& col = (deBoorCols[i]);
      col.push_back(pt);
    }
  }

  for (int i = 0; i < columns; i++) {
    intermBeziCols[i] = deBoorToBernstein3(deBoorCols[i]);
  }

  auto initRow = [&columns, &intermBeziRows, &intermBeziCols](Uint rowIdx) {
    for (int i = 0; i < columns; i++) {
      intermBeziRows[rowIdx].push_back((intermBeziCols[i])[rowIdx]);
    }
  };

  ////////////////////////////////////////////////////////////////////
  // Vertex Buffer:
  // Since it consists of Bezier control points, in case of
  // a cylindical patch, the last vertex of every row (column
  // slice) is at the same position as the first vertex of this row.
  /////////////////////////////////////////////////////////////////////
  int mergedCols = 0;
  if (m_wrMode == WrapMode::Right) {
    mergedCols = 1;
  }
  const Uint beziRows = (4 + (rows - 4) * 3);
  const Uint beziCols = (4 + (columns - 4) * 3) - mergedCols;

  std::vector<XMFLOAT3> verts(beziRows * beziCols);
  for (int i = 0, ctr = 0; i < beziRows; i++) {
    initRow(i);
    auto auxRow = deBoorToBernstein3(intermBeziRows[i]);
    for (int j = 0; j < beziCols; j++) {  // todo // skips the last one
      const auto& pt = auxRow[j];
      verts[ctr++] = XMFLOAT3(pt.x, pt.y, pt.z);
    }
  }

  // Update the Vertex Buffer 
  // The Index buffer is constant
  m_vertBuff.clear();
  m_vertBuff.push_back(m_dev.CreateVertexBuffer(verts));
  m_vertsNum = verts.size();
  m_meshReady = true;
}

///// TODO zmiana sposobu dzialania klasy, wiec sprawdzic 
// opis itp.
/**
 * Generates control points (DeBoor points) for the cylindrical 
 * version of the patch for given radius and height.
 * The amount of columns it generates is the amount required 
 * for rectangular patch MINUS 3, which is implied by the 
 * wrapping around of last 3 vertices of every column which
 * results in these having the same positions as 3 first verti-
 * ces of the same columns. @See TODO.
 */
std::vector<Vec3> CompBezier3PatchC2::genCyliVertsY_R(float radius,
                                                     float height, Uint across,
                                                     Uint lenwise) {
  Uint cols = s_vsPerSide + (across - 1);
  Uint rows = s_vsPerSide + (lenwise - 1);
  Uint c_cols = cols - 3; // unique column positions on the circle

  // TODO: Find the right factor. It varies depending on
  // the amount of atomic patches across, since the first
  // and last segment get smaller the more atomic patches
  // there is on the same space.
  const auto calcXZ = [&radius](float t) {
    return Vec3(1.333f * radius * std::cosf(t), 0.0f,
                1.333f * radius * std::sinf(t));
  };

  std::vector<Vec3> verts(c_cols * rows);
  float stepXZ = (XM_2PI) / (float)(c_cols);
  // This is: 1) minus first and last segment of every column 
  // since they dont contribute to the height (ON CONDITION
  // that vertices as evenly spaced like so) 2) minus 1 since
  // N vertes create N-1 segments.
  float stepY = height / (float)(rows - 1 - 2);  
  for (float i = 0, ctr = 0; i < rows; i++) {
    for (float j = 0; j < c_cols; j++) {         
      verts[ctr++] = calcXZ(stepXZ * j) + Vec3(0, stepY * i, 0);
    }
  }

  return verts;
}

// @See genCyliVertsY_R
std::vector<Vec3> CompBezier3PatchC2::genCyliVertsX_R(float radius, float height,
                                                     Uint across,
                                                     Uint lenwise) {
  Uint cols = s_vsPerSide + (across - 1);
  Uint rows = s_vsPerSide + (lenwise - 1);
  Uint c_cols = cols - 3;  // unique column positions on the circle

  const auto calcYZ = [&radius](float t) {
    return Vec3(0.0f, 1.333f * radius * std::cosf(t),
                1.333f * radius * std::sinf(t));
  };

  std::vector<Vec3> verts(c_cols * rows);
  float stepYZ = (XM_2PI) / (float)(c_cols);
  float stepX = height / (float)(rows - 1 - 2);
  for (float i = 0, ctr = 0; i < rows; i++) {
    for (float j = 0; j < c_cols; j++) {
      verts[ctr++] = calcYZ(stepYZ * j) + Vec3(stepX * i, 0, 0);
    }
  }

  return verts;
}

/**
 * Generates control points (DeBoor points) for the rectangular 
 * version of the patch for given length and width. 
 * 
 * Equation for stepX/Z (= distance between 2 given DeBoor points
 * in world coords) stems from the fact that, when the patch is flat
 * every vertex, MINUS the border vertices of the ENTIRE composite
 * patch, lay on the patch @See the function's body.
 * 
 * Uses row-major order.
 */
std::vector<Vec3> CompBezier3PatchC2::genRectVerts(float width, float length,
                                                      Uint across,
                                                      Uint lenwise) {
  Uint rows = s_vsPerSide +  (across - 1);
  Uint cols = s_vsPerSide + (lenwise - 1);

  std::vector<Vec3> verts(rows * cols);
  // Minus first and last vertex in the row/column (= -2)
  // and -1 since N-verts create N-1 segments.
  float stepX = width / (float)(rows - 2 - 1);
  float stepZ = length / (float)(cols -2 - 1);
  for (int i = 0, ctr = 0; i < cols; i++) {  // OZ
    for (int j = 0; j < rows; j++) {         // OX (first)
      verts[ctr] = {(float)j * stepX, 0.0f, (float)i * stepZ};
      ctr++;
    }
  }

  return verts;
}

void CompBezier3PatchC2::genPolygonVerts() {
  if (m_initialised == false) {
    return;
  }

  int ptsNum = m_concretePts.size();
  Uint columns = s_vsPerSide + (m_patAcross - 1);
  Uint rows = s_vsPerSide + (m_patLenwise - 1);
  if (m_wrMode == WrapMode::Left || m_wrMode == WrapMode::Right) {
    columns -= 3;
  }
  // PTODO:
  // if (m_wrMode == WrapMode::Top || m_wrMode == WrapMode::Bottom) {
  //  rows -= 3;
  //}

  // 2 sets of lines and x2 since LINELIST is being used
  // For "- 2*(rows + columns)" @see inner loops' iterator range
  std::size_t vertsSize = (2 * 2 * columns * rows - (2 * (rows + columns)));
  // additional set of lines for the circle part
  // to wrap them around
  if (m_wrMode == WrapMode::Left || m_wrMode == WrapMode::Right) {
    vertsSize += 2 * 2 * columns; 
  }
  std::vector<XMFLOAT3> verts;
  verts.reserve(vertsSize);

  // Lines parallel to OX-axis
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns-1; j++) {
      int idx1 = (i * columns) + j + 0;
      int idx2 = (i * columns) + j + 1;
      auto pt1 = (m_concretePts.at(idx1))->getCenterPos();
      auto pt2 = (m_concretePts.at(idx2))->getCenterPos();
      verts.emplace_back((float)pt1.x, (float)pt1.y, (float)pt1.z);
      verts.emplace_back((float)pt2.x, (float)pt2.y, (float)pt2.z);
    }
    // wrapping around:
    if (m_wrMode == WrapMode::Left || m_wrMode == WrapMode::Right) {
      int idx1 = (i * columns) + (columns - 1) + 0;
      int idx2 = (i * columns);
      auto pt1 = (m_concretePts.at(idx1))->getCenterPos();
      auto pt2 = (m_concretePts.at(idx2))->getCenterPos();
      verts.emplace_back((float)pt1.x, (float)pt1.y, (float)pt1.z);
      verts.emplace_back((float)pt2.x, (float)pt2.y, (float)pt2.z);
    }
  }

  // Lines parallel to OZ-axis
  for (int i = 0; i < columns; i++) {
    for (int j = 0; j < (rows - 1); j++) {
      int idx1 = i + (j + 0) * columns;
      int idx2 = i + (j + 1) * columns;
      auto pt1 = (m_concretePts.at(idx1))->getCenterPos();
      auto pt2 = (m_concretePts.at(idx2))->getCenterPos();
      verts.emplace_back((float)pt1.x, (float)pt1.y, (float)pt1.z);
      verts.emplace_back((float)pt2.x, (float)pt2.y, (float)pt2.z);
    }
  }

  m_polylineVertBuff.clear();
  m_polylineVertBuff.push_back(m_dev.CreateVertexBuffer(verts));
  m_polylineVertsNum = verts.size();
  m_polylineReady = true;
}


}  // namespace rch