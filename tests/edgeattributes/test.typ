/// [ppi:100]
#import "../../lib.typ": *

#set page(width: auto, height: auto, margin: 5pt)


#let result = layout-graph(
  node("A"),
  node("B"),
  node("C"),
  edge("A", "B"),
  edge("B", "C"),
  edge("A", "C", weight: "2"),
)

#result

#render-layout(result)
