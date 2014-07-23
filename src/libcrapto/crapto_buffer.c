#include "crapto_private.h"
#include "crapto_buffer.h"
#include "crapto_array.h"
#include "crapto_list.h"

#include <stdio.h>
#include <stdlib.h>

void crapto_buffer(crapto_buffer_t* buffer, crapto_crypto_t* c,  int op);
void crapto_encrypt_buffer(crapto_buffer_t* buffer, crapto_crypto_t* c);
void crapto_decrypt_buffer(crapto_buffer_t* buffer,crapto_crypto_t* c);

#define BUFFER_BLOCK_SIZE 1024

struct block_list_t;

typedef struct block_t
{ 
  struct block_t* next;
  struct block_t* prev;
  byte_t* end;
  byte_t* start;
}block_t;

#define BLOCK_READ    0
#define BLOCK_WRITE   1

CRAPTO_DEFINE_ARRAY_OF(block_t);
typedef struct block_list_t
{  
  block_t* head;
  block_t* tail;

  array_of_block_t* lookup;
  array_t*          pools;
}block_list_t;

block_list_t* block_list_ctor(runtime_t* rt);

typedef struct buffer_position_t
{
  length_t pos;
  block_t*  block_ptr;
  byte_t*   block_data_ptr;
}buffer_position_t;

typedef struct crapto_header_t
{
  length_t data_length;

  byte_t reserved[BUFFER_BLOCK_SIZE-sizeof(length_t)];

}crapto_header_t;

typedef struct buffer_t
{
  RUNTIME_OBJECT;
  crapto_header_t* header;
  block_list_t* blocks;
}buffer_t;


RUNTIME_OBJECT_CTOR(buffer_t)
{
  self->blocks = block_list_ctor(self->rt);
  CraptoAllocateObjectZero(crapto_header_t,self->header,self->rt);

}

struct CRAPTO_ARGS_T(buffer_io_base_t)
{
  buffer_t* buffer;
};


crapto_buffer_t* crapto_new_buffer(struct crapto_runtime_t* runtime)
{
  crapto_buffer_t* buffer;
  RUNTIME_CAST(runtime);
  CraptoAllocateObjectZero(crapto_buffer_t,buffer,rt);
  buffer->delete_reader = buffer_free_reader;
  buffer->delete_writer = buffer_free_writer;
  buffer->new_reader = buffer_allocate_reader;
  buffer->new_writer = buffer_allocate_writer;
  buffer->get_capacity = buffer_capacity;
  buffer->get_position = buffer_get_position;
  buffer->set_position = buffer_set_position;
  buffer->write_to_file = buffer_write_to_file;
  buffer->read_from_file=buffer_read_from_file;
  buffer->read = buffer_read;
  buffer->write = buffer_write;
  return buffer;
}

static __inline void buffer_initialize_head(buffer_t* buffer, block_t* block)
{
  block_list_t* table = buffer->blocks;
  assert(table);
  assert(block);
  assert(!table->head);
  assert(!table->tail);    
  table->head = table->tail = block;

  array_of_block_t_push_back(table->lookup,block);
}

static __inline void buffer_add_last(buffer_t* buffer, block_t* block)
{
  assert(buffer->blocks);
  assert(block);
  assert(buffer->blocks->tail);
  assert(!buffer->blocks->tail->next);
  block->prev = buffer->blocks->tail;
  block->next=0;
  buffer->blocks->tail->next = block;
  buffer->blocks->tail = block;  
  array_of_block_t_push_back(buffer->blocks->lookup,block);

}

block_t* buffer_allocate_lone_block(buffer_t* buffer)
{
  block_t* block;
  byte_t* mem= (byte_t*)buffer->rt->allocate((sizeof(block_t)+BUFFER_BLOCK_SIZE)*1);
  crapto_zmem(mem,(sizeof(block_t)+BUFFER_BLOCK_SIZE)*1);
  array_push_back(buffer->blocks->pools,mem);
  block = (block_t*)mem;    
  block->start = mem+sizeof(block_t);
  block->end = block->start+BUFFER_BLOCK_SIZE;

  buffer_add_last(buffer,block);
  return block;
}
void buffer_allocate_blocks(buffer_t* buffer, length_t count)
{
  
  unsigned n=0;
  byte_t* mem;
  if(!buffer->blocks->head)
  {
  }
  mem= (byte_t*)buffer->rt->allocate((sizeof(block_t)+BUFFER_BLOCK_SIZE)*count);
  crapto_zmem(mem,(sizeof(block_t)+BUFFER_BLOCK_SIZE)*count);
  array_push_back(buffer->blocks->pools,mem);

  for(;n<count;n++)
  {
    block_t* block = (block_t*)mem;    
    block->start = mem+sizeof(block_t);
    mem +=(sizeof(block_t)+BUFFER_BLOCK_SIZE);    

    block->end = block->start+BUFFER_BLOCK_SIZE;

    if(!buffer->blocks->head)
    {
      buffer_initialize_head(buffer,block);
    }
    else
    {
      buffer_add_last(buffer,block);
    }
  }
}

