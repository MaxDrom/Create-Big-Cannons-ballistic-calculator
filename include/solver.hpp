#pragma once
#include <algorithm>
#include <cmath>
#include <model.hpp>
#include <numbers>
#include <optional>
#include <utils.hpp>
#include <vector>
struct Solution {
  Real theta; // градусы
  Real time;  // T, тиков
};

template <class ModelT> class Solver {
public:
  ModelT model;
  Real velocity; // V
  Real barrel;   // k, длина дула

  Solver(ModelT model, Real velocity, Real barrel)
      : model{model}, velocity{velocity}, barrel{barrel} {}

  std::vector<Solution> find(Real l, Real h) const {
    if (velocity <= 0)
      return {};

    std::vector<std::pair<Real, Real>> starts;

    Real T0 = (l - barrel) / velocity;
    if (T0 > 0)
      starts.push_back({0.0, T0});

    Real disc = velocity * velocity - 2.0 * model.g * (barrel - h);
    if (disc > 0) {
      T0 = (-velocity - std::sqrt(disc)) / model.g;
      if (T0 > 0)
        starts.push_back({std::numbers::pi / 2.0, T0});
    }

    std::vector<Solution> out;
    for (const auto &st : starts) {
      auto sol = newton(st.first, st.second, l, h);
      if (sol)
        out.push_back(*sol);
    }
    return out;
  }

  std::optional<Solution> newton(Real theta0, Real T0, Real l, Real h) const {
    Real theta = theta0, T = T0;
    const Real scale = std::max({std::abs(l), std::abs(h), 1.0});
    Final fin;
    for (int it = 0; it < 50; ++it) {
      integrate(theta, T, fin);
      Real Fx = fin.z.x - l, Fy = fin.z.y - h;
      if (std::max(std::abs(Fx), std::abs(Fy)) < 1e-7 * scale)
        return Solution{theta * 180.0 / std::numbers::pi, T};

      Real c = std::cos(theta), s = std::sin(theta);
      Vec4 j{-barrel * s, barrel * c, -velocity * s, velocity * c};

      Vec4 Jt = mat4Vec(fin.J, j);
      Vec4 dT = fin.dT;
      Real J0x = Jt.x, J0y = Jt.y;
      Real dtx = dT.x, dty = dT.y;

      Real det = J0x * dty - dtx * J0y;
      if (std::abs(det) < 1e-12)
        return std::nullopt;
      Real dtheta = (dtx * Fy - dty * Fx) / det;
      Real dT_ = (J0y * Fx - J0x * Fy) / det;
      theta += dtheta;
      T += dT_;
      if (!std::isfinite(theta) || !std::isfinite(T) || T <= 0)
        return std::nullopt;
    }
    return std::nullopt;
  }

private:
  struct Final {
    Vec4 z;        // z^T (конечное)
    Vec4 dT;       // производная по времени на правом конце
    Mat4 J;        // dz^T/dz^0
    Real finalTau; // длина правого шага, frac = T - floor(T)
  };

  void integrate(Real theta, Real T, Final &result) const {
    Real c = std::cos(theta), s = std::sin(theta);

    result.z.x = barrel * c;
    result.z.y = barrel * s;
    result.z.vx = velocity * c;
    result.z.vy = velocity * s;
    int k = (int)std::floor(T);
    Real frac = T - k;
    result.J = identity4();
    for (int i = 0; i < k; ++i) {
      model.variationalStep(result.z, result.J, 1.0);
      model.step(result.z, 1.0);
    }
    result.dT = model.dDtau(result.z, frac);
    model.variationalStep(result.z, result.J, frac);
    model.step(result.z, frac);
    result.finalTau = frac;
  }
};
