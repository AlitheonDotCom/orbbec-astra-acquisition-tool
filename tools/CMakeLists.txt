# add examples
cmake_minimum_required(VERSION 3.2)
project(tools)

if (ANDROID)
  set(ASTRA_ANDROID TRUE)
elseif (WIN32)
  set(ASTRA_WINDOWS TRUE)
elseif (APPLE)
  set(ASTRA_OSX TRUE)
elseif (UNIX) #not android
  set(ASTRA_UNIX TRUE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../astra-build/sdk/samples/cmake")

#Override FindAstra equivalents
set(ASTRA_SDK_BUILD TRUE)
set(ASTRA_LIBRARIES
  astra_core
  astra_core_api
  astra
  )
set(ASTRA_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/../astra-build/sdk/include
  )

set(ASTRA_LIB_DIR
  ${PROJECT_SOURCE_DIR}/../astra-build/sdk/lib
  )


find_package(Astra REQUIRED)

set(ASTRA_SDK_BUILD FALSE)


if (NOT ASTRA_SDK_BUILD)
  #copy Astra SDK binaries to lib dir
  if (ASTRA_OSX)
    set(ASTRA_LIB_COPY_TARGET ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    add_custom_target(copy-astra-libs ALL
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${ASTRA_LIB_DIR}/
      ${ASTRA_LIB_COPY_TARGET})
    set_target_properties(copy-astra-libs PROPERTIES FOLDER CMakeCopyTargets)
  endif()
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(ASTRA_64 ON)
else()
  set(ASTRA_32 ON)
endif()

include(ConfigUserFile)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)

function(add_astra_sdk_dependency _targetname)
  if(NOT TARGET ${_targetname})
    message("add_sdk_dependency: There is no target named '${_targetname}'")
    return()
  endif()
  if(TARGET ${SDK_DEPENDENT_TARGET})
    add_dependencies(${_targetname} ${SDK_DEPENDENT_TARGET})
  endif()
endfunction()

function(install_sample _targetname)
  if (ASTRA_SDK_BUILD)
    install(TARGETS ${_targetname} DESTINATION bin/)
  endif()
endfunction()

include_directories(${ASTRA_INCLUDE_DIR})

get_filename_component(SAMPLE_COMMON_DIR "../astra-build/sdk/samples/common" ABSOLUTE)

include_directories(${SAMPLE_COMMON_DIR})

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(ASTRA_CLANG ON)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(ASTRA_GCC ON)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(ASTRA_INTELC ON)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(ASTRA_MSVC ON)
endif()

if (ASTRA_MSVC)
  if (MSVC_VERSION EQUAL 1800)
    set(VS_DIR "vs2013")
  elseif(MSVC_VERSION EQUAL 1900)
    set(VS_DIR "vs2015")
  else()
    error("MSVC other than VS2013 (VC12) or VS2015 (VC14) are not supported")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
  MESSAGE("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
  MESSAGE("CMAKE_C_FLAGS ${CMAKE_C_FLAGS}")
endif()

if(ASTRA_GCC OR ASTRA_CLANG)

  set(ASTRA_CXX_FLAGS "-Wall -fPIC")
  if (COMPILER_SUPPORTS_CXX14)
    set(ASTRA_CXX_FLAGS "${ASTRA_CXX_FLAGS} -std=c++14")
  elseif( COMPILER_SUPPORTS_CXX11)
    set(ASTRA_CXX_FLAGS "${ASTRA_CXX_FLAGS} -std=c++11")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASTRA_CXX_FLAGS}")

  if(ASTRA_ANDROID)
    set(ANDROID_CXX_FLAGS "${ANDROID_CXX_FLAGS} ${ASTRA_CXX_FLAGS}")
  endif()

endif()

link_directories(${ASTRA_LIB_DIR})
#set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#set the rpath to go up from the bin/ to to the lib/ dir to find the Astra libs
#if(ASTRA_OSX)
#  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
#  message("Setting rpath to ${CMAKE_INSTALL_RPATH}")
#elseif(ASTRA_UNIX)
#  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
#  message("Setting rpath to ${CMAKE_INSTALL_RPATH}")
#endif()


#SFML path varies depending upon if we're building the full SDK or just sample directory
set(SFML_PATH "${PROJECT_SOURCE_DIR}/../astra-build/sdk/samples/thirdparty/sfml-2.3.2-min/")

if (DEFINED SFML_PATH)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${SFML_PATH}")
endif()


if (ASTRA_OSX)
  add_custom_target(copy-sample-files ALL
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/../astra-build/sdk/samples/vendor/Inconsolata.otf"
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  set_target_properties(copy-sample-files PROPERTIES FOLDER CMakeCopyTargets)

  add_custom_command(TARGET copy-sample-files POST_BUILD
    COMMAND cp
    -R
    ${SFML_PATH}/lib/
    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/)

endif()

add_subdirectory(AcquisitionTool)



