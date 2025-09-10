/// [ppi:100]
#import "../../lib.typ": *

#set page(width: auto, height: auto, margin: 5pt)

#let result = build-layout(
  node("A", 20pt, 5pt),
  node("B", 50pt, 20pt),
  node("C", 4pt, 3pt),
  
  edge("A", "B"),
  edge("B", "C"),
  edge("A", "C"),
)

#layout-render(result)