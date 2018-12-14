/////////////
#ifndef PROBE_INCLUDES_HPP
#define PROBE_INCLUDES_HPP

#include <new>
#include <stddef.h>

#if defined(_MSC_VER)
#include <sal.h>
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#ifndef __has_extension
#define __has_extension(x) 0
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

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

#ifndef LLVM_GNUC_PREREQ
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#define LLVM_GNUC_PREREQ(maj, min, patch)                                      \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >=          \
   ((maj) << 20) + ((min) << 10) + (patch))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define LLVM_GNUC_PREREQ(maj, min, patch)                                      \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
#else
#define LLVM_GNUC_PREREQ(maj, min, patch) 0
#endif
#endif

/// \macro LLVM_ASSUME_ALIGNED
/// Returns a pointer with an assumed alignment.
#if __has_builtin(__builtin_assume_aligned) || LLVM_GNUC_PREREQ(4, 7, 0)
#define LLVM_ASSUME_ALIGNED(p, a) __builtin_assume_aligned(p, a)
#elif defined(LLVM_BUILTIN_UNREACHABLE)
// As of today, clang does not support __builtin_assume_aligned.
#define LLVM_ASSUME_ALIGNED(p, a)                                              \
  (((uintptr_t(p) % (a)) == 0) ? (p) : (LLVM_BUILTIN_UNREACHABLE, (p)))
#else
#define LLVM_ASSUME_ALIGNED(p, a) (p)
#endif

#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#endif

#include <inttypes.h>
#include <stdint.h>

#ifndef _MSC_VER

#if !defined(UINT32_MAX)
#error "The standard header <cstdint> is not C++11 compliant. Must #define "\
        "__STDC_LIMIT_MACROS before #including llvm-c/DataTypes.h"
#endif

#if !defined(UINT32_C)
#error "The standard header <cstdint> is not C++11 compliant. Must #define "\
        "__STDC_CONSTANT_MACROS before #including llvm-c/DataTypes.h"
#endif

/* Note that <inttypes.h> includes <stdint.h>, if this is a C99 system. */
#include <sys/types.h>

#ifdef _AIX
// GCC is strict about defining large constants: they must have LL modifier.
#undef INT64_MAX
#undef INT64_MIN
#endif

#else /* _MSC_VER */
#ifdef __cplusplus
#include <cstddef>
#include <cstdlib>
#else
#include <stddef.h>
#include <stdlib.h>
#endif
#include <sys/types.h>

#if defined(_WIN64)
typedef signed __int64 ssize_t;
#else
typedef signed int ssize_t;
#endif /* _WIN64 */

#endif /* _MSC_VER */

/* Set defaults for constants which we cannot find. */
#if !defined(INT64_MAX)
#define INT64_MAX 9223372036854775807LL
#endif
#if !defined(INT64_MIN)
#define INT64_MIN ((-INT64_MAX) - 1)
#endif
#if !defined(UINT64_MAX)
#define UINT64_MAX 0xffffffffffffffffULL
#endif

#ifndef HUGE_VALF
#define HUGE_VALF (float)HUGE_VAL
#endif

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <cstddef>

