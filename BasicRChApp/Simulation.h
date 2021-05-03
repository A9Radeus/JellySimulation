#pragma once
#include <functional>
#include <random>

#include "Maths3D.h"
#include "Physics.h"
#include "WiredCube.h"
#include "Array3D.h"


namespace rch {

constexpr static std::array<std::array<int, 3>, 18> getNeighbourInds() {
  std::array<std::array<int, 3>, 18> arr{};
  int ctr = 0;
  for (int di = -1; di <= 1; di++) {
    for (int dj = -1; dj <= 1; dj++) {
      for (int dk = -1; dk <= 1; dk++) {
        if ((di * dk * dj != 0) || (di == 0 && dj == 0 && dk == 0)) {
          // skips cube diagonals and itself (0, 0, 0)
          continue;
        }
        arr[ctr] = {di, dj, dk};
        ctr++;
      }
    }
  }
  return arr;
};

struct SimParticle {
  Vec3 pos = {0, 0, 0};
  Vec3 vel = {0, 0, 0};
};

struct JellySimParams {
  float dt = 0.001f;
  float cubeSide = 1.5f;
  float l0 = 1.f;
  float mass = 1.f;
  float k  = 1.f;
  float c1 = 50.f;
  float c2 = 100.f;
  float maxRandVel = 0.f;
  static constexpr float boundingBoxSide = 16.f;
  static constexpr int partiPerSide = 4;
};

class JellySim {
private:
  struct CollisionInfo {
    int idx;
    float dt;
    inline bool occured() const { return idx >= 0; }
  };
public:
  JellySim() : m_randGen(m_randDev()){};

  using State = Array3D<SimParticle, 4,4,4>;

  enum class CollisionMode { ALL_AXES, ONE_AXIS };

  void startNew(const State& state0, const JellySimParams& params,
                const std::shared_ptr<WiredCube>& frame);

  void startNew(State&& state0, const JellySimParams& params,
                const std::shared_ptr<rch::WiredCube>& frame);

  void activateFrame()   { m_frameActive = true;  } 
  void deactivateFrame() { m_frameActive = false; } 

  const State& getState() const { return m_state; }

  void step();

  float getRestiution() const    { return m_mi; }
  void  setRestiution(float val) { m_mi = val;  }
  void switchFrameActive() { m_frameActive = !m_frameActive; }
  
  CollisionMode getCollisionMode() const { return m_collMode; }
  void setCollisionMode(CollisionMode mode) { m_collMode = mode; }
  void switchCollisionMode();

private:
  /*
      Dynamic Parameters (changeable during the Sim) 
  */

  bool m_frameActive = false;
  CollisionMode m_collMode = CollisionMode::ALL_AXES; 

  /*
      Static Parameters
  */

  float m_dt = 0;    // time delta
  float m_mass = 0;  // mass
  float m_k = 0;     // damping 
  float m_c1 = 0;    // elasticity (jelly)
  float m_c2 = 0;    // elasticity (frame)
  float m_l0 = 0;    // rest length (jelly)
  float m_mi = 0.5f; // coefficient of restitution

  // rest length (frame / attached pts). 
  constexpr static float m_la = 0;    

  /*
      State
  */

  float m_timeAcc = 0; 
  State m_state;
  std::shared_ptr<rch::WiredCube> m_ctrlFrame;

  /*
      Subsystems
  */
  std::random_device m_randDev;
  std::mt19937 m_randGen;

  /* 
      Utility functions
  */

  // Calculates elasticity force acting on q1(!)
  inline static Vec3 elastForce(const Vec3& q1, const Vec3& q2, float c, float l0);

  // Utility overload that takes only Particles instead
  inline static Vec3 elastForce(const SimParticle& q1, const SimParticle& q2,
                                float c, float l0);

  // Calculates Forces acting on q1, coming from frame-particle 
  // at of indices (i/4, j/4, k/4). Q1 is ought to be at (i,j,k).
  Vec3 calcFrameForces(const SimParticle& q1, int i, int j, int k);
  
  // Looks for collisions with the bounding box withing the "dt". 
  // Returns smallest dt after which a collison occurs and index of
  // the axis it occurs on.
  CollisionInfo findCollisionWithinDt(const Vec3& pos, const Vec3& vel, float dt);

  // Doesn't perform any runtime bounds-checking
  inline Vec3 velocityAfterCollision(const Vec3& vBefore, int idx);

  // Collisions with the bounding box
  void updatePosition(SimParticle& ptl);

  //
  void randVelocities(State& st, float maxVel) {
    if (maxVel == 0) {
      return;
    }

    std::uniform_real_distribution<float> dist(0.f, maxVel);

    for (std::size_t idx = 0; idx < st.size(); ++idx) {
      auto& prt = st.objLin(idx);
      prt.vel.x = dist(m_randGen);
      prt.vel.y = dist(m_randGen);
      prt.vel.z = dist(m_randGen);
    }
  }
};

// Inner algorithm as a template provides additional type-safety
// since AsynchSimulation<A1> may be significantly different from
// AsynchSimulation<A2>.
// In addition, no vtable is required.
template <typename Algo>
class AsynchSimulation {
 public:
  using AStateType = typename Algo::State;

  //AsynchSimulation() {};

  template <typename... AlgoParams>
  void start(AlgoParams&&... params) {
    m_core.startNew(std::forward<AlgoParams>(params)...);
    m_step = 0;
    m_running = true;
  }

  void resume() { m_running = true;  }
  void pause()  { m_running = false; }

  void stop() {
    m_running = false;
    m_step = 0;
  }

  // TODO
  void restart() {
    m_running = true;
    m_step = 0;
    //m_core.startNew(OLD_PARAMS_HERE?)
  }

  bool step() {
    if (m_running == false) {
      return false;
    }

    m_core.step();
    m_step += 1;
    
    return true;
  }

  const AStateType& getState() const { return m_core.getState(); }

  // (!) Not advised to design Algo's system around this function
  Algo& core() { return m_core; }

 protected:
  std::size_t m_step = 0;
  bool m_running = false;

  Algo m_core;
};

}  // namespace rch