/////////////
#ifndef INQUISITIVE_INCLUDES_HPP
#define INQUISITIVE_INCLUDES_HPP

#if defined(__linux__) || defined(__GNU__) || defined(__HAIKU__)
#include <endian.h>
#elif defined(_AIX)
#include <sys/machine.h>
#elif defined(__sun)
/* Solaris provides _BIG_ENDIAN/_LITTLE_ENDIAN selector in sys/types.h */
#include <sys/types.h>
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#if defined(_BIG_ENDIAN)
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#else
#if !defined(BYTE_ORDER) && !defined(_WIN32)
#include <machine/endian.h>
#endif
#endif

#ifndef PLANCK_GNUC_PREREQ
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#define PLANCK_GNUC_PREREQ(maj, min, patch)                                    \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >=          \
   ((maj) << 20) + ((min) << 10) + (patch))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define PLANCK_GNUC_PREREQ(maj, min, patch)                                    \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
#else
#define PLANCK_GNUC_PREREQ(maj, min, patch) 0
#endif
#endif
#include <cstdint>

namespace inquisitive {
#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
constexpr bool IsBigEndianHost = true;
#else
constexpr bool IsBigEndianHost = false;
#endif

inline uint16_t swapbyte16(uint16_t value) {
#if defined(_MSC_VER) && !defined(_DEBUG)
  // The DLL version of the runtime lacks these functions (bug!?), but in a
  // release build they're replaced with BSWAP instructions anyway.
  return _byteswap_ushort(value);
#else
  uint16_t Hi = value << 8;
  uint16_t Lo = value >> 8;
  return Hi | Lo;
#endif
}

inline uint32_t swapbyte32(uint32_t value) {
#if defined(__llvm__) || (PLANCK_GNUC_PREREQ(4, 3, 0) && !defined(__ICC))
  return __builtin_bswap32(value);
#elif defined(_MSC_VER) && !defined(_DEBUG)
  return _byteswap_ulong(value);
#else
  uint32_t Byte0 = value & 0x000000FF;
  uint32_t Byte1 = value & 0x0000FF00;
  uint32_t Byte2 = value & 0x00FF0000;
  uint32_t Byte3 = value & 0xFF000000;
  return (Byte0 << 24) | (Byte1 << 8) | (Byte2 >> 8) | (Byte3 >> 24);
#endif
}

/// SwapByteOrder_64 - This function returns a byte-swapped representation of
/// the 64-bit argument.
inline uint64_t swapbyte64(uint64_t value) {
#if defined(__llvm__) || (PLANCK_GNUC_PREREQ(4, 3, 0) && !defined(__ICC))
  return __builtin_bswap64(value);
#elif defined(_MSC_VER) && !defined(_DEBUG)
  return _byteswap_uint64(value);
#else
  uint64_t Hi = swapbyte32(uint32_t(value));
  uint32_t Lo = swapbyte32(uint32_t(value >> 32));
  return (Hi << 32) | Lo;
#endif
}
inline uint8_t swapbyte(uint8_t c) { return c; }
inline int8_t swapbyte(int8_t c) { return c; }
inline uint16_t swapbyte(uint16_t value) { return swapbyte16(value); }
inline int16_t swapbyte(int16_t value) {
  return static_cast<int16_t>(swapbyte16(value));
}
inline uint32_t swapbyte(uint32_t value) { return swapbyte32(value); }
inline int32_t swapbyte(int32_t value) {
  return static_cast<int32_t>(swapbyte32(value));
}

#if __LONG_MAX__ == __INT_MAX__
inline unsigned long swapbyte(unsigned long C) { return swapbyte32(C); }
inline signed long swapbyte(signed long C) { return swapbyte32(C); }
#elif __LONG_MAX__ == __LONG_LONG_MAX__
inline unsigned long swapbyte(unsigned long C) { return swapbyte64(C); }
inline signed long swapbyte(signed long C) { return swapbyte64(C); }
#else
#error "Unknown long size!"
#endif
inline unsigned long long swapbyte(unsigned long long C) {
  return swapbyte64(C);
}
inline signed long long swapbyte(signed long long C) { return swapbyte64(C); }

template <typename Integer> Integer readle(void *p) {
  auto i = *reinterpret_cast<Integer *>(p);
  if (IsBigEndianHost) {
    return swapbyte(i);
  }
  return i;
}

template <typename Integer> Integer readbe(void *p) {
  auto i = *reinterpret_cast<Integer *>(p);
  if (IsBigEndianHost) {
    return i;
  }
  return swapbyte(i);
}

} // namespace inquisitive

#endif
