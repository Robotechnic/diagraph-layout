#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "emscripten.h"

#ifndef PROTOCOL_FUNCTION
#define PROTOCOL_FUNCTION __attribute__((import_module("typst_env"))) extern
#endif

PROTOCOL_FUNCTION void wasm_minimal_protocol_send_result_to_host(const uint8_t *ptr, size_t len);
PROTOCOL_FUNCTION void wasm_minimal_protocol_write_args_to_buffer(uint8_t *ptr);

typedef size_t (*size_function)(const void*);

#define TYPST_INT_SIZE 4

#define INIT_BUFFER_UNPACK(buffer_len)                                                             \
    size_t __buffer_offset = 0;                                                                    \
    uint8_t *__input_buffer = malloc((buffer_len));                                                \
    if (!__input_buffer) {                                                                         \
        return 1;                                                                                  \
    }                                                                                              \
    wasm_minimal_protocol_write_args_to_buffer(__input_buffer);

#define CHECK_BUFFER()                                                                             \
	if (__buffer_offset >= buffer_len) {                                                           \
		return 2;                                                                                  \
	}

#define NEXT_STR(dst)                                                                              \
	CHECK_BUFFER()                                                                                 \
    {                                                                                              \
		if (__input_buffer[__buffer_offset] == '\0') {                                            \
			(dst) = malloc(1);                                                                     \
			if (!(dst)) {                                                                          \
				return 1;                                                                          \
			}                                                                                      \
			(dst)[0] = '\0';                                                                      \
			__buffer_offset++;                                                                     \
		} else {                                                                                   \
			int __str_len = strlen((char *)__input_buffer + __buffer_offset);                      \
			(dst) = malloc(__str_len + 1);                                                         \
			if (!(dst)) {                                                                          \
				return 1;                                                                          \
			}                                                                                      \
			strcpy((dst), (char *)__input_buffer + __buffer_offset);                               \
			__buffer_offset += __str_len + 1;                                                      \
		}                                                                                          \
    }

#define NEXT_INT(dst)                                                                              \
	CHECK_BUFFER()                                                                                 \
    (dst) = big_endian_decode(__input_buffer + __buffer_offset, TYPST_INT_SIZE);                   \
    __buffer_offset += TYPST_INT_SIZE;

#define NEXT_CHAR(dst)                                                                             \
	CHECK_BUFFER()                                                                                 \
    (dst) = __input_buffer[__buffer_offset++];

#define NEXT_FLOAT(dst)                                                                            \
	CHECK_BUFFER()                                                                                 \
    (dst) = decode_float(__input_buffer + __buffer_offset);                                        \
	__buffer_offset += TYPST_INT_SIZE;
    
#define FREE_BUFFER()                                                                              \
    free(__input_buffer);                                                                          \
    __input_buffer = NULL;

#define INIT_BUFFER_PACK(buffer_len)                                                               \
    size_t __buffer_offset = 0;                                                                    \
    uint8_t *__input_buffer = malloc((buffer_len));                                                \
    if (!__input_buffer) {                                                                         \
        return 1;                                                                                  \
    }

#define FLOAT_PACK(fp)                                                                             \
    {                                                                                              \
		if (fp == 0.0f) {  																	       \
			big_endian_encode(0, __input_buffer + __buffer_offset, TYPST_INT_SIZE);                \
		} else {                                                                                   \
			union FloatBuffer { 																   \
				float f;   																	       \
				int i;   																	       \
			} __float_buffer;                                                                      \
			__float_buffer.f = (fp);                                                               \
			big_endian_encode(__float_buffer.i, __input_buffer + __buffer_offset, TYPST_INT_SIZE); \
		}                                                                                          \
		__buffer_offset += TYPST_INT_SIZE;                                                         \
	}

#define INT_PACK(i)                                                                                \
    big_endian_encode((i), __input_buffer + __buffer_offset, TYPST_INT_SIZE);                      \
    __buffer_offset += TYPST_INT_SIZE;

#define CHAR_PACK(c)                                                                               \
    __input_buffer[__buffer_offset++] = (c);

#define STR_PACK(s)                                                                                \
    if (s == NULL || s[0] == '\0') {                                                              \
        __input_buffer[__buffer_offset++] = '\0';                                                 \
    } else {                                                                                       \
        strcpy((char *)__input_buffer + __buffer_offset, (s));                                     \
        size_t __str_len = strlen((s));                                                            \
        __input_buffer[__buffer_offset + __str_len] = '\0';                                       \
        __buffer_offset += __str_len + 1;                                                          \
    }
typedef struct Attribute_t {
    char* key;
    char* value;
} Attribute;
void free_Attribute(Attribute *s);

typedef struct Size_t {
    float width;
    float height;
} Size;
void free_Size(Size *s);

typedef struct Node_t {
    char* name;
    float width;
    float height;
    struct Size_t * xlabel;
} Node;
void free_Node(Node *s);

typedef struct Edge_t {
    char* tail;
    char* head;
    struct Attribute_t * attributes;
    size_t attributes_len;
    struct Size_t * label;
    struct Size_t * xlabel;
    struct Size_t * headlabel;
    struct Size_t * taillabel;
} Edge;
void free_Edge(Edge *s);

typedef struct GraphAttribute_t {
    int for_;
    char* key;
    char* value;
} GraphAttribute;
void free_GraphAttribute(GraphAttribute *s);

typedef struct LayoutLabel_t {
    float x;
    float y;
    float width;
    float height;
} LayoutLabel;
void free_LayoutLabel(LayoutLabel *s);

typedef struct LayoutNode_t {
    char* name;
    float x;
    float y;
    float width;
    float height;
    struct LayoutLabel_t * xlabel;
} LayoutNode;
void free_LayoutNode(LayoutNode *s);

typedef struct ControlPoint_t {
    float x;
    float y;
} ControlPoint;
void free_ControlPoint(ControlPoint *s);

typedef struct LayoutEdge_t {
    struct ControlPoint_t * points;
    size_t points_len;
    char* head;
    char* tail;
    struct LayoutLabel_t * label;
    struct LayoutLabel_t * xlabel;
    struct LayoutLabel_t * headlabel;
    struct LayoutLabel_t * taillabel;
} LayoutEdge;
void free_LayoutEdge(LayoutEdge *s);

typedef struct Engines_t {
    char* * engines;
    size_t engines_len;
} Engines;
void free_Engines(Engines *s);
int encode_Engines(const Engines *s);

typedef struct Layout_t {
    bool errored;
    float scale;
    float width;
    float height;
    struct LayoutNode_t * nodes;
    size_t nodes_len;
    struct LayoutEdge_t * edges;
    size_t edges_len;
} Layout;
void free_Layout(Layout *s);
int encode_Layout(const Layout *s);

typedef struct Graph_t {
    char* engine;
    bool directed;
    struct Edge_t * edges;
    size_t edges_len;
    struct Node_t * nodes;
    size_t nodes_len;
    struct GraphAttribute_t * attributes;
    size_t attributes_len;
} Graph;
void free_Graph(Graph *s);
int decode_Graph(size_t buffer_len, Graph *out);

#endif