void buffer_attach_blocks(buffer_t* buffer, byte_t* data, length_t count)
{
}
void buffer_expand(buffer_t* buffer, length_t len)
{
  unsigned blocks;
  if(len <= BUFFER_BLOCK_SIZE)
  {
    blocks =1;
  }
  else
  {
    blocks = len / BUFFER_BLOCK_SIZE;
    if(len % BUFFER_BLOCK_SIZE)
      blocks++;
  }
  buffer_allocate_blocks(buffer,blocks);
}

// ////////////////////////////////////////////////////////////////////////////
// buffer_io_base_t
// ////////////////////////////////////////////////////////////////////////////

typedef struct buffer_io_base_t
{
  RUNTIME_OBJECT;
  buffer_t* buffer;
  buffer_position_t pos;
}buffer_io_base_t;


RUNTIME_OBJECT_CTOR(buffer_io_base_t)
{
  self->buffer = CtorArgs()->buffer;
}

// ////////////////////////////////////////////////////////////////////////////
// buffer_reader_t
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// buffer_writer_t
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// private: block_t
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// private: block_list_t
// ////////////////////////////////////////////////////////////////////////////
void block_list_free(runtime_t* rt, block_list_t* self)
{
  void* vp;
  vp = array_pop_back(self->pools);
  while(vp)
  {
    rt->deallocate(vp);
    vp = array_pop_back(self->pools);;
  }
  array_free_array((crapto_runtime_t*)rt,self->pools);
  array_of_block_t_free_array((crapto_runtime_t*)rt,self->lookup);
  rt->deallocate(self);
}
block_list_t* block_list_ctor(runtime_t* core)
{
  CraptoDeclareAllocateObject(block_list_t,blocks, core);
  blocks->lookup =array_of_block_t_create_array((crapto_runtime_t*)core,1024);
  blocks->pools = array_create_array((crapto_runtime_t*)core,1024);

  blocks->head = 0;
  blocks->tail = 0;

  return blocks;
}

// ////////////////////////////////////////////////////////////////////////////
// buffer private
// ////////////////////////////////////////////////////////////////////////////


length_t buffer_io(buffer_io_base_t* io, byte_t* src, length_t length, int op)
{
  length_t copied=0;

  while(length)
  {
    unsigned block_pos;
    unsigned block_remaining;        
    
    //
    // How much is left in the current block?
    //
    block_remaining = io->pos.block_ptr->end - io->pos.block_data_ptr;
    if(block_remaining > length)
      block_remaining=length;

    if(block_remaining == 0){
      io->pos.block_ptr = io->pos.block_ptr->next;
      io->pos.block_data_ptr = io->pos.block_ptr->start;      
      continue;
    }

    io->pos.pos += block_remaining;
    length -= block_remaining;    
    assert(length >= 0);
    if(op == BLOCK_READ){
      crapto_memcpy(src,io->pos.block_data_ptr,block_remaining);          
      src+=block_remaining;
    }
    else
    {
      crapto_memcpy(io->pos.block_data_ptr,src,block_remaining);    
      io->pos.block_data_ptr += block_remaining;    
      io->buffer->header->data_length+=block_remaining;
    }
    copied+=block_remaining;
  }
  return copied;
}

// ////////////////////////////////////////////////////////////////////////////
// crapto public
// ////////////////////////////////////////////////////////////////////////////



crapto_buffer_t* buffer_attach_buffer(struct crapto_runtime_t* runtime, byte_t* data, length_t len)
{
  return 0;
}

