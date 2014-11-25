#ifndef CRC7_H
#define CRC7_H

__attribute__ ((aligned (1)))
typedef union
{
  struct {
    char b0 :1;
    char b1 :1;
    char b2 :1;
    char b3 :1;
    char b4 :1;
    char b5 :1;
    char b6 :1;
  } bits;

  char u :7;
} uns7;

void crc7(unsigned char* buf, unsigned int size, uns7* crc);
void crc7_pushb(uns7* crc, char b);

#endif
