#include <ballistics.hpp>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
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
}
