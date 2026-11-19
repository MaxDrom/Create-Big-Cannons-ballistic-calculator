use super::Model;
extern crate nalgebra as na;
use na::Vector6;
pub trait NumericalModel {
    fn step(&self, z: &mut Vector6<f64>, t: &f64, tau: &f64);
    fn get_gravity(&self) -> f64;
    type Params;
    fn from_params(params: &Self::Params) -> Self;
}

impl<T: NumericalModel> Model for T {
    fn integrate(&self, time: &f64, z: &mut Vector6<f64>) {
        let mut t = 0.0;
        while t < (*time) {
            self.step(z, &t, &1.0);
            t += 1.0;
        }
        self.step(z, &t, &time.fract());
    }

    fn get_gravity(&self) -> f64 {
        self.get_gravity()
    }

    type Params = T::Params;

    fn from_params(params: &Self::Params) -> Self {
        T::from_params(params)
    }
}
