#pragma once
#include "BicubicPatchBase.h"
#include <cmath>


namespace rch {

class SOManager;

/** 
 * Composite Bezier Bicubic Patch with C0 continuity 
 * at the connection points.
 * 
 * User has to call init() to let the patch create all
 * the necessary virtual control points. These points 
 * are coordinates in the Bernstein basis.
 */
class CompBezier3PatchC0 : public BicubicPatch {
  friend SOManager;
  using Uint = unsigned int;
  using WrapMode = BicubicPatch::WrapMode;

public:
  CompBezier3PatchC0(GraphicsSystem& gxSys)
      : BicubicPatch(gxSys) {};
  virtual ~CompBezier3PatchC0() = default;

  CompBezier3PatchC0(const CompBezier3PatchC0&) = delete;
  CompBezier3PatchC0& operator=(const CompBezier3PatchC0&) = delete;

  CompBezier3PatchC0(CompBezier3PatchC0&&) = delete;
  CompBezier3PatchC0& operator=(CompBezier3PatchC0&&) = delete;

  virtual void update(double dtime = 0.0) override;
  virtual void render() override;
  virtual void init(PointPtrs&& ctrls, Uint across, Uint lenwise,
                    BicubicPatch::WrapMode wrap) override;
  virtual void init(const PointPtrs& ctrls, Uint across, Uint lenwise,
                    BicubicPatch::WrapMode wrap) override;

  virtual const PointPtrs& getConcreteControlPoints() override { return m_cpts; };

  virtual void renderPolyline();

  static std::vector<Vec3> genCyliVertsY_R(float radius, float height, Uint across,
                                        Uint lenwise);
  static std::vector<Vec3> genCyliVertsX_R(float radius, float height, Uint across,
                                        Uint lenwise);
  static std::vector<Vec3> genRectVerts(float width, float length, Uint across,
                                    Uint lenwise);
	// RTTI --
  virtual const char* getTypeName() const override { return "Bezier Patch C0"; }
  virtual SOType getType() const override { return SOType::BEZIER3_PATCH_C0; }
  constexpr static SOType getStaticType() { return SOType::BEZIER3_PATCH_C0; }

protected:
  // ----------- Functions -----------
  virtual void genPatchVerts();
  virtual void genPolygonVerts();

  virtual void initIndexBuffer(WrapMode wrap);

  // ----------- Variables -----------
  bool m_meshReady = false;
  mini::dx_ptr_vector<ID3D11Buffer> m_vertBuff;
  int m_vertsNum = 0;
  mini::dx_ptr<ID3D11Buffer> m_idxBuff;
  unsigned int m_idxNum = 0;

  mini::dx_ptr_vector<ID3D11Buffer> m_polylineVertBuff;
  bool m_polylineReady = false;
  int m_polylineVertsNum = 0;

  PointPtrs m_cpts;

  // The amount of control points in a standalone patch 
  static constexpr unsigned short s_patchSize = 16;
  static constexpr unsigned short s_vsPerSide = 4;
};

}  // namespace rch


