#include"crapto_private.h"
#include "crapto_list.h"

typedef struct node_t
{
  struct node_t* next;
  struct node_t* prev;
  void* value;
}node_t;

typedef struct list_t
{
  RUNTIME_OBJECT;
  node_t* head;
  node_t* tail;
}list_t;


RUNTIME_OBJECT_CTOR(list_t)
{  
}

crapto_list_node_t* list_add_first(crapto_list_t* list, void* value)
{
  node_t* node;
  SELF(list->reserved,list_t);

  CraptoAllocateObjectZero(node_t,node,self->rt);
  if(!self->head)
  {
    self->tail = self->head = node;
    self->head->value = value;
    return (crapto_list_node_t*)self->head;
  }

  self->head->prev = node;
  node->next = self->head;
  CraptoSwap(node_t*,node,self->head);

  return (crapto_list_node_t*)self->head;
}

crapto_list_node_t* list_add_last(crapto_list_t* list, void* value)
{
  node_t* node;
  SELF(list->reserved,list_t);
  CraptoAllocateObjectZero(node_t,self->head,self->rt);
  if(!self->head)
  {
    self->tail = self->head = node;
    self->head->value = value;
    return (crapto_list_node_t*)self->tail;
  }

  self->tail->next= node;
  node->prev = self->tail;
  CraptoSwap(node_t*,node,self->tail);
  return (crapto_list_node_t*)self->head;
}

crapto_list_node_t* list_head(crapto_list_t* list)
{
  SELF(list->reserved,list_t);
  return (crapto_list_node_t*)self->head;
}

crapto_list_node_t* list_tail(crapto_list_t* list)
{
  SELF(list->reserved,list_t);
  return (crapto_list_node_t*)self->tail;
}

crapto_list_node_t* node_next(crapto_list_node_t* node)
{
  SELF(node,node_t);
  return (crapto_list_node_t*)self->next;
}

crapto_list_node_t* node_prev(crapto_list_node_t* node)
{
  SELF(node,node_t);
  return (crapto_list_node_t*)self->prev;
}

void* node_get_value(crapto_list_node_t* node)
{
  SELF(node,node_t);
  return self->value;
}

void node_set_value(crapto_list_node_t* node, void* value)
{
  SELF(node,node_t);
  self->value = value;
  return;
}

void list_visit_from(crapto_list_node_t* node, fn_list_visitor pred)
{
  node_t* current;
  SELF(node,node_t);
  current= self;
  while(current)
  {
    pred((crapto_list_node_t*)current);
    current = current->next;
  }
}

void list_visit(crapto_list_t* list, fn_list_visitor pred)
{
  list_visit_from((crapto_list_node_t*)((list_t*)list)->head,pred);
}

crapto_list_t* crapto_new_list(crapto_runtime_t* runtime)
{
  RUNTIME_CAST(runtime);
  return 0;
}