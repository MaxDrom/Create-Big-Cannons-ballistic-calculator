extern crate nalgebra as na;
use na::Vector6;
pub trait Model {
    type Params;
    fn from_params(params: &Self::Params) -> Self;
    fn integrate(&self, time: &f64, z: &mut Vector6<f64>);
    fn get_gravity(&self) -> f64;
}
