#pragma once
#include "Maths3D.h"
#include <DirectXMath.h>

namespace rch {

template <typename Num>
static XMMATRIX tensorCubeCorner(Num a, Num rho) {
  Num a5 = a * a * a * a * a;
  Num diag = (2.0 / 3.0) * a5 * rho;
  Num rem = -1.0 * (a5 * rho) / 4.0;
  return XMMATRIX(diag, rem, rem, 0,
                  rem, diag, rem, 0, 
                  rem, rem, diag, 0,
                  0, 0, 0, 1);
}

template <typename Num>
static XMMATRIX invTensorCubeCorner(Num a, Num rho) {
  Num a5 = a * a * a * a * a;
  Num div = 11 * a5 * rho;
  Num diag = 30 / div;
  Num rem = 18 / div;
  return XMMATRIX(
    diag, rem, rem, 0,
    rem, diag, rem, 0,
    rem, rem, diag, 0,
    0, 0, 0, 1);
};

// Requires the TState to implement: "TState + TState" & "scalar * TState"
template <typename TParam, typename TState, typename TFunc>
static TState solveODE_Euler(TParam x0, TState y0, TParam h, TFunc dydx) {
  return y0 + (h * dydx(x0, y0));
}

// Requires the TState to implement: "TState + TState" & "scalar * TState"
// PTODO: Consider requiring "TState / scalar" to increase numerical stability
template <typename TParam, typename TState, typename TFunc>
static TState solveODE_RK4(TParam x0, TState y0, TParam h, TFunc dydx) {
  const TState k1 = h * dydx(x0, y0);
  const TState k2 = h * dydx(x0 + 0.5 * h, y0 + 0.5 * k1);
  const TState k3 = h * dydx(x0 + 0.5 * h, y0 + 0.5 * k2);
  const TState k4 = h * dydx(x0 + h, y0 + k3);

  const TState y = y0 + ( (1.0/6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4));

  return y;
}

}