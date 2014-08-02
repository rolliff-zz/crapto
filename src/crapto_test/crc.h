#pragma once

#define CrcStart(crc)\
  crc = 0xffffffff

#define CrcEnd(crc)\
  crc= crc ^ 0xffffffff

void crc_append(unsigned char* data, int length, unsigned int* CRC);
unsigned int crc_of (unsigned char *data, int length);