#include "stdafx.h"
#include "crc.h"
#include "crapto.h"
extern crapto_crypto_t* g_crypto;

ADDTEST(buffer_test_create)
{
  auto c = crapto();

  crapto_buffer_t* buffer=  c->new_buffer(c,500);
  
  CRAPTO_ASSERT_TRUE(buffer!=0);

  length_t cap = buffer->get_capacity(buffer);

  CRAPTO_ASSERT_EQ(CRAPTO_BLOCK_SIZE,cap);

  c->delete_buffer(buffer);
}

ADDTEST(buffer_test_write_small)
{
  unsigned n;
  auto c = crapto();
  byte_t buf[100];
  memset(buf,1,100);

  crapto_buffer_t* buffer=  c->new_buffer(c,500);

  CRAPTO_ASSERT_TRUE(buffer!=0);

  crapto_buffer_writer_t* writer= buffer->new_writer(buffer);
  
  buffer->set_position(writer->base,24);

  for(n=0;n<10;n++)
  {
    memset(buf,n+1,100);
    length_t wrote = buffer->write(writer,buf,100);
    CRAPTO_ASSERT_EQ(100,wrote);
  }

  length_t pos = buffer->get_position(writer->base);
  CRAPTO_ASSERT_EQ(1024,pos);

  buffer->delete_writer(writer);
  c->delete_buffer(buffer);
  
}

