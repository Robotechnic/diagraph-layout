#import "@preview/cetz:0.4.1": *

// Render graph as lines and rectangles,
#let layout-render(graph) = {
	canvas({
		import draw: *
		
		rect((0, 0), (graph.width, graph.height), stroke: red)
		for n in graph.nodes {
			let tx = n.x - n.width / 2
			let ty = n.y - n.height / 2
			let bx = n.x + n.width / 2
			let by = n.y + n.height / 2
			rect((tx, ty), (bx, by), stroke: blue)
		}
		for e in graph.edges {
			let control-points = for p in e.points { ((p.x, p.y),) }
			line(..control-points, stroke: gray)
		}
	})
}