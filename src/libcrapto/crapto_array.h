#pragma once
#include "crapto.h"

typedef struct array_t {void* reserved; }array_t;


CRAPTO_API array_t* array_create_array(crapto_runtime_t* runtime, length_t capacity);
CRAPTO_API void array_push_back(array_t* arr, void* obj);
CRAPTO_API void* array_pop_back(array_t* arr);
CRAPTO_API void* array_select(array_t* arr, unsigned index);
CRAPTO_API void array_free_array(crapto_runtime_t* rt, array_t* arr);
CRAPTO_API length_t array_size(array_t* arr);
CRAPTO_API length_t array_capacity(array_t* arr);
CRAPTO_API void array_dump(array_t* arr);

#define CRAPTO_DEFINE_ARRAY_OF(T)\
  typedef struct array_of_##T## {void* reserved; }array_of_##T##;\
  __inline array_of_##T##* array_of_##T##_create_array(crapto_runtime_t* runtime, length_t size) \
    { return (array_of_##T##*)array_create_array(runtime,size); }\
  __inline void array_of_##T##_push_back(array_of_##T##* arr, T* obj)\
    {array_push_back((array_t*)arr,obj);}\
  __inline T* array_of_##T##_pop_back(array_of_##T##* arr)\
    {return (T*)array_pop_back((array_t*)arr);}\
  __inline T* array_of_##T##_select(array_of_##T##* arr, unsigned index)\
    {return (T*)array_select((array_t*)arr,index);}\
  __inline void array_of_##T##_free_array(crapto_runtime_t* rt, array_of_##T##* arr)\
    {return array_free_array(rt,(array_t*)arr);}\
  __inline length_t array_of_##T##_size(array_of_##T##* arr)\
    {return array_size((array_t*)arr);}\
  __inline length_t array_of_##T##_capacity(array_of_##T##* arr)\
    {return array_capacity((array_t*)arr);}\
  __inline void array_of_##T##_dump(array_of_##T##* arr)\
    { array_dump((array_t*)arr); }
