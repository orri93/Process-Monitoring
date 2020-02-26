set(pm_library_source
  "pm.c")

add_library(${pm_library_target} ${pm_library_source})

if(CLANG_TIDY_EXE)
  set_target_properties(${pm_library_target} PROPERTIES
    CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
endif()

if(GENERATE_PDB_FOR_RELEASE AND CMAKE_BUILD_TYPE MATCHES "Release")
  target_compile_options(${pm_library_target}
    PRIVATE /Zi)
  # Tell linker to include symbol data
  set_target_properties(${pm_library_target} PROPERTIES 
    LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")
  # Set file name & location
  set_target_properties(${pm_library_target} PROPERTIES 
    COMPILE_PDB_NAME ${pm_library_target} 
    COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR})
  install(FILES "$<TARGET_FILE_DIR:${pm_library_target}>/${pm_library_target}.pdb"
    DESTINATION pdb)
endif()

target_include_directories(${pm_library_target} PUBLIC ${pm_include})

target_compile_definitions(${pm_library_target} PUBLIC
  _CRT_SECURE_NO_WARNINGS)

set_target_properties(${pm_library_target} PROPERTIES
  PUBLIC_HEADER "${pm_library_public_headers}")

install(TARGETS ${pm_library_target}
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/pm
  ARCHIVE DESTINATION lib)
