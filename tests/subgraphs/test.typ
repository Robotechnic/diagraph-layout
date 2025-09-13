/// [ppi:100]
#import "../../lib.typ": *

#set page(width: auto, height: auto, margin: 5pt)

#let result = layout-graph(
  node("A", width: 20pt, height: 20pt),
  node("B", width: 20pt, height: 20pt),
  node("C", width: 20pt, height: 20pt),
  node("D", width: 20pt, height: 20pt),

	edge("A", "B"),
	edge("B", "C"),
	edge("C", "D"),
	edge("A", "D"),

	subgraph("A", "D", rank: "same"),
	subgraph("B", "C", rank: "same"),
)
// #result
#render-layout(result)
