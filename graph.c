//
//  graph.h
//  adjacencylist
//
//  Created by Sam Goldman on 6/21/11.
//  Copyright 2011 Sam Goldman. All rights reserved.
//
//  Customized by Wu Jingbang on 2017
//

//#include "graph.h"
//#include <linux/string.h>


#include "graph.h"
#include <linux/string.h>
//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>

int compare_edges(const void *aa, const void *bb);
void vertex_free(const void *data);

int compare_edges(const void *aa, const void *bb) {
    const Edge *a = (Edge*)aa;
    const Edge *b = (Edge*)bb;
    return (a->road_id > b->road_id) - (a->road_id < b->road_id);
}

void vertex_free(const void *data) {
    Vertex *vertex = (Vertex*)data;
    list_free(vertex->edges);
    kfree(vertex);
}

Graph *graph_create() {
    Graph *graph = (Graph*)kmalloc(sizeof(Graph), GFP_KERNEL);
    graph->vertices = list_create(vertex_free);
    return graph;
}

Vertex *vertex_create(char *idStr, u64 geoHash) {
    Vertex *vertex = (Vertex*)kmalloc(sizeof(Vertex), GFP_KERNEL);
    strcpy(vertex->idStr, idStr);
    vertex->geoHash = geoHash;
    vertex->data = NULL;
    //vertex->edges = list_create(kfree);
    vertex->edges = list_create(kfree);
    vertex->indegree = 0;
    vertex->outdegree = 0;
    return vertex;
}

Edge *edge_create(Vertex *vertex, int road_id) {
    Edge *edge = kmalloc(sizeof(Edge), GFP_KERNEL);
    //Edge *edge = (Edge*)malloc(sizeof(Edge));
    edge->vertex = vertex;
    edge->road_id = road_id;
    return edge;
}

void graph_add_vertex(Graph *graph, Vertex *vertex) {
    list_add_data(graph->vertices, vertex);
}

void graph_add_vertex_sorted(Graph *graph, Vertex *vertex, int(*cmp)(const void *a, const void *b)) {
    list_add_data_sorted(graph->vertices, vertex, cmp);
}

void graph_remove_vertex(Graph *graph, Vertex *vertex) {
    Node *n = graph->vertices->head;
    Node *prev_n = NULL;
    while (n) {
        if (n->data == vertex) {
            if (!prev_n) {
                graph->vertices->head = n->next;
            }
            else {
                prev_n->next = n->next;
            }
            graph->vertices->count--;
            kfree(n);
        }
        else {
            vertex_remove_edge_to_vertex((Vertex*)n->data, vertex);
        }
        prev_n = n;
        n = n->next;
    }
    vertex_free(vertex);
}

void graph_remove_vertex_undirect(Graph *graph, Vertex *vertex) {
    Node *n = graph->vertices->head;
    Node *prev_n = NULL;
    while (n) {
        if (n->data == vertex) {
            if (!prev_n) {
                graph->vertices->head = n->next;
            }
            else {
                prev_n->next = n->next;
            }
            graph->vertices->count--;
            kfree(n);
        }
        else {
            vertex_remove_edge_to_vertex_undirect((Vertex*)n->data, vertex);
        }
        prev_n = n;
        n = n->next;
    }
    vertex_free(vertex);
}

void vertex_add_edge(Vertex *vertex, Edge *edge) {
    list_add_data(vertex->edges, edge);
    edge->vertex->indegree++;
    vertex->outdegree++;
}

void vertex_add_edge_sorted(Vertex *vertex, Edge *edge) {
    list_add_data_sorted(vertex->edges, edge, compare_edges);
    edge->vertex->indegree++;
    vertex->outdegree++;
}

void vertex_remove_edge(Vertex *vertex, Edge *edge) {
    list_remove_data(vertex->edges, edge);
    edge->vertex->indegree--;
    vertex->outdegree--;
}

void vertex_add_edge_to_vertex(Vertex *from, Vertex *to, int road_id) {
    Edge *edge = edge_create(to, road_id);
    list_add_data(from->edges, edge);
    to->indegree++;
    from->outdegree++;
}

void vertex_add_edge_to_vertex_sorted(Vertex *from, Vertex *to, int road_id) {
    Edge *edge = edge_create(to, road_id);
    list_add_data_sorted(from->edges, edge, compare_edges);
    to->indegree++;
    from->outdegree++;
}

// here
void vertex_remove_edge_to_vertex(Vertex *from, Vertex *to) {
    Node *e = from->edges->head;
    Node *prev_e = NULL;
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
            kfree(e->data);
            kfree(e);
            break;
        }
        prev_e = e;
        e = e->next;
    }
}


void vertex_add_edge_to_vertex_undirect(Vertex *from, Vertex *to, int road_id) {
	vertex_add_edge_to_vertex(from, to, road_id);
	vertex_add_edge_to_vertex(to, from, road_id);
}

