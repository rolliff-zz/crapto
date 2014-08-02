/**
  Crapto

  My understanding of encryption is that crypto algorithms operate on
  fixed sized blocks of data. Meaning, if you want to encrypt 64 bytes,
  you perform 4 Encipher operations on 4 blocks of 16 bytes.
  
  If your data doesn't align according to the cipher block size, you're
  going to be left with extra
*/
#pragma once 
// ////////////////////////////////////////////////////////////////////////////
// Basic Junk
// ////////////////////////////////////////////////////////////////////////////
#define CRAPTO_BLOCK_SIZE 1024
#define CRAPTO_WRITE_ALL -1
#define CRAPTO_READ_ALL -1

#define CRAPTO_API extern

typedef unsigned char byte_t;
typedef unsigned long length_t;
typedef void* (*allocator_fn)(size_t);
typedef void (*deallocator_fn)(void*);

/**
  32-bit zero memory function.

  Sounds like it should be faster than good ol' memset, but who knows.
  Probably doesn't make a difference.
*/
static __inline void crapto_zmem(void* ptr, length_t len)
{
  length_t steps;
  length_t remainder;
  steps = len / sizeof(unsigned long);
  remainder = len % sizeof(unsigned long);
  for(;steps;--steps)
  {
    ((unsigned long*)ptr)[steps-1] = 0L;
  }
  for(;steps<remainder;steps++)
  {
    ((unsigned char*)ptr)[steps] =0;
  }
}

/**
  32-bit memcpy
*/
static __inline void crapto_memcpy(void* dst, void* src, length_t len)
{
  length_t steps;
  length_t remainder;
  unsigned n;
  steps = len / sizeof(unsigned long);
  remainder = len % sizeof(unsigned long);
  
  // 32-bit Copy
  for(n=0;n<steps;++n)
    ((unsigned long*)dst)[n] = ((unsigned long*)src)[n];

  // Copy remaining bits
  for(n=0;n<remainder;++n)
    ((unsigned char*)((unsigned long*)dst+steps))[n] = ((unsigned char*)((unsigned long*)src+steps))[n];
  
}

typedef enum
{
  CRAPTO_NOERROR=0,
  CRAPTO_WTF,
  CRAPTO_NOMEM,
  CRAPTO_INVALID_ARG, 
  CRAPTO_NULL_PTR,

}crapto_result_t;


#define CRAPTO_OP_DECRYPT 0
#define CRAPTO_OP_ENCRYPT 1
typedef crapto_result_t (*crapto_op_fn)(struct crapto_crypto_t* c, int op, byte_t* data, length_t length);
// ////////////////////////////////////////////////////////////////////////////
// Crapto Crypto
// ////////////////////////////////////////////////////////////////////////////
typedef struct crapto_crypto_t
{
  crapto_op_fn crapto_op;

}crapto_crypto_t;


// ////////////////////////////////////////////////////////////////////////////
// Logging
// ////////////////////////////////////////////////////////////////////////////
typedef enum
{
  crapto_info,
  crapto_error
}crapto_log_level;
typedef void (*crapto_log_fn)(crapto_log_level level, const char* message);

CRAPTO_API void crapto_set_logger(crapto_log_fn fn);

// ////////////////////////////////////////////////////////////////////////////
// Buffer
// ////////////////////////////////////////////////////////////////////////////

/**
  Reference to a position in a buffer
*/
typedef struct { void* reserved; }* crapto_cursor;

typedef int (*crapto_block_visitor_fn)(void* ctx, length_t buffer_pos, byte_t* data, length_t data_length);

