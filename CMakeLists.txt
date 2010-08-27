cmake_minimum_required(VERSION 2.6)
project(DACT)

set(VERSION "0.0.1")

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
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${DACT_SOURCE_DIR}/resource/doc
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DACT_SOURCE_DIR}/resources/doc
  COMMAND ${CMAKE_COMMAND} -E copy ${DACT_SOURCE_DIR}/doc/index.html ${DACT_SOURCE_DIR}/resources/doc/index.html
  OUTPUT ${DACT_SOURCE_DIR}/resources/doc/index.html
)

# Dact sources
SET(dact_SRCS
  src/DactMainWindow/DactMainWindow.cpp
  src/DactFilterWindow/DactFilterWindow.cpp
  src/DactHelpWindow/DactHelpWindow.cpp
  src/DactQueryWindow/DactQueryWindow.cpp
  src/DactMacrosWindow/DactMacrosWindow.cpp
  src/DactMacrosModel/DactMacrosModel.cpp
  src/PercentageCellDelegate/PercentageCellDelegate.cpp
  src/XPathFilter/XPathFilter.cpp
  src/XPathMapper/XPathMapper.cpp
  src/XPathValidator/XPathValidator.cpp
  src/XSLTransformer/XSLTransformer.cpp
  src/main.cpp
)

SET(dact_MOC_HDRS
  include/DactMainWindow.h
  include/DactFilterWindow.h
  include/DactHelpWindow.h
  include/DactQueryWindow.h
  include/DactMacrosWindow.h
  include/DactMacrosModel.h
  include/PercentageCellDelegate.h
  include/XPathFilter.hh
  include/XPathMapper.hh
  include/XPathValidator.hh
)

SET(dact_UI
  ui/DactMainWindow.ui
  ui/DactFilterWindow.ui
  ui/DactHelpWindow.ui
  ui/DactQueryWindow.ui
  ui/DactMacrosWindow.ui
)

SET(dact_RCS
  resources/dact.qrc
)

set(dact_MACOSX_RESOURCE_FILES ${DACT_SOURCE_DIR}/resources/dact-espresso.icns)

set_source_files_properties(
  ${dact_MACOSX_RESOURCE_FILES}
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
)

qt4_add_resources(dact_SRCS ${dact_RCS})
qt4_wrap_ui(dact_UI_HDRS ${dact_UI})
qt4_wrap_cpp(dact_MOC_SRCS ${dact_MOC_HDRS})

add_executable(dact MACOSX_BUNDLE WIN32
	${dact_SRCS}
	${dact_MOC_SRCS}
	${dact_UI_HDRS}
	${dact_MACOSX_RESOURCE_FILES})
target_link_libraries(dact ${QT_LIBRARIES})
target_link_libraries(dact ${ALPINOCORPUS_LIBRARIES})
target_link_libraries(dact ${LIBXML2_LIBRARIES})
target_link_libraries(dact ${LIBXSLT_LIBRARIES})
target_link_libraries(dact ${LIBEXSLT_LIBRARIES})


install(TARGETS dact
  BUNDLE DESTINATION . COMPONENT Runtime
  RUNTIME DESTINATION bin COMPONENT Runtime) 

# OS X bundle creation
if(APPLE)

find_package(QDictZip REQUIRED)

get_filename_component(ALPINOCORPUS_LIBRARY_DIR
  ${ALPINOCORPUS_LIBRARIES} PATH)
get_filename_component(QDICTZIP_LIBRARY_DIR
  ${QDICTZIP_LIBRARIES} PATH)

set(DIRS
  ${QT_LIBRARY_DIR}
  ${ALPINOCORPUS_LIBRARY_DIR}
  ${QDICTZIP_LIBRARY_DIR})

set(MACOSX_BUNDLE_ICON_FILE "dact-espresso.icns")
set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION} )

INSTALL(CODE "
  file(WRITE \"${DACT_BINARY_DIR}/dact.app/Contents/Resources/qt.conf\")
  file(MAKE_DIRECTORY \"${DACT_BINARY_DIR}/dact.app/Contents/Resources/qt_menu.nib\")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
    \"${QT_LIBRARY_DIR}/QtGui.framework/Resources/qt_menu.nib\"
    \"${DACT_BINARY_DIR}/dact.app/Contents/Resources/qt_menu.nib\")
  include(BundleUtilities)
  fixup_bundle(\"${DACT_BINARY_DIR}/dact.app\" \"\" \"${DIRS}\")
" COMPONENT Runtime)

set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)

endif(APPLE)
