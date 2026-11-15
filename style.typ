#import "@preview/ctheorems:2.0.0": *
#import thm-themes.ams: *

#let appendix(body) = {
  set heading(numbering: "A", supplement: [Appendix])
  counter(heading).update(0)
  body
}

#let thm = thm.with(base: none, base-level: none)
#let proof = proof.with(supplement: "Доказательство")
#let theorem = thm.with(
  supplement: "Теорема",
)

#let lemma = thm.with(
  supplement: "Лемма",
)

#let proposition = thm.with(
  supplement: "Предложение",
)

#let hypothesis = thm.with(
  supplement: "Гипотеза",
)

#let zam = thm.with(
  supplement: "Замечание",
)
#let template(body) = {
  set page(
    paper: "a4",
    margin: (x: 2cm, y: 2cm),
    numbering: "1",
  )

  set text(
    font: "New Computer Modern",
    size: 12pt,
    lang: "ru",
  )
  show math.equation: set text(size: 12pt)
  show ref: it => {
    let eq = math.equation
    let el = it.element
    if el == none or el.func() != eq { return it }
    if it.supplement != none {
      it.supplement
      [ ]
    }
    link(el.location(), counter(eq).display(at: el.location()))
  }

  import "@preview/ctheorems:2.0.0": *
  import thm-themes.ams: *
  show: thm-rules.with(qed-symbol: $square$)
  set par(
    justify: true,
    first-line-indent: 1.25cm,
    spacing: 0.65em,
  )
  set math.equation(numbering: "(1)")

  set heading(numbering: "I.1.")
  body
}

