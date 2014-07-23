#pragma once
#include "crapto.h"
//#include "Crypto.h"
//#include "Tests.h"
#include "assert.h"


#define NV_(crap)
#define FUNCNAME(x,y) x ## _ ## y
#define __IMPNAME__(Imp,name) FUNCNAME(Imp,name)

#define SELF(obj,as)\
  as* self= (as*)obj;

#define SELF_RT(obj, as, runtime)\
  as* self;\
  runtime_t* rt;\
  self=(as*)obj;\
  rt = (runtime_t*)runtime;

static void null_init(void*p){};
#define CRAPTO_NO_ARGS_T {int _null; }

#define RUNTIME_CAST(obj)\
  runtime_t* rt = (runtime_t*)obj

#define RUNTIME_OBJECT\
  void* reserved;\
  runtime_t* rt

#define RUNTIME_OBJECT_CTOR(type)\
  void __IMPNAME__(type,init)(type* self, runtime_t* rt, struct CRAPTO_ARGS_T(type)* args);\
  type * __IMPNAME__(type,ctor_rt)(runtime_t* rt, struct CRAPTO_ARGS_T(type)* args)\
  {\
    type* var = (type*)rt->allocate(sizeof(type));\
    crapto_zmem(var, sizeof(type));\
    var->rt=rt;\
    __IMPNAME__(type,init)(var, rt, args);\
    return var;\
  }\
  type * __IMPNAME__(type,ctor)(crapto_runtime_t* core, struct CRAPTO_ARGS_T(type)* args)\
  {\
    RUNTIME_CAST(core);\
    return __IMPNAME__(type,ctor_rt)(rt,args);\
  }\
  void __IMPNAME__(type,init)(type* self, runtime_t* rt, struct CRAPTO_ARGS_T(type)* args)

#define CRAPTO_CTOR()\
  CRAPTO_CTORX(CRAPTO_NO_ARGS_T)

#define CRAPTO_ARGS_T(T)\
  __IMPNAME__(T,crapto_args)

#define CtorArgs() args

#define CraptoAllocateObject(type, core_ptr)\
  (type*)core_ptr->allocate(sizeof(type));

#define CraptoAllocateObjectZero(type, obj, core_ptr)\
  obj = CraptoAllocateObject(type,core_ptr);\
  crapto_zmem(obj,sizeof(type))

#define CraptoAllocateArray(type, core_ptr, count)\
  (type*)core_ptr->allocate(sizeof(type)*count);

#define CraptoDeclareAllocateObject(type, var, core_ptr)\
  type* var = CraptoAllocateObject(type,core_ptr);\
  crapto_zmem(var, sizeof(type))

#define CraptoAllocateArrayZero(type, var, core_ptr, count)\
  var = CraptoAllocateArray(type,core_ptr,count);\
  crapto_zmem(var, sizeof(type)*count)

#define CraptoDeclareAllocateArray(type, var, core_ptr, count)\
  type* var = CraptoAllocateArray(type,core_ptr,count);\
  crapto_zmem(var, sizeof(type)*count)

#define CraptoSwap(type,left,right)\
{\
  type __tmp = left;\
  left = right;\
  right = __tmp;\
}

typedef struct 
{
  byte_t reserved[sizeof(crapto_runtime_t)];

  int n;
  allocator_fn allocate;
  deallocator_fn deallocate;
}runtime_t;


extern void crapto_log(crapto_log_level level, const char* fmt, ...);