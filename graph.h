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

#include "list.h"
#include "debug.h"
#include "geohash.h"
#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct Edge {
    struct Vertex *vertex;
    int road_id;
} Edge;

typedef struct Vertex {
	//int id;
	char idStr[25];
	uint64_t geoHash;
    void *data;
    struct List *edges;
    int indegree;
    int outdegree;
    int isIntersection;
} Vertex;

typedef struct Graph {
    struct List *vertices;
} Graph;

extern Graph *graph_create(void);
extern Vertex *vertex_create(const char *idStr, uint64_t geoHash, int isIntersection);
extern Edge *edge_create(Vertex *vertex, int road_id);

extern void graph_add_vertex(Graph *graph, Vertex *vertex);
extern void graph_add_vertex_sorted(Graph *graph, Vertex *vertex, int(*cmp)(const void *a, const void *b));
extern void graph_remove_vertex(Graph *graph, Vertex *vertex);

extern void vertex_add_edge(Vertex *vertex, Edge *edge);
extern void vertex_add_edge_sorted(Vertex *vertex, Edge *edge);
extern void vertex_remove_edge(Vertex *vertex, Edge *edge);

extern void vertex_add_edge_to_vertex(Vertex *from, Vertex *to, int road_id);
extern void vertex_add_edge_to_vertex_sorted(Vertex *from, Vertex *to, int road_id);
extern void vertex_remove_edge_to_vertex(Vertex *from, Vertex *to);

/**
 * Tools
 */
extern Vertex * getVertex(Graph *graph, const char* idStr);
/**
 * API for undirect graph
 */
extern void graph_remove_vertex_undirect(Graph *graph, Vertex *vertex);
extern void vertex_add_edge_to_vertex_undirect(Vertex *from, Vertex *to, int road_id);
extern void vertex_remove_edge_to_vertex_undirect(Vertex *from, Vertex *to);

extern void vertex_add_edge_to_vertex_undirect_exclusive(Vertex *from, Vertex *to, int road_id);

extern void graph_print(Graph *g);

/**
 * mbr API
 */
extern Vertex* find_Vertex_by_VehiclePosition(Graph *g, uint64_t geoHash);
extern Vertex* cross_vertex(Vertex *from, Vertex *to);
extern void setIntersectionSize(GeoHashSetCoordinate * geohashset, Vertex * this_vertex, Vertex * dst_vertex);

//void graph_sort_vertices(Graph *graph, int(*cmp)(const void *a, const void *b));
//void vertex_sort_edges(Vertex *vertex);

extern int graph_is_balanced(Graph *graph);

extern void graph_free(Graph *graph);

#ifdef __cplusplus
}
#endif

#endif