void vertex_remove_edge_to_vertex_undirect(Vertex *from, Vertex *to) {
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

bool graph_is_balanced(Graph *g) {
    Node *n = g->vertices->head;
    Node *prev_n;
    while (n) {
        Vertex *v = (Vertex*)n->data;
        if (v->indegree != v->outdegree) {
            return false;
        }
        prev_n = n;
        n = n->next;
    }
    return true;
}

void graph_free(Graph *graph) {
    list_free(graph->vertices);
    //kfree(graph);
    kfree(graph);
}

Vertex * getVertex(Graph *graph, const char* idStr) {
    Node *n = graph->vertices->head;
    Node *prev_n = NULL;
    while (n)
    {
        if (!strcmp(((Vertex*)n->data)->idStr, idStr))
        {
            return (Vertex*)n->data;
        }
        prev_n = n;
        n = n->next;
    }
    return NULL;
}

void graph_print(Graph *g)
{
	Node *q;
    Node *p=g->vertices->head;
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

//计算两个geoHash差值的绝对值
u64 geohash_compare( u64 a, u64 b )
{
	if(a>b)
		return a-b;
	else
		return b-a;
}

Vertex* find_Vertex_by_VehiclePosition(Graph *g, u64 geoHash)
{
	u64 min=UINT64_MAX,temp;
	Vertex *v=NULL,*index;
	Node *p=g->vertices->head;
	while(p)//遍历一遍图节点，找到与给定geoHash最接近的图节点；
	{
		index=(Vertex*)p->data;
		temp=geohash_compare(index->geoHash, geoHash);
		if(temp<min)
		{
			min=temp;
			v=index;
		}
		p=p->next;
	}
	return v;
}

typedef struct path          	//BFS遍历数据结构；
{
	Vertex *v;					//保存当前找到的道路节点指针；
	struct path *ancest;		//保存BFS遍历树上的父节点；
	Edge *e;					//指向父节点的边；
	struct path *next;
}path;


int find_vertex(path *head,Vertex *v)	//判断图节点是否已经被遍历过；
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

Vertex* print_crossnode(path *p)	//返回BFS遍历路径上的交叉节点信息；
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
	//mbr_dbg(debug_level, ANY, "there is no crossnode between %s and %s!\n",p->v->idStr,p->ancest->v->idStr);  //如果没有交叉路口，输出相应信息；
	return NULL;
}


int free_path(path *p)
{
	path *temp;
	while(p)
	{
		temp=p;
		p=p->next;
		kfree(temp);
	}
	return 0;
}

/**
 * set size of the intersection based on the width of the roads.
 * unimplemented yet!
 */
void setIntersectionSize(GeoHashSetCoordinate * geohashset, Vertex * this_vertex, Vertex * dst_vertex)
{
	int i;
	geohashset->sx = 3;
	geohashset->sy = 3;
	geohashset->geohashset = (u64**)kmalloc(sizeof(u64*) * geohashset->sy, GFP_KERNEL);
    for (i = 0; i < geohashset->sy; ++i){
    	geohashset->geohashset[i] = (u64*)kmalloc(sizeof(u64) * geohashset->sx, GFP_KERNEL);
    }
}

Vertex* cross_vertex(Vertex *from, Vertex *to)    //查找从from到to路径上的交叉路口节点；
{
	path *head,*tail,*index,*temp;
	Vertex *v;
	int flag=1;

	//初始化数据结构；
	head=tail=index=(path*)kmalloc(sizeof(path),GFP_KERNEL);
	if(!index)
	{
		mbr_dbg(debug_level, ANY, "malloc  error!\n");
        //exit(0);
    }
	index->v=from;
	index->ancest=NULL;
	index->e=NULL;
	index->next=NULL;
	while(index)       //BFS遍历；
	{
		Node *n=index->v->edges->head;
		while(n)
		{
			if(find_vertex(head,((Edge*)n->data)->vertex)==1) //如果该节点已经被遍历过，则跳过；
			{
				n=n->next;
				continue;
			}
			temp=(path*)kmalloc(sizeof(path), GFP_KERNEL);
			temp->v=((Edge*)n->data)->vertex;
			temp->ancest=index;
			temp->e=(Edge*)n->data;
			temp->next=NULL;

			//将新遍历到的节点插入到链表尾部；
			tail->next=temp;
			tail=temp;
			if((flag=strcmp(temp->v->idStr,to->idStr))==0)   //已经遍历到目标节点，终止遍历；
				break;
			n=n->next;
		}
		if(!flag)
			break;
		index=index->next;
	}
	if(!flag)		//如果找到目标节点，则返回交叉路口节点信息；
	    {
	    	v=print_crossnode(tail);
	    	free_path(head);
	    	return v;
	    }
	else			//若没遍历到目标节点，输出不连通信息；
		{
			// mbr_dbg(debug_level, ANY, "%s unconnect to %s!",from->idStr,to->idStr);
			free_path(head);
			return NULL;
		}
}
