#include "crapto_private.h"
#include "crapto_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
typedef struct crapto_file_impl_t
{
  RUNTIME_OBJECT;
  FILE* file;
  int length;

}crapto_file_impl_t;

RUNTIME_OBJECT_CTOR(crapto_file_impl_t)
{
}

errno_t crapto_file_open(struct crapto_file_t* base, const char* filename, const char* mode)
{  
  SELF(base->reserved, crapto_file_impl_t);
  
  base->close(base);

  self->file=fopen(filename,mode);
  if(!self->file)
    return errno;

  return 0;
}

int crapto_file_close(struct crapto_file_t* base)
{
  SELF(base->reserved, crapto_file_impl_t);
  if(self->file)
  {
    fclose(self->file);
    self->file=0;
    self->length = 0;
  }
  return 0;
}

length_t crapto_file_length(struct crapto_file_t* base)
{
  length_t len;
  int err;
  SELF(base->reserved, crapto_file_impl_t);
  if(self->length)
    return self->length;
  err = fseek(self->file,0,SEEK_END);
  if(err)
    return EOF;
  
  len = ftell(self->file);
  if(len == -1)
    return EOF;

  fseek(self->file,0,SEEK_SET);

  self->length = len;
  return len;
}

int crapto_file_write(struct crapto_file_t* base,  const void* data, length_t length,length_t* written) 
{
  int n;
  SELF(base->reserved, crapto_file_impl_t);
  if(!data)
    return EADDRNOTAVAIL;

  n = fwrite(data,length,1,self->file);
  if(n != 1)
    return ferror(self->file);
  
  if(written)
    *written = n;

  if(EOF == fflush(self->file))
    return ferror(self->file);

  return 0;
}

int crapto_file_read(struct crapto_file_t* base,  void* data, length_t length)
{
  int n;
  SELF(base->reserved, crapto_file_impl_t);
  if(!data)
    return EADDRNOTAVAIL;

  n = fread(data,length,1,self->file);
  
  if(n!=1 && ! feof(self->file))
  {
    return ferror(self->file);
  }
  return 0;
}

int crapto_file_is_eof(struct crapto_file_t* base)
{
  SELF(base->reserved, crapto_file_impl_t);
  return feof(self->file);
}

int crapto_file_remaining(struct crapto_file_t* base)
{
  SELF(base->reserved, crapto_file_impl_t);
  if(!self->file)
    return EOF;

}
crapto_file_t* crapto_create_file(struct crapto_runtime_t* runtime)
{
  crapto_file_impl_t* impl;
  crapto_file_t* file;
  RUNTIME_CAST(runtime);
  CraptoAllocateObjectZero(crapto_file_t,file,rt);
  file->close = crapto_file_close;
  file->is_eof = crapto_file_is_eof;
  file->length = crapto_file_length;
  file->open = crapto_file_open;
  file->read = crapto_file_read;
  file->write  = crapto_file_write;
  file->remaining = crapto_file_remaining;
  impl= crapto_file_impl_t_ctor(runtime,0);
  file->reserved=impl;

  return file;
}
void crapto_free_file(crapto_file_t* file)
{
  SELF(file->reserved,crapto_file_impl_t);
  file->close(file);
  self->rt->deallocate(file);
  self->rt->deallocate(self);

}