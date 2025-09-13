/// [ppi:100]
#import "../../lib.typ": *


#let result = layout-graph(
	node("A", xlabel: (width: 50pt, height: 20pt)),
	node("B", xlabel: (width: 30pt, height: 15pt)),
	edge("A", "B", 
		label: (width: 50pt, height: 20pt),
		xlabel: (width: 30pt, height: 15pt),
		headlabel: (width: 40pt, height: 10pt),
		taillabel: (width: 20pt, height: 5pt)
	),
)

#result
#render-layout(result)
