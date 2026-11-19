#![feature(autodiff)]
mod bvp;
mod model;
mod solver;
pub use crate::model::{AnalyticalLinear, BaseParams, Model, NumericalLinear, NumericalQuadratic};
pub use crate::solver::{Solution, Solver};
pub use bvp::{Bvp, FixedTarget, MovingTarget};
