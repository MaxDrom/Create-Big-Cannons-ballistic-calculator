#include <ballistics.hpp>
#include <model.hpp>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <solver.hpp>
#include <vector>
namespace py = pybind11;

PYBIND11_MODULE(ballistics_py, m, py::mod_gil_not_used()) {
  m.doc() = "Create Big Cannons ballistic calculator";
  py::class_<Result>(m, "Result")
      .def_readonly("angle", &Result::angle)
      .def_readonly("maxH", &Result::maxH)
      .def_readonly("time", &Result::time);
  py::class_<BallisticCalculator>(m, "BallisticCalculator")
      .def(py::init<double, double>())
      .def_readonly("max_distance", &BallisticCalculator::max_distance)
      .def_readonly("optimal_angle", &BallisticCalculator::optimal_angle)
      .def("x", py::vectorize(&BallisticCalculator::x))
      .def("y", py::vectorize(&BallisticCalculator::y))
      .def("find_t_yzero", py::vectorize(&BallisticCalculator::find_t_yzero))
      .def("find_max_distance_for",
           py::vectorize(&BallisticCalculator::find_max_distance_for))
      .def("find",
           [](const BallisticCalculator &self, double l, double h) -> py::list {
             auto result = py::list{};
             std::vector<Result> result_v{};
             {
               py::gil_scoped_release release;
               self.find(l, h, result_v);
             }
             for (auto v : result_v)
               result.append(v);
             return result;
           });
  py::class_<Solution>(m, "Solution")
      .def_readonly("theta", &Solution::theta)
      .def_readonly("time", &Solution::time);
  py::class_<Solver<QuadraticModel>>(m, "QuadraticSolver")
      .def(py::init([](double drag, double velocity, double barrel, double g) {
             return Solver<QuadraticModel>{QuadraticModel{drag, g}, velocity,
                                           barrel};
           }),
           py::arg("drag"), py::arg("velocity"), py::arg("barrel"),
           py::arg("g"))
      .def("find",
           [](const Solver<QuadraticModel> &self, double l,
              double h) -> py::list {
             auto result = py::list{};
             std::vector<Solution> sols;
             {
               py::gil_scoped_release release;
               sols = self.find(l, h);
             }
             for (auto &s : sols)
               result.append(s);
             return result;
           });
}