crapto_buffer_t* buffer_allocate_buffer(struct crapto_runtime_t* runtime, length_t minimum)
{
  buffer_t* ptr;
  crapto_buffer_t* buffer = crapto_new_buffer(runtime);
  buffer->delete_reader = buffer_free_reader;
  buffer->delete_writer = buffer_free_writer;
  buffer->new_reader = buffer_allocate_reader;
  buffer->new_writer = buffer_allocate_writer;
  buffer->get_capacity = buffer_capacity;
  buffer->get_size=buffer_get_size;
  buffer->get_position = buffer_get_position;
  buffer->set_position = buffer_set_position;
  buffer->write_to_file = buffer_write_to_file;
  buffer->read_from_file=buffer_read_from_file;
  buffer->read = buffer_read;
  buffer->write = buffer_write;
  buffer->encrypt_buffer = crapto_encrypt_buffer;
  buffer->decrypt_buffer = crapto_decrypt_buffer;
  ptr = buffer_t_ctor(runtime,0);  
  buffer_expand(ptr,minimum);
  buffer->reserved = ptr;
  ptr->reserved=ptr;
  return buffer;
}

void buffer_free_buffer(crapto_buffer_t* buffer)
{
  runtime_t* rt;
  void* vp;
  unsigned n;
  SELF(buffer->reserved,buffer_t);
  rt = (runtime_t*) self->rt;
  
  block_list_free(rt,self->blocks);
  rt->deallocate(self->header);
  rt->deallocate(self);
  rt->deallocate(buffer);
  return;
}

void buffer_free_writer(crapto_buffer_writer_t* writer)
{
  SELF(writer,buffer_io_base_t);
  self->rt->deallocate(self);
}
void buffer_free_reader(crapto_buffer_reader_t* reader)
{
  SELF(reader,buffer_io_base_t);
  self->rt->deallocate(self);
}

length_t buffer_capacity(crapto_buffer_t* buffer)
{
  SELF(buffer->reserved,buffer_t);
  return array_of_block_t_size(self->blocks->lookup)*BUFFER_BLOCK_SIZE;
}

crapto_buffer_writer_t* buffer_allocate_writer2(crapto_buffer_reader_t* buffer)
{
}
crapto_buffer_writer_t* buffer_allocate_writer(crapto_buffer_t* buffer)
{
  buffer_io_base_t* io;
  struct buffer_io_base_t_crapto_args args;
  SELF(buffer->reserved,buffer_t);
  args.buffer = self;
  io = buffer_io_base_t_ctor_rt(self->rt,&args);
  io->reserved = io;
  buffer_set_position((crapto_buffer_io_base_t*) io,0);

  return (crapto_buffer_writer_t*)io;
}

crapto_buffer_reader_t* buffer_allocate_reader(crapto_buffer_t* buffer)
{
  return (crapto_buffer_reader_t*) buffer_allocate_writer(buffer);
}
  
length_t buffer_write(crapto_buffer_writer_t* writer, const byte_t* data, length_t length)
{
  SELF(writer, buffer_io_base_t);
  if( (length + self->pos.pos) >  buffer_capacity((crapto_buffer_t*)self->buffer))
  {
    buffer_expand(self->buffer, (length + self->pos.pos) -buffer_capacity((crapto_buffer_t*)self->buffer) );
  }
  
  return buffer_io(self,(byte_t*)data,length,BLOCK_WRITE);
}

length_t buffer_read(crapto_buffer_reader_t* reader, byte_t* data, length_t length)
{
  SELF(reader, buffer_io_base_t);
  return buffer_io(self,data,length,BLOCK_READ);
}

length_t buffer_get_position(crapto_buffer_io_base_t* buffer_io_base)
{
  SELF(buffer_io_base, buffer_io_base_t);
  return self->pos.pos;
}

void buffer_set_position(crapto_buffer_io_base_t* buffer_io_base, length_t pos)
{
  unsigned block_pos;
  unsigned block_index;
  buffer_position_t* bpos;
  block_list_t* blocks;
  SELF(buffer_io_base, buffer_io_base_t);
    
  bpos = &self->pos;
  blocks = self->buffer->blocks;

  block_index = pos / BUFFER_BLOCK_SIZE;
  block_pos   = pos % BUFFER_BLOCK_SIZE;

  assert(block_index < array_of_block_t_size(blocks->lookup));
  
  bpos->block_ptr = array_of_block_t_select(blocks->lookup,block_index);
  bpos->block_data_ptr =  bpos->block_ptr->start + block_pos;
  bpos->pos= pos;
  return;
}


