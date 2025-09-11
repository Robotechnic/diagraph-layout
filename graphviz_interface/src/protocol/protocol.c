#include "protocol.h"
int big_endian_decode(uint8_t const *buffer, int size){
    int value = 0;
    for (int i = 0; i < size; i++) {
        value |= buffer[i] << (8 * (size - i - 1));
    }
    return value;
}

void big_endian_encode(int value, uint8_t *buffer, int size) {
    for (int i = 0; i < sizeof(int); i++) {
        buffer[i] = (value >> (8 * (sizeof(int) - i - 1))) & 0xFF;
    }
}

float decode_float(uint8_t *buffer) {
	int value = big_endian_decode(buffer, TYPST_INT_SIZE);
	if (value == 0) {
		return 0.0f;
	}
	union FloatBuffer {
		float f;
		int i;
	} float_buffer;
	float_buffer.i = value;
	return float_buffer.f;
}

void encode_float(float value, uint8_t *buffer) {
	if (value == 0.0f) {
		big_endian_encode(0, buffer, TYPST_INT_SIZE);
	} else {
		union FloatBuffer {
			float f;
			int i;
		} float_buffer;
		float_buffer.f = value;
		big_endian_encode(float_buffer.i, buffer, TYPST_INT_SIZE);
	}
}

size_t list_size(void *list, size_t size, size_function sf, size_t element_size) {
    size_t result = 0;
    for (int i = 0; i < size; i++) {
        result += sf(list + i * element_size);
    }
    return result;
}

size_t optional_size(void *opt, size_function sf) {
    return 1 + (opt ? sf(opt) : 0);
}

size_t int_size(const void* elem) {
    return TYPST_INT_SIZE;
}
size_t float_size(const void *elem) {
    return TYPST_INT_SIZE;
}
size_t bool_size(const void *elem) {
    return TYPST_INT_SIZE;
}
size_t char_size(const void *elem) {
    return 1;
}
size_t string_size(const void *elem) {
    if (!elem || !((char *)elem)[0]) {
        return 1;
    }
    return strlen((char *)elem) + 1;
}
size_t string_list_size(char **list, size_t size) {
	size_t result = 0;
	for (size_t i = 0; i < size; i++) {
		result += string_size(list[i]);
	}
	return result;
}

