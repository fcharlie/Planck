#

add_library(Inquisitive STATIC
  archive.cc
  binexeobj.cc
  docs.cc
  elf.cc
  font.cc
  git.cc
  image.cc
  inquisitive.cc
  macho.cc
  media.cc
  mime.cc
  pe.cc
  resolve.cc
  shl.cc
  text.cc
  zip.cc
)

target_link_libraries(Inquisitive
  belawin
)