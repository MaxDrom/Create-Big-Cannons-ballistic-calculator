extern crate bal_core;
extern crate nalgebra as na;
use axum::{Router, routing::get};
use bal_core::{AnalyticalLinear, NumericalLinear, NumericalQuadratic};
use clap::Parser;
use tokio::net::TcpListener;
mod api;

#[derive(Parser, Debug)]
#[command()]
struct Config {
    /// Хост или URL для прослушивания
    #[arg(short, long, default_value = "127.0.0.1")]
    url: String,

    /// Порт сервера
    #[arg(short, long, default_value_t = 8080)]
    port: u16,
}

#[tokio::main]
async fn main() {
    let config = Config::parse();
    let url = format!("{}:{}", config.url, config.port);
    let app = Router::new()
        .route("/solve/linear", get(api::solve_handler::<AnalyticalLinear>))
        .route(
            "/solve/numerical/linear",
            get(api::solve_handler::<NumericalLinear>),
        )
        .route(
            "/solve/numerical/quadratic",
            get(api::solve_handler::<NumericalQuadratic>),
        );
    let listener = TcpListener::bind(&url)
        .await
        .expect("Не удалось привязаться к адресу {url}");

    println!("API готово к работе на http://{url}");

    axum::serve(listener, app).await.expect("Ошибка сервера");
}