namespace llvm {

/// \struct AlignedCharArray
/// Helper for building an aligned character array type.
///
/// This template is used to explicitly build up a collection of aligned
/// character array types. We have to build these up using a macro and explicit
/// specialization to cope with MSVC (at least till 2015) where only an
/// integer literal can be used to specify an alignment constraint. Once built
/// up here, we can then begin to indirect between these using normal C++
/// template parameters.

// MSVC requires special handling here.
#ifndef _MSC_VER

template <std::size_t Alignment, std::size_t Size> struct AlignedCharArray {
  alignas(Alignment) char buffer[Size];
};

#else // _MSC_VER

/// Create a type with an aligned char buffer.
template <std::size_t Alignment, std::size_t Size> struct AlignedCharArray;

// We provide special variations of this template for the most common
// alignments because __declspec(align(...)) doesn't actually work when it is
// a member of a by-value function argument in MSVC, even if the alignment
// request is something reasonably like 8-byte or 16-byte. Note that we can't
// even include the declspec with the union that forces the alignment because
// MSVC warns on the existence of the declspec despite the union member forcing
// proper alignment.

template <std::size_t Size> struct AlignedCharArray<1, Size> {
  union {
    char aligned;
    char buffer[Size];
  };
};

template <std::size_t Size> struct AlignedCharArray<2, Size> {
  union {
    short aligned;
    char buffer[Size];
  };
};

template <std::size_t Size> struct AlignedCharArray<4, Size> {
  union {
    int aligned;
    char buffer[Size];
  };
};

template <std::size_t Size> struct AlignedCharArray<8, Size> {
  union {
    double aligned;
    char buffer[Size];
  };
};

// The rest of these are provided with a __declspec(align(...)) and we simply
// can't pass them by-value as function arguments on MSVC.

#define LLVM_ALIGNEDCHARARRAY_TEMPLATE_ALIGNMENT(x)                            \
  template <std::size_t Size> struct AlignedCharArray<x, Size> {               \
    __declspec(align(x)) char buffer[Size];                                    \
  };

LLVM_ALIGNEDCHARARRAY_TEMPLATE_ALIGNMENT(16)
LLVM_ALIGNEDCHARARRAY_TEMPLATE_ALIGNMENT(32)
LLVM_ALIGNEDCHARARRAY_TEMPLATE_ALIGNMENT(64)
LLVM_ALIGNEDCHARARRAY_TEMPLATE_ALIGNMENT(128)

#undef LLVM_ALIGNEDCHARARRAY_TEMPLATE_ALIGNMENT

#endif // _MSC_VER

namespace detail {
template <typename T1, typename T2 = char, typename T3 = char,
          typename T4 = char, typename T5 = char, typename T6 = char,
          typename T7 = char, typename T8 = char, typename T9 = char,
          typename T10 = char>
class AlignerImpl {
  T1 t1;
  T2 t2;
  T3 t3;
  T4 t4;
  T5 t5;
  T6 t6;
  T7 t7;
  T8 t8;
  T9 t9;
  T10 t10;

  AlignerImpl() = delete;
};

template <typename T1, typename T2 = char, typename T3 = char,
          typename T4 = char, typename T5 = char, typename T6 = char,
          typename T7 = char, typename T8 = char, typename T9 = char,
          typename T10 = char>
union SizerImpl {
  char arr1[sizeof(T1)], arr2[sizeof(T2)], arr3[sizeof(T3)], arr4[sizeof(T4)],
      arr5[sizeof(T5)], arr6[sizeof(T6)], arr7[sizeof(T7)], arr8[sizeof(T8)],
      arr9[sizeof(T9)], arr10[sizeof(T10)];
};
} // end namespace detail

/// This union template exposes a suitably aligned and sized character
/// array member which can hold elements of any of up to ten types.
///
/// These types may be arrays, structs, or any other types. The goal is to
/// expose a char array buffer member which can be used as suitable storage for
/// a placement new of any of these types. Support for more than ten types can
/// be added at the cost of more boilerplate.
template <typename T1, typename T2 = char, typename T3 = char,
          typename T4 = char, typename T5 = char, typename T6 = char,
          typename T7 = char, typename T8 = char, typename T9 = char,
          typename T10 = char>
struct AlignedCharArrayUnion
    : llvm::AlignedCharArray<alignof(llvm::detail::AlignerImpl<
                                     T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>),
                             sizeof(::llvm::detail::SizerImpl<
                                    T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>)> {
};
} // end namespace llvm

namespace llvm {
namespace sys {

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
constexpr bool IsBigEndianHost = true;
#else
constexpr bool IsBigEndianHost = false;
#endif

static const bool IsLittleEndianHost = !IsBigEndianHost;

} // namespace sys
} // namespace llvm

