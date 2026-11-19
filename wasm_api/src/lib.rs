extern crate bal_core;
use bal_core::{
    AnalyticalLinear, BaseParams, FixedTarget, Model, MovingTarget, NumericalLinear,
    NumericalQuadratic, Solution, Solver,
};
use cc_wasm_api::export_funcs;
use cc_wasm_api::lua_api::Exportable;
use cc_wasm_api::lua_api::LuaResult;
use nalgebra::Vector3;
use paste::paste;

pub struct ExportableSolution {
    pub inner: Solution,
}

impl Exportable for ExportableSolution {
    fn export(&self) {
        let export_result = vec![self.inner.pitch, self.inner.yaw, self.inner.time];
        export_result.export();
    }
}

pub fn fixed_target_solve_for<T>(
    model: T,
    barrel: f64,
    v: f64,
    x: f64,
    y: f64,
    z: f64,
) -> Vec<Solution>
where
    T: Model,
{
    let solver = Solver {
        model,
        k: barrel,
        v,
    };
    let bvp = FixedTarget {
        pos: Vector3::new(x, y, z),
    };
    solver.solve(&bvp, &Some(1e-6))
}

pub fn moving_target_solve_for<T>(
    model: T,
    barrel: f64,
    v: f64,
    x: f64,
    y: f64,
    z: f64,
    vx: f64,
    vy: f64,
    vz: f64,
    ax: f64,
    ay: f64,
    az: f64,
) -> Vec<Solution>
where
    T: Model,
{
    let solver = Solver {
        model,
        k: barrel,
        v,
    };
    let bvp = MovingTarget {
        pos0: Vector3::new(x, y, z),
        vel0: Vector3::new(vx, vy, vz),
        acc: Vector3::new(ax, ay, az),
    };
    solver.solve(&bvp, &Some(1e-6))
}
macro_rules! instantiate_solvers {
    ($($t:ident),*) => {
        paste! {
            $(
                pub fn [<fixed_target_solve_for_ $t:snake>](drag: f64, gravity: f64, barrel: f64, v: f64, x: f64, y: f64, z: f64) -> LuaResult<Vec<ExportableSolution>> {
                    let params = BaseParams{
                        gravity: Some(gravity),
                        drag: Some(drag)
                    };
                    let model = $t::from_params(&params);
                    Ok(fixed_target_solve_for(model, barrel, v, x, y, z).into_iter().map(
                            |s| ExportableSolution {inner: s}
                    ).collect())
                }
                pub fn [<moving_target_solve_for_ $t:snake>](drag: f64,
                    gravity: f64,
                    barrel: f64,
                    v: f64,
                    x: f64,
                    y: f64,
                    z: f64,
                    vx: f64,
                    vy: f64,
                    vz: f64,
                    ax: f64,
                    ay: f64,
                    az: f64) -> LuaResult<Vec<ExportableSolution>> {
                    let params = BaseParams{
                        gravity: Some(gravity),
                        drag: Some(drag)
                    };
                    let model = $t::from_params(&params);
                    Ok(moving_target_solve_for(model, barrel, v, x, y, z, vx, vy, vz, ax, ay, az).into_iter().map(
                            |s| ExportableSolution {inner: s}
                    ).collect())
                }
            )*
            export_funcs!(
                $(
                    [<fixed_target_solve_for_ $t:snake>], [<moving_target_solve_for_ $t:snake>]
                ),*
            );
        }
    };
}

instantiate_solvers!(AnalyticalLinear, NumericalLinear, NumericalQuadratic);
