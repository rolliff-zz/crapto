#include "stdafx.h"

std::vector<test_t> add_test_t::tests;
bool add_test_t::locked=false;

add_test_t::add_test_t(fn_test f, const char* name)
{
  if(locked)
    return;

  test_t t = {f,name};
  tests.push_back(t);
}

only_test_t::only_test_t(fn_test f, const char* name)
{
  if(locked)
  {
    __debugbreak();
    // There can ONLY be one ONLY test
  }

  locked=true;
  tests.clear();
  test_t t = {f,name};
  tests.push_back(t);
}