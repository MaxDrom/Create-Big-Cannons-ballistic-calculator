use super::Bvp;
extern crate nalgebra as na;

use na::{Matrix3x6, Vector3, Vector6};
use serde::Deserialize;
use serde_aux::field_attributes::deserialize_number_from_string;

#[derive(Deserialize)]
struct FixedTargetRaw {
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub x: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub y: f64,
    #[serde(deserialize_with = "deserialize_number_from_string")]
    pub z: f64,
}

#[derive(Deserialize)]
#[serde(from = "FixedTargetRaw")]
pub struct FixedTarget {
    pub pos: Vector3<f64>,
}

impl From<FixedTargetRaw> for FixedTarget {
    fn from(raw: FixedTargetRaw) -> Self {
        Self {
            pos: Vector3::new(raw.x, raw.y, raw.z),
        }
    }
}

impl Bvp for FixedTarget {
    fn calculate(&self, z: &Vector6<f64>, _time: f64, result: &mut Vector3<f64>) {
        let x = z.fixed_rows::<3>(0);
        *result = x - self.pos;
    }

    fn jacobi(
        &self,
        _z: &Vector6<f64>,
        _time: f64,
        result_z: &mut Matrix3x6<f64>,
        resutl_t: &mut Vector3<f64>,
    ) {
        resutl_t.fill(0.0);

        result_z.fill(0.0);
        result_z[(0, 0)] = 1.0;
        result_z[(1, 1)] = 1.0;
        result_z[(2, 2)] = 1.0;
    }

    fn get_pos(&self) -> &Vector3<f64> {
        &self.pos
    }
}
