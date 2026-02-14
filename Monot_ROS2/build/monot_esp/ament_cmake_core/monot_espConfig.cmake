# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_monot_esp_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED monot_esp_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(monot_esp_FOUND FALSE)
  elseif(NOT monot_esp_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(monot_esp_FOUND FALSE)
  endif()
  return()
endif()
set(_monot_esp_CONFIG_INCLUDED TRUE)

# output package information
if(NOT monot_esp_FIND_QUIETLY)
  message(STATUS "Found monot_esp: 0.0.0 (${monot_esp_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'monot_esp' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${monot_esp_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(monot_esp_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "ament_cmake_export_dependencies-extras.cmake;ament_cmake_export_include_directories-extras.cmake;ament_cmake_export_libraries-extras.cmake")
foreach(_extra ${_extras})
  include("${monot_esp_DIR}/${_extra}")
endforeach()
