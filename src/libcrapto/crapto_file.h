#pragma once
#include "crapto.h"

int crapto_file_open(struct crapto_file_t* self, const char* filename, const char* mode);
int crapto_file_close(struct crapto_file_t* self);
length_t crapto_file_length(struct crapto_file_t* self);
int crapto_file_write(struct crapto_file_t* self,  const void* data, length_t length,length_t* written);
int crapto_file_read(struct crapto_file_t* self,  void* data, length_t length);
int crapto_file_is_eof(struct crapto_file_t* self);
crapto_file_t* crapto_create_file(struct crapto_runtime_t* runtime);
void crapto_free_file(crapto_file_t* file);
int crapto_file_remaining(struct crapto_file_t* self);