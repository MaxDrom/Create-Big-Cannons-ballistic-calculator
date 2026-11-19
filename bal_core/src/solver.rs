extern crate nalgebra as na;
use crate::bvp::Bvp;
use crate::model::Model;
use na::{Matrix3, Vector3, Vector6};
use std::autodiff::autodiff_forward;
use std::f64;
use thiserror::Error;
pub struct Solver<T: Model> {
    pub model: T,
    pub k: f64,
    pub v: f64,
}

#[derive(Error, Debug, PartialEq)]
enum NewtonError {
    #[error("Вырожденный якобиан на итерации {iteration}")]
    ZeroDerivative { iteration: usize },

    #[error("Превышен лимит итераций")]
    MaxIterationsReached,
}

use serde::Serialize;
#[derive(Debug, Serialize)]
pub struct Solution {
    pub pitch: f64,
    pub yaw: f64,
    pub time: f64,
}

impl<T: Model> Solver<T> {
    pub fn solve<U: Bvp>(&self, bvp: &U, tol: &Option<f64>) -> Vec<Solution> {
        let mut result: Vec<Solution> = vec![];
        let target_pos = bvp.get_pos();
        let yaw = target_pos[2].atan2(target_pos[0]);
        let h = target_pos[1];
        let l = (target_pos[0] * target_pos[0] + target_pos[2] * target_pos[2]).sqrt();
        let pitch = h.atan2(l);
        let time = (l - self.k) / self.v;
        if time.is_nan() || time.is_infinite() {
            return result;
        }
        match self.newton(yaw, pitch, time, bvp, tol) {
            Ok(sol) => {
                result.push(sol);
            }
            Err(err) => eprintln!("{err}"),
        }

        let yaw = target_pos[2].atan2(target_pos[0]);
        let pitch = f64::consts::PI / 3.0;
        let g = self.model.get_gravity();
        let disc = self.v * self.v - 2.0 * g * (self.k - h);
        let time = (-self.v - disc.sqrt()) / g;
        if time.is_nan() || time.is_infinite() {
            return result;
        }
        match self.newton(yaw, pitch, time, bvp, tol) {
            Ok(sol) => {
                result.push(sol);
            }
            Err(err) => eprintln!("{err}"),
        }

        result
    }

    #[autodiff_forward(df, Const, Const, Dual, Dual)]
    fn forward<U: Bvp>(&self, bvp: &U, vals: &Vector3<f64>, f: &mut Vector3<f64>) {
        let yaw = vals[0];
        let pitch = vals[1];
        let time = vals[2];
        let cp = pitch.cos();
        let sp = pitch.sin();
        let cy = yaw.cos();
        let sy = yaw.sin();
        let mut z = Vector6::<f64>::zeros();
        z[0] = self.k * cy * cp;
        z[1] = self.k * sp;
        z[2] = self.k * sy * cp;

        z[3] = self.v * cy * cp;
        z[4] = self.v * sp;
        z[5] = self.v * sy * cp;
        self.model.integrate(&time, &mut z);
        bvp.calculate(&z, time, f);
    }

    fn newton<U: Bvp>(
        &self,
        mut yaw: f64,
        mut pitch: f64,
        mut time: f64,
        bvp: &U,
        tol: &Option<f64>,
    ) -> Result<Solution, NewtonError> {
        let mut f = Vector3::<f64>::zeros();
        let mut df_col = Vector3::<f64>::zeros();
        let mut total_jac = Matrix3::<f64>::zeros();
        let tol = tol.unwrap_or(1e-8);
        let mut vals = Vector3::<f64>::zeros();
        let mut df_out = Vector3::<f64>::zeros();
        for it in 1..101 {
            vals[0] = yaw;
            vals[1] = pitch;
            vals[2] = time;
            for column in 0..3 {
                df_col.fill(0.0);
                df_col[column] = 1.0;
                self.df(bvp, &vals, &df_col, &mut f, &mut df_out);

                total_jac.set_column(column, &df_out);
            }
            if f.camax() <= tol {
                return Ok(Solution {
                    pitch: pitch / f64::consts::PI * 180.0,
                    yaw: yaw / f64::consts::PI * 180.0,
                    time,
                });
            }
            let Some(delta) = total_jac.lu().solve(&(-f)) else {
                return Err(NewtonError::ZeroDerivative { iteration: it });
            };

            yaw += delta[0];
            pitch += delta[1];
            time += delta[2];

            if time <= 0.0 {
                time = 1e-3;
            }
        }
        Err(NewtonError::MaxIterationsReached)
    }
}