length_t buffer_write_to_file(crapto_buffer_reader_t* reader, const char* filename, length_t limit)
{
  crapto_runtime_t* runtime;
  crapto_file_t* file;
  byte_t buf[CRAPTO_BLOCK_SIZE];
  length_t wrote;
  block_t* current;
  length_t written;
  length_t next_size;
  int index;
  int block_pos;
  SELF(reader,buffer_io_base_t);
  written = 0;
  
  crapto_zmem(buf,CRAPTO_BLOCK_SIZE);
  runtime = (crapto_runtime_t*)self->rt;
  file = runtime->new_file(runtime);
  file->open(file,filename,"wb");
  
  block_pos = self->pos.pos % BUFFER_BLOCK_SIZE;
  next_size = BUFFER_BLOCK_SIZE-block_pos;
  if(next_size > limit)
    next_size = limit;
  file->write(file,self->pos.block_data_ptr,next_size,&wrote);
  assert(wrote);
  written += next_size;
  
  current = self->pos.block_ptr->next;
  
  while(current && written < limit)
  {
    next_size = (limit-written)>BUFFER_BLOCK_SIZE?BUFFER_BLOCK_SIZE:(limit-written);
    file->write(file,current->start,next_size,&wrote);

    current = current->next;
    written += next_size;
  }
  file->close(file);
  runtime->delete_file(file);
  return written;
}

length_t buffer_read_from_file(crapto_buffer_writer_t* writer, const char* filename, length_t limit)
{
  crapto_runtime_t* runtime ;
  length_t len;
  length_t avail;
  length_t block_pos;
  length_t red;
  length_t next_size;
  FILE* file;
  byte_t tmp[BUFFER_BLOCK_SIZE]; 
  SELF(writer,buffer_io_base_t);
  runtime = (crapto_runtime_t*)self->rt;

  file = fopen(filename,"rb");
  fseek(file,0,SEEK_END);
  len = ftell(file);
  crapto_log(crapto_info, "Read: length of file: %d\n",len);
  fseek(file,0,SEEK_SET);
  len = len > limit?limit:len;

  avail = (array_of_block_t_size(self->buffer->blocks->lookup)*BUFFER_BLOCK_SIZE) - self->pos.pos;
  if(avail < len)
  {
    buffer_expand(self->buffer,len-avail);
  }

  block_pos = self->pos.pos % BUFFER_BLOCK_SIZE;
  next_size = BUFFER_BLOCK_SIZE-block_pos;
  red= fread(tmp,next_size,1,file);
  assert(red);
  buffer_write(writer,tmp,next_size);
  len -= next_size;
  next_size = len>=BUFFER_BLOCK_SIZE?BUFFER_BLOCK_SIZE:len;
  while(!feof(file) && len)
  {
    red= fread(tmp,next_size,1,file);
    assert(red);
    if(!red)
      break;
    buffer_write(writer,tmp,next_size);
    len -= next_size;
    next_size = len>=BUFFER_BLOCK_SIZE?BUFFER_BLOCK_SIZE:len;
  }
  fclose(file);

  crapto_log(crapto_info, "Read: buffer pos: %d\n",buffer_get_position(writer->base));
  return 0;
}



void crapto_buffer(crapto_buffer_t* buffer, crapto_crypto_t* c,  int op)
{
  block_t* current;
  SELF(buffer->reserved, buffer_t);
  if(CRAPTO_OP_ENCRYPT == op)
  {
    current= buffer_allocate_lone_block(self);
    crapto_memcpy(current->start,self->header,BUFFER_BLOCK_SIZE);  

    crapto_log(crapto_info, "==>Encrypting: size(%d), blocks(%d), data_length(%d)\n"
      ,array_of_block_t_size(self->blocks->lookup)*BUFFER_BLOCK_SIZE
      ,array_of_block_t_size(self->blocks->lookup)
      ,self->header->data_length
    );
  }

  current = self->blocks->head;
  while(current)
  {
    c->crapto_op(c,op,current->start,BUFFER_BLOCK_SIZE);
      current = current->next;
  }

  if(CRAPTO_OP_DECRYPT == op)
  {
    crapto_memcpy(self->header,self->blocks->tail->start, BUFFER_BLOCK_SIZE);
    crapto_log(crapto_info, "<==Decrypting: size(%d) blocks(%d), data_length(%d)\n"
      ,array_of_block_t_size(self->blocks->lookup)*BUFFER_BLOCK_SIZE
      ,array_of_block_t_size(self->blocks->lookup)
      ,self->header->data_length
    );
  }
}
void crapto_encrypt_buffer(crapto_buffer_t* buffer, crapto_crypto_t* c)
{
  crapto_buffer(buffer,c,1);
}

void crapto_decrypt_buffer(crapto_buffer_t* buffer,crapto_crypto_t* c)
{
  crapto_buffer(buffer,c, 0);
}

length_t buffer_get_size(crapto_buffer_t* buffer)
{
  SELF(buffer->reserved, buffer_t);
  return self->header->data_length;
}