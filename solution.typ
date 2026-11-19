#import "style.typ": *
#show: template

= Линейная баллистика
== Решение уравнений движения

Если покопаться в коде Create: Big Cannons, то мы увидим такую формулу расчета скорости и положения снаряда на следующий тик:

$
  cases(
    v^(i+1) = (1-d)v^i + g,
    x^(i+1) = x^i + v^i - v^i d/2 + g/2
  )
$<base>
где $v^i, x^i$ --- скорость и положение на тике $i$, $d$ --- коэффициент драга, $g$ --- ускорение свободного падения. Запись здесь векторная, единицы времени: тики, единицы длины: блоки.

#lemma(
  [Решение @base[уравнений] в виде $v = v(t, v^0, x^0), x=x(t, v^0, x^0)$ имеет вид:
    $
      cases(
        v(t) = v^0 (1-d)^t + g (1-(1-d)^t)/d,
        x(t) = x^0 + (1- d/2)(v^0 - g/d) (1-(1-d)^t)/d + t/d g
      )
    $<solution>
  ],
)
#proof([
  Если подставить $t = i+1$, то это получается по индукции. Для $t in.not bb(Z)$ эта формула получается как аналитическое продолжение формулы для целых $t$.
])

Матрица чувствительности дается уравнениями:
$
  cases(
    (partial v)/(partial x^0) = 0,
    (partial x)/(partial x^0) = E,
    (partial v)/(partial v^0) = (1-d)^t,
    (partial x)/(partial v^0) = (1-d/2)(1-1(1-d)^t)/d
  )
$

Производная по времени на правом конце:
$
  cases(
    (partial v)/(partial t) = v^0 ln(1-d) (1-d)^t -g ln(1-d) (1-d)^t/d,
    (partial x)/(partial t) = g/d - (1-d/2)(v^0 - g/d)ln(1-d)(1-d)^t
  )
$

= Квадратичная баллистика
== Уравнения движения и уравнение в вариациях

Для квадратичной баллистики уравнения движения:

$
  cases(
    v^(i+1) = v^i -d norm(v^i)_2 v^i + g,
    x^(i+1) = x^i + v^i - d norm(v^i)_2 v^i/2 + g/2
  )
$

Уравнение в вариациях:

$
  cases(
    (partial v^(i+1))/(partial x^0) = 0,
    (partial x^(i+1))/(partial x^0) = E,
    (partial v^(i+1))/(partial v^0) = (partial v^i)/(partial v^0) - d/norm(v^i)_2 v^i ((partial v^(i))/(partial v^0) v^i) - d norm(v^i)_2 (partial v^i)/(partial v^0),
    (partial x^(i+1))/(partial v^0) = (partial x^i)/(partial v^0) + (partial v^i)/(partial v^0) - d/(2 norm(v^i)_2) v^i ((partial v^(i))/(partial v^0) v^i) - d/2 norm(v^i)_2 (partial v^i)/(partial v^0)
  )
$

Эти уравнения записаны в тиках, но у нас задача пристрелки со свободным концом, поэтому сначала надо перейти к произвольному времени шага $tau$:
$
  cases(
    v^(i+1) = v^i -d norm(v^i)_2 v^i tau + g tau,
    x^(i+1) = x^i + v^i tau - d norm(v^i)_2 v^i/2 tau^2 + g/2 tau^2,
    (partial v^(i+1))/(partial x^0) = 0,
    (partial x^(i+1))/(partial x^0) = E,
    (partial v^(i+1))/(partial v^0) = (partial v^i)/(partial v^0) - d/norm(v^i)_2 v^i ((partial v^(i))/(partial v^0) v^i) tau - d norm(v^i)_2 (partial v^i)/(partial v^0) tau,
    (partial x^(i+1))/(partial v^0) = (partial x^i)/(partial v^0) + (partial v^i)/(partial v^0) tau - d/(2 norm(v^i)_2) v^i ((partial v^(i))/(partial v^0) v^i) tau^2 - d/2 norm(v^i)_2 (partial v^i)/(partial v^0) tau^2
  )
$
Теперь для решения задачи мы зафиксируем количество шагов $N$ и будем варьировать $tau$, поэтому нужно добавить еще частные производные по ней к уравнениям в вариациях, но при достаточно большом числе шагов, честная частная производная по $tau$ будет близка к производной по $tau$ в предположении, что $v^i$ и $x_i$ от $tau$ не зависят, умноженной на $N$. Чем мы воспользуемся для того, чтобы не переусложнить данную заметку.

$
  cases(
    (partial v^(i+1))/(partial tau) = -N d norm(v^i)_2 v^i + N g,
    (partial x^(i+1))/(partial tau) = N v^i - N d norm(v^i)_2 v^i tau + N g tau
  )
$

И того, теперь мы знаем как найти матрицу чувствительности $Phi = (partial z)/(partial z^0)$ (пусть $z=(x, v)$), а так же производную по варьируемому параметру $tau$ на правом конце ($partial$), что позволяет нам найти матрицу частных производных от варьируемых параметров, благодаря чему, мы можем воспользоваться многомерным методом Ньютона.

== Решение задачи
Пусть пушка находится на координатах $(0, 0)$, скорость снаряда $V$, длина дула $k$, мы хотим попасть в цель на расстоянии $l$ с высотой (относительно пушки) $h$. Пусть $theta$ это угол возвышения пушки. Тогда точка вылета снаряда из дула и скорость снаряда:
$
  cases(
    x_0 = k cos(theta),
    y_0 = k sin(theta),
    v_(x 0) = V cos(theta),
    v_(y 0) = V sin(theta)
  )
$

Вектор производных по $theta$:
$
  j = vec(
    -k sin(theta),
    k cos(theta),
    -V sin(theta),
    V cos(theta)
  )
$

Вектор производных по $theta$ на правом конце:

$
  J_0 = Phi j
$

Тогда полный якобиан (с учетом того, что мы ищем ноль $F = vec(x-l, y-h)$):

$
  J = (partial F)/(partial z) (Phi j, partial) = (mat(
      1, 0, 0, 0;
      0, 1, 0, 0;
    ) Phi j, mat(
      1, 0, 0, 0;
      0, 1, 0, 0
    ) partial)
$

Шаг метода ньютона:
$
  vec(
    Delta theta,
    Delta tau
  ) = -J^(-1) F(theta, tau)
$
