cmake_minimum_required(VERSION 2.6)
project(DACT)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

list(APPEND CMAKE_MODULE_PATH "${DACT_SOURCE_DIR}/cmake")

include_directories(${DACT_SOURCE_DIR}/include)
include_directories(${DACT_BINARY_DIR})

find_package(AlpinoCorpus REQUIRED)
if(ALPINOCORPUS_FOUND)
	INCLUDE_DIRECTORIES(${ALPINOCORPUS_INCLUDE_DIR})
endif(ALPINOCORPUS_FOUND)

find_package(Iconv REQUIRED)
if(ICONV_FOUND)
  include_directories(${ICONV_INCLUDE_DIR})
endif()

find_package(LibXml2 REQUIRED)
if(LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIR})
endif()

find_package(LibXslt REQUIRED)
if(LIBXSLT_FOUND)
  include_directories(${LIBXSLT_INCLUDE_DIR})
endif()

find_package(LibExslt REQUIRED)
if(LIBEXSLT_FOUND)
  include_directories(${LIBEXSLT_INCLUDE_DIR})
endif()

find_package(Qt4 COMPONENTS QtCore QtGui QtMain QtSvg REQUIRED)
INCLUDE(${QT_USE_FILE})

# Documentation
find_program(QHELP_GENERATOR_COMMAND NAMES qhelpgenerator)
find_program(QCOLLECTION_GENERATOR_COMMAND NAMES qcollectiongenerator)

add_custom_command(
  DEPENDS
  ${DACT_SOURCE_DIR}/doc/index.html
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${DACT_SOURCE_DIR}/qrc/doc
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DACT_SOURCE_DIR}/qrc/doc
  COMMAND ${CMAKE_COMMAND} -E copy ${DACT_SOURCE_DIR}/doc/index.html ${DACT_SOURCE_DIR}/qrc/doc/index.html
  OUTPUT ${DACT_SOURCE_DIR}/qrc/doc/index.html
)

# Dact sources
SET(dact_SRCS
  src/DactMainWindow/DactMainWindow.cpp
  src/DactFilterWindow/DactFilterWindow.cpp
  src/DactHelpWindow/DactHelpWindow.cpp
  src/XPathFilter/XPathFilter.cpp
  src/XPathValidator/XPathValidator.cpp
  src/XSLTransformer/XSLTransformer.cpp
  src/main.cpp
)

SET(dact_MOC_HDRS
  include/DactMainWindow.h
  include/DactFilterWindow.h
  include/DactHelpWindow.h
  include/XPathValidator.hh
)

SET(dact_UI
  ui/DactMainWindow.ui
  ui/DactFilterWindow.ui
  ui/DactHelpWindow.ui
)

SET(dact_RCS
  qrc/dact.qrc
)

qt4_add_resources(dact_SRCS ${dact_RCS})
qt4_wrap_ui(dact_UI_HDRS ${dact_UI})
qt4_wrap_cpp(dact_MOC_SRCS ${dact_MOC_HDRS})

add_executable(dact WIN32 ${dact_SRCS} ${dact_MOC_SRCS} ${dact_UI_HDRS})
target_link_libraries(dact ${QT_LIBRARIES})
target_link_libraries(dact ${ALPINOCORPUS_LIBRARIES})
target_link_libraries(dact ${LIBXML2_LIBRARIES})
target_link_libraries(dact ${LIBXSLT_LIBRARIES})
target_link_libraries(dact ${LIBEXSLT_LIBRARIES})

install(TARGETS dact
  RUNTIME DESTINATION bin) 
