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

  crapto_cursor w_cursor= buffer->open_cursor(buffer);
  
  buffer->set_position(w_cursor,24);

  for(n=0;n<10;n++)
  {
    memset(buf,n+1,100);
    length_t wrote = buffer->write(w_cursor,buf,100);
    CRAPTO_ASSERT_EQ(100,wrote);
  }

  length_t pos = buffer->get_position(w_cursor);
  CRAPTO_ASSERT_EQ(1024,pos);

  buffer->close_cursor(w_cursor);
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

  crapto_cursor w_cursor= buffer->open_cursor(buffer);    

  for(n=0;n<10;n++)
  {
    memset(buf,n+1,1000);
    //printf("Writing %d to %d...\n",n*1000, n*1000+1000);
    length_t wrote = buffer->write(w_cursor,buf,1000);
    CRAPTO_ASSERT_EQ(1000,wrote);
  }

  length_t pos = buffer->get_position(w_cursor);
  CRAPTO_ASSERT_EQ(10000,pos);

  buffer->close_cursor(w_cursor);
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

  crapto_cursor w_cursor= buffer->open_cursor(buffer);    

  for(n=0;n<10;n++)
  {
    memset(buf,n+1,10000);
   // printf("Writing %d to %d...\n",n*10000, n*10000+10000);
    length_t wrote = buffer->write(w_cursor,buf,10000);
    CRAPTO_ASSERT_EQ(10000,wrote);
  }

  length_t pos = buffer->get_position(w_cursor);
  CRAPTO_ASSERT_EQ(100000,pos);

  buffer->close_cursor(w_cursor);
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

  crapto_cursor w_cursor= buffer->open_cursor(buffer);      
  length_t wrote = buffer->write(w_cursor,buf,10000);
  CRAPTO_ASSERT_EQ(10000,wrote);
  
  length_t pos = buffer->get_position(w_cursor);
  CRAPTO_ASSERT_EQ(10000,pos);

  buffer->close_cursor(w_cursor);
  memset(buf,0,10000);

  crapto_cursor r_cursor = buffer->open_cursor(buffer);

  length_t read = buffer->read(r_cursor,buf,10000);
  CRAPTO_ASSERT_EQ(10000,read);

  for(n=0;n<10000;n++)
  {
    CRAPTO_ASSERT_EQ(1,buf[n]);
  }

  buffer->close_cursor(r_cursor);
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_read_write)
{
  auto c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);

  crapto_buffer_t* buffer=  c->new_buffer(c,2020);
  CRAPTO_ASSERT_TRUE(buffer!=0);
  
  crapto_cursor w_cursor= buffer->open_cursor(buffer);

  buffer->read_from_file(w_cursor,test_file,EOF);
  
  crapto_cursor r_cursor= buffer->open_cursor(buffer);  
  buffer->write_to_file(r_cursor,test_file_lck,buffer->get_position(w_cursor));

  buffer->close_cursor(w_cursor);  
  buffer->close_cursor(r_cursor);
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_encrypt)
{

  crapto_runtime_t* c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);

  crapto_buffer_t* buffer=  c->new_buffer(c,2020);
  CRAPTO_ASSERT_TRUE(buffer!=0);
  
  crapto_cursor w_cursor= buffer->open_cursor(buffer);
  

  buffer->read_from_file(w_cursor,test_file,EOF);
  
  length_t pos = buffer->get_position(w_cursor);
  byte_t* tmp = new byte_t[pos];

  crapto_cursor r_cursor= buffer->open_cursor(buffer);    
  
  buffer->read(r_cursor,tmp,pos);
  buffer->set_position(r_cursor,0);
  unsigned int crc = crc_of(tmp,pos);
  
  buffer->encrypt_buffer(buffer,g_crypto);  

  buffer->decrypt_buffer(buffer,g_crypto);
  
  pos = buffer->get_size(buffer);
  delete [] tmp;
  tmp = new byte_t[pos];

  buffer->read(r_cursor,tmp,pos);
  buffer->set_position(r_cursor,0);
  crc = crc_of(tmp,pos);
  delete [] tmp;

  
  buffer->close_cursor(r_cursor);
  buffer->close_cursor(w_cursor);  
  c->delete_buffer(buffer);
}

