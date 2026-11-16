#pragma once
#include <array>
#include <cmath>
#include <utils.hpp>

// ---------------------------------------------------------------------------
// Типы вариационной формализма.
// Состояние z = (x, y, vx, vy); матрица чувствительности J = dz^i/dz^0.
// ---------------------------------------------------------------------------
struct Vec4 {
  Real x, y, vx, vy;
};

struct Mat4 {
  std::array<std::array<Real, 4>, 4> m{};
  Real &at(int i, int j) { return m[i][j]; }
  const Real &at(int i, int j) const { return m[i][j]; }
};

inline Vec4 mat4Vec(const Mat4 &a, const Vec4 &v) {
  return {
      a.m[0][0] * v.x + a.m[0][1] * v.y + a.m[0][2] * v.vx + a.m[0][3] * v.vy,
      a.m[1][0] * v.x + a.m[1][1] * v.y + a.m[1][2] * v.vx + a.m[1][3] * v.vy,
      a.m[2][0] * v.x + a.m[2][1] * v.y + a.m[2][2] * v.vx + a.m[2][3] * v.vy,
      a.m[3][0] * v.x + a.m[3][1] * v.y + a.m[3][2] * v.vx + a.m[3][3] * v.vy};
}

inline Mat4 identity4() {
  Mat4 r;
  for (int i = 0; i < 4; ++i)
    r.at(i, i) = 1;
  return r;
}

// ---------------------------------------------------------------------------
// Модель квадратичного драга:
//   v^(i+1) = v^i - d norm(v^i) v^i tau + g tau
//   x^(i+1) = x^i + v^i tau - d norm(v^i) v^i tau^2/2 + g tau^2/2
// Модель знает только физику (d, g); о начальных условиях она не знает.
// ---------------------------------------------------------------------------

class LinearModel {
public:
  Real d; // коэффициент драга
  Real g;
  LinearModel(Real d, Real g) : d{d}, g{g} {}
  void step(Vec4 &z, Real tau) const {
    Real t2 = tau * tau;
    z.x += z.vx * tau - 0.5 * d * z.vx * t2;
    z.y += z.vy * tau - 0.5 * d * z.vy * t2 + 0.5 * g * t2;
    z.vx = z.vx - d * z.vx * tau;
    z.vy = z.vy - d * z.vy * tau + g * tau;
  } // ускорение по y, блоков/тик^2

  void variationalStep(const Vec4 &z, Mat4 &J, Real tau) const {
    Real nv = 1;

    Real dn = d / nv;
    Real t2 = tau * tau;
    // (W v)_b, b = (vx, vy)
    Real wv0 = J.at(2, 2) * z.vx + J.at(3, 2) * z.vy;
    Real wv1 = J.at(2, 3) * z.vx + J.at(3, 3) * z.vy;
    // (A W)_{ab} = (d/nv) v_a (W v)_b + d nv W_{ab}
    Real aw00 = d * J.at(2, 2);
    Real aw01 = d * J.at(2, 3);
    Real aw10 = d * J.at(3, 2);
    Real aw11 = d * J.at(3, 3);
    // B — по старому W, до обновления W:
    J.at(0, 2) += tau * J.at(2, 2) - 0.5 * t2 * aw00;
    J.at(0, 3) += tau * J.at(2, 3) - 0.5 * t2 * aw01;
    J.at(1, 2) += tau * J.at(3, 2) - 0.5 * t2 * aw10;
    J.at(1, 3) += tau * J.at(3, 3) - 0.5 * t2 * aw11;
    // W:
    J.at(2, 2) -= tau * aw00;
    J.at(2, 3) -= tau * aw01;
    J.at(3, 2) -= tau * aw10;
    J.at(3, 3) -= tau * aw11;
  }

  Vec4 dDtau(const Vec4 &z, Real tau) const {
    Real nv = 1;
    return {z.vx - d * z.vx * tau, z.vy - d * z.vy * tau + g * tau, -d * z.vx,
            -d * z.vy + g};
  }
};

class QuadraticModel {
public:
  Real d; // коэффициент драга
  Real g; // ускорение по y, блоков/тик^2

  QuadraticModel(Real d, Real g) : d{d}, g{g} {}

  // Один шаг интегрирования длины tau, in-place.
  void step(Vec4 &z, Real tau) const {
    Real nv = std::sqrt(z.vx * z.vx + z.vy * z.vy);
    Real t2 = tau * tau;
    z.x += z.vx * tau - 0.5 * d * nv * z.vx * t2;
    z.y += z.vy * tau - 0.5 * d * nv * z.vy * t2 + 0.5 * g * t2;
    z.vx = z.vx - d * nv * z.vx * tau;
    z.vy = z.vy - d * nv * z.vy * tau + g * tau;
  }

  // Обновление матрицы чувствительности in-place по рекурренте
  void variationalStep(const Vec4 &z, Mat4 &J, Real tau) const {
    Real nv = std::sqrt(z.vx * z.vx + z.vy * z.vy);
    if (nv <= 1e-12) {
      // A = 0: W не меняется, B += tau * W.
      J.at(0, 2) += tau * J.at(2, 2);
      J.at(0, 3) += tau * J.at(2, 3);
      J.at(1, 2) += tau * J.at(3, 2);
      J.at(1, 3) += tau * J.at(3, 3);
      return;
    }
    Real dn = d / nv;
    Real t2 = tau * tau;
    // (W v)_b, b = (vx, vy)
    Real wv0 = J.at(2, 2) * z.vx + J.at(3, 2) * z.vy;
    Real wv1 = J.at(2, 3) * z.vx + J.at(3, 3) * z.vy;
    // (A W)_{ab} = (d/nv) v_a (W v)_b + d nv W_{ab}
    Real aw00 = dn * z.vx * wv0 + d * nv * J.at(2, 2);
    Real aw01 = dn * z.vx * wv1 + d * nv * J.at(2, 3);
    Real aw10 = dn * z.vy * wv0 + d * nv * J.at(3, 2);
    Real aw11 = dn * z.vy * wv1 + d * nv * J.at(3, 3);
    // B — по старому W, до обновления W:
    J.at(0, 2) += tau * J.at(2, 2) - 0.5 * t2 * aw00;
    J.at(0, 3) += tau * J.at(2, 3) - 0.5 * t2 * aw01;
    J.at(1, 2) += tau * J.at(3, 2) - 0.5 * t2 * aw10;
    J.at(1, 3) += tau * J.at(3, 3) - 0.5 * t2 * aw11;
    // W:
    J.at(2, 2) -= tau * aw00;
    J.at(2, 3) -= tau * aw01;
    J.at(3, 2) -= tau * aw10;
    J.at(3, 3) -= tau * aw11;
  }

  // Производная шага по tau: dz^(i+1)/dtau без множителя N,
  // в предположении, что z^i от tau не зависит (правый конец).
  Vec4 dDtau(const Vec4 &z, Real tau) const {
    Real nv = std::sqrt(z.vx * z.vx + z.vy * z.vy);
    return {z.vx - d * nv * z.vx * tau, z.vy - d * nv * z.vy * tau + g * tau,
            -d * nv * z.vx, -d * nv * z.vy + g};
  }
};
