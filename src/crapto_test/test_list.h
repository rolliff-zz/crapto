#pragma once

#include <vector>
#include <string>

typedef void (*fn_test)();
typedef struct 
{
  fn_test test;
  std::string name;
}test_t;
struct add_test_t
{
  static std::vector<test_t> tests;
  static bool locked;
  add_test_t(fn_test f, const char* name);
protected:
  add_test_t(){}
};


struct only_test_t
  : public add_test_t
{
  only_test_t(fn_test f, const char* name);
};

#define ADDTEST(name)\
  void crapto_test_##name();\
  add_test_t add_crapto_test_##name(crapto_test_##name, #name);\
  void crapto_test_##name()

#define SKIPTEST(name)\
  void crapto_test_##name()

#define ONLYTEST(name)\
  void crapto_test_##name();\
  only_test_t only_crapto_test_##name(crapto_test_##name, #name);\
  void crapto_test_##name()
