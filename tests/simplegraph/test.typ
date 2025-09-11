/// [ppi:100]
#import "../../lib.typ": *

#set page(width: auto, height: auto, margin: 5pt)

#let result = build-layout(
  node("A", width: 20pt, height: 5pt),
  node("B", width: 50pt, height: 20pt),
  node("C", width: 4pt, height: 3pt),
  
  edge("A", "B"),
  edge("B", "C"),
  edge("A", "C"),
)

#layout-render(result)