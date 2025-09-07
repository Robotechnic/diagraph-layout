#import "./src/internals.typ": engine-list, build-layout, edge, node, graph-attribute

#let result = build-layout(
	node("A", 20pt, 5pt),
	node("B", 8pt, 101pt),
	node("C", 4pt, 3pt),
	
	edge("A", "B"),
	// edge("B", "C"),
	// edge("A", "C"),
	engine: "osag"
)

#result

#import "@preview/cetz:0.4.1": *

#canvas({
	import draw: *

	rect((0, 0), (result.width, result.height), stroke: red)
	for n in result.nodes {
		let tx = n.x - n.width / 2
		let ty = n.y - n.height / 2
		let bx = n.x + n.width / 2
		let by = n.y + n.height / 2
		rect((tx, ty), (bx, by), stroke: blue)
	}
	for e in result.edges {
		let control-points = for p in e.points { ((p.x, p.y),) }
		line(..control-points, stroke: gray)
	}
})