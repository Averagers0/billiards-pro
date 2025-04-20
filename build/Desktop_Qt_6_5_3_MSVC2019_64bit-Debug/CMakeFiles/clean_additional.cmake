# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\billiards_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\billiards_autogen.dir\\ParseCache.txt"
  "billiards_autogen"
  )
endif()
