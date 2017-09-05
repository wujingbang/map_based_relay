//
//  list.h
//  adjacencylist
//
//  Created by Sam Goldman on 6/21/11.
//  Copyright 2011 Sam Goldman. All rights reserved.
//
//  Customized by Wu Jingbang on 2017
//
#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*node_data_free_callback_t)(void *);

typedef struct Node_list {
    void *data;
    struct Node_list *next;
} Node_list;

typedef struct List {
    Node_list *head;
    int count;
    node_data_free_callback_t node_data_free_callback;
} List;


extern List *list_create(node_data_free_callback_t node_data_free_callback);
extern void list_add_data(List *list, void *data);
extern void list_add_data_tail(List *list, void *data);
extern void list_add_data_sorted(List *list, void *data, int (*cmp)(const void *a, const void *b));
extern void list_remove_data(List *list, void *data);
extern void list_free(List *list);
extern void list_sort(List *list, int(*cmp)(const void *a, const void *b));

#ifdef __cplusplus
}
#endif

#endif
