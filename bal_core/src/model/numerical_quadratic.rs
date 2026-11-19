use super::NumericalModel;
extern crate nalgebra as na;
use super::BaseParams;
use na::{Vector3, Vector6};
pub struct NumericalQuadratic {
    pub drag: f64,
    pub gravity: f64,
}

impl NumericalModel for NumericalQuadratic {
    #[inline]
    fn step(&self, z: &mut Vector6<f64>, _t: &f64, tau: &f64) {
        let tau = *tau;

        let v0 = Vector3::from(z.fixed_rows::<3>(3));
        let nn = (v0.norm_squared().ln()).exp();
        let drag = self.drag * nn;
        let q = tau - drag * 0.5 * tau * tau;

        let gravity = Vector3::new(0.0, self.gravity, 0.0);
        let mut x = z.fixed_rows_mut::<3>(0);
        x += q * v0 + gravity * 0.5 * tau * tau;
        let mut v = z.fixed_rows_mut::<3>(3);
        v += -drag * v0 * tau + gravity * tau;
    }

    fn get_gravity(&self) -> f64 {
        self.gravity
    }

    type Params = BaseParams;

    fn from_params(params: &Self::Params) -> Self {
        Self {
            drag: params.drag.unwrap_or(0.001),
            gravity: params.gravity.unwrap_or(-0.05),
        }
    }
}
