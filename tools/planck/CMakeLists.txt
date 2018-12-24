##


add_executable(planck
    main.cc
)

if(lto_supported)
  set_property(TARGET planck PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(planck
    Inquisitive
    Shlwapi
    Pathcch
    Kernel32
    Advapi32
)



install(TARGETS planck
    DESTINATION bin
)
