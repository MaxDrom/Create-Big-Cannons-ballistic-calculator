use super::Model;
extern crate nalgebra as na;
use na::{Vector3, Vector6};
pub struct AnalyticalLinear {
    pub drag: f64,
    pub gravity: f64,
}

impl Model for AnalyticalLinear {
    fn integrate(&self, time: &f64, z: &mut Vector6<f64>) {
        let gravity = Vector3::new(0.0, self.gravity, 0.0);
        let v0 = Vector3::from(z.fixed_rows::<3>(3));
        let time = *time;
        let q = 1.0 - self.drag;
        let q_t = (q.ln() * time).exp();

        let pos_factor = (1.0 - self.drag * 0.5) * (1.0 - q_t) / self.drag;
        let v_term = gravity / self.drag;

        let mut x = z.fixed_rows_mut::<3>(0);
        x += (v0 - v_term) * pos_factor + time * v_term;

        let mut v = z.fixed_rows_mut::<3>(3);
        v *= q_t;
        v += v_term * (1.0 - q_t);
    }

    fn get_gravity(&self) -> f64 {
        self.gravity
    }

    type Params = super::BaseParams;

    fn from_params(params: &Self::Params) -> Self {
        Self {
            drag: params.drag.unwrap_or(0.01),
            gravity: params.gravity.unwrap_or(-0.05),
        }
    }
}