ADDTEST( buffer_test_encrypt_file)
{

  crapto_runtime_t* c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);

  crapto_buffer_t* buffer=  c->new_buffer(c,2020);
  CRAPTO_ASSERT_TRUE(buffer!=0);
  
  crapto_cursor w_cursor= buffer->open_cursor(buffer);
  buffer->read_from_file(w_cursor,test_file,EOF);
  
  length_t pos = buffer->get_position(w_cursor);
  byte_t* tmp = new byte_t[pos];

  crapto_cursor r_cursor= buffer->open_cursor(buffer);    
  
  buffer->read(r_cursor,tmp,pos);
  buffer->set_position(r_cursor,0);
  unsigned int crc = crc_of(tmp,pos);
  

  buffer->encrypt_buffer(buffer,g_crypto);  

  buffer->write_to_file(r_cursor,test_file_lck,CRAPTO_WRITE_ALL);
  delete [] tmp;
  buffer->close_cursor(r_cursor);  
  buffer->close_cursor(w_cursor);  
  c->delete_buffer(buffer);

  buffer =  c->new_buffer(c,2020);

  w_cursor= buffer->open_cursor(buffer);
  buffer->read_from_file(w_cursor,test_file_lck,CRAPTO_READ_ALL);
  
  buffer->decrypt_buffer(buffer,g_crypto);

  r_cursor= buffer->open_cursor(buffer);        
  pos = buffer->get_size(buffer);  
  tmp = new byte_t[pos];
  buffer->read(r_cursor,tmp,pos);  
  unsigned int crc2 = crc_of(tmp,pos);
  delete [] tmp;
  

  CRAPTO_ASSERT_EQ(crc,crc2);
  buffer->close_cursor(r_cursor);  
  buffer->close_cursor(w_cursor);  
  c->delete_buffer(buffer);
}


ADDTEST(something)
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
  crapto_cursor w_cursor;
  crapto_cursor r_cursor;                	      
	      
        
  //
  // Get the runtime
  //
  rt = crapto();
  //
  // Make a buffer
  //
  buffer = rt->new_buffer(rt,size);
        
  //
  // Make a w_cursor for the buffer
  //
  w_cursor = buffer->open_cursor(buffer);
        
  //
  // Write input data
  //
  buffer->write(w_cursor,(const byte_t*)in,size);
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
  r_cursor = buffer->open_cursor(buffer);
  buffer->read(r_cursor,(byte_t*)in,size);
        
  //
  // Cleanup
  //
  buffer->close_cursor(r_cursor);
  buffer->close_cursor(w_cursor);
  rt->delete_buffer(buffer);
}

void dump_crypto_stats();

int crc_visitor(void* ctx, length_t buffer_pos, byte_t* data, length_t data_length)
{
  crc_append(data,data_length,(unsigned int*)ctx);
  return 1;
}
ONLYTEST(read_write_cursor)
{
  crapto_runtime_t* c = crapto();
  byte_t buf[1000];
  memset(buf,1,1000);
  
  int FILE_SIZE;
  length_t READ_FROM_FILE;
  length_t BUFFER_POS_POST_READ;
  {
    auto f = fopen(test_file,"rb");
    fseek(f,0,SEEK_END);
    FILE_SIZE= ftell(f);
    fseek(f,0,SEEK_SET);
    fclose(f);
  }
  crapto_buffer_t* buffer=  c->new_buffer(c,2020);
  CRAPTO_ASSERT_TRUE(buffer!=0);
  
  crapto_cursor cursor= buffer->open_cursor(buffer);

  //
  // Read from clean input
  //
  READ_FROM_FILE = buffer->read_from_file(cursor,test_file,CRAPTO_READ_ALL);    
  CRAPTO_ASSERT_EQ(FILE_SIZE,READ_FROM_FILE);
  
  unsigned int CRC;
  CrcStart(CRC);
  buffer->visit(buffer,crc_visitor,&CRC,0,EOF);
  CrcEnd(CRC);

  BUFFER_POS_POST_READ = buffer->get_position(cursor);
  CRAPTO_ASSERT_EQ(FILE_SIZE,BUFFER_POS_POST_READ);

  buffer->encrypt_buffer(buffer,g_crypto);  

  buffer->set_position(cursor,0);
  length_t amount_wrote = buffer->write_to_file(cursor,test_file_lck,CRAPTO_WRITE_ALL);
  
  buffer->close_cursor(cursor);  
  c->delete_buffer(buffer);

  buffer =  c->new_buffer(c,2020);
  cursor= buffer->open_cursor(buffer);
  buffer->read_from_file(cursor,test_file_lck,CRAPTO_READ_ALL);  
  buffer->decrypt_buffer(buffer,g_crypto);
  
  unsigned int CRC2;
  CrcStart(CRC2);
  buffer->visit(buffer,crc_visitor,&CRC2,0,EOF);
  CrcEnd(CRC2);

  buffer->set_position(cursor,0);
  buffer->write_to_file(cursor,test_file_unlocked,BUFFER_POS_POST_READ);  
  
  CRAPTO_ASSERT_EQ(CRC,CRC2);  
  buffer->close_cursor(cursor);  
  c->delete_buffer(buffer);
}