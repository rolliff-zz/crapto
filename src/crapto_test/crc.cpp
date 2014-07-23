#include "stdafx.h"
#include "crc.h"
#	define CRC_ANSWER 0x6fcf9e13

unsigned int crc_thing[256] = {0};

inline unsigned int upd(unsigned char octet, unsigned int crc)
{
  auto n = (crc ^ octet) & 0xFF;

  return (unsigned int)(crc_thing[n] ^ (crc >> 8));
}
void init_crc()
{
  unsigned long z;
  static bool yea = false;
    
  if(yea)
    return;

  yea = true;

  for (int x = 0; x < 256; ++x) 
  {
    z = (unsigned long) x;
    for (int y = 0; y < 8; ++y) 
    {
      if (z & 1) 
      {
        z = 0xEDB88320L ^ (z >> 1);
        continue;
      }
        
      z >>= 1;
    } 
    crc_thing[x] = z;
  }

  //
  // Check Work
  //

  int i;
  unsigned int crc  = 0xffffffff;
  int pass = 0;

  for (i = 0; i < (int)sizeof(crc_thing); i++)
	  crc = upd(((unsigned char *) crc_thing)[i], crc);

  pass = CRC_ANSWER == (crc ^ 0xffffffff);

  pass &= crc_of((unsigned char *)crc_thing, sizeof(crc_thing)) == CRC_ANSWER;
  if(0 == pass)
  {
    __debugbreak();
    int zero = 0;
    int x = 1 / zero;
  }
}
unsigned int crc_of (unsigned char *data, int length)
{
	unsigned int CRC = 0xffffffff;

	while (length--)
	{
		CRC = (CRC >> 8) ^ crc_thing[ (CRC ^ *data++) & 0xFF ];
	}

	return CRC ^ 0xffffffff;
}
struct crc32init_t
{
  crc32init_t()
  {
    init_crc();
  }
}crc32init;

