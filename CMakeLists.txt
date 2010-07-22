cmake_minimum_required(VERSION 2.6)
project(DACT)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

list(APPEND CMAKE_MODULE_PATH "${DACT_SOURCE_DIR}/cmake")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(IndexedCorpus REQUIRED)
if(INDEXEDCORPUS_FOUND)
	INCLUDE_DIRECTORIES(${INDEXEDCORPUS_INCLUDE_DIR})
endif(INDEXEDCORPUS_FOUND)

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

find_package(Qt4 COMPONENTS QtCore QtGui QtSvg REQUIRED)
INCLUDE(${QT_USE_FILE})

SET(dact_SRCS
  DactMainWindow.cpp
  XPathValidator.cpp
  XSLTransformer.cpp
  main.cpp
)

SET(dact_MOC_HDRS
  DactMainWindow.h
  XPathValidator.hh
)

SET(dact_UI
  DactMainWindow.ui
)

SET(dact_RCS
  application.qrc
)

qt4_add_resources(dact_SRCS ${dact_RCS})
qt4_wrap_ui(dact_UI_HDRS ${dact_UI})
qt4_wrap_cpp(dact_MOC_SRCS ${dact_MOC_HDRS})

add_executable(dact ${dact_SRCS} ${dact_MOC_SRCS} ${dact_UI_HDRS})
target_link_libraries(dact ${QT_LIBRARIES})
target_link_libraries(dact ${INDEXEDCORPUS_LIBRARIES})
target_link_libraries(dact ${LIBXML2_LIBRARIES})
target_link_libraries(dact ${LIBXSLT_LIBRARIES})
target_link_libraries(dact ${LIBEXSLT_LIBRARIES})

