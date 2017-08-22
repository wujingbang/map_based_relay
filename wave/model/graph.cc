//
//  graph.h
//  adjacencylist
//
//  Created by Sam Goldman on 6/21/11.
//  Copyright 2011 Sam Goldman. All rights reserved.
//
//  Customized by Wu Jingbang on 2017
//
#include "graph.h"
#include "mbr-utils.h"
#include <string.h>

using namespace ns3;
using namespace mbr;

int compare_edges(const void *aa, const void *bb);
void vertex_free(void *data);

int compare_edges(const void *aa, const void *bb) {
    const Edge *a = (Edge*)aa;
    const Edge *b = (Edge*)bb;
    return (a->road_id > b->road_id) - (a->road_id < b->road_id);
}

void vertex_free(void *data) {
    Vertex *vertex = (Vertex*)data;
    list_free(vertex->edges);
    mbr_free(vertex);
}

Graph *MbrGraph::graph_create() {
    Graph *graph = (Graph*)mbr_malloc(sizeof(Graph));
    graph->vertices = list_create(vertex_free);
    return graph;
}

Vertex *MbrGraph::vertex_create(const char *idStr, double x, double y, uint64_t geoHash, int isIntersection) {
    Vertex *vertex = (Vertex*)mbr_malloc(sizeof(Vertex));
    strcpy(vertex->idStr, idStr);
    vertex->x = x;
    vertex->y = y;
    vertex->geoHash = geoHash;
    vertex->data = NULL;
    //vertex->edges = list_create(kfree);
    vertex->edges = list_create(mbr_free);
    vertex->indegree = 0;
    vertex->outdegree = 0;
    vertex->isIntersection = isIntersection;
    return vertex;
}

Edge *MbrGraph::edge_create(Vertex *vertex, int road_id) {
    Edge *edge = (Edge*)mbr_malloc(sizeof(Edge));
    //Edge *edge = (Edge*)malloc(sizeof(Edge));
    edge->vertex = vertex;
    edge->road_id = road_id;
    return edge;
}

void MbrGraph::graph_add_vertex(Graph *graph, Vertex *vertex) {
    list_add_data(graph->vertices, vertex);
}

void MbrGraph::graph_add_vertex_sorted(Graph *graph, Vertex *vertex, int(*cmp)(const void *a, const void *b)) {
    list_add_data_sorted(graph->vertices, vertex, cmp);
}

void MbrGraph::graph_remove_vertex(Graph *graph, Vertex *vertex) {
    Node_list *n = graph->vertices->head;
    Node_list *prev_n = NULL;
    while (n) {
        if (n->data == vertex) {
            if (!prev_n) {
                graph->vertices->head = n->next;
            }
            else {
                prev_n->next = n->next;
            }
            graph->vertices->count--;
            mbr_free(n);
        }
        else {
            vertex_remove_edge_to_vertex((Vertex*)n->data, vertex);
        }
        prev_n = n;
        n = n->next;
    }
    vertex_free(vertex);
}

void MbrGraph::graph_remove_vertex_undirect(Graph *graph, Vertex *vertex) {
    Node_list *n = graph->vertices->head;
    Node_list *prev_n = NULL;
    while (n) {
        if (n->data == vertex) {
            if (!prev_n) {
                graph->vertices->head = n->next;
            }
            else {
                prev_n->next = n->next;
            }
            graph->vertices->count--;
            mbr_free(n);
        }
        else {
            vertex_remove_edge_to_vertex_undirect((Vertex*)n->data, vertex);
        }
        prev_n = n;
        n = n->next;
    }
    vertex_free(vertex);
}

void MbrGraph::vertex_add_edge(Vertex *vertex, Edge *edge) {
    list_add_data(vertex->edges, edge);
    edge->vertex->indegree++;
    vertex->outdegree++;
}

void MbrGraph::vertex_add_edge_sorted(Vertex *vertex, Edge *edge) {
    list_add_data_sorted(vertex->edges, edge, compare_edges);
    edge->vertex->indegree++;
    vertex->outdegree++;
}

void MbrGraph::vertex_remove_edge(Vertex *vertex, Edge *edge) {
    list_remove_data(vertex->edges, edge);
    edge->vertex->indegree--;
    vertex->outdegree--;
}

void MbrGraph::vertex_add_edge_to_vertex(Vertex *from, Vertex *to, int road_id) {
    Edge *edge = edge_create(to, road_id);
    list_add_data(from->edges, edge);
    to->indegree++;
    from->outdegree++;
}

