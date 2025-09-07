#import "../graphviz_interface/protocol.typ": *
#let graphviz = plugin("../graphviz_interface/diagraph.wasm")

#let repr-bytes(data) = {
  for b in data [
    0x#str(b, base: 16),
  ]
}

#let node(name, width, height) = {
  (
    type: "node",
    name: name,
    width: width,
    height: height,
  )
}

#let edge(head, tail) = {
  (
    type: "edge",
    head: head,
    tail: tail,
  )
}

#let graph-attribute(type, key, value) = {
  let type = if type == "GRAPH" { 0 } else if type == "NODE" { 1 } else if type == "EDGE" { 2 } else {
    panic("Invalid graph attribute type for '" + key + "': expected GRAPH, NODE, or EDGE")
  }
  (
    type: "graph-attribute",
    attr-type: type,
    key: key,
    value: value,
  )
}

#let build-layout(engine: "dot", directed: false, ..graph) = {
  let pos = graph.pos()
  for n in graph.named().pairs() {
    pos.push(graph-attribute("GRAPH", n.at(0), n.at(1)))
  }
  let nodes = ()
  let edges = ()
  let graph-attributes = ()
  let nodes-id = (:)
  for item in pos {
    if item.type == "node" {
      nodes.push((
        name: item.name,
        width: item.width,
        height: item.height,
      ))
			if item.name in nodes-id {
				panic("Duplicate node name: " + item.name)
			}
      nodes-id.insert(item.name, nodes.len() - 1)
    } else if item.type == "edge" {
      let head-id = nodes-id.at(item.head, default: none)
      let tail-id = nodes-id.at(item.tail, default: none)
      if head-id == none {
        panic("Edge references unknown node: " + item.head)
      }
      if tail-id == none {
        panic("Edge references unknown node: " + item.tail)
      }
      edges.push((
        tail: item.tail,
        head: item.head,
      ))
    } else if item.type == "graph-attribute" {
      graph-attributes.push((
        for_: item.attr-type,
        key: item.key,
        value: item.value,
      ))
    } else {
      panic("Unknown graph item type: " + item.type)
    }
  }
	let input = (
		engine: engine,
		directed: directed,
		nodes: nodes,
		edges: edges,
		attributes: graph-attributes,
	)
	let input = encode-Graph(input)
  // return repr-bytes(input)
	let output = graphviz.layout_graph(input)
	if output.at(0) == 1 {
		panic("Graph layout failed:" + str(output.slice(1)))
	}
	decode-Layout(output).at(0)
}


#let engine-list() = {
  let engines = graphviz.engine_list()
  decode-Engines(engines).at(0).engines
}
