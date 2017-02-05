//
//  graph.h
//  adjacencylist
//
//  Created by Sam Goldman on 6/21/11.
//  Copyright 2011 Sam Goldman. All rights reserved.
//
//  Customized by Wu Jingbang on 2017
//

#ifndef GRAPH_H
#define GRAPH_H

#include <linux/types.h>
#include "list.h"
#include "debug.h"

#define bool int
#define true 1
#define false 0

#define UINT64_MAX  0xffffffffffffffff

typedef struct Edge {
    struct Vertex *vertex;
    int road_id;
} Edge;

typedef struct Vertex {
	//int id;
	unsigned char idStr[25];
	u64 geoHash;
    void *data;
    struct List *edges;
    int indegree;
    int outdegree;
} Vertex;

typedef struct Graph {
    struct List *vertices;
} Graph;

Graph *graph_create(void);
Vertex *vertex_create(char *idStr);
Edge *edge_create(Vertex *vertex, int road_id);

void graph_add_vertex(Graph *graph, Vertex *vertex);
void graph_add_vertex_sorted(Graph *graph, Vertex *vertex, int(*cmp)(const void *a, const void *b));
void graph_remove_vertex(Graph *graph, Vertex *vertex);

void vertex_add_edge(Vertex *vertex, Edge *edge);
void vertex_add_edge_sorted(Vertex *vertex, Edge *edge);
void vertex_remove_edge(Vertex *vertex, Edge *edge);

void vertex_add_edge_to_vertex(Vertex *from, Vertex *to, int road_id);
void vertex_add_edge_to_vertex_sorted(Vertex *from, Vertex *to, int road_id);
void vertex_remove_edge_to_vertex(Vertex *from, Vertex *to);

/**
 * Tools
 */
Vertex * getVertex(Graph *graph, const char* idStr);
/**
 * API for undirect graph
 */
void graph_remove_vertex_undirect(Graph *graph, Vertex *vertex);
void vertex_add_edge_to_vertex_undirect(Vertex *from, Vertex *to, int road_id);
void vertex_remove_edge_to_vertex_undirect(Vertex *from, Vertex *to);

/**
 * mbr API
 */
Vertex* find_Vertex_by_VehiclePosition(Graph *g, u64 geoHash);
Vertex* cross_vertex(Vertex *from, Vertex *to);

//void graph_sort_vertices(Graph *graph, int(*cmp)(const void *a, const void *b));
//void vertex_sort_edges(Vertex *vertex);

bool graph_is_balanced(Graph *graph);

void graph_free(Graph *graph);

#endif