namespace llvm {
namespace sys {

/// SwapByteOrder_16 - This function returns a byte-swapped representation of
/// the 16-bit argument.
inline uint16_t SwapByteOrder_16(uint16_t value) {
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

/// SwapByteOrder_32 - This function returns a byte-swapped representation of
/// the 32-bit argument.
inline uint32_t SwapByteOrder_32(uint32_t value) {
#if defined(__llvm__) || (LLVM_GNUC_PREREQ(4, 3, 0) && !defined(__ICC))
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
inline uint64_t SwapByteOrder_64(uint64_t value) {
#if defined(__llvm__) || (LLVM_GNUC_PREREQ(4, 3, 0) && !defined(__ICC))
  return __builtin_bswap64(value);
#elif defined(_MSC_VER) && !defined(_DEBUG)
  return _byteswap_uint64(value);
#else
  uint64_t Hi = SwapByteOrder_32(uint32_t(value));
  uint32_t Lo = SwapByteOrder_32(uint32_t(value >> 32));
  return (Hi << 32) | Lo;
#endif
}

inline unsigned char getSwappedBytes(unsigned char C) { return C; }
inline signed char getSwappedBytes(signed char C) { return C; }
inline char getSwappedBytes(char C) { return C; }

inline unsigned short getSwappedBytes(unsigned short C) {
  return SwapByteOrder_16(C);
}
inline signed short getSwappedBytes(signed short C) {
  return SwapByteOrder_16(C);
}

inline unsigned int getSwappedBytes(unsigned int C) {
  return SwapByteOrder_32(C);
}
inline signed int getSwappedBytes(signed int C) { return SwapByteOrder_32(C); }

#if __LONG_MAX__ == __INT_MAX__
inline unsigned long getSwappedBytes(unsigned long C) {
  return SwapByteOrder_32(C);
}
inline signed long getSwappedBytes(signed long C) {
  return SwapByteOrder_32(C);
}
#elif __LONG_MAX__ == __LONG_LONG_MAX__
inline unsigned long getSwappedBytes(unsigned long C) {
  return SwapByteOrder_64(C);
}
inline signed long getSwappedBytes(signed long C) {
  return SwapByteOrder_64(C);
}
#else
#error "Unknown long size!"
#endif

inline unsigned long long getSwappedBytes(unsigned long long C) {
  return SwapByteOrder_64(C);
}
inline signed long long getSwappedBytes(signed long long C) {
  return SwapByteOrder_64(C);
}

inline float getSwappedBytes(float C) {
  union {
    uint32_t i;
    float f;
  } in, out;
  in.f = C;
  out.i = SwapByteOrder_32(in.i);
  return out.f;
}

inline double getSwappedBytes(double C) {
  union {
    uint64_t i;
    double d;
  } in, out;
  in.d = C;
  out.i = SwapByteOrder_64(in.i);
  return out.d;
}

template <typename T> inline void swapByteOrder(T &Value) {
  Value = getSwappedBytes(Value);
}

} // end namespace sys
} // end namespace llvm

namespace llvm {
namespace support {

enum endianness { big, little, native };

// These are named values for common alignments.
enum { aligned = 0, unaligned = 1 };

namespace detail {

/// ::value is either alignment, or alignof(T) if alignment is 0.
template <class T, int alignment> struct PickAlignment {
  enum { value = alignment == 0 ? alignof(T) : alignment };
};

} // end namespace detail

namespace endian {

constexpr endianness system_endianness() {
  return sys::IsBigEndianHost ? big : little;
}

template <typename value_type>
inline value_type byte_swap(value_type value, endianness endian) {
  if ((endian != native) && (endian != system_endianness()))
    sys::swapByteOrder(value);
  return value;
}

/// Swap the bytes of value to match the given endianness.
template <typename value_type, endianness endian>
inline value_type byte_swap(value_type value) {
  return byte_swap(value, endian);
}

/// Read a value of a particular endianness from memory.
template <typename value_type, std::size_t alignment>
inline value_type read(const void *memory, endianness endian) {
  value_type ret;

  memcpy(&ret,
         LLVM_ASSUME_ALIGNED(
             memory, (detail::PickAlignment<value_type, alignment>::value)),
         sizeof(value_type));
  return byte_swap<value_type>(ret, endian);
}

template <typename value_type, endianness endian, std::size_t alignment>
inline value_type read(const void *memory) {
  return read<value_type, alignment>(memory, endian);
}

/// Read a value of a particular endianness from a buffer, and increment the
/// buffer past that value.
template <typename value_type, std::size_t alignment, typename CharT>
inline value_type readNext(const CharT *&memory, endianness endian) {
  value_type ret = read<value_type, alignment>(memory, endian);
  memory += sizeof(value_type);
  return ret;
}

template <typename value_type, endianness endian, std::size_t alignment,
          typename CharT>
inline value_type readNext(const CharT *&memory) {
  return readNext<value_type, alignment, CharT>(memory, endian);
}

/// Write a value to memory with a particular endianness.
template <typename value_type, std::size_t alignment>
inline void write(void *memory, value_type value, endianness endian) {
  value = byte_swap<value_type>(value, endian);
  memcpy(LLVM_ASSUME_ALIGNED(
             memory, (detail::PickAlignment<value_type, alignment>::value)),
         &value, sizeof(value_type));
}

template <typename value_type, endianness endian, std::size_t alignment>
inline void write(void *memory, value_type value) {
  write<value_type, alignment>(memory, value, endian);
}

template <typename value_type>
using make_unsigned_t = typename std::make_unsigned<value_type>::type;

/// Read a value of a particular endianness from memory, for a location
/// that starts at the given bit offset within the first byte.
template <typename value_type, endianness endian, std::size_t alignment>
inline value_type readAtBitAlignment(const void *memory, uint64_t startBit) {
  assert(startBit < 8);
  if (startBit == 0)
    return read<value_type, endian, alignment>(memory);
  else {
    // Read two values and compose the result from them.
    value_type val[2];
    memcpy(&val[0],
           LLVM_ASSUME_ALIGNED(
               memory, (detail::PickAlignment<value_type, alignment>::value)),
           sizeof(value_type) * 2);
    val[0] = byte_swap<value_type, endian>(val[0]);
    val[1] = byte_swap<value_type, endian>(val[1]);

    // Shift bits from the lower value into place.
    make_unsigned_t<value_type> lowerVal = val[0] >> startBit;
    // Mask off upper bits after right shift in case of signed type.
    make_unsigned_t<value_type> numBitsFirstVal =
        (sizeof(value_type) * 8) - startBit;
    lowerVal &= ((make_unsigned_t<value_type>)1 << numBitsFirstVal) - 1;

    // Get the bits from the upper value.
    make_unsigned_t<value_type> upperVal =
        val[1] & (((make_unsigned_t<value_type>)1 << startBit) - 1);
    // Shift them in to place.
    upperVal <<= numBitsFirstVal;

    return lowerVal | upperVal;
  }
}

/// Write a value to memory with a particular endianness, for a location
/// that starts at the given bit offset within the first byte.
template <typename value_type, endianness endian, std::size_t alignment>
inline void writeAtBitAlignment(void *memory, value_type value,
                                uint64_t startBit) {
  assert(startBit < 8);
  if (startBit == 0)
    write<value_type, endian, alignment>(memory, value);
  else {
    // Read two values and shift the result into them.
    value_type val[2];
    memcpy(&val[0],
           LLVM_ASSUME_ALIGNED(
               memory, (detail::PickAlignment<value_type, alignment>::value)),
           sizeof(value_type) * 2);
    val[0] = byte_swap<value_type, endian>(val[0]);
    val[1] = byte_swap<value_type, endian>(val[1]);

    // Mask off any existing bits in the upper part of the lower value that
    // we want to replace.
    val[0] &= ((make_unsigned_t<value_type>)1 << startBit) - 1;
    make_unsigned_t<value_type> numBitsFirstVal =
        (sizeof(value_type) * 8) - startBit;
    make_unsigned_t<value_type> lowerVal = value;
    if (startBit > 0) {
      // Mask off the upper bits in the new value that are not going to go into
      // the lower value. This avoids a left shift of a negative value, which
      // is undefined behavior.
      lowerVal &= (((make_unsigned_t<value_type>)1 << numBitsFirstVal) - 1);
      // Now shift the new bits into place
      lowerVal <<= startBit;
    }
    val[0] |= lowerVal;

    // Mask off any existing bits in the lower part of the upper value that
    // we want to replace.
    val[1] &= ~(((make_unsigned_t<value_type>)1 << startBit) - 1);
    // Next shift the bits that go into the upper value into position.
    make_unsigned_t<value_type> upperVal = value >> numBitsFirstVal;
    // Mask off upper bits after right shift in case of signed type.
    upperVal &= ((make_unsigned_t<value_type>)1 << startBit) - 1;
    val[1] |= upperVal;

    // Finally, rewrite values.
    val[0] = byte_swap<value_type, endian>(val[0]);
    val[1] = byte_swap<value_type, endian>(val[1]);
    memcpy(LLVM_ASSUME_ALIGNED(
               memory, (detail::PickAlignment<value_type, alignment>::value)),
           &val[0], sizeof(value_type) * 2);
  }
}

} // end namespace endian

namespace detail {

template <typename value_type, endianness endian, std::size_t alignment>
struct packed_endian_specific_integral {
  packed_endian_specific_integral() = default;

