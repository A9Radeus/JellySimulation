#pragma once
#include "CompBezier3PatchC0.h"

namespace rch {

class SOManager;
  

/**
 * Composite Bezier Bicubic Patch with C2 continuity
 * at the connection points.
 *
 * User has to call init() to let the patch create all
 * the necessary virtual control points. These points
 * are the DeBoor points.
 * 
 * TODO: doesn't quite work with cylinder that has only 2 patches across!
 *       But it seems to be the default so maybe just add safety checks
 */
class CompBezier3PatchC2 : public CompBezier3PatchC0 {
  friend SOManager;
  using Uint = unsigned int;
  using WrapMode = BicubicPatch::WrapMode;
 public:
  CompBezier3PatchC2(GraphicsSystem& gxSys)
      : CompBezier3PatchC0(gxSys){};
  virtual ~CompBezier3PatchC2() = default;

  CompBezier3PatchC2(const CompBezier3PatchC2&) = delete;
  CompBezier3PatchC2& operator=(const CompBezier3PatchC2&) = delete;

  CompBezier3PatchC2(CompBezier3PatchC2&&) = delete;
  CompBezier3PatchC2& operator=(CompBezier3PatchC2&&) = delete;

  // Uses update and render od the CompBezier3PatchC0
  // virtual void update(double dtime = 0.0);
  // virtual void render();
  //virtual void init(std::vector<Point*>&& ctrls, Uint across, Uint lenwise,
  //                  BicubicPatch::WrapMode wrap) override;
  // TODO

  //virtual const std::vector<Point*>& getConcreteControlPoints(){
  //  return m_concretePts;
  //};
  // TODO

  static std::vector<Vec3> genCyliVertsY_R(float radius, float height,
                                           Uint across, Uint lenwise);
  static std::vector<Vec3> genCyliVertsX_R(float radius, float height,
                                           Uint across, Uint lenwise);
  static std::vector<Vec3> genRectVerts(float width, float length, Uint across,
                                        Uint lenwise);

  // RTTI --
  virtual const char* getTypeName() const override { return "Bezier Patch C2"; }
  virtual SOType getType() const override { return SOType::BEZIER3_PATCH_C2; }
  constexpr static SOType getStaticType() { return SOType::BEZIER3_PATCH_C2; }

 protected:
  // Unique control points. Contrary to m_cpts which 
  // in case of wrapping may store multiple references
  // to the same Point
  // PTODO: use m_cpts as concrete ones, and create a new vector 
  // of the "non-concrete" ones instead.
  std::vector<Point*> m_concretePts;  
  void initCtrlPtsView(std::vector<Point*> pts, WrapMode mode); 

  //std::vector<Vec3> deBoors4x4ToBern3(); // PTODO
  virtual void genPatchVerts() override;
  virtual void genPolygonVerts() override;

  // Since this is also a Bezier Patch, the index buffers
  // are indentical. (Vertex Buffer has Bernstein coords
  // stored and not the DeBoor points)
  /* virtual void initIndexBuffer() override; */
};

}  // namespace rch