#include "crapto.h"

crapto_buffer_t* buffer_attach_buffer(struct crapto_runtime_t* runtime, byte_t* data, length_t len);
crapto_buffer_t* buffer_allocate_buffer(struct crapto_runtime_t* runtime, length_t minimum);
void buffer_free_buffer(crapto_buffer_t* buffer);

length_t buffer_get_size(crapto_buffer_t* buffer);
length_t buffer_capacity(crapto_buffer_t* buffer);  
crapto_cursor buffer_open_cursor(crapto_buffer_t* buffer);
void buffer_close_cursor( crapto_cursor writer);


length_t buffer_write(crapto_cursor writer, const byte_t* data, length_t length);
length_t buffer_read(crapto_cursor writer, byte_t* data, length_t length);

length_t buffer_get_position(crapto_cursor buffer_io_base);
void buffer_set_position(crapto_cursor buffer_io_base, length_t pos);

length_t buffer_write_to_file(crapto_cursor reader, const char* filename, length_t limit);
length_t buffer_read_from_file(crapto_cursor writer, const char* filename, length_t limit);

void buffer_visit(struct crapto_buffer_t* buffer, crapto_block_visitor_fn visitor,void* ctx, length_t start, length_t count);