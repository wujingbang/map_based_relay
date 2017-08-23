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
#include "mbr-common.h"

typedef struct Edge {
    struct Vertex *vertex;
    int road_id;
} Edge;

typedef struct Vertex {
	//int id;
	char idStr[25];
	double x;
	double y;
	uint64_t geoHash;
    void *data;
    struct List *edges;
    int indegree;
    int outdegree;
    int isIntersection;
} Vertex;

typedef struct vertexlist
{
    Vertex *v;
    vertexlist *next;
}vertexlist;

typedef struct Graph {
    struct List *vertices;
} Graph;

namespace ns3{
namespace mbr{

class MbrGraph{
public:
static Graph *graph_create(void);
//Vertex *vertex_create(const char *idStr, uint64_t geoHash, int isIntersection);
static Vertex *vertex_create(const char *idStr, double x, double y, uint64_t geoHash, int isIntersection);
static Edge *edge_create(Vertex *vertex, int road_id);

static void graph_add_vertex(Graph *graph, Vertex *vertex);
static void graph_add_vertex_sorted(Graph *graph, Vertex *vertex, int(*cmp)(const void *a, const void *b));
static void graph_remove_vertex(Graph *graph, Vertex *vertex);

static void vertex_add_edge(Vertex *vertex, Edge *edge);
static void vertex_add_edge_sorted(Vertex *vertex, Edge *edge);
static void vertex_remove_edge(Vertex *vertex, Edge *edge);

static void vertex_add_edge_to_vertex(Vertex *from, Vertex *to, int road_id);
static void vertex_add_edge_to_vertex_sorted(Vertex *from, Vertex *to, int road_id);
static void vertex_remove_edge_to_vertex(Vertex *from, Vertex *to);

/**
 * Tools
 */
static Vertex * getVertex(Graph *graph, const char* idStr);
/**
 * API for undirect graph
 */
static void graph_remove_vertex_undirect(Graph *graph, Vertex *vertex);
static void vertex_add_edge_to_vertex_undirect(Vertex *from, Vertex *to, int road_id);
static void vertex_remove_edge_to_vertex_undirect(Vertex *from, Vertex *to);

static void vertex_add_edge_to_vertex_undirect_exclusive(Vertex *from, Vertex *to, int road_id);

static void graph_print(Graph *g);

/**
 * mbr API
 */
static Vertex* find_Vertex_by_VehiclePosition(Graph *g, uint64_t geoHash, double x, double y);
static vertexlist* cross_vertex(Vertex *from, Vertex *to);
static void setIntersectionSize(GeoHashSetCoordinate * geohashset, Vertex * this_vertex, Vertex * dst_vertex);

//void graph_sort_vertices(Graph *graph, int(*cmp)(const void *a, const void *b));
//void vertex_sort_edges(Vertex *vertex);

static int graph_is_balanced(Graph *graph);

static void graph_free(Graph *graph);

};

}//namespace mbr
}//namespace ns3

#endif
