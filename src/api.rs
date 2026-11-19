extern crate bal_core;
use crate::bal_core::{FixedTarget, Model, MovingTarget, Solution, Solver};
use axum::Json;
use axum::extract::Query;
use axum::http::StatusCode;
use serde::{Deserialize, Serialize};

macro_rules! define_bvp_query {
    ( $( $Variant:ident ( $Type:ty ) ),* $(,)? ) => {
        #[derive(Deserialize)]
        #[serde(untagged)]
        pub enum BvpQuery {
            $( $Variant($Type), )*
        }

        impl BvpQuery {
            pub fn solve<M: Model>(&self, solver: &Solver<M>, tol: &Option<f64>) -> Vec<Solution> {
                match &self {
                    $( BvpQuery::$Variant(bvp) => solver.solve(bvp, tol), )*
                }
            }
        }
    };
}

// Перечисление всех стратегий с переключением по количеству и типу параметров.
define_bvp_query! {
    FixedTarget(FixedTarget),
    MovingTarget(MovingTarget),
}

// Общие параметры запроса
#[derive(Deserialize)]
pub struct SolveQuery<M: Model> {
    #[serde(flatten)]
    pub params: M::Params,
    pub k: f64,
    pub v: f64,
    pub tol: Option<f64>,
    #[serde(flatten)]
    pub bvp: BvpQuery,
}

#[derive(Serialize)]
pub struct SolveResponse {
    pub solutions: Vec<Solution>,
}

pub async fn solve_handler<M>(
    Query(query): Query<SolveQuery<M>>,
) -> Result<Json<SolveResponse>, StatusCode>
where
    M: Model,
    M::Params: Send + Sync + 'static,
{
    let solutions = tokio::task::spawn_blocking(move || {
        let model = M::from_params(&query.params);

        let solver = Solver {
            k: query.k,
            v: query.v,
            model,
        };
        query.bvp.solve(&solver, &query.tol)
    })
    .await
    .inspect_err(|err| eprintln!("Ошибка фоновой задачи: {err:?}"))
    .map_err(|_| StatusCode::INTERNAL_SERVER_ERROR)?;
    Ok(Json(SolveResponse { solutions }))
}
