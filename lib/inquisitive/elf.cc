/// ELF details
#include <elf.h>
#include "inquisitive.hpp"
#include "includes.hpp"

//  Executable and Linkable Format ELF
// Thanks musl libc
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://hg.icculus.org/icculus/fatelf/raw-file/tip/docs/fatelf-specification.txt

namespace inquisitive {

const wchar_t *elf_osabi(uint8_t osabi) {
  switch (osabi) {
  case ELFOSABI_SYSV:
    return L"SYSV";
  case ELFOSABI_HPUX:
    return L"HP-UX";
  case ELFOSABI_NETBSD:
    return L"NetBSD";
  case ELFOSABI_LINUX:
    return L"Linux";
  case 4: /// musl not defined
    return L"GNU Hurd";
  case ELFOSABI_SOLARIS:
    return L"Solaris";
  case ELFOSABI_AIX:
    return L"AIX";
  case ELFOSABI_IRIX:
    return L"IRIX";
  case ELFOSABI_FREEBSD:
    return L"FreeBSD";
  case ELFOSABI_TRU64:
    return L"Tru64";
  case ELFOSABI_MODESTO:
    return L"Novell Modesto";
  case ELFOSABI_OPENBSD:
    return L"OpenBSD";
  case 0x0D:
    return L"OpenVMS";
  case 0x0E:
    return L"NonStop Kernel";
  case 0x0F:
    return L"AROS";
  case 0x10:
    return L"Fenix OS";
  case 0x11:
    return L"CloudABI";
  case ELFOSABI_ARM:
    return L"ARM";
  default:
    break;
  }
  return L"UNKNOWN";
}
struct elf_kv_t {
  uint32_t index;
  const wchar_t *value;
};

const wchar_t *elf_machine(uint32_t e) {
  const elf_kv_t kv[] = {
      {EM_M32, L"M32"},
      {EM_SPARC, L"SPARC"},
      {EM_386, L"x86"},
      {EM_68K, L"Motorola 68000"},
      {EM_88K, L"Motorola 88000"},
      {EM_860, L"860"},
      {EM_MIPS, L"MIPS"},
      {EM_S370, L"IBM System/370"},
      {EM_MIPS_RS3_LE, L"MIPS RS3 LE"},
      {EM_PARISC, L"PA-RISC"},
      {EM_VPP500, L"VPP500"},
      {EM_SPARC32PLUS, L"SPARC32PLUS"},
      {EM_960, L"960"},
      {EM_PPC, L"PPC"},
      {EM_PPC64, L"PPC64"},
      {EM_S390, L"IBM System/390"},
      {EM_V800, L"V800"},
      {EM_FR20, L"FR20"},
      {EM_RH32, L"RH32"},
      {EM_RCE, L"RCE"},
      {EM_ARM, L"ARM"},
      {EM_FAKE_ALPHA, L"EM_FAKE_ALPHA"},
      {EM_SH, L"SuperH"},
      {EM_SPARCV9, L"SPARC V9"},
      {EM_TRICORE, L"TRICORE"},
      {EM_ARC, L"ARC"},
      {EM_H8_300, L"H8 300"},
      {EM_H8_300H, L"H8 300H"},
      {EM_H8S, L"EM_H8S"},
      {EM_H8_500, L"EM_H8_500"},
      {EM_IA_64, L"IA64"},
      {EM_MIPS_X, L"MIPS X"},
      {EM_COLDFIRE, L"ColdFire"},
      {EM_68HC12, L"68HC12"},
      {EM_MMA, L"MMA"},
      {EM_PCP, L"PCP"},
      {EM_NCPU, L"NCPU"},
      {EM_NDR1, L"NDR1"},
      {EM_STARCORE, L"STARCORE"},
      {EM_ME16, L"ME16"},
      {EM_ST100, L"ST100"},
      {EM_TINYJ, L"TINYJ"},
      {EM_X86_64, L"x86-64"},
      {EM_PDSP, L"PDSP"},
      {EM_FX66, L"FX66"},
      {EM_ST9PLUS, L"ST9Plus"},
      {EM_ST7, L"ST7"},
      {EM_68HC16, L"68HC16"},
      {EM_68HC11, L"68HC11"},
      {EM_68HC08, L"68HC08"},
      {EM_68HC05, L"68HC05"},
      {EM_SVX, L"SVX"},
      {EM_ST19, L"ST19"},
      {EM_VAX, L"VAX"},
      {EM_CRIS, L"CRIS"},
      {EM_JAVELIN, L"Javelin"},
      {EM_FIREPATH, L"Firepath"},
      {EM_ZSP, L"ZSP"},
      {EM_MMIX, L"MMIX"},
      {EM_HUANY, L"Huany"},
      {EM_PRISM, L"Prism"},
      {EM_AVR, L"AVR"},
      {EM_FR30, L"FR30"},
      {EM_D10V, L"D10V"},
      {EM_D30V, L"D30V"},
      {EM_V850, L"V850"},
      {EM_M32R, L"M32R"},
      {EM_MN10300, L"MN10300"},
      {EM_MN10200, L"MN10200"},
      {EM_PJ, L"PJ"},
      {EM_OR1K, L"OR1K"},
      {EM_OPENRISC, L"OpenRISC"},
      {EM_ARC_A5, L"ARC A5"},
      {EM_ARC_COMPACT, L"ARC Compact"},
      {EM_XTENSA, L"Xtensa"},
      {EM_VIDEOCORE, L"Videocore"},
      {EM_TMM_GPP, L"TMM GPP"},
      {EM_NS32K, L"NS32K"},
      {EM_TPC, L"TPC"},
      {EM_SNP1K, L"SNP1K"},
      {EM_ST200, L"ST200"},
      {EM_IP2K, L"IP2K"},
      {EM_MAX, L"MAX"},
      {EM_CR, L"CR"},
      {EM_F2MC16, L"F2MC16"},
      {EM_MSP430, L"MSP430"},
      {EM_BLACKFIN, L"Blackfin"},
      {EM_SE_C33, L"SE C33"},
      {EM_SEP, L"SEP"},
      {EM_ARCA, L"ARCA"},
      {EM_UNICORE, L"Unicore"},
      {EM_EXCESS, L"Excess"},
      {EM_DXP, L"DXP"},
      {EM_ALTERA_NIOS2, L"Altera NIOS2"},
      {EM_CRX, L"CRX"},
      {EM_XGATE, L"XGate"},
      {EM_C166, L"C166"},
      {EM_M16C, L"M16C"},
      {EM_DSPIC30F, L"DSPIC30F"},
      {EM_CE, L"CE"},
      {EM_M32C, L"M32C"},
      {EM_TSK3000, L"TSK3000"},
      {EM_RS08, L"RS08"},
      {EM_SHARC, L"SHARC"},
      {EM_ECOG2, L"ECOG2"},
      {EM_SCORE7, L"SCORE7"},
      {EM_DSP24, L"DSP24"},
      {EM_VIDEOCORE3, L"Videocore3"},
      {EM_LATTICEMICO32, L"Latticemico32"},
      {EM_SE_C17, L"SE C17"},
      {EM_TI_C6000, L"TI C6000"},
      {EM_TI_C2000, L"TI C2000"},
      {EM_TI_C5500, L"TI C5500"},
      {EM_TI_ARP32, L"TI ARP32"},
      {EM_TI_PRU, L"TI PRU"},
      {EM_MMDSP_PLUS, L"MMDSP Plus"},
      {EM_CYPRESS_M8C, L"CYRPESS M8C"},
      {EM_R32C, L"R32C"},
      {EM_TRIMEDIA, L"Trimedia"},
      {EM_QDSP6, L"QDSP6"},
      {EM_8051, L"8051"},
      {EM_STXP7X, L"STXP7X"},
      {EM_NDS32, L"NDS32"},
      {EM_ECOG1X, L"ECOG1X"},
      {EM_MAXQ30, L"MAXQ30"},
      {EM_XIMO16, L"XIMO16"},
      {EM_MANIK, L"Manik"},
      {EM_CRAYNV2, L"Craynv2"},
      {EM_RX, L"RX"},
      {EM_METAG, L"Metag"},
      {EM_MCST_ELBRUS, L"MCST ELBRUS"},
      {EM_ECOG16, L"ECOG16"},
      {EM_CR16, L"CR16"},
      {EM_ETPU, L"ETPU"},
      {EM_SLE9X, L"SLE9X"},
      {EM_L10M, L"L10M"},
      {EM_K10M, L"K10M"},
      {EM_AARCH64, L"AArch64"},
      {EM_AVR32, L"AVR32"},
      {EM_STM8, L"STM8"},
      {EM_TILE64, L"Tile64"},
      {EM_TILEPRO, L"TilePro"},
      {EM_MICROBLAZE, L"Microblaze"},
      {EM_CUDA, L"CUDA"},
      {EM_TILEGX, L"Tilegx"},
      {EM_CLOUDSHIELD, L"CloudShield"},
      {EM_COREA_1ST, L"Corea 1st"},
      {EM_COREA_2ND, L"Corea 2nd"},
      {EM_ARC_COMPACT2, L"ARC Compact2"},
      {EM_OPEN8, L"Open8"},
      {EM_RL78, L"Rl78"},
      {EM_VIDEOCORE5, L"Videocore5"},
      {EM_78KOR, L"78KOR"},
      {EM_56800EX, L"56800EX"},
      {EM_BA1, L"BA1"},
      {EM_BA2, L"BA2"},
      {EM_XCORE, L"XCore"},
      {EM_MCHP_PIC, L"Mchp Pic"},
      {EM_KM32, L"KM32"},
      {EM_KMX32, L"KMX32"},
      {EM_EMX16, L"EMX16"},
      {EM_EMX8, L"EMX8"},
      {EM_KVARC, L"KVARC"},
      {EM_CDP, L"CDP"},
      {EM_COGE, L"Coge"},
      {EM_COOL, L"COOL"},
      {EM_NORC, L"Norc"},
      {EM_CSR_KALIMBA, L"Kalimba"},
      {EM_Z80, L"z80"},
      {EM_VISIUM, L"Visium"},
      {EM_FT32, L"FT32"},
      {EM_MOXIE, L"Moxie"},
      {EM_AMDGPU, L"AMDGPU"},
      {EM_RISCV, L"RISC-V"},
      {EM_BPF, L"BPF"},
      {EM_NUM, L"NUM"},
      {EM_ALPHA, L"EM_ALPHA"}
      ///
  };
  for (const auto &k : kv) {
    if (k.index == e) {
      return k.value;
    }
  }
  return L"No specific instruction set";
}

const wchar_t *elf_object_type(uint16_t t) {
  switch (t) {
  case ET_NONE:
    return L"No file type";
  case ET_REL:
    return L"Relocatable file ";
  case ET_EXEC:
    return L"Executable file";
  case ET_DYN:
    return L"Shared object file";
  case ET_CORE:
    return L"Core file";
  }
  return L"UNKNOWN";
}

inline endian::endian_t Endina(uint8_t t) {
  switch (t) {
  case ELFDATANONE:
    return endian::None;
  case ELFDATA2LSB:
    return endian::LittleEndian;
  case ELFDATA2MSB:
    return endian::BigEndian;
  default:
    break;
  }
  return endian::None;
}

class elf_memview {
public:
  elf_memview(const char *data__, size_t size__)
      : data_(data__), size_(size__) {
    //
  }
  const char *data() const { return data_; }
  size_t size() const { return size_; }
  template <typename T> T *cast(size_t off) {
    if (off >= size_) {
      return nullptr;
    }
    return reinterpret_cast<T *>(data_ + off);
  }
  template <typename Integer> Integer resive(Integer i) {
    if (!resiveable) {
      return i;
    }
    return swap(i);
  }
  std::string stroffset(size_t off, size_t end);
  bool inquisitive(elf_minutiae_t &em, std::error_code &ec);
  bool inquisitive64(elf_minutiae_t &em, std::error_code &ec);

private:
  const char *data_{nullptr};
  size_t size_{0};
  bool resiveable{false};
};
std::string elf_memview::stroffset(size_t off, size_t end) {
  std::string s;
  for (size_t i = off; i < end; i++) {
    if (data_[i] == 0) {
      break;
    }
    s.push_back(data_[i]);
  }
  return s;
}

//
bool elf_memview::inquisitive64(elf_minutiae_t &em, std::error_code &ec) {
  auto h = cast<Elf64_Ehdr>(0);
  if (h == nullptr) {
    return false;
  }
  em.machine = elf_machine(resive(h->e_machine));
  em.etype = elf_object_type(resive(h->e_type));
  auto off = resive(h->e_shoff);
  auto sects = cast<Elf64_Shdr>(off);
  auto shnum = resive(h->e_shnum);
  if (shnum * sizeof(Elf64_Shdr) + off > size_) {
    return false;
  }
  Elf64_Off sh_offset = 0;
  Elf64_Xword sh_entsize = 0;
  Elf64_Xword sh_size = 0;
  Elf64_Word sh_link = 0;
  for (Elf64_Word i = 0; i < shnum; i++) {
    auto st = resive(sects[i].sh_type);
    if (st == SHT_DYNAMIC) {
      sh_entsize = resive(sects[i].sh_entsize);
      sh_offset = resive(sects[i].sh_offset);
      sh_size = resive(sects[i].sh_size);
      sh_link = resive(sects[i].sh_link);
      continue;
    }
  }

  if (sh_offset == 0 || sh_entsize == 0 || sh_offset >= size_) {
    fprintf(stderr, "invalid value \n");
    return false;
  }
  auto strtab = &sects[sh_link];
  if (sh_link >= shnum) {
    return false;
  }

  Elf64_Off soff = resive(strtab->sh_offset);
  Elf64_Off send = soff + resive(strtab->sh_size);
  auto n = sh_size / sh_entsize;
  auto dyn = cast<Elf64_Dyn>(sh_offset);
  for (decltype(n) i = 0; i < n; i++) {
    auto first = resive(dyn[i].d_un.d_val);
    switch (resive(dyn[i].d_tag)) {
    case DT_NEEDED: {
      auto deps = stroffset(soff + first, send);
      em.depends.push_back(fromutf8(deps));
    } break;
    case DT_SONAME:
      em.soname = fromutf8(stroffset(soff + first, send));
      break;
    case DT_RUNPATH:
      em.rupath = fromutf8(stroffset(soff + first, send));
      break;
    case DT_RPATH:
      em.rpath = fromutf8(stroffset(soff + first, send));
      break;
    default:
      break;
    }
  }

  return true;
}

bool elf_memview::inquisitive(elf_minutiae_t &em, std::error_code &ec) {
  em.endian = Endina(static_cast<uint8_t>(data_[EI_DATA]));
  em.osabi = elf_osabi(data_[EI_OSABI]);
  em.version = data_[EI_VERSION];
  auto msb = (em.endian == endian::BigEndian);
  resiveable = (msb != IsBigEndianHost);
  if (data_[EI_CLASS] == ELFCLASS64) {
    em.bit64 = true;
    return inquisitive64(em, ec);
  }
  if (data_[EI_CLASS] != ELFCLASS32) {
    return false;
  }
  auto h = cast<Elf32_Ehdr>(0);
  em.machine = elf_machine(resive(h->e_machine));
  em.etype = elf_object_type(resive(h->e_type));
  auto off = resive(h->e_shoff);
  auto sects = cast<Elf64_Shdr>(off);
  auto shnum = resive(h->e_shnum);
  if (shnum * sizeof(Elf64_Shdr) + off > size_) {
    return false;
  }
  Elf32_Off sh_offset = 0;
  Elf32_Xword sh_entsize = 0;
  Elf32_Xword sh_size = 0;
  Elf32_Word sh_link = 0;
  for (Elf32_Word i = 0; i < shnum; i++) {
    auto st = resive(sects[i].sh_type);
    if (st == SHT_DYNAMIC) {
      sh_entsize = resive(sects[i].sh_entsize);
      sh_offset = resive(sects[i].sh_offset);
      sh_size = resive(sects[i].sh_size);
      sh_link = resive(sects[i].sh_link);
      continue;
    }
  }

  if (sh_offset == 0 || sh_entsize == 0 || sh_offset >= size_) {
    fprintf(stderr, "invalid value \n");
    return false;
  }
  auto strtab = &sects[sh_link];
  if (sh_link >= shnum) {
    return false;
  }

  Elf32_Off soff = resive(strtab->sh_offset);
  Elf32_Off send = soff + resive(strtab->sh_size);
  auto n = sh_size / sh_entsize;
  auto dyn = cast<Elf32_Dyn>(sh_offset);

  for (decltype(n) i = 0; i < n; i++) {
    auto first = resive(dyn[i].d_un.d_val);
    switch (resive(dyn[i].d_tag)) {
    case DT_NEEDED: {
      auto deps = stroffset(soff + first, send);
      em.depends.push_back(fromutf8(deps));
    } break;
    case DT_SONAME:
      em.soname = fromutf8(stroffset(soff + first, send));
      break;
    case DT_RUNPATH:
      em.rupath = fromutf8(stroffset(soff + first, send));
      break;
    case DT_RPATH:
      em.rpath = fromutf8(stroffset(soff + first, send));
      break;
    default:
      break;
    }
  }
  return true;
}
std::optional<elf_minutiae_t> inquisitive_elf(std::wstring_view sv,
                                              std::error_code &ec) {
  planck::mapview mv;
  if (!mv.mapfile(sv, sizeof(Elf32_Ehdr))) {
    return std::nullopt;
  }
  elf_memview emv(mv.data(), mv.size());
  elf_minutiae_t em;
  if (emv.inquisitive(em, ec)) {
    return std::make_optional<elf_minutiae_t>(std::move(em));
  }
  return std::nullopt;
}
} // namespace inquisitive
