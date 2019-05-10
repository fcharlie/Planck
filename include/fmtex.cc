#include "span.hpp"
#include <climits>
#include <cstdio>
#include <cstdlib>

namespace base {
using nonstd::span;
}

struct Arg {
  Arg() = default;
};

int StrFormatInternal(wchar_t *buf, size_t buflen, const wchar_t *fmt,
                      base::span<const Arg> args) {
  auto begin = buf;
  wchar_t *p, zero;
  auto last = buf + buflen;
  int d;
  double f;
  size_t len, slen;
  int64_t i64;
  uint64_t ui64, frac;
  uint32_t width, sign, hex, max_width, frac_width, scale, n;

  while (*fmt && buf < last) {

    /*
     * "buf < last" means that we could copy at least one character:
     * the plain character, "%%", "%c", and minus without the checking
     */

    if (*fmt == '%') {

      i64 = 0;
      ui64 = 0;

      zero = (wchar_t)((*++fmt == '0') ? '0' : ' ');
      width = 0;
      sign = 1;
      hex = 0;
      max_width = 0;
      frac_width = 0;
      slen = (size_t)-1;

      while (*fmt >= '0' && *fmt <= '9') {
        width = width * 10 + (*fmt++ - '0');
      }

      for (;;) {
        switch (*fmt) {

        case 'u':
          sign = 0;
          fmt++;
          continue;

        case 'm':
          max_width = 1;
          fmt++;
          continue;

        case 'X':
          hex = 2;
          sign = 0;
          fmt++;
          continue;

        case 'x':
          hex = 1;
          sign = 0;
          fmt++;
          continue;

        case '.':
          fmt++;

          while (*fmt >= '0' && *fmt <= '9') {
            frac_width = frac_width * 10 + (*fmt++ - '0');
          }

          break;

        default:
          break;
        }

        break;
      }

      switch (*fmt) {

      case 's':
        // string -value

        fmt++;

        continue;

      case 'O':
        // i64 = (int64_t)va_arg(args, off_t);
        // sign = 1;
        break;

      case 'P':
        // i64 = (int64_t)va_arg(args, ngx_pid_t);
        // sign = 1;
        break;

      case 'T':
        // i64 = (int64_t)va_arg(args, time_t);
        // sign = 1;
        break;

      case 'z':
        // if (sign) {
        //   i64 = (int64_t)va_arg(args, ssize_t);
        // } else {
        //   ui64 = (uint64_t)va_arg(args, size_t);
        // }
        break;

      case 'i':
        // if (sign) {
        //   i64 = (int64_t)va_arg(args, ngx_int_t);
        // } else {
        //   ui64 = (uint64_t)va_arg(args, ngx_uint_t);
        // }

        // if (max_width) {
        //   width = NGX_INT_T_LEN;
        // }

        break;

      case 'd':
        // if (sign) {
        //   i64 = (int64_t)va_arg(args, int);
        // } else {
        //   ui64 = (uint64_t)va_arg(args, u_int);
        // }
        break;

      case 'l':
        // if (sign) {
        //   i64 = (int64_t)va_arg(args, long);
        // } else {
        //   ui64 = (uint64_t)va_arg(args, u_long);
        // }
        break;

      case 'D':
        // if (sign) {
        //   i64 = (int64_t)va_arg(args, int32_t);
        // } else {
        //   ui64 = (uint64_t)va_arg(args, uint32_t);
        // }
        break;

      case 'L':
        // if (sign) {
        //   i64 = va_arg(args, int64_t);
        // } else {
        //   ui64 = va_arg(args, uint64_t);
        // }
        break;

      case 'f':
        // f = va_arg(args, double);

        // if (f < 0) {
        //   *buf++ = '-';
        //   f = -f;
        // }

        // ui64 = (int64_t)f;
        // frac = 0;

        // if (frac_width) {

        //   scale = 1;
        //   for (n = frac_width; n; n--) {
        //     scale *= 10;
        //   }

        //   frac = (uint64_t)((f - (double)ui64) * scale + 0.5);

        //   if (frac == scale) {
        //     ui64++;
        //     frac = 0;
        //   }
        // }

        // buf = ngx_sprintf_num(buf, last, ui64, zero, 0, width);

        // if (frac_width) {
        //   if (buf < last) {
        //     *buf++ = '.';
        //   }

        //   buf = ngx_sprintf_num(buf, last, frac, '0', 0, frac_width);
        // }

        // fmt++;

        continue;

      case 'c':
        // d = va_arg(args, int);
        // *buf++ = (u_char)(d & 0xff);
        // fmt++;

        continue;

      case '%':
        *buf++ = '%';
        fmt++;

        continue;

      default:
        *buf++ = *fmt++;

        continue;
      }

      if (sign) {
        if (i64 < 0) {
          *buf++ = '-';
          ui64 = (uint64_t)-i64;

        } else {
          ui64 = (uint64_t)i64;
        }
      }

      // buf = ngx_sprintf_num(buf, last, ui64, zero, hex, width);

      fmt++;

    } else {
      *buf++ = *fmt++;
    }
  }
  return buf - begin;
}