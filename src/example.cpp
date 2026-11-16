#include <cstdio>
#include <model.hpp>
#include <solver.hpp>
#include <vector>

int main() {
  // Модель: d (драг), g (ускорение по y).
  const Real d = 0.001;
  const Real g = -0.05;
  QuadraticModel model(d, g);

  // Солвер: V (скорость), k (длина дула);
  const Real V = 8.0;
  const Real k = 6.0;
  Solver<QuadraticModel> solver(model, V, k);

  const Real l = 400.0; // расстояние до цели
  const Real h = -2.0;  // высота цели (относительно пушки)

  auto sols = solver.find(l, h);

  std::printf("Target: l = %.3f, h = %.3f\n", l, h);
  std::printf("Found %zu solution(s):\n", sols.size());
  for (const auto &s : sols)
    std::printf("  theta = %9.5f deg, time = %12.5f ticks\n", s.theta, s.time);

  Solver<LinearModel> linearSolver(LinearModel(0.01, g), V, k);

  std::printf("LinearModel solutions:\n");
  for (const auto &r : linearSolver.find(l, h))
    std::printf("  theta = %9.5f deg, time = %12.5f ticks\n", r.theta, r.time);
  return 0;
}
