import asyncio
from concurrent.futures import ThreadPoolExecutor
from fastapi import FastAPI
from ballistics_py import BallisticCalculator
from async_lru import alru_cache
from pydantic import BaseModel

app = FastAPI()


class ResultSchema(BaseModel):
    angle: float
    maxH: float
    time: float

    class Config:
        from_attributes = True


thread_executor = ThreadPoolExecutor(max_workers=2)


@alru_cache(maxsize=None)
async def get_ballistic_calculator(
    velocity: float, drag: float
) -> BallisticCalculator:
    loop = asyncio.get_running_loop()

    return await loop.run_in_executor(
        thread_executor, BallisticCalculator, drag, velocity
    )


@app.get("/{velocity}/{drag}/max_distance")
async def max_distance(velocity: float, drag: float):
    calculator = await get_ballistic_calculator(velocity, drag)
    return {"max_distance": calculator.max_distance}


@app.get("/{velocity}/{drag}/find", response_model=list[ResultSchema])
async def calculate(velocity: float, drag: float, x: float, y: float):
    cpp_obj = await get_ballistic_calculator(velocity, drag)
    loop = asyncio.get_running_loop()
    result = await loop.run_in_executor(thread_executor, cpp_obj.find, x, y)

    return result


@app.get("/{velocity}/{drag}/at")
async def at(velocity: float, drag: float, time: float, angle: float):
    calculator = await get_ballistic_calculator(velocity, drag)
    loop = asyncio.get_running_loop()
    x, y = await asyncio.gather(
        loop.run_in_executor(thread_executor, calculator.x, time, angle),
        loop.run_in_executor(thread_executor, calculator.y, time, angle),
    )
    return {"x": x, "y": y}