void MbrGraph::vertex_add_edge_to_vertex_sorted(Vertex *from, Vertex *to, int road_id) {
    Edge *edge = edge_create(to, road_id);
    list_add_data_sorted(from->edges, edge, compare_edges);
    to->indegree++;
    from->outdegree++;
}

// here
void MbrGraph::vertex_remove_edge_to_vertex(Vertex *from, Vertex *to) {
    Node_list *e = from->edges->head;
    Node_list *prev_e = NULL;
    while (e) {
        if (((Edge *)e->data)->vertex == to) {
            if (!prev_e) {
                from->edges->head = e->next;
            }
            else {
                prev_e->next = e->next;
            }
            to->indegree--;
            from->outdegree--;
            //kfree(e->data);
            //kfree(e);
            mbr_free(e->data);
            mbr_free(e);
            break;
        }
        prev_e = e;
        e = e->next;
    }
}


void MbrGraph::vertex_add_edge_to_vertex_undirect(Vertex *from, Vertex *to, int road_id) {
	vertex_add_edge_to_vertex(from, to, road_id);
	vertex_add_edge_to_vertex(to, from, road_id);
}

void MbrGraph::vertex_add_edge_to_vertex_undirect_exclusive(Vertex *from, Vertex *to, int road_id){
	vertex_remove_edge_to_vertex_undirect(from, to);
	vertex_add_edge_to_vertex_undirect(from, to, road_id);
}
void MbrGraph::vertex_remove_edge_to_vertex_undirect(Vertex *from, Vertex *to) {
	vertex_remove_edge_to_vertex(from, to);
	vertex_remove_edge_to_vertex(to, from);
}

//void graph_sort_vertices(Graph *graph, int(*cmp)(const void *a, const void *b)) {
//    list_sort(graph->vertices, cmp);
//}
//
//void vertex_sort_edges(Vertex *vertex) {
//    list_sort(vertex->edges, compare_edges);
//}

int MbrGraph::graph_is_balanced(Graph *g) {
    Node_list *n = g->vertices->head;
//    Node_list *prev_n;
    while (n) {
        Vertex *v = (Vertex*)n->data;
        if (v->indegree != v->outdegree) {
            return 0;
        }
//        prev_n = n;
        n = n->next;
    }
    return 1;
}

void MbrGraph::graph_free(Graph *graph) {
    list_free(graph->vertices);
    //kfree(graph);
    mbr_free(graph);
}

Vertex * MbrGraph::getVertex(Graph *graph, const char* idStr) {
    Node_list *n = graph->vertices->head;
//    Node_list *prev_n = NULL;
    while (n)
    {
        if (!strcmp(((Vertex*)n->data)->idStr, idStr))
        {
            return (Vertex*)n->data;
        }
//        prev_n = n;
        n = n->next;
    }
    return NULL;
}

void MbrGraph::graph_print(Graph *g)
{
	Node_list *q;
    Node_list *p=g->vertices->head;
    while(p)
    {
        Vertex *v = (Vertex*)p->data;
        mbr_dbg(debug_level, ANY, "%s, %lld ",v->idStr, v->geoHash);
        q = v->edges->head;
        while(q)
        {
        	mbr_dbg(debug_level, ANY, "Connect to Edge:%s ",((Edge*)q->data)->vertex->idStr);
            q=q->next;
        }
        mbr_dbg(debug_level, ANY, "\n");
        p=p->next;
    }
}

//��������geoHash��ֵ�ľ���ֵ
uint64_t geohash_compare( uint64_t a, uint64_t b )
{
	if(a>b)
		return a-b;
	else
		return b-a;
}

Vertex* MbrGraph::find_Vertex_by_VehiclePosition(Graph *g, uint64_t geoHash, double x, double y)
{
	uint64_t temp;
	double dist;
	double distmin = 999.0;
	uint64_t min1 = UINT64_MAX;
	uint64_t min2 = UINT64_MAX;
	uint64_t min3 = UINT64_MAX;
	Vertex *vmin, *v[3],*index;
	vmin = NULL;
	v[0] = NULL;
	v[1] = NULL;
	v[2] = NULL;
	Node_list *p=g->vertices->head;
	int i;
	while(p)//����һ��ͼ�ڵ㣬�ҵ������geoHash��ӽ���ͼ�ڵ㣻
	{
		index=(Vertex*)p->data;
		temp=geohash_compare(index->geoHash, geoHash);
		if(temp<min1)
		{
			min3 = min2;
			v[2] = v[1];
			min2 = min1;
			v[1] = v[0];
			min1=temp;
			v[0]=index;
		} else if (temp < min2) {
			min3 = min2;
			v[2] = v[1];
			min2 = temp;
			v[1] = index;
		} else if (temp < min3) {
			min3 = temp;
			v[2] = index;
		}
		p=p->next;
	}
	for (i=0; i<3 && v[i] != NULL; i++)
	{
		dist = get_distance(v[i]->y, v[i]->x, y, x);
		if (dist < distmin)
		{
			distmin = dist;
			vmin = v[i];
		}
	}
	return vmin;
}