ADDTEST(buffer_test_expand)
{
  unsigned n;
  auto c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);

  crapto_buffer_t* buffer=  c->new_buffer(c,500);

  CRAPTO_ASSERT_TRUE(buffer!=0);

  crapto_buffer_writer_t* writer= buffer->new_writer(buffer);    

  for(n=0;n<10;n++)
  {
    memset(buf,n+1,1000);
    //printf("Writing %d to %d...\n",n*1000, n*1000+1000);
    length_t wrote = buffer->write(writer,buf,1000);
    CRAPTO_ASSERT_EQ(1000,wrote);
  }

  length_t pos = buffer->get_position(writer->base);
  CRAPTO_ASSERT_EQ(10000,pos);

  buffer->delete_writer(writer);
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_expand_many)
{
  unsigned n;
  auto c = crapto();
  byte_t buf[10000];
  memset(buf,1,10000);

  crapto_buffer_t* buffer=  c->new_buffer(c,500);

  CRAPTO_ASSERT_TRUE(buffer!=0);

  crapto_buffer_writer_t* writer= buffer->new_writer(buffer);    

  for(n=0;n<10;n++)
  {
    memset(buf,n+1,10000);
   // printf("Writing %d to %d...\n",n*10000, n*10000+10000);
    length_t wrote = buffer->write(writer,buf,10000);
    CRAPTO_ASSERT_EQ(10000,wrote);
  }

  length_t pos = buffer->get_position(writer->base);
  CRAPTO_ASSERT_EQ(100000,pos);

  buffer->delete_writer(writer);
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_read_some)
{
  unsigned n;
  auto c = crapto();
  byte_t buf[10000];
  memset(buf,1,10000);

  crapto_buffer_t* buffer=  c->new_buffer(c,500);

  CRAPTO_ASSERT_TRUE(buffer!=0);

  crapto_buffer_writer_t* writer= buffer->new_writer(buffer);      
  length_t wrote = buffer->write(writer,buf,10000);
  CRAPTO_ASSERT_EQ(10000,wrote);
  
  length_t pos = buffer->get_position(writer->base);
  CRAPTO_ASSERT_EQ(10000,pos);

  buffer->delete_writer(writer);
  memset(buf,0,10000);

  crapto_buffer_reader_t* reader = buffer->new_reader(buffer);

  length_t read = buffer->read(reader,buf,10000);
  CRAPTO_ASSERT_EQ(10000,read);

  for(n=0;n<10000;n++)
  {
    CRAPTO_ASSERT_EQ(1,buf[n]);
  }

  buffer->delete_reader(reader);
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_read_write)
{
  auto c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);

  crapto_buffer_t* buffer=  c->new_buffer(c,2020);
  CRAPTO_ASSERT_TRUE(buffer!=0);
  
  crapto_buffer_writer_t* writer= buffer->new_writer(buffer);

  buffer->read_from_file(writer,test_file,EOF);
  
  crapto_buffer_reader_t* reader= buffer->new_reader(buffer);  
  buffer->write_to_file(reader,test_file_lck,buffer->get_position(writer->base));

  buffer->delete_writer(writer);  
  buffer->delete_reader(reader);
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_encrypt)
{

  crapto_runtime_t* c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);

  crapto_buffer_t* buffer=  c->new_buffer(c,2020);
  CRAPTO_ASSERT_TRUE(buffer!=0);
  
  crapto_buffer_writer_t* writer= buffer->new_writer(buffer);
  

  buffer->read_from_file(writer,test_file,EOF);
  
  length_t pos = buffer->get_position(writer->base);
  byte_t* tmp = new byte_t[pos];

  crapto_buffer_reader_t* reader= buffer->new_reader(buffer);    
  
  buffer->read(reader,tmp,pos);
  buffer->set_position(reader->base,0);
  unsigned int crc = crc_of(tmp,pos);
  printf("CRC: 0x%08X\n", crc);

  buffer->encrypt_buffer(buffer,g_crypto);

  //buffer->write_to_file(reader,test_file_lck,buffer->get_position(writer->base));

  buffer->decrypt_buffer(buffer,g_crypto);
  pos = buffer->get_size(buffer);
  delete [] tmp;
  tmp = new byte_t[pos];

  buffer->read(reader,tmp,pos);
  buffer->set_position(reader->base,0);
  crc = crc_of(tmp,pos);
  delete [] tmp;

  printf("CRC: 0x%08X\n", crc);
  buffer->delete_reader(reader);
  buffer->delete_writer(writer);  
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_encrypt_file)
{

  crapto_runtime_t* c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);

  crapto_buffer_t* buffer=  c->new_buffer(c,2020);
  CRAPTO_ASSERT_TRUE(buffer!=0);
  
  crapto_buffer_writer_t* writer= buffer->new_writer(buffer);
  buffer->read_from_file(writer,test_file,EOF);
  
  length_t pos = buffer->get_position(writer->base);
  byte_t* tmp = new byte_t[pos];

  crapto_buffer_reader_t* reader= buffer->new_reader(buffer);    
  
  buffer->read(reader,tmp,pos);
  buffer->set_position(reader->base,0);
  unsigned int crc = crc_of(tmp,pos);
  printf("CRC: 0x%08X\n", crc);

  buffer->encrypt_buffer(buffer,g_crypto);  

  buffer->write_to_file(reader,test_file_lck,CRAPTO_WRITE_ALL);
  delete [] tmp;
  buffer->delete_reader(reader);  
  buffer->delete_writer(writer);  
  c->delete_buffer(buffer);

  buffer =  c->new_buffer(c,2020);

  writer= buffer->new_writer(buffer);
  buffer->read_from_file(writer,test_file_lck,CRAPTO_READ_ALL);
  
  buffer->decrypt_buffer(buffer,g_crypto);

  reader= buffer->new_reader(buffer);        
  pos = buffer->get_size(buffer);  
  tmp = new byte_t[pos];
  buffer->read(reader,tmp,pos);  
  unsigned int crc2 = crc_of(tmp,pos);
  delete [] tmp;
  printf("CRC: 0x%08X\n", crc);

  CRAPTO_ASSERT_EQ(crc,crc2);
  buffer->delete_reader(reader);  
  buffer->delete_writer(writer);  
  c->delete_buffer(buffer);
}


ONLYTEST(shit)
{
  int size = 0xab;
  void* in = malloc(size);
  memset(in,9,0xab);
  //
  // Crapto
  //
  length_t buf_len;        
  crapto_runtime_t* rt;
  crapto_buffer_t* buffer;
  crapto_buffer_writer_t* writer;
  crapto_buffer_reader_t* reader;                	      
	      
        
  //
  // Get the runtime
  //
  rt = crapto();
  //
  // Make a buffer
  //
  buffer = rt->new_buffer(rt,size);
        
  //
  // Make a writer for the buffer
  //
  writer = buffer->new_writer(buffer);
        
  //
  // Write input data
  //
  buffer->write(writer,(const byte_t*)in,size);
  //
  // Encrypt input data
  //
  buffer->encrypt_buffer(buffer,g_crypto);
        
  //
  // reallocate input to match encrypted buffer len
  //
  buf_len = buffer->get_size(buffer);        
  in = realloc(in,buf_len);          
        
  //
  // Fill input data with encrypted data
  //
  reader = buffer->new_reader(buffer);
  buffer->read(reader,(byte_t*)in,size);
        
  //
  // Cleanup
  //
  buffer->delete_reader(reader);
  buffer->delete_writer(writer);
  rt->delete_buffer(buffer);
}