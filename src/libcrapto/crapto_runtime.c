#include "crapto.h"
#include "crapto_private.h"
#include "crapto_buffer.h"
#include "crapto_file.h"
#include "assert.h"

crapto_list_t* crapto_new_list(crapto_runtime_t* rt);
void crapto_encrypt_buffer(crapto_buffer_t* buffer);
void crapto_decrypt_buffer(crapto_buffer_t* buffer);


//int CraptoCrypt(runtime_t* rt, int encrypt, byte_t* bytes, length_t size)
//{
//  int cipher = AES;
//  int x = CipherGetBlockSize(AES);
//
//  assert(!(size %  x));
//  
//  if(encrypt == 1)
//    EncipherBlocks(cipher, bytes, rt->context, size / CipherGetBlockSize(AES));	
//  else
//    DecipherBlocks(cipher, bytes, rt->context,size / CipherGetBlockSize(AES));		    
//  
//
//  return FALSE;
//}
//void crapto_set_key2(crapto_runtime_t* base, byte_t key[32])
//{
//  RUNTIME_CAST(base);
//  CipherInit(AES, key,rt->context);
//}

void crapto_free_runtime(crapto_runtime_t* runtime)
{
  RUNTIME_CAST(runtime);
  
  rt->deallocate(rt);

}

crapto_runtime_t* crapto_create_runtime(allocator_fn allocator, deallocator_fn deallocator)
{
  runtime_t* rt = (runtime_t*)allocator(sizeof(runtime_t));
  crapto_runtime_t* base = (crapto_runtime_t*)rt;
  crapto_zmem(rt,sizeof(runtime_t));

  base->new_buffer = buffer_allocate_buffer;
  base->new_buffer_from_data = buffer_attach_buffer;
  base->delete_buffer = buffer_free_buffer;
  base->destroy = crapto_free_runtime;
  base->new_file = crapto_create_file;
  base->new_file1=0;
  base->delete_file = crapto_free_file;
  base->new_list = crapto_new_list;
  //base->buffer_write_to_file = buffer_write_to_file;
  //base->buffer_read_from_file = buffer_read_from_file;
  rt->allocate = allocator;
  rt->deallocate = deallocator;  
  /*rt->ci = crypto_open ();

	if (!rt->ci)
		return 0;

	memset(rt->ci, 0, sizeof (*rt->ci));*/

  return base;
}

#include <stdarg.h>
#include <stdio.h>
static crapto_log_fn g_log_fn=0;

void crapto_set_logger(crapto_log_fn fn)
{
  g_log_fn = fn;
}
void crapto_log(crapto_log_level level, const char* fmt, ...)
{
  char lazy[4096];
  va_list ap;
  if(!g_log_fn)
    return;

  crapto_zmem(lazy,4096);

  va_start(ap,fmt);  
  vsprintf(lazy,fmt,ap);
  g_log_fn(level,lazy);
  va_end(ap);
}