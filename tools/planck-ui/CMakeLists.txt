##

add_executable(planck-ui WIN32
    main.cc
)

if(lto_supported)
  set_property(TARGET planck-ui PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(planck-ui
    Comctl32
    Shlwapi
    Pathcch
    Kernel32
    Gdi32
    Advapi32
)


install(TARGETS planck-ui
    DESTINATION bin
)