#pragma once
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
using Real = double;
template <typename Fn>
  requires std::invocable<Fn &, Real>
bool binarySearch(Fn &&f, Real a, Real b, Real &result, Real f1, Real f2) {
  if (b < a) {
    std::swap(a, b);
    std::swap(f1, f2);
  }
  Real mid, fmid;
  Real dx = (b - a);
  while (dx > 2 * std::numeric_limits<Real>::epsilon()) {
    if ((f1 * f2 > 0) | std::isnan(f1) | std::isnan(f2))
      return false;
    mid = (a + b) / 2;
    fmid = std::invoke_r<Real>(f, mid);
    dx *= 0.5;
    if (fmid * f1 > 0) {
      a = mid;
      f1 = fmid;
      continue;
    }
    b = mid;
    f2 = fmid;
  }
  result = mid;
  return true;
};

template <typename Fn>
  requires std::invocable<Fn &, Real>
bool binarySearch(Fn &&f, Real a, Real b, Real &result) {
  Real f1 = std::invoke_r<Real>(f, a);
  Real f2 = std::invoke_r<Real>(f, b);
  return binarySearch(f, a, b, result, f1, f2);
};

template <typename Fn>
  requires std::invocable<Fn &, Real>
Real goldenRatioSearch(Fn &&f, Real a, Real b) {
  if (a > b)
    std::swap(a, b);
  Real phi = (1 + sqrt(5)) / 2;
  Real x1 = a + (b - a) * (2 - phi);
  Real x2 = b - (b - a) * (2 - phi);
  Real width = (b - a);
  auto f1 = std::invoke_r<Real>(f, x1);
  auto f2 = std::invoke_r<Real>(f, x2);
  while (width > 2 * std::numeric_limits<Real>::epsilon()) {
    width = width * (phi - 1);
    if (f1 > f2) {
      b = x2;
      x2 = x1;
      f2 = f1;
      x1 = a + width * (2 - phi);
      f1 = std::invoke_r<Real>(f, x1);
    } else {
      a = x1;
      x1 = x2;
      x2 = b - width * (2 - phi);
      f1 = f2;
      f2 = std::invoke_r<Real>(f, x2);
    }
  }
  return 0.5 * (b + a);
}
