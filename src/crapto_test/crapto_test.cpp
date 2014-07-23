// crapto_test.cpp : Defines the entry point for the console application.
//
#include "crapto.h"
#include "stdafx.h"
#include <tc.h>
HMODULE g_crapto_module(0);
crapto_runtime_t* g_crapto(0);

void buffer_test_create();
void buffer_test_write_small();
void buffer_test_expand();
void buffer_test_expand_many();
void buffer_test_read_some();
void buffer_test_encrypt();
void buffer_test_read_write();
void buffer_test_encrypt_file();

crapto_crypto_t* g_crypto=0;
static crapto_result_t _crapto_op (struct crapto_crypto_t* c, int op, byte_t* data, length_t length);


struct crypto_impl
{
  crapto_op_fn crapto_op;

  void init()
  {
    this->crapto_op = _crapto_op;
    tc = tc_create(key);
  }

  void kill()
  {
    tc_destroy(tc);
  }
  tc_handle tc;
};

static crapto_result_t _crapto_op (struct crapto_crypto_t* c, int op, byte_t* data, length_t length)
{
  tc_crypto_action( ((crypto_impl*)c)->tc,op,data,length);
  return CRAPTO_NOERROR;
}
crapto_runtime_t* crapto()
{
  
  if(g_crapto)
    return g_crapto;

  g_crapto_module = LoadLibraryA("crapto.dll");
  fn_crapto_create_runtime p = (fn_crapto_create_runtime)GetProcAddress(g_crapto_module,"crapto_create_runtime");
  g_crapto = p(crapto_malloc,crapto_free);
  crypto_impl* ptr = new crypto_impl;
  ptr->init();
  g_crypto = (crapto_crypto_t*)ptr;

  return g_crapto;
}

void cleanup(crapto_runtime_t* c)
{ 
  if(!c)
    return;

  crypto_impl* ptr = (crypto_impl*)g_crypto;
  ptr->kill();

  c->destroy(c);
  g_crapto=0;
  
  g_crypto=0;
}

int _tmain(int argc, _TCHAR* argv[])
{

  for( auto& test : add_test_t::tests)
  {    
    try
    {      
      test.test();
      printf("[PASS] %s\n\n", test.name.c_str());
    }
    catch(crapto_exception& x)
    {
      
      printf(
             "Message    : %s\n"
             "Details    : %s\n",x.what(), x.expr());     
      printf("[FAIL] %s\n\n", test.name.c_str());      
    }    
    
    cleanup(g_crapto);
    if(memstats.memuse != 0 && memstats.log)
    {
      printf("[!MEM] memory in use 0x%08X\n",memstats.memuse);
      dump_memloc();
    }
  }

  if(memstats.memuse != 0)
  {
    printf("[!MEM] memory in use 0x%08X\n",memstats.memuse);
    dump_memloc();
  }
  system("pause");
  return 0;
}

