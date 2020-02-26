list(APPEND pmcli_source
  pmcli.c)

set(pmcli_target pmcli)

if(MSVC)
  set(PM_FILEDESCRIPTION "Simple Process Monitoring Tool")
  set(PM_INTERNALNAME "${pmcli_target}")
  set(PM_ORIGINALFILENAME "${pmcli_target}.exe")
  set(PM_PRODUCTNAME "${pmcli_target}")
  configure_file("version.rc.in" "version.rc" @ONLY)
  list(APPEND pmcli_source
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
endif()

add_executable(${pmcli_target} ${pmcli_source})

target_include_directories(${pmcli_target} PUBLIC
  ${pm_optparse_include}
  ${pm_include})

target_compile_definitions(${pmcli_target} PUBLIC
  _CRT_SECURE_NO_WARNINGS
  OPTPARSE_IMPLEMENTATION
  OPTPARSE_API=static)

list(APPEND pmcli_libraries ${pm_library_target})

target_link_libraries(${pmcli_target}
  ${pmcli_libraries})

if(CLANG_TIDY_EXE)
  set_target_properties(${pmcli_target} PROPERTIES
    CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
endif()

if(GENERATE_PDB_FOR_RELEASE AND CMAKE_BUILD_TYPE MATCHES "Release")
  target_compile_options(${pmcli_target}
    PRIVATE /Zi)
  # Tell linker to include symbol data
  set_target_properties(${pmcli_target} PROPERTIES 
    LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")
  # Set file name & location
  set_target_properties(${pmcli_target} PROPERTIES 
    COMPILE_PDB_NAME ${pmcli_target} 
    COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR})
  install(FILES "$<TARGET_FILE_DIR:${pmcli_target}>/${pmcli_target}.pdb"
    DESTINATION pdb)
endif()

add_custom_command(TARGET ${pmcli_target} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${pm_etc_path}/pm.yaml"
    "$<TARGET_FILE_DIR:${pmcli_target}>/pm.yaml")

install(TARGETS ${pmcli_target}
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION bin)
install(FILES "$<TARGET_FILE_DIR:${pmcli_target}>/pm.yaml"
  DESTINATION bin
  COMPONENT config)
