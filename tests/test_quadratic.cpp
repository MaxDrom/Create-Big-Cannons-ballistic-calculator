// Тесты квадратичной баллистики для случая:
//   V = 8, drag = 0.001, k = 6, g = -0.05, интеграция потиковая.
//
// Проверка: каждое решение Solver::find, пересимулированное потиково
// (floor(T) шагов по tau = 1 + правый дробный шаг до T), попадает в (l, h)
// в пределах допуска.
#include <cmath>
#include <cstdio>
#include <model.hpp>
#include <solver.hpp>

namespace {

const Real kDrag = 0.001;
const Real g = -0.05;
const Real V = 8.0;
const Real K = 5.5;

const Real hitTol = 1e-4; // блоки

// Пересимуляция: попадает ли (theta, T) в (l, h) при потиковой интеграции:
// floor(T) шагов по tau = 1 и правый дробный шаг tau = T - floor(T).
bool hits(Real thetaDeg, Real T, Real l, Real h) {
  QuadraticModel model(kDrag, g);
  Real th = thetaDeg * std::numbers::pi / 180.0;
  Real c = std::cos(th), s = std::sin(th);
  Vec4 z{K * c, K * s, V * c, V * s};
  int ticks = (int)std::floor(T);
  Real frac = T - ticks;
  for (int i = 0; i < ticks; ++i)
    model.step(z, 1.0);
  if (frac > 0)
    model.step(z, frac);
  return std::abs(z.x - l) < hitTol && std::abs(z.y - h) < hitTol;
}

} // namespace

int main() {
  Solver<QuadraticModel> solver(QuadraticModel(kDrag, g), V, K);

  const double ls[] = {20.0,  50.0,  100.0, 150.0, 200.0,
                       250.0, 300.0, 350.0, 400.0};
  const double hs[] = {-20.0, -5.0, 0.0, 10.0, 25.0};

  int targets = 0, solutions = 0, failures = 0;
  for (double l : ls) {
    for (double h : hs) {
      ++targets;
      auto sols = solver.find(l, h);
      std::printf("l=%4.0f h=%4.0f: %zu sol", l, h, sols.size());
      for (const auto &s : sols) {
        bool ok = hits(s.theta, s.time, l, h);
        std::printf(" | %.3f deg %.2f T (%s)", s.theta, s.time,
                    ok ? "hit" : "MISS");
        ++solutions;
        if (!ok)
          ++failures;
      }
      std::printf("\n");
    }
  }

  std::printf("%d target(s), %d solution(s), %d failure(s)\n", targets,
              solutions, failures);
  return failures > 0;
}
