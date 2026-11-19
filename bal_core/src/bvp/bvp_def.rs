extern crate nalgebra as na;
use na::{Matrix3x6, Vector3, Vector6};

pub trait Bvp {
    fn calculate(&self, z: &Vector6<f64>, time: f64, result: &mut Vector3<f64>);
    fn jacobi(
        &self,
        z: &Vector6<f64>,
        time: f64,
        result_z: &mut Matrix3x6<f64>,
        resutl_t: &mut Vector3<f64>,
    );
    fn get_pos(&self) -> &Vector3<f64>;
}
