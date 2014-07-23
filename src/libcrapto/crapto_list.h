#pragma once
#include "crapto.h"



#undef T

#define CRAPTO_DEFINE_LIST_OF_(T, L, N)\
  typedef struct N\
{ \
  void* reserved;\
  T value;\
}N;\
  \
  typedef int (*fn_list_of_##T##_visitor)(N* node);\
  \
typedef struct L{ \
  void * reserved;\
  \
  N* (*add_first)(struct crapto_list_t* list, T value);\
  N* (*add_last)(struct crapto_list_t* list, T value);\
  N* (*head)(struct L* list);\
  N* (*tail)(struct L* list);\
  \
  N* (*next)(N* node);\
  N* (*prev)(N* node);\
  T (*get_value)(N* node);\
  void (*set_value)(N* node, T value);\
  \
  void (*visit)(struct L* list, fn_list_of_##T##_visitor pred);\
  void (*visit_from)(N* node, fn_list_of_##T##_visitor pred);\
}L;\
  L* crapto_new_list_of_##T(crapto_runtime_t* runtime)\
  {\
    L* list = (L*)runtime->new_list(runtime);\
  }


#define CRAPTO_DEFINE_LIST_OF(T)\
  CRAPTO_DEFINE_LIST_OF_(T,crapto_list_of_##T##_t,crapto_list_of_##T##_node_t)
  