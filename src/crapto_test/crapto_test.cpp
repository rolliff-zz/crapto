// crapto_test.cpp : Defines the entry point for the console application.
//
#include "crapto.h"
#include "stdafx.h"
#include "assert.h"
#include <tc.h>
#include "crc.h"
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

struct crypto_stats
{
  std::vector<unsigned> encrypt_crc;
  std::vector<unsigned> decrypt_crc;
}crypto_stats;

void dump_crypto_stats()
{
  assert(crypto_stats.encrypt_crc.size() == crypto_stats.decrypt_crc.size());
  for(unsigned n=0; n < crypto_stats.encrypt_crc.size();++n)
  {
    printf("[%4d] 0x%08X <-> 0x%08X %s\n"
      ,n
      ,crypto_stats.encrypt_crc[n]
      ,crypto_stats.decrypt_crc[n]
      ,crypto_stats.encrypt_crc[n]==crypto_stats.decrypt_crc[n]?"":"!!"
    );
  }

}
static crapto_result_t _crapto_op (struct crapto_crypto_t* c, int op, byte_t* data, length_t length)
{
  if(CRAPTO_OP_ENCRYPT != op &&false)
  {
    unsigned n =crc_of(data,length);
    printf("PRE-DECRYPT: 0x%08X 0x%08X\n",length,n);
    crypto_stats.encrypt_crc.push_back(n);
    if(crypto_stats.encrypt_crc.size() > 236)
    {
      for(unsigned x=0; x< length;++x)
      {
        printf("%02X ",data[x]);
        if( ! (x%16) && x )
          printf("\n");
      }
      printf("\n");
    }
  }
  
  tc_crypto_action( ((crypto_impl*)c)->tc,op,data,length);

  if(CRAPTO_OP_DECRYPT != op &&false)
  {
    unsigned n =crc_of(data,length);
    printf("POST-ENCRYPT: 0x%08X 0x%08X\n",length,n);
    crypto_stats.decrypt_crc.push_back(n);
    if(crypto_stats.decrypt_crc.size() > 236)
    {
    for(unsigned x=0; x< length;++x)
    {
      printf("%02X ",data[x]);
      if( ! (x%16) && x )
        printf("\n");
    }
    printf("\n");
    }
  }

  return CRAPTO_NOERROR;
}
crapto_runtime_t* crapto()
{
  
  if(g_crapto)
    return g_crapto;

  g_crapto_module = LoadLibraryA("crapto.dll");
  fn_crapto_create_runtime p = (fn_crapto_create_runtime)GetProcAddress(g_crapto_module,"crapto_create_runtime");
  g_crapto = p(crapto_malloc,crapto_free);
  typedef void(*F)(crapto_log_fn);
  F xcrapto_set_logger = (F)GetProcAddress(g_crapto_module,"crapto_set_logger");
  auto fn = [](crapto_log_level level, const char* msg){
    printf(msg);
  };
  xcrapto_set_logger(fn);

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