/**
  A Buffer
*/
typedef struct crapto_buffer_t 
{
  void* reserved; 

  // ///////////////////////
  // Crapto
  // ///////////////////////
  void (*encrypt_buffer)(struct crapto_buffer_t* buffer, crapto_crypto_t* c);
  void (*decrypt_buffer)(struct crapto_buffer_t* buffer, crapto_crypto_t* c);

  
  // ///////////////////////
  // Cursor
  // ///////////////////////
  crapto_cursor (*open_cursor)(struct crapto_buffer_t* buffer);
  void (*close_cursor)(crapto_cursor ref);
  length_t (*write)(crapto_cursor writer, const byte_t* data, length_t length);
  length_t (*read)(crapto_cursor reader, byte_t* data, length_t length);
  void (*set_position)(crapto_cursor buffer_io_base, length_t pos);  
  length_t (*get_position)(crapto_cursor buffer_io_base);
  length_t (*write_to_file)(crapto_cursor reader, const char* filename, length_t limit);
  length_t (*read_from_file)(crapto_cursor writer, const char * filename, length_t limit);

  // ///////////////////////
  // Buffer Position
  // ///////////////////////
  length_t (*get_capacity)(struct crapto_buffer_t* buffer);
  length_t (*get_size)(struct crapto_buffer_t* buffer);
  
  void (*visit)(struct crapto_buffer_t* buffer, crapto_block_visitor_fn visitor, void* ctx, length_t start, length_t count);

} crapto_buffer_t;

// ////////////////////////////////////////////////////////////////////////////
// File
// ////////////////////////////////////////////////////////////////////////////

/**
  File I/O
*/
typedef struct crapto_file_t
{
  void* reserved;
  int (*open )(struct crapto_file_t* self, const char* filename, const char* mode);
  int (*close)(struct crapto_file_t* self);
  length_t (*length)(struct crapto_file_t* self);
  int (*write)(struct crapto_file_t* self,  const void* data, length_t length, length_t* written);
  int (*read)(struct crapto_file_t* self,  void* data, length_t length);
  int (*is_eof)(struct crapto_file_t* self);
  int (*remaining)(struct crapto_file_t* self);
}crapto_file_t;


// ////////////////////////////////////////////////////////////////////////////
// List
// ////////////////////////////////////////////////////////////////////////////

#undef T
#define T void*

/**
  List of things
*/
typedef struct crapto_list_node_t
{ 
  void* reserved;
  T value;
}crapto_list_node_t;

typedef int (*fn_list_visitor)(crapto_list_node_t* node);

typedef struct crapto_list_t{ 
  void * reserved;

  crapto_list_node_t* (*add_first)(struct crapto_list_t* list, T value);
  crapto_list_node_t* (*add_last)(struct crapto_list_t* list, T value);
  crapto_list_node_t* (*head)(struct crapto_list_t* list);
  crapto_list_node_t* (*tail)(struct crapto_list_t* list);

  crapto_list_node_t* (*next)(crapto_list_node_t* node);
  crapto_list_node_t* (*prev)(crapto_list_node_t* node);
  T (*get_value)(crapto_list_node_t* node);
  void (*set_value)(crapto_list_node_t* node, T value);

  void (*visit)(struct crapto_list_t* list, fn_list_visitor pred);
  void (*visit_from)(crapto_list_node_t* node, fn_list_visitor pred);
}crapto_list_t;
#undef T

// ////////////////////////////////////////////////////////////////////////////
// Runtime
// ////////////////////////////////////////////////////////////////////////////

/**
  The runtime
*/
typedef struct crapto_runtime_t
{
  void (*destroy)(struct crapto_runtime_t* runtime);

  // ///////////////////////
  // Files
  // ///////////////////////
  crapto_file_t* (*new_file)(struct crapto_runtime_t* runtime);
  crapto_file_t* (*new_file1)(struct crapto_runtime_t* runtime, const char* filename);

  void (*delete_file)(crapto_file_t* file);


  // ///////////////////////
  // Buffers
  // ///////////////////////
  crapto_buffer_t* (*new_buffer_from_data)(struct crapto_runtime_t* mgr, byte_t* data, length_t len);
  crapto_buffer_t* (*new_buffer)(struct crapto_runtime_t* mgr, length_t minimum);
  void (*delete_buffer)(crapto_buffer_t* buffer);  

  // ///////////////////////
  // List
  // ///////////////////////
  crapto_list_t* (*new_list)(struct crapto_runtime_t* runtime);
  void (*delete_list)(crapto_list_t* list);

}crapto_runtime_t;


typedef crapto_runtime_t* (*fn_crapto_create_runtime)(allocator_fn allocator, deallocator_fn deallocator);
CRAPTO_API crapto_runtime_t* crapto_create_runtime(allocator_fn allocator, deallocator_fn deallocator);

