
dword fnv(const byte *buf, size sz, dword seed)
{
    for(int i = 0; i < sz; i++) {
        seed += (seed<<1) + (seed<<4) + (seed<<7) + (seed<<8) + (seed<<24);
        seed ^= buf[i];
    }

    return seed;
}

