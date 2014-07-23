#include "crapto.h"

crapto_buffer_t* buffer_attach_buffer(struct crapto_runtime_t* runtime, byte_t* data, length_t len);
crapto_buffer_t* buffer_allocate_buffer(struct crapto_runtime_t* runtime, length_t minimum);
void buffer_free_buffer(crapto_buffer_t* buffer);

length_t buffer_get_size(crapto_buffer_t* buffer);
length_t buffer_capacity(crapto_buffer_t* buffer);  
crapto_buffer_writer_t* buffer_allocate_writer(crapto_buffer_t* buffer);
crapto_buffer_reader_t* buffer_allocate_reader(crapto_buffer_t* buffer);
void buffer_free_writer( crapto_buffer_writer_t* writer);
void buffer_free_reader(crapto_buffer_reader_t* reader);

length_t buffer_write(crapto_buffer_writer_t* writer, const byte_t* data, length_t length);
length_t buffer_read(crapto_buffer_reader_t* writer, byte_t* data, length_t length);

length_t buffer_get_position(crapto_buffer_io_base_t* buffer_io_base);
void buffer_set_position(crapto_buffer_io_base_t* buffer_io_base, length_t pos);

length_t buffer_write_to_file(crapto_buffer_reader_t* reader, const char* filename, length_t limit);
length_t buffer_read_from_file(crapto_buffer_writer_t* writer, const char* filename, length_t limit);