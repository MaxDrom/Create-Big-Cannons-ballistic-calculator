use super::Bvp;
extern crate nalgebra as na;
use na::{Matrix3x6, Vector3, Vector6};
use serde::Deserialize;
use serde_aux::field_attributes::deserialize_number_from_string;
#[derive(Deserialize)]
struct MovingTargetRaw {
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub x: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub y: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub z: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub vx: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub vy: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub vz: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub ax: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub ay: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub az: f64,
}

#[derive(Deserialize)]
#[serde(from = "MovingTargetRaw")]
pub struct MovingTarget {
    pub pos0: Vector3<f64>,
    pub vel0: Vector3<f64>,
    pub acc: Vector3<f64>,
}

impl From<MovingTargetRaw> for MovingTarget {
    fn from(value: MovingTargetRaw) -> Self {
        Self {
            pos0: Vector3::new(value.x, value.y, value.z),
            vel0: Vector3::new(value.vx, value.vy, value.vz),
            acc: Vector3::new(value.ax, value.ay, value.az),
        }
    }
}

impl Bvp for MovingTarget {
    fn calculate(&self, z: &Vector6<f64>, time: f64, result: &mut Vector3<f64>) {
        let target_pos = self.pos0 + self.vel0 * time + self.acc * (0.5 * time * time);

        *result = z.fixed_rows::<3>(0) - target_pos;
    }

    fn jacobi(
        &self,
        _z: &Vector6<f64>,
        time: f64,
        result_z: &mut Matrix3x6<f64>,
        result_t: &mut Vector3<f64>,
    ) {
        // Частная производная dF/dz = [I_3x3 | 0_3x3]
        result_z.fill(0.0);
        result_z[(0, 0)] = 1.0;
        result_z[(1, 1)] = 1.0;
        result_z[(2, 2)] = 1.0;

        // Частная производная dF/dt = -v_target(t) = -(v0 + a*t)
        *result_t = -(self.vel0 + self.acc * time);
    }

    fn get_pos(&self) -> &Vector3<f64> {
        &self.pos0
    }
}
