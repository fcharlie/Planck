/*===--- ConvertUTF.c - Universal Character Names conversions ---------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 *===------------------------------------------------------------------------=*/
/*
 * Copyright 2001-2004 Unicode, Inc.
 *
 * Disclaimer
 *
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 *
 * Limitations on Rights to Redistribute This Code
 *
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------
    Conversions between UTF32, UTF-16, and UTF-8. Source code file.
    Author: Mark E. Davis, 1994.
    Rev History: Rick McGowan, fixes & updates May 2001.
    Sept 2001: fixed const & error conditions per
        mods suggested by S. Parent & A. Lillich.
    June 2002: Tim Dodd added detection and handling of incomplete
        source sequences, enhanced error detection, added casts
        to eliminate compiler warnings.
    July 2003: slight mods to back out aggressive FFFE detection.
    Jan 2004: updated switches in from-UTF8 conversions.
    Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.
    See the header file "ConvertUTF.h" for complete documentation.
------------------------------------------------------------------------ */
#include "inquisitive_fwd.hpp"

namespace inquisitive {

using utf8_t = uint8_t;
using utf16_t = uint16_t;
using utf32_t = uint32_t;
static bool isLegalUTF8(const utf8_t *source, int length) {
  utf8_t a;
  const utf8_t *srcptr = source + length;
  switch (length) {
  default:
    return false;
    /* Everything else falls through when "true"... */
  case 4:
    if ((a = (*--srcptr)) < 0x80 || a > 0xBF)
      return false;
  case 3:
    if ((a = (*--srcptr)) < 0x80 || a > 0xBF)
      return false;
  case 2:
    if ((a = (*--srcptr)) < 0x80 || a > 0xBF)
      return false;

    switch (*source) {
    /* no fall-through in this inner switch */
    case 0xE0:
      if (a < 0xA0)
        return false;
      break;
    case 0xED:
      if (a > 0x9F)
        return false;
      break;
    case 0xF0:
      if (a < 0x90)
        return false;
      break;
    case 0xF4:
      if (a > 0x8F)
        return false;
      break;
    default:
      if (a < 0x80)
        return false;
    }

  case 1:
    if (*source >= 0x80 && *source < 0xC2)
      return false;
  }
  if (*source > 0xF4)
    return false;
  return true;
}

typedef enum {
  conversionOK,    /* conversion successful */
  sourceExhausted, /* partial character in source, but hit end */
  targetExhausted, /* insuff. room in target for conversion */
  sourceIllegal    /* source sequence is illegal/malformed */
} ConversionResult;

#define UNI_REPLACEMENT_CHAR (utf32_t)0x0000FFFD
#define UNI_MAX_BMP (utf32_t)0x0000FFFF
#define UNI_MAX_UTF16 (utf32_t)0x0010FFFF
#define UNI_MAX_UTF32 (utf32_t)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (utf32_t)0x0010FFFF

#define UNI_MAX_UTF8_BYTES_PER_CODE_POINT 4

#define UNI_UTF16_BYTE_ORDER_MARK_NATIVE 0xFEFF
#define UNI_UTF16_BYTE_ORDER_MARK_SWAPPED 0xFFFE

constexpr const int halfShift = 10; /* used for shifting by 10 bits */

constexpr const utf32_t halfBase = 0x0010000UL;
constexpr const utf32_t halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START (utf32_t)0xD800
#define UNI_SUR_HIGH_END (utf32_t)0xDBFF
#define UNI_SUR_LOW_START (utf32_t)0xDC00
#define UNI_SUR_LOW_END (utf32_t)0xDFFF

static constexpr const char trailingBytesForUTF8[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5};

static const utf32_t offsetsFromUTF8[6] = {0x00000000UL, 0x00003080UL,
                                           0x000E2080UL, 0x03C82080UL,
                                           0xFA082080UL, 0x82082080UL};

std::wstring fromutf8(std::string_view text) {
  std::wstring w;
  w.reserve(text.size());
  auto it = reinterpret_cast<const utf8_t *>(text.data());
  auto end = reinterpret_cast<const utf8_t *>(text.data() + text.size());
  while (it < end) {
    uint32_t ch = 0;
    uint8_t extraBytesToRead = trailingBytesForUTF8[*it];
    if (extraBytesToRead >= end - it) {
      break;
    }
    if (!isLegalUTF8(it, extraBytesToRead + 1)) {
      break;
    }
    switch (extraBytesToRead) {
    case 5:
      ch += *it++;
      ch <<= 6; /* remember, illegal UTF-8 */
    case 4:
      ch += *it++;
      ch <<= 6; /* remember, illegal UTF-8 */
    case 3:
      ch += *it++;
      ch <<= 6;
    case 2:
      ch += *it++;
      ch <<= 6;
    case 1:
      ch += *it++;
      ch <<= 6;
    case 0:
      ch += *it++;
    }
    ch -= offsetsFromUTF8[extraBytesToRead];
    if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
      /* UTF-16 surrogate values are illegal in UTF-32 */
      if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
        w.push_back(UNI_REPLACEMENT_CHAR);
      } else {
        w.push_back(static_cast<wchar_t>((utf16_t)ch));
      }
    } else if (ch > UNI_MAX_UTF16) {
      w.push_back(UNI_REPLACEMENT_CHAR);
    } else {
      /* target is a character in range 0xFFFF - 0x10FFFF. */
      ch -= halfBase;
      auto wc1 = (utf16_t)((ch >> halfShift) + UNI_SUR_HIGH_START);
      w.push_back(static_cast<wchar_t>(wc1));
      auto wc2 = (utf16_t)((ch & halfMask) + UNI_SUR_LOW_START);
      w.push_back(static_cast<wchar_t>(wc2));
    }
  }
  return w;
}

} // namespace inquisitive
