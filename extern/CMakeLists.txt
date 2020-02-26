# Include sub-projects.

#add_subdirectory (containers)
add_subdirectory (libyaml)

if (NOV_BUILD_TESTS)
  add_subdirectory (googletest)
endif ()