void free_Attribute(Attribute *s) {
    if (s->key) {
        free(s->key);
    }
    if (s->value) {
        free(s->value);
    }
}
int decode_Attribute(uint8_t *__input_buffer, size_t buffer_len, Attribute *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_STR(out->key)
    NEXT_STR(out->value)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_Size(Size *s) {
}
int decode_Size(uint8_t *__input_buffer, size_t buffer_len, Size *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_Node(Node *s) {
    if (s->name) {
        free(s->name);
    }
    if (s->xlabel) {
    free_Size(&s->xlabel[0]);
        free(s->xlabel);
    }
}
int decode_Node(uint8_t *__input_buffer, size_t buffer_len, Node *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_STR(out->name)
    NEXT_FLOAT(out->width)
    NEXT_FLOAT(out->height)
    bool has_xlabel;
    NEXT_CHAR(has_xlabel)
    if (has_xlabel) {
        out->xlabel = malloc(sizeof(Size));
    if ((err = decode_Size(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->xlabel[0], &__buffer_offset))){return err;}
    } else {
        out->xlabel = NULL;
    }
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_Edge(Edge *s) {
    if (s->tail) {
        free(s->tail);
    }
    if (s->head) {
        free(s->head);
    }
    for (size_t i = 0; i < s->attributes_len; i++) {
    free_Attribute(&s->attributes[i]);
    }
    free(s->attributes);
    if (s->label) {
    free_Size(&s->label[0]);
        free(s->label);
    }
    if (s->xlabel) {
    free_Size(&s->xlabel[0]);
        free(s->xlabel);
    }
    if (s->headlabel) {
    free_Size(&s->headlabel[0]);
        free(s->headlabel);
    }
    if (s->taillabel) {
    free_Size(&s->taillabel[0]);
        free(s->taillabel);
    }
}
int decode_Edge(uint8_t *__input_buffer, size_t buffer_len, Edge *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_STR(out->tail)
    NEXT_STR(out->head)
    NEXT_INT(out->attributes_len)
    if (out->attributes_len == 0) {
        out->attributes = NULL;
    } else {
        out->attributes = malloc(out->attributes_len * sizeof(Attribute));
        if (!out->attributes){
            return 1;
        }
        for (size_t i = 0; i < out->attributes_len; i++) {
    if ((err = decode_Attribute(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->attributes[i], &__buffer_offset))){return err;}
        }
    }
    bool has_label;
    NEXT_CHAR(has_label)
    if (has_label) {
        out->label = malloc(sizeof(Size));
    if ((err = decode_Size(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->label[0], &__buffer_offset))){return err;}
    } else {
        out->label = NULL;
    }
    bool has_xlabel;
    NEXT_CHAR(has_xlabel)
    if (has_xlabel) {
        out->xlabel = malloc(sizeof(Size));
    if ((err = decode_Size(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->xlabel[0], &__buffer_offset))){return err;}
    } else {
        out->xlabel = NULL;
    }
    bool has_headlabel;
    NEXT_CHAR(has_headlabel)
    if (has_headlabel) {
        out->headlabel = malloc(sizeof(Size));
    if ((err = decode_Size(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->headlabel[0], &__buffer_offset))){return err;}
    } else {
        out->headlabel = NULL;
    }
    bool has_taillabel;
    NEXT_CHAR(has_taillabel)
    if (has_taillabel) {
        out->taillabel = malloc(sizeof(Size));
    if ((err = decode_Size(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->taillabel[0], &__buffer_offset))){return err;}
    } else {
        out->taillabel = NULL;
    }
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_GraphAttribute(GraphAttribute *s) {
    if (s->key) {
        free(s->key);
    }
    if (s->value) {
        free(s->value);
    }
}
int decode_GraphAttribute(uint8_t *__input_buffer, size_t buffer_len, GraphAttribute *out, size_t *buffer_offset) {
    size_t __buffer_offset = 0;
    int err;
    (void)err;
    NEXT_INT(out->for_)
    NEXT_STR(out->key)
    NEXT_STR(out->value)
    *buffer_offset += __buffer_offset;
    return 0;
}
void free_LayoutLabel(LayoutLabel *s) {
}
size_t LayoutLabel_size(const void *s){
	return TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE;
}
int encode_LayoutLabel(const LayoutLabel *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = LayoutLabel_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    FLOAT_PACK(s->x)
    FLOAT_PACK(s->y)
    FLOAT_PACK(s->width)
    FLOAT_PACK(s->height)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_LayoutNode(LayoutNode *s) {
    if (s->name) {
        free(s->name);
    }
    if (s->xlabel) {
    free_LayoutLabel(&s->xlabel[0]);
        free(s->xlabel);
    }
}
size_t LayoutNode_size(const void *s){
	return string_size(((LayoutNode*)s)->name) + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + optional_size(((LayoutNode*)s)->xlabel, LayoutLabel_size);
}
int encode_LayoutNode(const LayoutNode *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = LayoutNode_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    STR_PACK(s->name)
    FLOAT_PACK(s->x)
    FLOAT_PACK(s->y)
    FLOAT_PACK(s->width)
    FLOAT_PACK(s->height)
    CHAR_PACK(s->xlabel != NULL)
    if (s->xlabel) {
        if ((err = encode_LayoutLabel(&s->xlabel[0], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_ControlPoint(ControlPoint *s) {
}
size_t ControlPoint_size(const void *s){
	return TYPST_INT_SIZE + TYPST_INT_SIZE;
}
int encode_ControlPoint(const ControlPoint *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = ControlPoint_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    FLOAT_PACK(s->x)
    FLOAT_PACK(s->y)

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_LayoutEdge(LayoutEdge *s) {
    for (size_t i = 0; i < s->points_len; i++) {
    free_ControlPoint(&s->points[i]);
    }
    free(s->points);
    if (s->head) {
        free(s->head);
    }
    if (s->tail) {
        free(s->tail);
    }
    if (s->label) {
    free_LayoutLabel(&s->label[0]);
        free(s->label);
    }
    if (s->xlabel) {
    free_LayoutLabel(&s->xlabel[0]);
        free(s->xlabel);
    }
    if (s->headlabel) {
    free_LayoutLabel(&s->headlabel[0]);
        free(s->headlabel);
    }
    if (s->taillabel) {
    free_LayoutLabel(&s->taillabel[0]);
        free(s->taillabel);
    }
}
size_t LayoutEdge_size(const void *s){
	return TYPST_INT_SIZE + list_size(((LayoutEdge*)s)->points, ((LayoutEdge*)s)->points_len, ControlPoint_size, sizeof(*((LayoutEdge*)s)->points)) + string_size(((LayoutEdge*)s)->head) + string_size(((LayoutEdge*)s)->tail) + optional_size(((LayoutEdge*)s)->label, LayoutLabel_size) + optional_size(((LayoutEdge*)s)->xlabel, LayoutLabel_size) + optional_size(((LayoutEdge*)s)->headlabel, LayoutLabel_size) + optional_size(((LayoutEdge*)s)->taillabel, LayoutLabel_size);
}
int encode_LayoutEdge(const LayoutEdge *s, uint8_t *__input_buffer, size_t *buffer_len, size_t *buffer_offset) {
    size_t __buffer_offset = 0;    size_t s_size = LayoutEdge_size(s);
    if (s_size > *buffer_len) {
        return 2;
    }
    int err;
	(void)err;
    INT_PACK(s->points_len)
    for (size_t i = 0; i < s->points_len; i++) {
        if ((err = encode_ControlPoint(&s->points[i], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    STR_PACK(s->head)
    STR_PACK(s->tail)
    CHAR_PACK(s->label != NULL)
    if (s->label) {
        if ((err = encode_LayoutLabel(&s->label[0], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    CHAR_PACK(s->xlabel != NULL)
    if (s->xlabel) {
        if ((err = encode_LayoutLabel(&s->xlabel[0], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    CHAR_PACK(s->headlabel != NULL)
    if (s->headlabel) {
        if ((err = encode_LayoutLabel(&s->headlabel[0], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    CHAR_PACK(s->taillabel != NULL)
    if (s->taillabel) {
        if ((err = encode_LayoutLabel(&s->taillabel[0], __input_buffer + __buffer_offset, buffer_len, &__buffer_offset))) {
            return err;
        }
    }

    *buffer_offset += __buffer_offset;
    return 0;
}
void free_Engines(Engines *s) {
    for (size_t i = 0; i < s->engines_len; i++) {
    if (s->engines[i]) {
        free(s->engines[i]);
    }
    }
    free(s->engines);
}
size_t Engines_size(const void *s){
	return TYPST_INT_SIZE + string_list_size(((Engines*)s)->engines, ((Engines*)s)->engines_len);
}
int encode_Engines(const Engines *s) {
    size_t buffer_len = Engines_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    INT_PACK(s->engines_len)
    for (size_t i = 0; i < s->engines_len; i++) {
    STR_PACK(s->engines[i])
    }

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
    return 0;
}
void free_Layout(Layout *s) {
    for (size_t i = 0; i < s->nodes_len; i++) {
    free_LayoutNode(&s->nodes[i]);
    }
    free(s->nodes);
    for (size_t i = 0; i < s->edges_len; i++) {
    free_LayoutEdge(&s->edges[i]);
    }
    free(s->edges);
}
size_t Layout_size(const void *s){
	return 1 + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + TYPST_INT_SIZE + list_size(((Layout*)s)->nodes, ((Layout*)s)->nodes_len, LayoutNode_size, sizeof(*((Layout*)s)->nodes)) + TYPST_INT_SIZE + list_size(((Layout*)s)->edges, ((Layout*)s)->edges_len, LayoutEdge_size, sizeof(*((Layout*)s)->edges));
}
int encode_Layout(const Layout *s) {
    size_t buffer_len = Layout_size(s);
    INIT_BUFFER_PACK(buffer_len)
    int err;
	(void)err;
    CHAR_PACK(s->errored)
    FLOAT_PACK(s->scale)
    FLOAT_PACK(s->width)
    FLOAT_PACK(s->height)
    INT_PACK(s->nodes_len)
    for (size_t i = 0; i < s->nodes_len; i++) {
        if ((err = encode_LayoutNode(&s->nodes[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }
    INT_PACK(s->edges_len)
    for (size_t i = 0; i < s->edges_len; i++) {
        if ((err = encode_LayoutEdge(&s->edges[i], __input_buffer + __buffer_offset, &buffer_len, &__buffer_offset))) {
            return err;
        }
    }

    wasm_minimal_protocol_send_result_to_host(__input_buffer, buffer_len);
    return 0;
}
void free_Graph(Graph *s) {
    if (s->engine) {
        free(s->engine);
    }
    for (size_t i = 0; i < s->edges_len; i++) {
    free_Edge(&s->edges[i]);
    }
    free(s->edges);
    for (size_t i = 0; i < s->nodes_len; i++) {
    free_Node(&s->nodes[i]);
    }
    free(s->nodes);
    for (size_t i = 0; i < s->attributes_len; i++) {
    free_GraphAttribute(&s->attributes[i]);
    }
    free(s->attributes);
}
int decode_Graph(size_t buffer_len, Graph *out) {
    INIT_BUFFER_UNPACK(buffer_len)
    int err;
    (void)err;
    NEXT_STR(out->engine)
    NEXT_CHAR(out->directed)
    NEXT_INT(out->edges_len)
    if (out->edges_len == 0) {
        out->edges = NULL;
    } else {
        out->edges = malloc(out->edges_len * sizeof(Edge));
        if (!out->edges){
            return 1;
        }
        for (size_t i = 0; i < out->edges_len; i++) {
    if ((err = decode_Edge(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->edges[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_INT(out->nodes_len)
    if (out->nodes_len == 0) {
        out->nodes = NULL;
    } else {
        out->nodes = malloc(out->nodes_len * sizeof(Node));
        if (!out->nodes){
            return 1;
        }
        for (size_t i = 0; i < out->nodes_len; i++) {
    if ((err = decode_Node(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->nodes[i], &__buffer_offset))){return err;}
        }
    }
    NEXT_INT(out->attributes_len)
    if (out->attributes_len == 0) {
        out->attributes = NULL;
    } else {
        out->attributes = malloc(out->attributes_len * sizeof(GraphAttribute));
        if (!out->attributes){
            return 1;
        }
        for (size_t i = 0; i < out->attributes_len; i++) {
    if ((err = decode_GraphAttribute(__input_buffer + __buffer_offset, buffer_len - __buffer_offset, &out->attributes[i], &__buffer_offset))){return err;}
        }
    }
    FREE_BUFFER()
    return 0;
}
