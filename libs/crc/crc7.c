#include "crc7.h"

void crc7(unsigned char* buf, unsigned int size, uns7* crc)
{
  while(size--) {
    unsigned char ch = *buf++;
    char i;

    for(i = 0; i < 8; i++)
    {
      uns7 oldcrc;
      char b = ((ch&(1<<i)) != 0);

      oldcrc.u = crc->u;

      crc->bits.b0 = oldcrc.bits.b6 ^ b;
      crc->bits.b1 = oldcrc.bits.b0;
      crc->bits.b2 = oldcrc.bits.b1;
      crc->bits.b3 = oldcrc.bits.b2 ^ (crc->bits.b0);
      crc->bits.b4 = oldcrc.bits.b3;
      crc->bits.b5 = oldcrc.bits.b4;
      crc->bits.b6 = oldcrc.bits.b5;
    } 
  }
}

void crc7_pushb(uns7* crc, char b)
{
	uns7 oldcrc;
	oldcrc.u = crc->u;

	b = (b != 0);

	crc->bits.b0 = oldcrc.bits.b6 ^ b;
	crc->bits.b1 = oldcrc.bits.b0;
	crc->bits.b2 = oldcrc.bits.b1;
	crc->bits.b3 = oldcrc.bits.b2 ^ (oldcrc.bits.b6 ^ b);
	crc->bits.b4 = oldcrc.bits.b3;
	crc->bits.b5 = oldcrc.bits.b4;
	crc->bits.b6 = oldcrc.bits.b5;
}
