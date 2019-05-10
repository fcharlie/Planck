///////
#ifndef CLANGBUILDER_STRCAT_HPP
#define CLANGBUILDER_STRCAT_HPP
#pragma once
#include <string_view>
#include <string>
#include <array>
#include <algorithm>
#include <cassert>
#include <cfloat> // for DBL_DIG and FLT_DIG
#include <cmath>  // for HUGE_VAL
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>
// Clang on Windows has __builtin_clzll; otherwise we need to use the
// windows intrinsic functions.
#if defined(_MSC_VER)
#include <intrin.h>
#if defined(_M_X64)
#pragma intrinsic(_BitScanReverse64)
#pragma intrinsic(_BitScanForward64)
#endif
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#endif

namespace base {

namespace numbers_internal {
inline int CountLeadingZeros64(uint64_t n) {
#if defined(_MSC_VER) && defined(_M_X64)
  // MSVC does not have __buitin_clzll. Use _BitScanReverse64.
  unsigned long result = 0; // NOLINT(runtime/int)
  if (_BitScanReverse64(&result, n)) {
    return 63 - result;
  }
  return 64;
#elif defined(_MSC_VER)
  // MSVC does not have __buitin_clzll. Compose two calls to _BitScanReverse
  unsigned long result = 0; // NOLINT(runtime/int)
  if ((n >> 32) && _BitScanReverse(&result, n >> 32)) {
    return 31 - result;
  }
  if (_BitScanReverse(&result, n)) {
    return 63 - result;
  }
  return 64;
#elif defined(__GNUC__)
  // Use __builtin_clzll, which uses the following instructions:
  //  x86: bsr
  //  ARM64: clz
  //  PPC: cntlzd
  static_assert(sizeof(unsigned long long) == sizeof(n), // NOLINT(runtime/int)
                "__builtin_clzll does not take 64-bit arg");

  // Handle 0 as a special case because __builtin_clzll(0) is undefined.
  if (n == 0) {
    return 64;
  }
  return __builtin_clzll(n);
#else
  return CountLeadingZeros64Slow(n);
#endif
}

// Writes a two-character representation of 'i' to 'buf'. 'i' must be in the
// range 0 <= i < 100, and buf must have space for two characters. Example:
//   char buf[2];
//   PutTwoDigits(42, buf);
//   // buf[0] == '4'
//   // buf[1] == '2'
inline void PutTwoDigits(size_t i, wchar_t *buf) {
  static const wchar_t two_ASCII_digits[100][2] = {
      {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, {'0', '5'},
      {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, {'1', '0'}, {'1', '1'},
      {'1', '2'}, {'1', '3'}, {'1', '4'}, {'1', '5'}, {'1', '6'}, {'1', '7'},
      {'1', '8'}, {'1', '9'}, {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'},
      {'2', '4'}, {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'},
      {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, {'3', '5'},
      {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, {'4', '0'}, {'4', '1'},
      {'4', '2'}, {'4', '3'}, {'4', '4'}, {'4', '5'}, {'4', '6'}, {'4', '7'},
      {'4', '8'}, {'4', '9'}, {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'},
      {'5', '4'}, {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'},
      {'6', '0'}, {'6', '1'}, {'6', '2'}, {'6', '3'}, {'6', '4'}, {'6', '5'},
      {'6', '6'}, {'6', '7'}, {'6', '8'}, {'6', '9'}, {'7', '0'}, {'7', '1'},
      {'7', '2'}, {'7', '3'}, {'7', '4'}, {'7', '5'}, {'7', '6'}, {'7', '7'},
      {'7', '8'}, {'7', '9'}, {'8', '0'}, {'8', '1'}, {'8', '2'}, {'8', '3'},
      {'8', '4'}, {'8', '5'}, {'8', '6'}, {'8', '7'}, {'8', '8'}, {'8', '9'},
      {'9', '0'}, {'9', '1'}, {'9', '2'}, {'9', '3'}, {'9', '4'}, {'9', '5'},
      {'9', '6'}, {'9', '7'}, {'9', '8'}, {'9', '9'}};
  memcpy(buf, two_ASCII_digits[i], 2 * sizeof(wchar_t));
}

inline wchar_t *FastIntToBuffer(uint32_t i, wchar_t *buffer) {
  // Used to optimize printing a decimal number's final digit.
  const wchar_t one_ASCII_final_digits[10][2]{
      {'0', 0}, {'1', 0}, {'2', 0}, {'3', 0}, {'4', 0},
      {'5', 0}, {'6', 0}, {'7', 0}, {'8', 0}, {'9', 0},
  };
  uint32_t digits;
  // The idea of this implementation is to trim the number of divides to as few
  // as possible, and also reducing memory stores and branches, by going in
  // steps of two digits at a time rather than one whenever possible.
  // The huge-number case is first, in the hopes that the compiler will output
  // that case in one branch-free block of code, and only output conditional
  // branches into it from below.
  if (i >= 1000000000) {    // >= 1,000,000,000
    digits = i / 100000000; //      100,000,000
    i -= digits * 100000000;
    PutTwoDigits(digits, buffer);
    buffer += 2;
  lt100_000_000:
    digits = i / 1000000; // 1,000,000
    i -= digits * 1000000;
    PutTwoDigits(digits, buffer);
    buffer += 2;
  lt1_000_000:
    digits = i / 10000; // 10,000
    i -= digits * 10000;
    PutTwoDigits(digits, buffer);
    buffer += 2;
  lt10_000:
    digits = i / 100;
    i -= digits * 100;
    PutTwoDigits(digits, buffer);
    buffer += 2;
  lt100:
    digits = i;
    PutTwoDigits(digits, buffer);
    buffer += 2;
    *buffer = 0;
    return buffer;
  }

  if (i < 100) {
    digits = i;
    if (i >= 10)
      goto lt100;
    memcpy(buffer, one_ASCII_final_digits[i], 2 * sizeof(wchar_t));
    return buffer + 1;
  }
  if (i < 10000) { //    10,000
    if (i >= 1000)
      goto lt10_000;
    digits = i / 100;
    i -= digits * 100;
    *buffer++ = static_cast<wchar_t>('0' + digits);
    goto lt100;
  }
  if (i < 1000000) { //    1,000,000
    if (i >= 100000)
      goto lt1_000_000;
    digits = i / 10000; //    10,000
    i -= digits * 10000;
    *buffer++ = static_cast<wchar_t>('0' + digits);
    goto lt10_000;
  }
  if (i < 100000000) { //    100,000,000
    if (i >= 10000000)
      goto lt100_000_000;
    digits = i / 1000000; //   1,000,000
    i -= digits * 1000000;
    *buffer++ = static_cast<wchar_t>('0' + digits);
    goto lt1_000_000;
  }
  // we already know that i < 1,000,000,000
  digits = i / 100000000; //   100,000,000
  i -= digits * 100000000;
  *buffer++ = static_cast<wchar_t>('0' + digits);
  goto lt100_000_000;
}

inline wchar_t *FastIntToBuffer(int32_t i, wchar_t *buffer) {
  uint32_t u = i;
  if (i < 0) {
    *buffer++ = '-';
    // We need to do the negation in modular (i.e., "unsigned")
    // arithmetic; MSVC++ apprently warns for plain "-u", so
    // we write the equivalent expression "0 - u" instead.
    u = 0 - u;
  }
  return FastIntToBuffer(u, buffer);
}

inline wchar_t *FastIntToBuffer(uint64_t i, wchar_t *buffer) {
  uint32_t u32 = static_cast<uint32_t>(i);
  if (u32 == i)
    return FastIntToBuffer(u32, buffer);

  // Here we know i has at least 10 decimal digits.
  uint64_t top_1to11 = i / 1000000000;
  u32 = static_cast<uint32_t>(i - top_1to11 * 1000000000);
  uint32_t top_1to11_32 = static_cast<uint32_t>(top_1to11);

  if (top_1to11_32 == top_1to11) {
    buffer = numbers_internal::FastIntToBuffer(top_1to11_32, buffer);
  } else {
    // top_1to11 has more than 32 bits too; print it in two steps.
    uint32_t top_8to9 = static_cast<uint32_t>(top_1to11 / 100);
    uint32_t mid_2 = static_cast<uint32_t>(top_1to11 - top_8to9 * 100);
    buffer = numbers_internal::FastIntToBuffer(top_8to9, buffer);
    PutTwoDigits(mid_2, buffer);
    buffer += 2;
  }

  // We have only 9 digits now, again the maximum uint32_t can handle fully.
  uint32_t digits = u32 / 10000000; // 10,000,000
  u32 -= digits * 10000000;
  PutTwoDigits(digits, buffer);
  buffer += 2;
  digits = u32 / 100000; // 100,000
  u32 -= digits * 100000;
  PutTwoDigits(digits, buffer);
  buffer += 2;
  digits = u32 / 1000; // 1,000
  u32 -= digits * 1000;
  PutTwoDigits(digits, buffer);
  buffer += 2;
  digits = u32 / 10;
  u32 -= digits * 10;
  PutTwoDigits(digits, buffer);
  buffer += 2;
  // Used to optimize printing a decimal number's final digit.
  const wchar_t one_ASCII_final_digits[10][2]{
      {L'0', 0}, {L'1', 0}, {L'2', 0}, {L'3', 0}, {L'4', 0},
      {L'5', 0}, {L'6', 0}, {L'7', 0}, {L'8', 0}, {L'9', 0},
  };
  memcpy(buffer, one_ASCII_final_digits[u32], 2 * sizeof(wchar_t));
  return buffer + 1;
}

inline wchar_t *FastIntToBuffer(int64_t i, wchar_t *buffer) {
  uint64_t u = i;
  if (i < 0) {
    *buffer++ = '-';
    u = 0 - u;
  }
  return FastIntToBuffer(u, buffer);
}
// For enums and integer types that are not an exact match for the types above,
// use templates to call the appropriate one of the four overloads above.
template <typename int_type>
wchar_t *FastIntToBuffer(int_type i, wchar_t *buffer) {
  static_assert(sizeof(i) <= 64 / 8,
                "FastIntToBuffer works only with 64-bit-or-less integers.");
  // TODO(jorg): This signed-ness check is used because it works correctly
  // with enums, and it also serves to check that int_type is not a pointer.
  // If one day something like std::is_signed<enum E> works, switch to it.
  if (static_cast<int_type>(1) - 2 < 0) { // Signed
    if (sizeof(i) > 32 / 8) {             // 33-bit to 64-bit
      return numbers_internal::FastIntToBuffer(static_cast<int64_t>(i), buffer);
    } else { // 32-bit or less
      return numbers_internal::FastIntToBuffer(static_cast<int32_t>(i), buffer);
    }
  } else {                    // Unsigned
    if (sizeof(i) > 32 / 8) { // 33-bit to 64-bit
      return numbers_internal::FastIntToBuffer(static_cast<uint64_t>(i),
                                               buffer);
    } else { // 32-bit or less
      return numbers_internal::FastIntToBuffer(static_cast<uint32_t>(i),
                                               buffer);
    }
  }
}

// Given a 128-bit number expressed as a pair of uint64_t, high half first,
// return that number multiplied by the given 32-bit value.  If the result is
// too large to fit in a 128-bit number, divide it by 2 until it fits.
inline std::pair<uint64_t, uint64_t> Mul32(std::pair<uint64_t, uint64_t> num,
                                           uint32_t mul) {
  uint64_t bits0_31 = num.second & 0xFFFFFFFF;
  uint64_t bits32_63 = num.second >> 32;
  uint64_t bits64_95 = num.first & 0xFFFFFFFF;
  uint64_t bits96_127 = num.first >> 32;

  // The picture so far: each of these 64-bit values has only the lower 32 bits
  // filled in.
  // bits96_127:          [ 00000000 xxxxxxxx ]
  // bits64_95:                    [ 00000000 xxxxxxxx ]
  // bits32_63:                             [ 00000000 xxxxxxxx ]
  // bits0_31:                                       [ 00000000 xxxxxxxx ]

  bits0_31 *= mul;
  bits32_63 *= mul;
  bits64_95 *= mul;
  bits96_127 *= mul;

  // Now the top halves may also have value, though all 64 of their bits will
  // never be set at the same time, since they are a result of a 32x32 bit
  // multiply.  This makes the carry calculation slightly easier.
  // bits96_127:          [ mmmmmmmm | mmmmmmmm ]
  // bits64_95:                    [ | mmmmmmmm mmmmmmmm | ]
  // bits32_63:                      |        [ mmmmmmmm | mmmmmmmm ]
  // bits0_31:                       |                 [ | mmmmmmmm mmmmmmmm ]
  // eventually:        [ bits128_up | ...bits64_127.... | ..bits0_63... ]

  uint64_t bits0_63 = bits0_31 + (bits32_63 << 32);
  uint64_t bits64_127 = bits64_95 + (bits96_127 << 32) + (bits32_63 >> 32) +
                        (bits0_63 < bits0_31);
  uint64_t bits128_up = (bits96_127 >> 32) + (bits64_127 < bits64_95);
  if (bits128_up == 0)
    return {bits64_127, bits0_63};

  int shift = 64 - CountLeadingZeros64(bits128_up);
  uint64_t lo = (bits0_63 >> shift) + (bits64_127 << (64 - shift));
  uint64_t hi = (bits64_127 >> shift) + (bits128_up << (64 - shift));
  return {hi, lo};
}
// Compute num * 5 ^ expfive, and return the first 128 bits of the result,
// where the first bit is always a one.  So PowFive(1, 0) starts 0b100000,
// PowFive(1, 1) starts 0b101000, PowFive(1, 2) starts 0b110010, etc.
inline std::pair<uint64_t, uint64_t> PowFive(uint64_t num, int expfive) {
  std::pair<uint64_t, uint64_t> result = {num, 0};
  while (expfive >= 13) {
    // 5^13 is the highest power of five that will fit in a 32-bit integer.
    result = Mul32(result, 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5);
    expfive -= 13;
  }
  constexpr int powers_of_five[13] = {1,
                                      5,
                                      5 * 5,
                                      5 * 5 * 5,
                                      5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
                                      5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 *
                                          5 * 5};
  result = Mul32(result, powers_of_five[expfive & 15]);
  int shift = CountLeadingZeros64(result.first);
  if (shift != 0) {
    result.first = (result.first << shift) + (result.second >> (64 - shift));
    result.second = (result.second << shift);
  }
  return result;
}

struct ExpDigits {
  int32_t exponent;
  wchar_t digits[6];
};

// SplitToSix converts value, a positive double-precision floating-point number,
// into a base-10 exponent and 6 ASCII digits, where the first digit is never
// zero.  For example, SplitToSix(1) returns an exponent of zero and a digits
// array of {'1', '0', '0', '0', '0', '0'}.  If value is exactly halfway between
// two possible representations, e.g. value = 100000.5, then "round to even" is
// performed.
inline ExpDigits SplitToSix(const double value) {
  ExpDigits exp_dig;
  int exp = 5;
  double d = value;
  // First step: calculate a close approximation of the output, where the
  // value d will be between 100,000 and 999,999, representing the digits
  // in the output ASCII array, and exp is the base-10 exponent.  It would be
  // faster to use a table here, and to look up the base-2 exponent of value,
  // however value is an IEEE-754 64-bit number, so the table would have 2,000
  // entries, which is not cache-friendly.
  if (d >= 999999.5) {
    if (d >= 1e+261)
      exp += 256, d *= 1e-256;
    if (d >= 1e+133)
      exp += 128, d *= 1e-128;
    if (d >= 1e+69)
      exp += 64, d *= 1e-64;
    if (d >= 1e+37)
      exp += 32, d *= 1e-32;
    if (d >= 1e+21)
      exp += 16, d *= 1e-16;
    if (d >= 1e+13)
      exp += 8, d *= 1e-8;
    if (d >= 1e+9)
      exp += 4, d *= 1e-4;
    if (d >= 1e+7)
      exp += 2, d *= 1e-2;
    if (d >= 1e+6)
      exp += 1, d *= 1e-1;
  } else {
    if (d < 1e-250)
      exp -= 256, d *= 1e256;
    if (d < 1e-122)
      exp -= 128, d *= 1e128;
    if (d < 1e-58)
      exp -= 64, d *= 1e64;
    if (d < 1e-26)
      exp -= 32, d *= 1e32;
    if (d < 1e-10)
      exp -= 16, d *= 1e16;
    if (d < 1e-2)
      exp -= 8, d *= 1e8;
    if (d < 1e+2)
      exp -= 4, d *= 1e4;
    if (d < 1e+4)
      exp -= 2, d *= 1e2;
    if (d < 1e+5)
      exp -= 1, d *= 1e1;
  }
  // At this point, d is in the range [99999.5..999999.5) and exp is in the
  // range [-324..308]. Since we need to round d up, we want to add a half
  // and truncate.
  // However, the technique above may have lost some precision, due to its
  // repeated multiplication by constants that each may be off by half a bit
  // of precision.  This only matters if we're close to the edge though.
  // Since we'd like to know if the fractional part of d is close to a half,
  // we multiply it by 65536 and see if the fractional part is close to 32768.
  // (The number doesn't have to be a power of two,but powers of two are faster)
  uint64_t d64k = d * 65536;
  int dddddd; // A 6-digit decimal integer.
  if ((d64k % 65536) == 32767 || (d64k % 65536) == 32768) {
    // OK, it's fairly likely that precision was lost above, which is
    // not a surprise given only 52 mantissa bits are available.  Therefore
    // redo the calculation using 128-bit numbers.  (64 bits are not enough).

    // Start out with digits rounded down; maybe add one below.
    dddddd = static_cast<int>(d64k / 65536);

    // mantissa is a 64-bit integer representing M.mmm... * 2^63.  The actual
    // value we're representing, of course, is M.mmm... * 2^exp2.
    int exp2;
    double m = std::frexp(value, &exp2);
    uint64_t mantissa = m * (32768.0 * 65536.0 * 65536.0 * 65536.0);
    // std::frexp returns an m value in the range [0.5, 1.0), however we
    // can't multiply it by 2^64 and convert to an integer because some FPUs
    // throw an exception when converting an number higher than 2^63 into an
    // integer - even an unsigned 64-bit integer!  Fortunately it doesn't matter
    // since m only has 52 significant bits anyway.
    mantissa <<= 1;
    exp2 -= 64; // not needed, but nice for debugging

    // OK, we are here to compare:
    //     (dddddd + 0.5) * 10^(exp-5)  vs.  mantissa * 2^exp2
    // so we can round up dddddd if appropriate.  Those values span the full
    // range of 600 orders of magnitude of IEE 64-bit floating-point.
    // Fortunately, we already know they are very close, so we don't need to
    // track the base-2 exponent of both sides.  This greatly simplifies the
    // the math since the 2^exp2 calculation is unnecessary and the power-of-10
    // calculation can become a power-of-5 instead.

    std::pair<uint64_t, uint64_t> edge, val;
    if (exp >= 6) {
      // Compare (dddddd + 0.5) * 5 ^ (exp - 5) to mantissa
      // Since we're tossing powers of two, 2 * dddddd + 1 is the
      // same as dddddd + 0.5
      edge = PowFive(2 * dddddd + 1, exp - 5);

      val.first = mantissa;
      val.second = 0;
    } else {
      // We can't compare (dddddd + 0.5) * 5 ^ (exp - 5) to mantissa as we did
      // above because (exp - 5) is negative.  So we compare (dddddd + 0.5) to
      // mantissa * 5 ^ (5 - exp)
      edge = PowFive(2 * dddddd + 1, 0);

      val = PowFive(mantissa, 5 - exp);
    }
    // printf("exp=%d %016lx %016lx vs %016lx %016lx\n", exp, val.first,
    //        val.second, edge.first, edge.second);
    if (val > edge) {
      dddddd++;
    } else if (val == edge) {
      dddddd += (dddddd & 1);
    }
  } else {
    // Here, we are not close to the edge.
    dddddd = static_cast<int>((d64k + 32768) / 65536);
  }
  if (dddddd == 1000000) {
    dddddd = 100000;
    exp += 1;
  }
  exp_dig.exponent = exp;

  int two_digits = dddddd / 10000;
  dddddd -= two_digits * 10000;
  PutTwoDigits(two_digits, &exp_dig.digits[0]);

  two_digits = dddddd / 100;
  dddddd -= two_digits * 100;
  PutTwoDigits(two_digits, &exp_dig.digits[2]);

  PutTwoDigits(dddddd, &exp_dig.digits[4]);
  return exp_dig;
}
// Helper function for fast formatting of floating-point.
// The result is the same as "%g", a.k.a. "%.6g".
inline size_t SixDigitsToBuffer(double d, wchar_t *const buffer) {
  static_assert(std::numeric_limits<float>::is_iec559,
                "IEEE-754/IEC-559 support only");

  wchar_t *out = buffer; // we write data to out, incrementing as we go, but
                         // FloatToBuffer always returns the address of the
                         // buffer passed in.

  if (std::isnan(d)) {
    wcscpy(out, L"nan"); // NOLINT(runtime/printf)
    return 3;
  }
  if (d == 0) { // +0 and -0 are handled here
    if (std::signbit(d))
      *out++ = '-';
    *out++ = '0';
    *out = 0;
    return out - buffer;
  }
  if (d < 0) {
    *out++ = '-';
    d = -d;
  }
  if (std::isinf(d)) {
    wcscpy(out, L"inf"); // NOLINT(runtime/printf)
    return out + 3 - buffer;
  }

  auto exp_dig = SplitToSix(d);
  int exp = exp_dig.exponent;
  const char *digits = exp_dig.digits;
  out[0] = '0';
  out[1] = '.';
  switch (exp) {
  case 5:
    wmemcpy(out, &digits[0], 6), out += 6;
    *out = 0;
    return out - buffer;
  case 4:
    wmemcpy(out, &digits[0], 5), out += 5;
    if (digits[5] != '0') {
      *out++ = '.';
      *out++ = digits[5];
    }
    *out = 0;
    return out - buffer;
  case 3:
    wmemcpy(out, &digits[0], 4), out += 4;
    if ((digits[5] | digits[4]) != '0') {
      *out++ = '.';
      *out++ = digits[4];
      if (digits[5] != '0')
        *out++ = digits[5];
    }
    *out = 0;
    return out - buffer;
  case 2:
    wmemcpy(out, &digits[0], 3), out += 3;
    *out++ = '.';
    wmemcpy(out, &digits[3], 3);
    out += 3;
    while (out[-1] == '0')
      --out;
    if (out[-1] == '.')
      --out;
    *out = 0;
    return out - buffer;
  case 1:
    wmemcpy(out, &digits[0], 2), out += 2;
    *out++ = '.';
    wmemcpy(out, &digits[2], 4);
    out += 4;
    while (out[-1] == '0')
      --out;
    if (out[-1] == '.')
      --out;
    *out = 0;
    return out - buffer;
  case 0:
    wmemcpy(out, &digits[0], 1), out += 1;
    *out++ = '.';
    wmemcpy(out, &digits[1], 5);
    out += 5;
    while (out[-1] == '0')
      --out;
    if (out[-1] == '.')
      --out;
    *out = 0;
    return out - buffer;
  case -4:
    out[2] = '0';
    ++out;
  case -3:
    out[2] = '0';
    ++out;
  case -2:
    out[2] = '0';
    ++out;
  case -1:
    out += 2;
    wmemcpy(out, &digits[0], 6);
    out += 6;
    while (out[-1] == '0')
      --out;
    *out = 0;
    return out - buffer;
  }
  out[0] = digits[0];
  out += 2;
  wmemcpy(out, &digits[1], 5), out += 5;
  while (out[-1] == '0')
    --out;
  if (out[-1] == '.')
    --out;
  *out++ = 'e';
  if (exp > 0) {
    *out++ = '+';
  } else {
    *out++ = '-';
    exp = -exp;
  }
  if (exp > 99) {
    int dig1 = exp / 100;
    exp -= dig1 * 100;
    *out++ = '0' + dig1;
  }
  PutTwoDigits(exp, out);
  out += 2;
  *out = 0;
  return out - buffer;
}
} // namespace numbers_internal

namespace strings_internal {
// AlphaNumBuffer allows a way to pass a string to StrCat without having to do
// memory allocation.  It is simply a pair of a fixed-size character array, and
// a size.  Please don't use outside of absl, yet.
template <size_t max_size> struct AlphaNumBuffer {
  std::array<wchar_t, max_size> data;
  size_t size;
};
} // namespace strings_internal

// Enum that specifies the number of significant digits to return in a `Hex` or
// `Dec` conversion and fill character to use. A `kZeroPad2` value, for example,
// would produce hexadecimal strings such as "0a","0f" and a 'kSpacePad5' value
// would produce hexadecimal strings such as "    a","    f".
enum PadSpec : uint8_t {
  kNoPad = 1,
  kZeroPad2,
  kZeroPad3,
  kZeroPad4,
  kZeroPad5,
  kZeroPad6,
  kZeroPad7,
  kZeroPad8,
  kZeroPad9,
  kZeroPad10,
  kZeroPad11,
  kZeroPad12,
  kZeroPad13,
  kZeroPad14,
  kZeroPad15,
  kZeroPad16,
  kZeroPad17,
  kZeroPad18,
  kZeroPad19,
  kZeroPad20,

  kSpacePad2 = kZeroPad2 + 64,
  kSpacePad3,
  kSpacePad4,
  kSpacePad5,
  kSpacePad6,
  kSpacePad7,
  kSpacePad8,
  kSpacePad9,
  kSpacePad10,
  kSpacePad11,
  kSpacePad12,
  kSpacePad13,
  kSpacePad14,
  kSpacePad15,
  kSpacePad16,
  kSpacePad17,
  kSpacePad18,
  kSpacePad19,
  kSpacePad20,
};

// -----------------------------------------------------------------------------
// Hex
// -----------------------------------------------------------------------------
//
// `Hex` stores a set of hexadecimal string conversion parameters for use
// within `AlphaNum` string conversions.
struct Hex {
  uint64_t value;
  uint8_t width;
  wchar_t fill;

  template <typename Int>
  explicit Hex(
      Int v, PadSpec spec = kNoPad,
      typename std::enable_if<sizeof(Int) == 1 &&
                              !std::is_pointer<Int>::value>::type * = nullptr)
      : Hex(spec, static_cast<uint8_t>(v)) {}
  template <typename Int>
  explicit Hex(
      Int v, PadSpec spec = kNoPad,
      typename std::enable_if<sizeof(Int) == 2 &&
                              !std::is_pointer<Int>::value>::type * = nullptr)
      : Hex(spec, static_cast<uint16_t>(v)) {}
  template <typename Int>
  explicit Hex(
      Int v, PadSpec spec = kNoPad,
      typename std::enable_if<sizeof(Int) == 4 &&
                              !std::is_pointer<Int>::value>::type * = nullptr)
      : Hex(spec, static_cast<uint32_t>(v)) {}
  template <typename Int>
  explicit Hex(
      Int v, PadSpec spec = kNoPad,
      typename std::enable_if<sizeof(Int) == 8 &&
                              !std::is_pointer<Int>::value>::type * = nullptr)
      : Hex(spec, static_cast<uint64_t>(v)) {}
  template <typename Pointee>
  explicit Hex(Pointee *v, PadSpec spec = kNoPad)
      : Hex(spec, reinterpret_cast<uintptr_t>(v)) {}

private:
  Hex(PadSpec spec, uint64_t v)
      : value(v),
        width(spec == kNoPad ? 1
                             : spec >= kSpacePad2 ? spec - kSpacePad2 + 2
                                                  : spec - kZeroPad2 + 2),
        fill(spec >= kSpacePad2 ? L' ' : L'0') {}
};

// -----------------------------------------------------------------------------
// Dec
// -----------------------------------------------------------------------------
//
// `Dec` stores a set of decimal string conversion parameters for use
// within `AlphaNum` string conversions.  Dec is slower than the default
// integer conversion, so use it only if you need padding.
struct Dec {
  uint64_t value;
  uint8_t width;
  wchar_t fill;
  bool neg;

  template <typename Int>
  explicit Dec(Int v, PadSpec spec = kNoPad,
               typename std::enable_if<(sizeof(Int) <= 8)>::type * = nullptr)
      : value(v >= 0 ? static_cast<uint64_t>(v)
                     : uint64_t{0} - static_cast<uint64_t>(v)),
        width(spec == kNoPad ? 1
                             : spec >= kSpacePad2 ? spec - kSpacePad2 + 2
                                                  : spec - kZeroPad2 + 2),
        fill(spec >= kSpacePad2 ? L' ' : L'0'), neg(v < 0) {}
};

constexpr size_t kFastToBufferSize = 32;

class AlphaNum {
public:
  AlphaNum(bool v) : piece_(v ? L"true" : L"false") {} // TRUE FALSE
  AlphaNum(short x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(unsigned short x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(int x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(unsigned int x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(long x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}

  AlphaNum(unsigned long x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(long long x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}
  AlphaNum(unsigned long long x)
      : piece_(digits_,
               numbers_internal::FastIntToBuffer(x, digits_) - &digits_[0]) {}

  AlphaNum(float f) // NOLINT(runtime/explicit)
      : piece_(digits_, numbers_internal::SixDigitsToBuffer(f, digits_)) {}
  AlphaNum(double f) // NOLINT(runtime/explicit)
      : piece_(digits_, numbers_internal::SixDigitsToBuffer(f, digits_)) {}
  // NOLINT(runtime/explicit)
  AlphaNum(Hex hex) {
    wchar_t *const end = &digits_[kFastToBufferSize];
    wchar_t *writer = end;
    uint64_t value = hex.value;
    static const wchar_t hexdigits[] = L"0123456789abcdef";
    do {
      *--writer = hexdigits[value & 0xF];
      value >>= 4;
    } while (value != 0);

    wchar_t *beg;
    if (end - writer < hex.width) {
      beg = end - hex.width;
      std::fill_n(beg, writer - beg, hex.fill);
    } else {
      beg = writer;
    }

    piece_ = std::wstring_view(beg, end - beg);
  }
  AlphaNum(Dec dec) {
    wchar_t *const end = &digits_[kFastToBufferSize];
    wchar_t *const minfill = end - dec.width;
    wchar_t *writer = end;
    uint64_t value = dec.value;
    bool neg = dec.neg;
    while (value > 9) {
      *--writer = static_cast<wchar_t>(L'0' + (value % 10));
      value /= 10;
    }
    *--writer = static_cast<wchar_t>(L'0' + value);
    if (neg)
      *--writer = '-';

    ptrdiff_t fillers = writer - minfill;
    if (fillers > 0) {
      // Tricky: if the fill character is ' ', then it's <fill><+/-><digits>
      // But...: if the fill character is '0', then it's <+/-><fill><digits>
      bool add_sign_again = false;
      if (neg && dec.fill == L'0') { // If filling with '0',
        ++writer;                    // ignore the sign we just added
        add_sign_again = true;       // and re-add the sign later.
      }
      writer -= fillers;
      std::fill_n(writer, fillers, dec.fill);
      if (add_sign_again)
        *--writer = L'-';
    }

    piece_ = std::wstring_view(writer, end - writer);
  }

  template <size_t size>
  AlphaNum( // NOLINT(runtime/explicit)
      const strings_internal::AlphaNumBuffer<size> &buf)
      : piece_(&buf.data[0], buf.size) {}

  AlphaNum(const wchar_t *cstr) : piece_(cstr) {}
  AlphaNum(std::wstring_view sv) : piece_(sv) {}
  template <typename Allocator>
  AlphaNum( // NOLINT(runtime/explicit)
      const std::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>
          &str)
      : piece_(str) {}
  AlphaNum(wchar_t c) = delete;
  AlphaNum(const AlphaNum &) = delete;
  AlphaNum &operator=(const AlphaNum &) = delete;
  std::wstring_view::size_type size() const { return piece_.size(); }
  const wchar_t *data() const { return piece_.data(); }
  std::wstring_view Piece() const { return piece_; }

private:
  std::wstring_view piece_;
  wchar_t digits_[kFastToBufferSize];
};

namespace strings_internal {

inline void AppendPieces(std::wstring *result,
                         std::initializer_list<std::wstring_view> pieces) {
  auto old_size = result->size();
  size_t total_size = old_size;
  for (const std::wstring_view piece : pieces) {
    total_size += piece.size();
  }
  result->resize(total_size);
  wchar_t *const begin = &*result->begin() + old_size;
  wchar_t *out = begin;
  for (const std::wstring_view piece : pieces) {
    const size_t this_size = piece.size();
    wmemcpy(out, piece.data(), this_size);
    out += this_size;
  }
}

inline std::wstring CatPieces(std::initializer_list<std::wstring_view> pieces) {
  std::wstring result;
  size_t total_size = 0;
  for (const std::wstring_view piece : pieces) {
    total_size += piece.size();
  }
  result.resize(total_size);

  wchar_t *const begin = &*result.begin();
  wchar_t *out = begin;
  for (const std::wstring_view piece : pieces) {
    const size_t this_size = piece.size();
    wmemcpy(out, piece.data(), this_size);
    out += this_size;
  }
  return result;
}

static inline wchar_t *Append(wchar_t *out, const AlphaNum &x) {
  // memcpy is allowed to overwrite arbitrary memory, so doing this after the
  // call would force an extra fetch of x.size().
  wchar_t *after = out + x.size();
  wmemcpy(out, x.data(), x.size());
  return after;
}

} // namespace strings_internal
inline std::wstring StringCat() { return std::wstring(); }

inline std::wstring StringCat(const AlphaNum &a) {
  return std::wstring(a.Piece());
}

inline std::wstring StringCat(const AlphaNum &a, const AlphaNum &b) {
  std::wstring result;
  result.resize(a.size() + b.size());
  wchar_t *const begin = &*result.begin();
  wchar_t *out = begin;
  out = strings_internal::Append(out, a);
  out = strings_internal::Append(out, b);
  return result;
}
inline std::wstring StringCat(const AlphaNum &a, const AlphaNum &b,
                              const AlphaNum &c) {
  std::wstring result;
  result.resize(a.size() + b.size() + c.size());
  wchar_t *const begin = &*result.begin();
  wchar_t *out = begin;
  out = strings_internal::Append(out, a);
  out = strings_internal::Append(out, b);
  out = strings_internal::Append(out, c);
  return result;
}
inline std::wstring StringCat(const AlphaNum &a, const AlphaNum &b,
                              const AlphaNum &c, const AlphaNum &d) {
  std::wstring result;
  result.resize(a.size() + b.size() + c.size() + d.size());
  wchar_t *const begin = &*result.begin();
  wchar_t *out = begin;
  out = strings_internal::Append(out, a);
  out = strings_internal::Append(out, b);
  out = strings_internal::Append(out, c);
  out = strings_internal::Append(out, d);
  return result;
}

// Support 5 or more arguments
template <typename... AV>
inline std::wstring StringCat(const AlphaNum &a, const AlphaNum &b,
                              const AlphaNum &c, const AlphaNum &d,
                              const AlphaNum &e, const AV &... args) {
  return strings_internal::CatPieces(
      {a.Piece(), b.Piece(), c.Piece(), d.Piece(), e.Piece(),
       static_cast<const AlphaNum &>(args).Piece()...});
}

inline void StrAppend(std::wstring *dest, const AlphaNum &a) {
  dest->append(a.data(), a.size());
}

inline void StrAppend(std::wstring *dest, const AlphaNum &a,
                      const AlphaNum &b) {
  auto old_size = dest->size();
  dest->resize(old_size + a.size() + b.size());
  wchar_t *const begin = &*dest->begin();
  wchar_t *out = begin + old_size;
  out = strings_internal::Append(out, a);
  out = strings_internal::Append(out, b);
}

inline void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b,
                      const AlphaNum &c) {
  auto old_size = dest->size();
  dest->resize(old_size + a.size() + b.size() + c.size());
  wchar_t *const begin = &*dest->begin();
  wchar_t *out = begin + old_size;
  out = strings_internal::Append(out, a);
  out = strings_internal::Append(out, b);
  out = strings_internal::Append(out, c);
}

inline void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b,
                      const AlphaNum &c, const AlphaNum &d) {
  auto old_size = dest->size();
  dest->resize(old_size + a.size() + b.size() + c.size() + d.size());
  wchar_t *const begin = &*dest->begin();
  wchar_t *out = begin + old_size;
  out = strings_internal::Append(out, a);
  out = strings_internal::Append(out, b);
  out = strings_internal::Append(out, c);
  out = strings_internal::Append(out, d);
}

template <typename... AV>
inline void StrAppend(std::wstring *dest, const AlphaNum &a, const AlphaNum &b,
                      const AlphaNum &c, const AlphaNum &d, const AlphaNum &e,
                      const AV &... args) {
  strings_internal::AppendPieces(
      dest, {a.Piece(), b.Piece(), c.Piece(), d.Piece(), e.Piece(),
             static_cast<const AlphaNum &>(args).Piece()...});
}

} // namespace base

#endif
