#include "protocol/protocol.h"

#ifdef TEST
#define EMSCRIPTEN_KEEPALIVE
#define DEBUG(...) printf(__VA_ARGS__)
#define ERROR(str) printf("Error: %s\n", str)
#else
#define DEBUG(...)
#define ERROR(str) write_error_message(str)
#include "protocol/plugin.h"
#endif
        
#define GRAPHVIZ_ERROR wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff))


#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char errBuff[1024];
int vizErrorf(char *str) {
    const char *intro = "Diagraph error: ";
    size_t intro_len = strlen(intro);
    strncpy(errBuff + intro_len, str, sizeof(errBuff) - intro_len);
    memcpy(errBuff, intro, intro_len);
    return 0;
}

/**
 * @brief Write an error message to the host.
 *
 * @param message the message to write
 */
void write_error_message(char *message) {
    wasm_minimal_protocol_send_result_to_host((uint8_t *)message, strlen(message));
}


EMSCRIPTEN_KEEPALIVE
int layout_graph(size_t buffer_len) {
    agseterr(AGERR);
    agseterrf(vizErrorf);

    Graph input_graph = {0};
    int err;
    if ((err = decode_Graph(buffer_len, &input_graph))) {
        if (err == 1) {
            ERROR("Failled to allocate memory for graph");
        } else if (err == 2) {
            ERROR("Buffer length is too small");
        } else {
            ERROR("Failed to decode graph");
        }
        return 1;
    }

#ifdef TEST
    GVC_t *gvc = gvContext();
#else
    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
#endif

    if (!gvc) {
        ERROR("Failed to create Graphviz context");
        free_Graph(&input_graph);
        return 1;
    }
    Agraph_t *g = agopen("G", input_graph.directed ? Agdirected : Agundirected, 0);

    // Set global graph attributes
    for (int i = 0; i < input_graph.attributes_len; i++) {
        agattr_text(
            g,
            input_graph.attributes[i].for_ == 1 ? AGNODE : 
                input_graph.attributes[i].for_ == 2 ? AGEDGE : AGRAPH,
            input_graph.attributes[i].key,
            input_graph.attributes[i].value
        );
    }
    agattr_text(g, AGEDGE, "dir", "none");
    agattr_text(g, AGNODE, "arrowhead", "none");
    agattr_text(g, AGNODE, "arrowtail", "none");
    agattr_text(g, AGRAPH, "pad", "0");
    agattr_text(g, AGRAPH, "margin", "0");

    // Create nodes and edges
    for (int i = 0; i < input_graph.nodes_len; i++) {
        Agnode_t *n = agnode(g, input_graph.nodes[i].name, true);
        if (!n) {
            ERROR("Failed to create node");
            free_Graph(&input_graph);
            agclose(g);
            return 1;
        }
        char width_str[32];
        char height_str[32];
        snprintf(width_str, sizeof(width_str), "%f", input_graph.nodes[i].width);
        snprintf(height_str, sizeof(height_str), "%f", input_graph.nodes[i].height);
        
        agset_text(n, "fixedsize", "true");
        agset_text(n, "shape", "none");
        agset_text(n, "width", "50");
        agset_text(n, "height", height_str);
        agset_text(n, "margin", "0");
        agset_text(n, "label", "");
    }
    for (int i = 0; i < input_graph.edges_len; i++) {
        Agnode_t *tail = agnode(g, input_graph.edges[i].tail, false);
        Agnode_t *head = agnode(g, input_graph.edges[i].head, false);
        if (!tail || !head) {
            ERROR("Failed to find node for edge");
            free_Graph(&input_graph);
            agclose(g);
            return 1;
        }

        Agedge_t *e = agedge(g, head, tail, NULL, true);
        if (!e) {
            ERROR("Failed to create edge");
            free_Graph(&input_graph);
            agclose(g);
            return 1;
        }
    }

    // Layout the graph
    if (gvLayout(gvc, g, input_graph.engine)) {
        GRAPHVIZ_ERROR;
        agclose(g);
        gvFreeContext(gvc);
        free_Graph(&input_graph);
        return 1;
    }

    // Extract the layout information
    Layout layout = {0};
    layout.nodes_len = input_graph.nodes_len;
    layout.nodes = calloc(layout.nodes_len, sizeof(LayoutNode));
    layout.edges_len = input_graph.edges_len;
    layout.edges = calloc(layout.edges_len, sizeof(LayoutEdge));
    free_Graph(&input_graph);
    if (!layout.nodes || !layout.edges) {
        ERROR("Failed to allocate memory for layout nodes or edges");
        gvFreeLayout(gvc, g);
        agclose(g);
        gvFreeContext(gvc);
        return 1;
    }

    int node_index = 0;
    int edges_index = 0;
    for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) {
        assert(node_index < layout.nodes_len);
        layout.nodes[node_index].x = (float)ND_coord(n).x;
        layout.nodes[node_index].y = (float)ND_coord(n).y;
        layout.nodes[node_index].width = ND_width(n) * 72.0f; // Convert from inches to points
        layout.nodes[node_index].height = ND_height(n) * 72.0f;
        char *name =  agnameof(n);
        DEBUG("Node %s\n", name);
        layout.nodes[node_index].name = malloc(strlen(name) + 1);
        if (!layout.nodes[node_index].name) {
            ERROR("Failed to allocate memory for node name");
            free_Layout(&layout);
            gvFreeLayout(gvc, g);
            agclose(g);
            gvFreeContext(gvc);
            return 1;
        }
        strcpy(layout.nodes[node_index].name, name);
        for (Agedge_t *e = agfstedge(g, n); e; e = agnxtedge(g, e, n)) {
            if (AGID(agtail(e)) != AGID(n)) {
                continue; // Only process outgoing edges
            }
            assert(edges_index < layout.edges_len);
            Agnode_t *head = aghead(e);
            if (!head) {
                ERROR("Failed to get edge node names");
                free_Layout(&layout);
                gvFreeLayout(gvc, g);
                agclose(g);
                gvFreeContext(gvc);
                return 1;
            }
            char *head_name = agnameof(head);
            layout.edges[edges_index].tail = malloc(strlen(name) + 1);
            layout.edges[edges_index].head = malloc(strlen(head_name) + 1);
            if (!layout.edges[edges_index].tail || !layout.edges[edges_index].head) {
                ERROR("Failed to allocate memory for edge nodes");
                free_Layout(&layout);
                gvFreeLayout(gvc, g);
                agclose(g);
                gvFreeContext(gvc);
                return 1;
            }
            strcpy(layout.edges[edges_index].tail, name);
            strcpy(layout.edges[edges_index].head, head_name);
            
            DEBUG("Edge from %s to %s\n", layout.edges[edges_index].tail, layout.edges[edges_index].head);

            splines *es = ED_spl(e);
            size_t points_len = 0;
            for (int k = 0; k < es->size; k++) {
                points_len += es->list[k].size;
            }
            layout.edges[edges_index].points_len = points_len;
            layout.edges[edges_index].points = malloc(sizeof(ControlPoint) * points_len);
            if (!layout.edges[edges_index].points) {
                ERROR("Failed to allocate memory for edge control points");
                free_Layout(&layout);
                gvFreeLayout(gvc, g);
                agclose(g);
                gvFreeContext(gvc);
                return 1;
            }
            size_t point_index = 0;
            for (int k = 0; k < es->size; k++) {
                bezier bz = es->list[k];
                for (int l = 0; l < bz.size; l++) {
                    if (point_index >= points_len) {
                        ERROR("Point index out of bounds");
                        free_Layout(&layout);
                        gvFreeLayout(gvc, g);
                        agclose(g);
                        gvFreeContext(gvc);
                        return 1;
                    }
                    layout.edges[edges_index].points[point_index].x = (float)bz.list[l].x;
                    layout.edges[edges_index].points[point_index].y = (float)bz.list[l].y;
                    point_index++;
                }
            }
            edges_index++;
        }
        node_index++;
    }
    layout.width = (float)(GD_bb(g).UR.x - GD_bb(g).LL.x);
    layout.height = (float)(GD_bb(g).UR.y - GD_bb(g).LL.y);
    layout.scale = 1.0f; // Currently not used
    layout.errored = false;
    if (encode_Layout(&layout)) {
        ERROR("Failed to encode layout");
        free_Layout(&layout);
        gvFreeLayout(gvc, g);
        agclose(g);
        gvFreeContext(gvc);
        return 1;
    }
    free_Layout(&layout);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
    return 0;
}


EMSCRIPTEN_KEEPALIVE
int engine_list() {
#ifdef TEST
    GVC_t *gvc = gvContext();
#else
    GVC_t *gvc = gvContextPlugins(lt_preloaded_symbols, false);
#endif

    if (!gvc) {
        ERROR("Failed to create Graphviz context");
        return 1;
    }
    char *kind = "layout";
    int size = 0;
    char **engines_list = gvPluginList(gvc, kind, &size);
    if (!engines_list) {
        ERROR("Failed to get engines list");
        return 1;
    }
    Engines engines;
    engines.engines_len = size;
    engines.engines = engines_list;
    if (encode_Engines(&engines)) {
        ERROR("Failed to encode engines list");
        return 1;
    }
    free_Engines(&engines);
    return 0;
}