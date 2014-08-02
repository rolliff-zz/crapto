#include "crapto_private.h"
#include "crapto_buffer.h"
#include "crapto_array.h"
#include "crapto_list.h"

#include <stdio.h>
#include <stdlib.h>

void crapto_buffer(crapto_buffer_t* buffer, crapto_crypto_t* c,  int op);
void crapto_encrypt_buffer(crapto_buffer_t* buffer, crapto_crypto_t* c);
void crapto_decrypt_buffer(crapto_buffer_t* buffer,crapto_crypto_t* c);

#define BUFFER_BLOCK_SIZE 0x100
#define BUFFER_ALLOCATION_SIZE 0x100

struct block_list_t;

#define BLOCK_FLAG_FREE (0)
#define BLOCK_FLAG_INUSE (1<<0)

unsigned __inline flag_set(unsigned value, unsigned flag)
{
  return (value & flag) == flag?1:0;
}
typedef struct block_t
{ 
  struct block_t* next;
  struct block_t* prev;
  unsigned flags;
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


typedef struct crapto_sig_t
{
  unsigned long sig;
  length_t data_length;  
}crapto_sig_t;
//typedef struct crapto_header_t
//{
//  length_t data_length;
//
//  byte_t reserved[BUFFER_BLOCK_SIZE-sizeof(length_t)];
//
//}crapto_header_t;

typedef struct buffer_t
{
  RUNTIME_OBJECT;
  /// Amount of allocated data that has been written to
  crapto_sig_t* sig;
  block_t* info;
  block_list_t* blocks;
}buffer_t;


RUNTIME_OBJECT_CTOR(buffer_t)
{
  self->blocks = block_list_ctor(self->rt);
  //CraptoAllocateObjectZero(block_t,self->info,self->rt);

  
  //CraptoAllocateObjectZero(crapto_header_t,self->header,self->rt);

}

struct CRAPTO_ARGS_T(cursor_t)
{
  buffer_t* buffer;
};

// ////////////////////////////////////////////////////////////////////////////
// cursor_t
// ////////////////////////////////////////////////////////////////////////////

typedef struct cursor_t
{
  RUNTIME_OBJECT;
  buffer_t* buffer;
  length_t pos;
  block_t*  block_ptr;
  byte_t*   block_data_ptr;
}cursor_t;


RUNTIME_OBJECT_CTOR(cursor_t)
{
  self->buffer = CtorArgs()->buffer;
}

static __inline length_t cursor_blocks_in_use(cursor_t* c)
{
  unsigned n = c->pos / BUFFER_BLOCK_SIZE;
  if(c->pos % BUFFER_BLOCK_SIZE)
    n++;
  return n;
}

static __inline void buffer_initialize_head(buffer_t* buffer, block_t* block)
{
  byte_t* mem;
  block_list_t* table = buffer->blocks;
  assert(table);
  assert(block);
  assert(!table->head);
  assert(!table->tail);    
  table->head = table->tail = block;
  
  CraptoAllocateArrayZero(byte_t, mem, buffer->rt, sizeof(block_t) + BUFFER_BLOCK_SIZE);

  buffer->info = (block_t*)mem;
  array_push_back(buffer->blocks->pools,mem);

  buffer->sig = (crapto_sig_t*)buffer->info->start;
  buffer->info->end =  buffer->info->start+BUFFER_BLOCK_SIZE;
  buffer->info->next = buffer->blocks->head;

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

block_t* buffer_allocate_crypto_tail(buffer_t* buffer)
{
  block_t* block;
  unsigned block_index ;
  unsigned block_pos;

  block_index = buffer->sig->data_length / BUFFER_BLOCK_SIZE;
  block_pos   = buffer->sig->data_length % BUFFER_BLOCK_SIZE;
  block = array_of_block_t_select(buffer->blocks->lookup,block_index);

  if(block_pos > 0)
  {
    block = block->next;
    assert(block);

    /*byte_t* mem= (byte_t*)buffer->rt->allocate((sizeof(block_t)+BUFFER_BLOCK_SIZE)*1);
    crapto_zmem(mem,(sizeof(block_t)+BUFFER_BLOCK_SIZE)*1);  
    array_push_back(buffer->blocks->pools,mem);
    block = (block_t*)mem;    
    block->start = mem+sizeof(block_t);
    block->end = block->start+BUFFER_BLOCK_SIZE;
    buffer_add_last(buffer,block);*/
  }
  
  *((length_t*)block->start) = buffer->sig->data_length;
  block->flags = BLOCK_FLAG_INUSE;
  return block;
}

block_t* buffer_adjust_decrypted_data_length(buffer_t* buffer)
{
  block_t* block;
  unsigned block_index ;
  unsigned block_pos;

  block_index = buffer->sig->data_length / BUFFER_BLOCK_SIZE;
  block_pos   = buffer->sig->data_length % BUFFER_BLOCK_SIZE;
  block = array_of_block_t_select(buffer->blocks->lookup,block_index);

  if(block_pos > 0)
  {
    block = block->next;
    assert(block);

    /*byte_t* mem= (byte_t*)buffer->rt->allocate((sizeof(block_t)+BUFFER_BLOCK_SIZE)*1);
    crapto_zmem(mem,(sizeof(block_t)+BUFFER_BLOCK_SIZE)*1);  
    array_push_back(buffer->blocks->pools,mem);
    block = (block_t*)mem;    
    block->start = mem+sizeof(block_t);
    block->end = block->start+BUFFER_BLOCK_SIZE;
    buffer_add_last(buffer,block);*/
  }
  
  buffer->sig->data_length =*((length_t*)block->start) ;
  block->flags = BLOCK_FLAG_FREE;
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
    block->flags = BLOCK_FLAG_FREE;
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
  length_t size;


  size = array_of_block_t_size(buffer->blocks->lookup);
  if(!size)
    size++;

  //
  // Wow, should have went to math class...
  //
  while(len > (size*BUFFER_BLOCK_SIZE))
  {
    size*=2;
  }

  buffer_allocate_blocks(buffer,size);

  
  
}


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


length_t buffer_io(cursor_t* io, byte_t* src, length_t length, int op)
{
  length_t copied=0;

  while(length)
  {
    unsigned block_pos;
    unsigned block_remaining;        
    
    //
    // How much is left in the current block?
    //
    block_remaining = io->block_ptr->end - io->block_data_ptr;
    if(block_remaining > length)
      block_remaining=length;

    if(block_remaining == 0){
      io->block_ptr = io->block_ptr->next;
      io->block_data_ptr = io->block_ptr->start;      
      continue;
    }

    io->pos += block_remaining;
    length -= block_remaining;    
    assert(length >= 0);
    if(op == BLOCK_READ){
      crapto_memcpy(src,io->block_data_ptr,block_remaining);          
      src+=block_remaining;
      io->block_data_ptr += block_remaining;
    }
    else
    {
      crapto_memcpy(io->block_data_ptr,src,block_remaining);    
      io->block_data_ptr += block_remaining;    
      io->buffer->sig->data_length+=block_remaining;
      io->block_ptr->flags |= BLOCK_FLAG_INUSE;
      //io->buffer->header->data_length+=block_remaining;
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
  crapto_buffer_t* buffer;
  RUNTIME_CAST(runtime);
  CraptoAllocateObjectZero(crapto_buffer_t,buffer,rt);
  buffer->close_cursor = buffer_close_cursor;  
  buffer->open_cursor = buffer_open_cursor;
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
  buffer->visit = buffer_visit;
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
  rt->deallocate(self);
  rt->deallocate(buffer);
  return;
}

void buffer_close_cursor(crapto_cursor c)
{
  SELF(c,cursor_t);
  self->rt->deallocate(self);
}

length_t buffer_capacity(crapto_buffer_t* buffer)
{
  SELF(buffer->reserved,buffer_t);
  return array_of_block_t_size(self->blocks->lookup)*BUFFER_BLOCK_SIZE;
}

crapto_cursor buffer_allocate_writer2(crapto_cursor buffer)
{
}
crapto_cursor buffer_open_cursor(crapto_buffer_t* buffer)
{
  cursor_t* c;
  struct cursor_t_crapto_args args;
  SELF(buffer->reserved,buffer_t);

  args.buffer = self;
  c = cursor_t_ctor_rt(self->rt,&args);
  c->reserved = c;
  buffer_set_position((crapto_cursor) c,0);

  return (crapto_cursor)c;
}

length_t buffer_write(crapto_cursor c, const byte_t* data, length_t length)
{
  SELF(c, cursor_t);
  if( (length + self->pos) >  buffer_capacity((crapto_buffer_t*)self->buffer))
  {
    buffer_expand(self->buffer, (length + self->pos) -buffer_capacity((crapto_buffer_t*)self->buffer) );
  }
  
  return buffer_io(self,(byte_t*)data,length,BLOCK_WRITE);
}

length_t buffer_read(crapto_cursor c, byte_t* data, length_t length)
{
  SELF(c, cursor_t);
  return buffer_io(self,data,length,BLOCK_READ);
}

length_t buffer_get_position(crapto_cursor c)
{
  SELF(c, cursor_t);
  return self->pos;
}

void buffer_set_position(crapto_cursor c, length_t pos)
{
  unsigned block_pos;
  unsigned block_index;
  block_list_t* blocks;
  SELF(c, cursor_t);
    
  blocks = self->buffer->blocks;

  block_index = pos / BUFFER_BLOCK_SIZE;
  block_pos   = pos % BUFFER_BLOCK_SIZE;

  assert(block_index < array_of_block_t_size(blocks->lookup));
  
  self->block_ptr = array_of_block_t_select(blocks->lookup,block_index);
  self->block_data_ptr =  self->block_ptr->start + block_pos;
  self->pos= pos;
  return;
}


length_t buffer_write_to_file(crapto_cursor c, const char* filename, length_t limit)
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
  SELF(c,cursor_t);
  written = 0;
  
  crapto_zmem(buf,CRAPTO_BLOCK_SIZE);
  runtime = (crapto_runtime_t*)self->rt;
  file = runtime->new_file(runtime);
  file->open(file,filename,"wb");
  
  block_pos = self->pos % BUFFER_BLOCK_SIZE;
  next_size = BUFFER_BLOCK_SIZE-block_pos;
  if(next_size > limit)
    next_size = limit;
  file->write(file,self->block_data_ptr,next_size,&wrote);
  assert(wrote);
  written += next_size;
  
  current = self->block_ptr->next;
  
  while(current && written < limit)
  {
    if(flag_set(self->block_ptr->flags,BLOCK_FLAG_INUSE))
    {
      next_size = (limit-written)>BUFFER_BLOCK_SIZE?BUFFER_BLOCK_SIZE:(limit-written);
      file->write(file,current->start,next_size,&wrote);
      written += next_size;
    }
    current = current->next;
    
  }
  file->close(file);
  runtime->delete_file(file);
  crapto_log(crapto_info, "WriteToFile: length of file: %d\n",written);
  return written;
}
#include <fcntl.h>
#include <stdlib.h>
#include <io.h>
#include <share.h>
length_t buffer_read_from_file(crapto_cursor c, const char* filename, length_t limit)
{
  crapto_runtime_t* runtime ;
  length_t len;
  length_t avail;
  length_t block_pos;
  length_t red;
  length_t next_size;  
  int fh;
  int this_red;
  unsigned n;
  errno_t err;
  byte_t tmp[BUFFER_BLOCK_SIZE]; 
  SELF(c,cursor_t);
  red =0;
  runtime = (crapto_runtime_t*)self->rt;

  printf("read\n");
  {
    unsigned n;
    block_t* current = self->buffer->blocks->head;
    n=0;
    while(current)
    {
      printf("[%02d] Flags(0x%08X)\n",n++,current->flags);
      current=current->next;
    }
  }

  err = _sopen_s(&fh,filename,_O_BINARY | _O_RDONLY,_SH_DENYNO,0);
  
  
  _lseek(fh,0,SEEK_END);
  len = _tell(fh);
  crapto_log(crapto_info, "Read: length of file: %d\n",len);
  _lseek(fh,0,SEEK_SET);
  len = len > limit?limit:len;

  crapto_log(crapto_info, "Array Size: %d\n",array_of_block_t_size(self->buffer->blocks->lookup)*BUFFER_BLOCK_SIZE);

  avail = (array_of_block_t_size(self->buffer->blocks->lookup)*BUFFER_BLOCK_SIZE) - self->pos;
  if(avail < len)
  {
    buffer_expand(self->buffer,len-avail);
  }
  crapto_log(crapto_info, "Array Size After: %d\n",array_of_block_t_size(self->buffer->blocks->lookup)*BUFFER_BLOCK_SIZE);
  
  block_pos = self->pos % BUFFER_BLOCK_SIZE;
  next_size = BUFFER_BLOCK_SIZE-block_pos;
  this_red= _read(fh,tmp,next_size);
  assert(this_red);
  red+=this_red;
  buffer_write(c,tmp,next_size);
  
  
  len -= next_size;
  next_size = len>=BUFFER_BLOCK_SIZE?BUFFER_BLOCK_SIZE:len;
  while(!_eof(fh) && len)
  {
    
    this_red = _read(fh,tmp,next_size);
    assert(this_red);
    if(!this_red)
      break;
    red+=this_red;
    buffer_write(c,tmp,next_size);
    len -= next_size;
    next_size = len>=BUFFER_BLOCK_SIZE?BUFFER_BLOCK_SIZE:len;
  }
  _close(fh);

  crapto_log(crapto_info, "Read: buffer pos: %d\n",buffer_get_position(c));
  return red;
}



void crapto_buffer(crapto_buffer_t* buffer, crapto_crypto_t* c,  int op)
{
  block_t* current;
  SELF(buffer->reserved, buffer_t);


  if(CRAPTO_OP_ENCRYPT == op)
  {
    //current= buffer_allocate_crypto_tail(self);
    crapto_log(crapto_info, "==>Encrypting: size(%d), blocks(%d), data_length(%d)\n"
      ,array_of_block_t_size(self->blocks->lookup)*BUFFER_BLOCK_SIZE
      ,array_of_block_t_size(self->blocks->lookup)
      ,self->sig->data_length
    );
  }
  CraptoSwap(block_t*,self->info,self->blocks->head);
  
  current = self->blocks->head;
  while(current)
  {
    if(flag_set(current->flags,BLOCK_FLAG_INUSE))
      c->crapto_op(c,op,current->start,BUFFER_BLOCK_SIZE);
    current = current->next;
  }

  if(CRAPTO_OP_DECRYPT == op)
  {
    //
    // Make info the new tail
    //
    self->info->next = self->blocks->tail->next;        
    self->blocks->tail->next=self->info;
    self->info->prev = self->blocks->tail;
    self->blocks->tail =self->blocks->tail->next;

    //
    // Make head the new info
    //
    self->info = self->blocks->head;

    //
    // Make next the new head
    //
    self->blocks->head = self->blocks->head->next;

    //
    // Unlink the new head from the new info
    //
    self->blocks->head->prev=0;

    //buffer_adjust_decrypted_data_length(self);
    crapto_log(crapto_info, "<==Decrypting: size(%d) blocks(%d), data_length(%d)\n"
      ,array_of_block_t_size(self->blocks->lookup)*BUFFER_BLOCK_SIZE
      ,array_of_block_t_size(self->blocks->lookup)
      ,self->sig->data_length
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
  return self->sig->data_length;
}

void buffer_visit(struct crapto_buffer_t* buffer, crapto_block_visitor_fn visitor,void* ctx, length_t start, length_t count)
{
  unsigned block_pos;
  unsigned buffer_pos;
  unsigned block_index;
  block_list_t* blocks;
  block_t* current;
  cursor_t* c;
  struct cursor_t_crapto_args args;
  SELF(buffer->reserved, buffer_t);
  if(count == EOF)
  {
    count = self->sig->data_length;
  }
  blocks = self->blocks;
  
  args.buffer =self;
  c = cursor_t_ctor_rt(self->rt,&args);

  block_index = start / BUFFER_BLOCK_SIZE;
  block_pos   = start % BUFFER_BLOCK_SIZE;
  buffer_pos = start;
  assert(block_index < array_of_block_t_size(blocks->lookup));
  
  current = array_of_block_t_select(blocks->lookup,block_index);
  c->block_data_ptr= current->start;
  c->block_ptr = current;
  c->pos = start;
  {
    length_t copied=0;

    while(count)
    {
      unsigned block_remaining;        
    
      //
      // How much is left in the current block?
      //
      block_remaining = c->block_ptr->end - c->block_data_ptr;
      if(block_remaining > count)
        block_remaining=count;

      if(block_remaining == 0){
        c->block_ptr = c->block_ptr->next;
        c->block_data_ptr = c->block_ptr->start;      
        continue;
      }

      c->pos += block_remaining;
      count -= block_remaining;    
      assert(count >= 0);
      {
        if(flag_set(c->block_ptr->flags, BLOCK_FLAG_INUSE))
        {
          if(!visitor(ctx,c->pos , c->block_data_ptr, block_remaining))
            break;
        }   
        c->block_data_ptr += block_remaining;    
        //c->buffer->data_length+=block_remaining;
        //c->block_ptr->flags |= BLOCK_FLAG_INUSE;
      }
      copied+=block_remaining;
    }
  }

  self->rt->deallocate(c);
}