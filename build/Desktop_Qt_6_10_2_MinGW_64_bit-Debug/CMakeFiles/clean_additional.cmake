# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appSimpleFileViewer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appSimpleFileViewer_autogen.dir\\ParseCache.txt"
  "appSimpleFileViewer_autogen"
  )
endif()
