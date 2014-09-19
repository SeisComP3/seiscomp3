/***************************************************************************** 
 * endian_swap.h
 *
 * C++ classes for automatic byteorder conversion
 *
 * (c) 2000 Andres Heinloo
 *****************************************************************************/

#ifndef BYTEORDER_SWAP_H
#define BYTEORDER_SWAP_H

#define PACKED __attribute__ ((packed))

namespace ByteOrderSwap {

inline u_int16_t swap16(u_int16_t val)
  {
    return (val >> 8) | (val << 8);
  }

inline u_int32_t swap32(u_int32_t val)
  {
    return ((val>>24) | ((val>>8)&0xFF00) | ((val<<8)&0xFF0000) | (val<<24));
  }

class int16_swap
  {
  private:
    u_int16_t value;

  public:
    int16_t operator=(int16_t cpuval)
      {
        value = swap16(cpuval);
        return cpuval;
      }

    operator int16_t() const
      {
        return swap16(value);
      }
  } PACKED;
  
class u_int16_swap
  {
  private:
    u_int16_t value;

  public:
    u_int16_t operator=(u_int16_t cpuval)
      {
        value = swap16(cpuval);
        return cpuval;
      }

    operator u_int16_t() const
      {
        return swap16(value);
      }
  } PACKED;
  
class int32_swap
  {
  private:
    u_int32_t value;

  public:
    int32_t operator=(int32_t cpuval)
      {
        value = swap32(cpuval);
        return cpuval;
      }

    operator int32_t() const
      {
        return swap32(value);
      }
  } PACKED;
  
class u_int32_swap
  {
  private:
    u_int32_t value;

  public:
    u_int32_t operator=(u_int32_t cpuval)
      {
        value = swap32(cpuval);
        return cpuval;
      }

    operator u_int32_t() const
      {
        return swap32(value);
      }
  } PACKED;

/* Assume 32-bit float in IEEE format */

class float32_swap
  {
  private:
    u_int32_t value;

  public:
    float operator=(float cpuval)
      {
        value = swap32(*reinterpret_cast<u_int32_t *>(&cpuval));
        return cpuval;
      }

    operator float() const
      {
        u_int32_t tmp = swap32(value);
        return *reinterpret_cast<float *>(&tmp);
      }
  } PACKED;

} // namespace ByteOrderSwap
  
#endif // BYTEORDER_SWAP_H