  explicit packed_endian_specific_integral(value_type val) { *this = val; }

  operator value_type() const {
    return endian::read<value_type, endian, alignment>(
        (const void *)Value.buffer);
  }

  void operator=(value_type newValue) {
    endian::write<value_type, endian, alignment>((void *)Value.buffer,
                                                 newValue);
  }

  packed_endian_specific_integral &operator+=(value_type newValue) {
    *this = *this + newValue;
    return *this;
  }

  packed_endian_specific_integral &operator-=(value_type newValue) {
    *this = *this - newValue;
    return *this;
  }

  packed_endian_specific_integral &operator|=(value_type newValue) {
    *this = *this | newValue;
    return *this;
  }

  packed_endian_specific_integral &operator&=(value_type newValue) {
    *this = *this & newValue;
    return *this;
  }

private:
  AlignedCharArray<PickAlignment<value_type, alignment>::value,
                   sizeof(value_type)>
      Value;

public:
  struct ref {
    explicit ref(void *Ptr) : Ptr(Ptr) {}

    operator value_type() const {
      return endian::read<value_type, endian, alignment>(Ptr);
    }

    void operator=(value_type NewValue) {
      endian::write<value_type, endian, alignment>(Ptr, NewValue);
    }

  private:
    void *Ptr;
  };
};

} // end namespace detail

using ulittle16_t =
    detail::packed_endian_specific_integral<uint16_t, little, unaligned>;
using ulittle32_t =
    detail::packed_endian_specific_integral<uint32_t, little, unaligned>;
using ulittle64_t =
    detail::packed_endian_specific_integral<uint64_t, little, unaligned>;

using little16_t =
    detail::packed_endian_specific_integral<int16_t, little, unaligned>;
using little32_t =
    detail::packed_endian_specific_integral<int32_t, little, unaligned>;
using little64_t =
    detail::packed_endian_specific_integral<int64_t, little, unaligned>;

using aligned_ulittle16_t =
    detail::packed_endian_specific_integral<uint16_t, little, aligned>;
using aligned_ulittle32_t =
    detail::packed_endian_specific_integral<uint32_t, little, aligned>;
using aligned_ulittle64_t =
    detail::packed_endian_specific_integral<uint64_t, little, aligned>;

using aligned_little16_t =
    detail::packed_endian_specific_integral<int16_t, little, aligned>;
using aligned_little32_t =
    detail::packed_endian_specific_integral<int32_t, little, aligned>;
using aligned_little64_t =
    detail::packed_endian_specific_integral<int64_t, little, aligned>;

using ubig16_t =
    detail::packed_endian_specific_integral<uint16_t, big, unaligned>;
using ubig32_t =
    detail::packed_endian_specific_integral<uint32_t, big, unaligned>;
using ubig64_t =
    detail::packed_endian_specific_integral<uint64_t, big, unaligned>;

using big16_t =
    detail::packed_endian_specific_integral<int16_t, big, unaligned>;
using big32_t =
    detail::packed_endian_specific_integral<int32_t, big, unaligned>;
using big64_t =
    detail::packed_endian_specific_integral<int64_t, big, unaligned>;

using aligned_ubig16_t =
    detail::packed_endian_specific_integral<uint16_t, big, aligned>;
using aligned_ubig32_t =
    detail::packed_endian_specific_integral<uint32_t, big, aligned>;
using aligned_ubig64_t =
    detail::packed_endian_specific_integral<uint64_t, big, aligned>;

using aligned_big16_t =
    detail::packed_endian_specific_integral<int16_t, big, aligned>;
using aligned_big32_t =
    detail::packed_endian_specific_integral<int32_t, big, aligned>;
using aligned_big64_t =
    detail::packed_endian_specific_integral<int64_t, big, aligned>;

using unaligned_uint16_t =
    detail::packed_endian_specific_integral<uint16_t, native, unaligned>;
using unaligned_uint32_t =
    detail::packed_endian_specific_integral<uint32_t, native, unaligned>;
using unaligned_uint64_t =
    detail::packed_endian_specific_integral<uint64_t, native, unaligned>;

using unaligned_int16_t =
    detail::packed_endian_specific_integral<int16_t, native, unaligned>;
using unaligned_int32_t =
    detail::packed_endian_specific_integral<int32_t, native, unaligned>;
using unaligned_int64_t =
    detail::packed_endian_specific_integral<int64_t, native, unaligned>;

namespace endian {

template <typename T> inline T read(const void *P, endianness E) {
  return read<T, unaligned>(P, E);
}

template <typename T, endianness E> inline T read(const void *P) {
  return *(const detail::packed_endian_specific_integral<T, E, unaligned> *)P;
}

inline uint16_t read16(const void *P, endianness E) {
  return read<uint16_t>(P, E);
}
inline uint32_t read32(const void *P, endianness E) {
  return read<uint32_t>(P, E);
}
inline uint64_t read64(const void *P, endianness E) {
  return read<uint64_t>(P, E);
}

template <endianness E> inline uint16_t read16(const void *P) {
  return read<uint16_t, E>(P);
}
template <endianness E> inline uint32_t read32(const void *P) {
  return read<uint32_t, E>(P);
}
template <endianness E> inline uint64_t read64(const void *P) {
  return read<uint64_t, E>(P);
}

inline uint16_t read16le(const void *P) { return read16<little>(P); }
inline uint32_t read32le(const void *P) { return read32<little>(P); }
inline uint64_t read64le(const void *P) { return read64<little>(P); }
inline uint16_t read16be(const void *P) { return read16<big>(P); }
inline uint32_t read32be(const void *P) { return read32<big>(P); }
inline uint64_t read64be(const void *P) { return read64<big>(P); }

template <typename T> inline void write(void *P, T V, endianness E) {
  write<T, unaligned>(P, V, E);
}

template <typename T, endianness E> inline void write(void *P, T V) {
  *(detail::packed_endian_specific_integral<T, E, unaligned> *)P = V;
}

inline void write16(void *P, uint16_t V, endianness E) {
  write<uint16_t>(P, V, E);
}
inline void write32(void *P, uint32_t V, endianness E) {
  write<uint32_t>(P, V, E);
}
inline void write64(void *P, uint64_t V, endianness E) {
  write<uint64_t>(P, V, E);
}

template <endianness E> inline void write16(void *P, uint16_t V) {
  write<uint16_t, E>(P, V);
}
template <endianness E> inline void write32(void *P, uint32_t V) {
  write<uint32_t, E>(P, V);
}
template <endianness E> inline void write64(void *P, uint64_t V) {
  write<uint64_t, E>(P, V);
}

inline void write16le(void *P, uint16_t V) { write16<little>(P, V); }
inline void write32le(void *P, uint32_t V) { write32<little>(P, V); }
inline void write64le(void *P, uint64_t V) { write64<little>(P, V); }
inline void write16be(void *P, uint16_t V) { write16<big>(P, V); }
inline void write32be(void *P, uint32_t V) { write32<big>(P, V); }
inline void write64be(void *P, uint64_t V) { write64<big>(P, V); }

} // end namespace endian

} // end namespace support
} // end namespace llvm

#endif