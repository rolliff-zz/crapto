#include "crapto_private.h"
#include "crapto_array.h"


typedef struct array_impl_t
{
  void* reserved;
  unsigned  capacity;
  unsigned  size;
  runtime_t* rt;
  void**    data;
}array_impl_t;

array_t* array_create_array(crapto_runtime_t* runtime, length_t capacity)
{
  array_impl_t* arr;
  RUNTIME_CAST(runtime);
  CraptoAllocateObjectZero(array_impl_t,arr,rt);
  CraptoAllocateArrayZero(void*,arr->data, rt,capacity);
  arr->size=0;
  arr->capacity=capacity;
  arr->rt = rt;
  return (array_t*)arr;
}

void array_push_back(array_t* _arr, void* obj)
{
  array_impl_t* arr = (array_impl_t*) _arr;
  if(arr->size == arr->capacity)
  {
    unsigned ocapacity;
    void** tmp = arr->data;
    ocapacity = arr->capacity;
    arr->capacity *=2;
    CraptoAllocateArrayZero(void*,arr->data, arr->rt,arr->capacity);
    crapto_memcpy(arr->data,tmp,ocapacity);
    arr->rt->deallocate(tmp);
  }
  arr->data[arr->size++] = obj;
}

void* array_pop_back(array_t* _arr)
{
  array_impl_t* arr;
  arr = (array_impl_t*) _arr;
  if(!arr->size)
    return 0;

  return arr->data[--arr->size];
}


void* array_select(array_t* _arr, unsigned index)
{
  array_impl_t* arr;
  arr = (array_impl_t*) _arr;
  assert(index < arr->capacity);
  return arr->data[index];
}

void array_free_array(crapto_runtime_t* runtime, array_t* _arr)
{
  array_impl_t* arr = (array_impl_t*) _arr;
  arr->rt->deallocate(arr->data);
  arr->rt->deallocate(arr);
}


length_t array_size(array_t* _arr)
{
  array_impl_t* arr;
  arr = (array_impl_t*) _arr;
  return arr->size;
}
length_t array_capacity(array_t* _arr)
{
  array_impl_t* arr;
  arr = (array_impl_t*) _arr;
  return arr->capacity;
}