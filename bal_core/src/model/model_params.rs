use serde::Deserialize;
#[derive(Deserialize)]
pub struct BaseParams {
    pub gravity: Option<f64>,
    pub drag: Option<f64>,
}
