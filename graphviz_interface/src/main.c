#ifdef TEST
#define EMSCRIPTEN_KEEPALIVE
#define DEBUG(...) printf(__VA_ARGS__)
#define ERROR(str) printf("Error: %s\n", str)
#define DEBUG_BLOCK(block)                                                                                   \
    do {                                                                                                     \
        block                                                                                                \
    } while (0)
#else
#define DEBUG(...)
#define DEBUG_BLOCK(block)
#define ERROR(str) write_error_message(str)
#include "protocol/plugin.h"
#endif

#define GRAPHVIZ_ERROR wasm_minimal_protocol_send_result_to_host((uint8_t *)errBuff, strlen(errBuff))

#include "protocol/protocol.h"
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ELEMENT_LABEL_INFOS(elem, out_label, attr_name, macro)                                               \
    if (aghtmlstr(agget(elem, attr_name))) {                                                                 \
        (out_label) = malloc(sizeof(LayoutLabel));                                                           \
        if (!(out_label)) {                                                                                  \
            ERROR("Failed to allocate memory for element label");                                            \
            return 1;                                                                                        \
        }                                                                                                    \
        (out_label)->x = (float)(macro(elem)->pos.x);                                                        \
        (out_label)->y = (float)(macro(elem)->pos.y);                                                        \
        (out_label)->width = (float)(macro(elem)->dimen.x);                                            \
        (out_label)->height = (float)(macro(elem)->dimen.y);                                           \
    }

char errBuff[1024];
int vizErrorf(char *str) {
    const char *intro = "Diagraph error: ";
    size_t intro_len = strlen(intro);
    strncpy(errBuff + intro_len, str, sizeof(errBuff) - intro_len);
    memcpy(errBuff, intro, intro_len);
    return 0;
}

/**
 * Creates and returns a Graphviz label that has the specified dimensions.
 */
char *create_label_for_dimension(graph_t *g, double width, double height) {
    if (width < 1e-5 && height < 1e-5) {
        return agstrdup_text(g, "");
    }
    char label[2048];
    snprintf(label, sizeof(label),
             "<TABLE "
#ifndef SHOW_DEBUG_BOXES
             "BORDER=\"0\" "
#endif
             "FIXEDSIZE=\"true\" WIDTH=\"%lf\" "
             "HEIGHT=\"%lf\"><TR><TD></TD></TR></TABLE>",
             width, height);
    return agstrdup_html(g, label);
}

/**
 * @brief Write an error message to the host.
 *
 * @param message the message to write
 */
void write_error_message(char *message) {
    wasm_minimal_protocol_send_result_to_host((uint8_t *)message, strlen(message));
}

void set_edge_label(graph_t *g, void *e, const struct Size_t *label_data, char *attr_name) {
    if (label_data) {
        DEBUG("Setting label %s: width=%f, height=%f\n", attr_name, label_data->width, label_data->height);
        const char *label =
            create_label_for_dimension(g, label_data->width, label_data->height);
        agset_html(e, attr_name, label);
        agstrfree(g, label, true);
    }
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
        agattr_text(g,
                    input_graph.attributes[i].for_ == 1   ? AGNODE
                    : input_graph.attributes[i].for_ == 2 ? AGEDGE
                                                          : AGRAPH,
                    input_graph.attributes[i].key, input_graph.attributes[i].value);
    }

    // initialize attribute table with defaults
    agattr_text(g, AGEDGE, "dir", "none");
    agattr_text(g, AGNODE, "arrowhead", "none");
    agattr_text(g, AGNODE, "arrowtail", "none");
    agattr_text(g, AGRAPH, "pad", "0");
    agattr_text(g, AGRAPH, "margin", "0");

    agattr_text(g, AGNODE, "width", "0.1");
    agattr_text(g, AGNODE, "height", "0.1");
    agattr_text(g, AGNODE, "margin", "0");
    agattr_text(g, AGNODE, "xlabel", "");
    agattr_text(g, AGNODE, "shape", "none");
    agattr_text(g, AGNODE, "fixedsize", "true");

    agattr_text(g, AGEDGE, "label", "");
    agattr_text(g, AGEDGE, "xlabel", "");
    agattr_text(g, AGEDGE, "headlabel", "");
    agattr_text(g, AGEDGE, "taillabel", "");

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
        snprintf(width_str, sizeof(width_str), "%.3f", PS2INCH(input_graph.nodes[i].width));
        snprintf(height_str, sizeof(height_str), "%.3f", PS2INCH(input_graph.nodes[i].height));

        DEBUG("Node %s: width=%s, height=%s\n", input_graph.nodes[i].name, width_str, height_str);

        agset_text(n, "width", width_str);
        agset_text(n, "height", height_str);
        agset_text(n, "label", "");

        set_edge_label(g, n, input_graph.nodes[i].xlabel, "xlabel");
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
        set_edge_label(g, e, input_graph.edges[i].label, "label");
        set_edge_label(g, e, input_graph.edges[i].xlabel, "xlabel");
        set_edge_label(g, e, input_graph.edges[i].headlabel, "headlabel");
        set_edge_label(g, e, input_graph.edges[i].taillabel, "taillabel");

        for (int j = 0; j < input_graph.edges[i].attributes_len; j++) {
            agset_text(e, input_graph.edges[i].attributes[j].key, input_graph.edges[i].attributes[j].value);
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

    DEBUG_BLOCK({
        char *result;
        size_t svg_chunk_size;
        gvRenderData(gvc, g, "dot", &result, &svg_chunk_size);
        DEBUG("Graphviz output:\n%s\n", result);
        free(result);
    });

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
        layout.nodes[node_index].width = (float)INCH2PS(ND_width(n)); // Convert from inches to points
        layout.nodes[node_index].height = (float)INCH2PS(ND_height(n));
        const char *name = agnameof(n);
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

        ELEMENT_LABEL_INFOS(n, layout.nodes[node_index].xlabel, "xlabel", ND_xlabel)

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
            const char *head_name = agnameof(head);
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

            ELEMENT_LABEL_INFOS(e, layout.edges[edges_index].label, "label", ED_label)
            ELEMENT_LABEL_INFOS(e, layout.edges[edges_index].xlabel, "xlabel", ED_xlabel)
            ELEMENT_LABEL_INFOS(e, layout.edges[edges_index].headlabel, "headlabel", ED_head_label)
            ELEMENT_LABEL_INFOS(e, layout.edges[edges_index].taillabel, "taillabel", ED_tail_label)

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
    const char *kind = "layout";
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