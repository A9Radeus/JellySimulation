#include "BicubicPatchBase.h"

namespace rch {

BicubicPatch::WrapMode toWrapDir(const char* name) {
  std::string nameStr = name;
  std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  BicubicPatch::WrapMode wrap = BicubicPatch::WrapMode::None;
  if (nameStr == "left") {
    wrap = BicubicPatch::WrapMode::Left;
  } else if (nameStr == "right" || (nameStr == "column")) {
    wrap = BicubicPatch::WrapMode::Right;
  } else if ((nameStr == "top") || (nameStr == "row")) {
    wrap = BicubicPatch::WrapMode::Top;
  } else if (nameStr == "bottom") {
    wrap = BicubicPatch::WrapMode::Bottom;
  }

  return wrap;
};

void BicubicPatch::setInsideTessFtr(float inside) {
  std::clamp(inside, s_tessFtrMin, s_tessFtrMax);
  m_insideTessFtr = inside;
};

void BicubicPatch::setEdgeTessFtrs(float rowSlices, float colSlices) {
  std::clamp(rowSlices, s_tessFtrMin, s_tessFtrMax);
  std::clamp(colSlices, s_tessFtrMin, s_tessFtrMax);
  m_edgeRowTessFtr = rowSlices;
  m_edgeColTessFtr = colSlices;
}
std::array<float, 3> BicubicPatch::getTessFtrs() {
  return {m_edgeRowTessFtr, m_edgeColTessFtr, m_insideTessFtr};
};

}  // namespace rch