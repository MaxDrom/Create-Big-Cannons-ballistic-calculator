#pragma once
#include <utils.hpp>

struct Result {
  Real angle;
  Real maxH;
  Real time;
};

class BallisticCalculator {
public:
  Real max_distance;
  Real optimal_angle;
  BallisticCalculator(Real drag, Real velocity)
      : drag{drag}, velocity{velocity} {
    auto eq = [this](Real angle) -> Real {
      return find_max_distance_for(angle);
    };
    optimal_angle = goldenRatioSearch(eq, 0, 90);
    max_distance = find_max_distance_for(optimal_angle);
  };

  Real find_t_yzero(Real angle) const {
    auto v0y = velocity * std::sin(angle / 180 * std::numbers::pi);
    auto zz = -g / (drag * v0y - g);
    auto eq = [this, angle](Real z) -> Real {
      auto t = log(z) / log(1 - drag);
      return y(t, angle);
    };

    Real result;
    binarySearch(eq, zz, 0, result, eq(zz), -1);
    return timeFromZ(result);
  }

  Real find_max_distance_for(Real angle) const {
    return x(find_t_yzero(angle), angle);
  }

  void find(Real l, Real h, std::vector<Result> &results) const {
    results.clear();
    Real crit_z;

    auto eq = [this, l, h](Real z) -> Real { return _y(z, l) - h; };
    auto K = l * drag / (1 - drag * 0.5);
    auto max_z = 1 - K / velocity;
    auto eq_on_max = eq(1 - K / velocity);
    if (binarySearch([this, l](Real z) -> Real { return z_eq(z, l); }, 0, max_z,
                     crit_z)) {
      Real z1, z2;
      Real y_crit = eq(crit_z);
      if (binarySearch(eq, 0, crit_z, z1, -1, y_crit))
        results.push_back(fromZL(z1, l));
      if (binarySearch(eq, max_z, crit_z, z2, eq_on_max, y_crit))
        results.push_back(fromZL(z2, l));
    } else {
      Real z;
      if (binarySearch(eq, 0, max_z, z))
        results.push_back(fromZL(z, l));
    }
    // if (h < 0) {
    //   auto min_angle = std::atan2(h, l) * 180 / std::numbers::pi;
    //   Real result;
    //   if (binarySearch([this](Real angle)->Real{
    //
    //               }, min_angle, 0, result))
    // }
  }

  Real y(Real t, Real angle) const {
    Real z = std::pow(1 - drag, t);
    Real v0y = velocity * std::sin(angle / 180 * std::numbers::pi);
    return t * (0.5 + 1 / drag) * g +
           (1 - drag * 0.5) * (v0y - g / drag) * (1 - z) / drag;
  }

  Real x(Real t, Real angle) const {
    Real z = std::pow(1 - drag, t);
    Real v0x = velocity * std::cos(angle / 180 * std::numbers::pi);
    return (1 - drag * 0.5) * v0x * (1 - z) / drag;
  }

private:
  Result fromZL(Real z, Real l) const {
    auto angle = angleFromZ(z, l);
    auto v0y = velocity * std::sin(angle / 180 * std::numbers::pi);
    auto zz = -g / (drag * v0y - g);
    auto time_max = timeFromZ(zz);
    return Result{angle, y(time_max, angle), timeFromZ(z)};
  }

  Real z_eq(Real z, Real l) const {
    auto K = l * drag / (1 - drag * 0.5);
    Real v0y = velocity * velocity - K * K / ((1 - z) * (1 - z));
    if (v0y < 0)
      v0y = 0;
    v0y = sqrt(v0y);
    return g / (z * std::log(1 - drag)) * (0.5 + 1 / drag) -
           (1 - drag * 0.5) * (v0y - g / drag) / drag -
           (1 - drag * 0.5) * K * K / (drag * v0y * (1 - z) * (1 - z));
  }

  Real timeFromZ(Real z) const { return log(z) / log(1 - drag); }

  Real angleFromZ(Real z, Real l) const {
    return std::acos(l * drag / (1 - drag * 0.5) / velocity / (1 - z)) /
           std::numbers::pi * 180;
  }

  Real _y(Real z, Real l) const {
    auto K = l * drag / (1 - drag * 0.5);
    Real v0y = velocity * velocity - K * K / ((1 - z) * (1 - z));
    if (v0y < 0)
      v0y = 0;
    v0y = sqrt(v0y);
    return log(z) / log(1 - drag) * (0.5 + 1 / drag) * g +
           (1 - drag * 0.5) * (v0y - g / drag) * (1 - z) / drag;
  }

  Real drag;
  Real velocity;
  Real g = -0.05;
};
