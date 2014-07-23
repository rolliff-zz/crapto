#pragma once

// Standard
#include <stdio.h>
#include <tchar.h>
#include <exception>
#include <string>

// OS
#include <Windows.h>

// Crap
#include "crapto.h"

// Local
#include "test_list.h"
#include "mem.h"

static unsigned char key[] ={
  0x8f, 0xc9, 0x1b, 0xbf, 0xa1, 0xf0, 0x7e, 0xa6,
  0xa7, 0x6f, 0x25, 0x23, 0xf6, 0x92, 0xeb, 0x7e,
  0xa7, 0xc1, 0x99, 0x5f, 0x5d, 0x6a, 0xb1, 0x1c,
  0xb7, 0xe9, 0x42, 0xd9, 0x2b, 0x3c, 0xbe, 0xc1,
};


static const char test_file[] = "E:\\src\\GitLibGit2Sharp\\libgit2\\deps\\http-parser\\http_parser.c";
static const char test_file_unlocked[] = "E:\\src\\GitLibGit2Sharp\\libgit2\\deps\\http-parser\\http_parser_unlocked.c";
static const char test_file_lck[] = "E:\\src\\GitLibGit2Sharp\\libgit2\\deps\\http-parser\\http_parser.c.lck";


crapto_runtime_t* crapto();

void cleanup(crapto_runtime_t* c);

/**
  A dumb exception.

  This gets thrown by the the assert macros below.
*/
class crapto_exception 
  : public std::exception
{
public:
  crapto_exception(const char* expr, const char* msg) 
    : std::exception(msg,EOTHER), m_expr(expr)
  {}
  
  crapto_exception(const char* expr, const char* msg, int code) 
    : std::exception(msg,code), m_expr(expr)
  {}
  
  const char* expr() const { return m_expr.c_str(); }

private:
  std::string m_expr;
};


/**
  True or die
*/
#define CRAPTO_ASSERT_TRUE(expr)\
  if(!(expr))\
  {\
    throw crapto_exception(#expr, "Expression evaluated to false");\
  }

/**
  Equal or die
*/
#define CRAPTO_ASSERT_EQ(expected, actual)\
  if(expected != actual)\
  {\
    printf("expected %d, actual %d\n",expected, actual);\
    CRAPTO_ASSERT_TRUE(expected == actual);\
  }
