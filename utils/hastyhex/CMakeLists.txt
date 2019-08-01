#


add_executable(hastyhex
    hastyhex.cc
)

if(lto_supported)
  set_property(TARGET hastyhex PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(hastyhex
    bela
    Pathcch
    Kernel32
    Advapi32
)



install(TARGETS hastyhex
    DESTINATION bin
)