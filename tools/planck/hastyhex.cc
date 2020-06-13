///////////// DUMP HEX
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <algorithm>
#include <bela/base.hpp>
#include <bela/terminal.hpp>

static const char hex[] = "0123456789abcdef";

static int color(int b) {
#define CN 0x37 /* null    */
#define CS 0x92 /* space   */
#define CP 0x96 /* print   */
#define CC 0x95 /* control */
#define CH 0x93 /* high    */
  static const unsigned char table[] = {
      CN, CC, CC, CC, CC, CC, CC, CC, CC, CC, CS, CS, CS, CS, CC, CC, CC, CC, CC, CC, CC, CC,
      CC, CC, CC, CC, CC, CC, CC, CC, CC, CC, CS, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CC, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH};
  return table[b];
}

static int display(int b) {
  static const char table[] = {
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
      0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
      0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
      0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
      0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
      0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
      0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e,
  };
  return table[b];
}

class BinaryFile {
public:
  using ssize_type = SSIZE_T;
  BinaryFile() = default;
  BinaryFile(const BinaryFile &) = delete;
  BinaryFile &operator=(const BinaryFile &) = delete;
  ~BinaryFile() {
    if (hFile != INVALID_HANDLE_VALUE) {
      CloseHandle(hFile);
    }
  }
  bool Open(std::wstring_view sv) {
    hFile = CreateFileW(sv.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    return (hFile == INVALID_HANDLE_VALUE);
  }
  ssize_type BinaryRead(uint8_t *buf, size_t bufsize) {
    DWORD dwRead = 0;
    if (ReadFile(hFile, buf, (DWORD)bufsize, &dwRead, nullptr) == TRUE) {
      return static_cast<ssize_type>(dwRead);
    }
    return -1;
  }

private:
  HANDLE hFile{INVALID_HANDLE_VALUE};
};

bool Processcolor(std::wstring_view sv, FILE *out, int64_t len) {
  BinaryFile bin;
  if (!bin.Open(sv)) {
    auto ec = bela::make_system_error_code();
    bela::FPrintF(stderr, L"planck: open binary %s\n", ec.message);
    return false;
  }
  uint64_t maxlen = len > 0 ? len : UINT64_MAX;

  size_t i, n;
  unsigned long offset = 0;
  unsigned char input[16] = {0};
  char cmt[] = "00000000  "
               "\33[XXm## \33[XXm## \33[XXm## \33[XXm## "
               "\33[XXm## \33[XXm## \33[XXm## \33[XXm##  "
               "\33[XXm## \33[XXm## \33[XXm## \33[XXm## "
               "\33[XXm## \33[XXm## \33[XXm## \33[XXm##  "
               "\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm."
               "\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm."
               "\33[0m\n";
  static const int slots[] = {/* ANSI-color, hex, ANSI-color, ASCII */
                              12,  15,  142, 145, 20,  23,  148, 151, 28,  31,  154, 157, 36,
                              39,  160, 163, 44,  47,  166, 169, 52,  55,  172, 175, 60,  63,
                              178, 181, 68,  71,  184, 187, 77,  80,  190, 193, 85,  88,  196,
                              199, 93,  96,  202, 205, 101, 104, 208, 211, 109, 112, 214, 217,
                              117, 120, 220, 223, 125, 128, 226, 229, 133, 136, 232, 235};
  constexpr const uint64_t linelen = sizeof(input);
  do {
    n = bin.BinaryRead(input, (std::min)(linelen, maxlen));
    if (n <= 0) {
      break;
    }
    maxlen -= n;
    /* Write the offset */
    for (i = 0; i < 8; i++)
      cmt[i] = hex[(offset >> (28 - i * 4)) & 15];

    /* Fill out the cmt */
    for (i = 0; i < 16; i++) {
      /* Use a fixed loop count instead of "n" to encourage loop
       * unrolling by the compiler. Empty bytes will be erased
       * later.
       */
      int v = input[i];
      int c = color(v);
      cmt[slots[i * 4 + 0] + 0] = hex[c >> 4];
      cmt[slots[i * 4 + 0] + 1] = hex[c & 15];
      cmt[slots[i * 4 + 1] + 0] = hex[v >> 4];
      cmt[slots[i * 4 + 1] + 1] = hex[v & 15];
      cmt[slots[i * 4 + 2] + 0] = hex[c >> 4];
      cmt[slots[i * 4 + 2] + 1] = hex[c & 15];
      cmt[slots[i * 4 + 3] + 0] = display(v);
    }

    /* Erase any trailing bytes */
    for (i = n; i < 16; i++) {
      /* This loop is only used once: the last line of output. The
       * branch predictor will quickly learn that it's never taken.
       */
      cmt[slots[i * 4 + 0] + 0] = '0';
      cmt[slots[i * 4 + 0] + 1] = '0';
      cmt[slots[i * 4 + 1] + 0] = ' ';
      cmt[slots[i * 4 + 1] + 1] = ' ';
      cmt[slots[i * 4 + 2] + 0] = '0';
      cmt[slots[i * 4 + 2] + 1] = '0';
      cmt[slots[i * 4 + 3] + 0] = ' ';
    }

    if (!fwrite(cmt, sizeof(cmt) - 1, 1, out))
      break; /* Output error */
    offset += 16;
  } while (n == 16);

  return true;
}
