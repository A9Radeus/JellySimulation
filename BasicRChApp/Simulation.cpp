#include "Simulation.h"

namespace rch {

inline constexpr bool isFaceDiagonal(const std::array<int, 3>& is) {
  return (is[0] != 0 && is[1] != 0) || (is[1] != 0 && is[2] != 0) ||
         (is[2] != 0 && is[0] != 0);
}

void JellySim::startNew(const State& state0, const JellySimParams& params,
                        const std::shared_ptr<WiredCube>& frame) {
  m_timeAcc = 0;

  m_state = state0;
  randVelocities(m_state, params.maxRandVel);
  m_ctrlFrame = frame;

  m_dt = params.dt;
  m_mass = params.mass;
  m_k = params.k;
  m_c1 = params.c1;
  m_c2 = params.c2;
  m_l0 = params.l0;
}
void JellySim::startNew(State&& state0, const JellySimParams& params,
                        const std::shared_ptr<rch::WiredCube>& frame) {
  m_timeAcc = 0;

  m_ctrlFrame = frame;
  m_state = std::move(state0);
  randVelocities(m_state, params.maxRandVel);

  m_dt = params.dt;
  m_mass = params.mass;
  m_k = params.k;
  m_c1 = params.c1;
  m_c2 = params.c2;
  m_l0 = params.l0;
};

void JellySim::step() {
  State newState = m_state;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 4; k++) {
        const auto& q1 = m_state.val(i, j, k);  // current q

        /* 
          Calculate forces acting on q1
        */
        Vec3 force = {0, 0, 0};

        /* Elasticity force of it's neighbours */
        constexpr auto neighInds = getNeighbourInds();
        for (const auto& inds : neighInds) {
          if (m_state.checkBoundsSign(i + inds[0], j + inds[1], k + inds[2])) {
            const auto& q2 = m_state.val((std::size_t)i + inds[0], (std::size_t)j + inds[1],
                                         (std::size_t)k + inds[2]);
            float l0 = isFaceDiagonal(inds) ? m_l0 * std::sqrtf(2) : m_l0;
            force += elastForce(q1, q2, m_c1, l0);
          }
        }

        /* Elasticity force if it's attached to a control-frame */
        if (m_frameActive) {
          force += calcFrameForces(q1, i, j, k);
        }

        /* Damping force */
        force += -m_k * q1.vel; 

        /* 
            Integrate the current state
        */

        auto& newQ = newState.obj(i, j, k);
        newQ.pos = q1.pos;
        newQ.vel = q1.vel; 

        // Internally applies Euler Method to get the new position
        // and updates it alongside it's velocity if a collision occurs
        updatePosition(newQ);

        // Euler Method for the velocity
        newQ.vel += (force / m_mass) * m_dt;
      }
    }
  }
  m_state = newState;
  m_timeAcc += m_dt;
}

void JellySim::switchCollisionMode() {
  if (m_collMode == CollisionMode::ALL_AXES) {
    m_collMode = CollisionMode::ONE_AXIS;
  } else {
    m_collMode = CollisionMode::ALL_AXES;
  }
}

inline Vec3 JellySim::elastForce(const Vec3& q1, const Vec3& q2, float c, float l0) {
  if (q1 == q2) { return {0, 0, 0}; }
  const Vec3 dir = Vec3::getNormalized(q2 - q1);  // acting on q1
  return dir * (c * (Vec3::distance(q1, q2) - l0));
};

inline Vec3 JellySim::elastForce(const SimParticle& q1, const SimParticle& q2,
                                 float c, float l0) {
  return elastForce(q1.pos, q2.pos, c, l0);
}

Vec3 JellySim::calcFrameForces(const SimParticle& q1, int i, int j, int k) {
  // Frame is attached only to Particles with indices = 0 or 4
  if ((i != 3 && i != 0) || (j != 3 && j != 0) || (k != 3 && k != 0)) {
    return {0, 0, 0};
  }

  // Ints have to transformed: 3 -> true and 0 -> false
  Vec3 cornerPos = m_ctrlFrame->getCorner(
      static_cast<bool>(i), static_cast<bool>(j), static_cast<bool>(k));

  return elastForce(q1.pos, cornerPos, m_c2, m_la);
}

// PTODO: Simplify IFs
JellySim::CollisionInfo 
JellySim::findCollisionWithinDt(const Vec3& pos, const Vec3& vel, float dt) {
  float hbbSide = JellySimParams::boundingBoxSide / 2.f;
  CollisionInfo min = {-1, 0.f};

  for (int i = 0; i < 3; i++) {
    float axisPos = pos[i] + vel[i] * dt;
    if (hbbSide < axisPos) {
      float dtColl = (hbbSide - pos[i]) / vel[i];
      if (min.occured() == false || (min.occured() && dtColl < min.dt)) {
        min = {i, dtColl};
      }
    } else if (-hbbSide > axisPos) {
      float dtColl = (-hbbSide - pos[i]) / vel[i];
      if (min.occured() == false || (min.occured() && dtColl < min.dt)) {
        min = {i, dtColl};
      }
    }
  }

  return min;
}

inline Vec3 JellySim::velocityAfterCollision(const Vec3& vBefore, int idx) {
  Vec3 vAfter = vBefore;
  vAfter[idx] *= -1;

  if (m_collMode == CollisionMode::ONE_AXIS) {
    vAfter[idx] *= m_mi;
  } else if (m_collMode == CollisionMode::ALL_AXES) {
    vAfter *= m_mi;
  }

  return vAfter;
}

void JellySim::updatePosition(SimParticle& ptl) {
  float dtRem = m_dt;
  Vec3 currPos = ptl.pos;
  Vec3 currVel = ptl.vel;

  auto collInfo = findCollisionWithinDt(currPos, currVel, dtRem);
  while (collInfo.occured() == true) {
    currPos += currVel * collInfo.dt;
    currVel = velocityAfterCollision(currVel, collInfo.idx);
    dtRem -= collInfo.dt;

    collInfo = findCollisionWithinDt(currPos, currVel, dtRem);
  }

  ptl.pos = currPos + currVel * dtRem;
  ptl.vel = currVel;
};

}  // namespace rch