typedef struct path          	//BFS�������ݽṹ��
{
	Vertex *v;					//���浱ǰ�ҵ��ĵ�·�ڵ�ָ�룻
	struct path *ancest;		//����BFS�������ϵĸ��ڵ㣻
	Edge *e;					//ָ�򸸽ڵ�ıߣ�
	struct path *next;
}path;


int find_vertex(path *head,Vertex *v)	//�ж�ͼ�ڵ��Ƿ��Ѿ�����������
{
	path *temp=head;
	while(temp)
	{
		if(temp->v == v)
			return 1;
		temp=temp->next;
	}
    return 0;
}

Vertex* print_crossnode(path *p)	//����BFS����·���ϵĽ���ڵ���Ϣ��
{
	int first,second;
	second=p->e->road_id;
	p=p->ancest;
	while(p->ancest != NULL)
	{
		first=second;
		second=p->e->road_id;
		if(first != second)
			return p->v;
		p=p->ancest;
	}
	mbr_dbg(debug_level, ANY, "there is no crossnode between %s and %s!\n",p->v->idStr,p->ancest->v->idStr);  //���û�н���·�ڣ������Ӧ��Ϣ��
	return NULL;
}


int free_path(path *p)
{
	path *temp;
	while(p)
	{
		temp=p;
		p=p->next;
		mbr_free(temp);
	}
	return 0;
}

/**
 * set size of the intersection based on the width of the roads.
 * unimplemented yet!
 */
void MbrGraph::setIntersectionSize(GeoHashSetCoordinate * geohashset, Vertex * this_vertex, Vertex * dst_vertex)
{
	int i;
	geohashset->sx = 3;
	geohashset->sy = 3;
	geohashset->geohashset = (uint64_t**)mbr_malloc(sizeof(uint64_t*) * geohashset->sy);
    for (i = 0; i < geohashset->sy; ++i){
    	geohashset->geohashset[i] = (uint64_t*)mbr_malloc(sizeof(uint64_t) * geohashset->sx);
    }
}

Vertex* MbrGraph::cross_vertex(Vertex *from, Vertex *to)    //���Ҵ�from��to·���ϵĽ���·�ڽڵ㣻
{
	path *head,*tail,*index,*temp;
	Vertex *v;
	int flag=1;

	//��ʼ�����ݽṹ��
	head=tail=index=(path*)mbr_malloc(sizeof(path));
	if(!index)
	{
		mbr_dbg(debug_level, ANY, "malloc  error!\n");
        //exit(0);
    }
	index->v=from;
	index->ancest=NULL;
	index->e=NULL;
	index->next=NULL;
	while(index)       //BFS������
	{
		Node_list *n=index->v->edges->head;
		while(n)
		{
			if(find_vertex(head,((Edge*)n->data)->vertex)==1) //����ýڵ��Ѿ�������������������
			{
				n=n->next;
				continue;
			}
			temp=(path*)mbr_malloc(sizeof(path));
			temp->v=((Edge*)n->data)->vertex;
			temp->ancest=index;
			temp->e=(Edge*)n->data;
			temp->next=NULL;

			//���±������Ľڵ���뵽����β����
			tail->next=temp;
			tail=temp;
			if((flag=strcmp(temp->v->idStr,to->idStr))==0)   //�Ѿ�������Ŀ��ڵ㣬��ֹ������
				break;
			n=n->next;
		}
		if(!flag)
			break;
		index=index->next;
	}
	if(!flag)		//����ҵ�Ŀ��ڵ㣬�򷵻ؽ���·�ڽڵ���Ϣ��
	    {
	    	v=print_crossnode(tail);
	    	free_path(head);
	    	return v;
	    }
	else			//��û������Ŀ��ڵ㣬�������ͨ��Ϣ��
		{
			mbr_dbg(debug_level, ANY, "%s unconnect to %s!",from->idStr,to->idStr);
			free_path(head);
			return NULL;
		}
}
