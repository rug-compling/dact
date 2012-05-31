cmake_minimum_required(VERSION 2.6)
project(DACT)

set(CMAKE_CXX_FLAGS "-pthread")

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF("${isSystemDir}" STREQUAL "-1")

if(WIN32)
  # Let CPack include the required libraries.
  include (InstallRequiredSystemLibraries)
endif()

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.5")
  set(ENABLE_BUNDLE OFF CACHE BOOL "Compile an application bundle")
  set(ENABLE_CODESIGN OFF CACHE BOOL "Sign the application bundle")
  if(ENABLE_CODESIGN)
    set(CODESIGN_IDENTITY "John Doe" CACHE STRING "Code signing identity")
  endif(ENABLE_CODESIGN)
else(APPLE)
  set(ENABLE_BUNDLE OFF)
endif(APPLE)

if(ENABLE_BUNDLE)
  set(PROGNAME Dact)
else()
  set(PROGNAME dact)
endif()


set(VERSION "0.0.1")

set(GIT_REVISION "unknown")
exec_program(git
  ARGS "log --pretty=format:\"%ad (commit: %h)\" --date=iso -2"
  OUTPUT_VARIABLE GIT_REVISION 
)

message(STATUS "Building revision: ${GIT_REVISION}")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/config.hh.cmake 
  ${CMAKE_SOURCE_DIR}/include/config.hh)

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

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

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

find_package(Qt4 4.8.0 COMPONENTS QtCore QtGui QtMain QtNetwork REQUIRED)
INCLUDE(${QT_USE_FILE})

option(USE_REMOTE_CORPUS "Enable remote corpus" ON)

option(USE_WEBSERVICE "Enable Alpino webservice client" ON)

# Dact sources
SET(dact_SRCS
  src/AboutWindow.cpp
  src/ArchiveModel.cpp
  src/DactColorWell.cpp
  src/BracketedDelegates/BracketedDelegate.cpp
  src/BracketedDelegates/BracketedColorDelegate.cpp
  src/BracketedDelegates/BracketedVisibilityDelegate.cpp
  src/BracketedDelegates/BracketedKeywordInContextDelegate.cpp
  src/BracketedSentenceWidget.cpp
  src/BracketedWindow.cpp
  src/Chunk.cpp
  src/DependencyTreeWidget.cpp
  src/DownloadWindow.cpp
  src/DactApplication.cpp
  src/HistoryComboBox.cpp
  src/DactListView.cpp
  src/DactMacrosFile.cpp
  src/DactMacrosMenu.cpp
  src/DactMacrosModel.cpp
  src/DactTableView.cpp
  src/DactTool.cpp
  src/DactToolMenu.cpp
  src/DactToolModel.cpp
  src/DelayedLoadFileCallback.cpp
  src/QueryModel.cpp
  src/QtIOCompressor.cpp
  src/DactTreeScene.cpp
  src/DactTreeView.cpp
  src/FilterModel.cpp
  src/GlobalCopyCommand.cpp
  src/GlobalCutCommand.cpp
  src/GlobalEditCommand.cpp
  src/GlobalPasteCommand.cpp
  src/MainWindow.cpp
  src/PercentageCellDelegate.cpp
  src/PopupItem.cpp
  src/PreferencesWindow.cpp
  src/RecentFilesMenu.cpp
  src/StatisticsWindow.cpp
  src/TreeNode.cpp
  src/TreeNodeInspector.cpp
  src/XPathValidator.cpp
  src/XSLTransformer.cpp
  src/generateQuery.cpp
  src/main.cpp
  src/validityColor.cpp
)

SET(dact_HDRS
  include/AboutWindow.hh
  include/ArchiveModel.hh
  include/BracketedColorDelegate.hh
  include/BracketedDelegate.hh
  include/BracketedKeywordInContextDelegate.hh
  include/BracketedSentenceWidget.hh
  include/BracketedVisibilityDelegate.hh
  include/BracketedWindow.hh
  include/CorpusWidget.hh
  include/DependencyTreeWidget.hh
  include/DownloadWindow.hh
  include/config.hh
  include/DactApplication.hh
  include/DactColorWell.hh
  include/DactListView.hh
  include/DactMacro.hh
  include/DactMacrosFile.hh
  include/DactMacrosMenu.hh
  include/DactMacrosModel.hh
  include/DactQueryHistory.hh
  include/DactTableView.hh
  include/DactTool.hh
  include/DactToolMenu.hh
  include/DactToolModel.hh
  include/DactTreeScene.hh
  include/DactTreeView.hh
  include/DelayedLoadFileCallback.hh
  include/FilterModel.hh
  include/GlobalCopyCommand.hh
  include/GlobalCutCommand.hh
  include/GlobalEditCommand.hh
  include/GlobalPasteCommand.hh
  include/HistoryComboBox.hh
  include/MainWindow.hh
  include/PercentageCellDelegate.hh
  include/PopupItem.hh
  include/PreferencesWindow.hh
  include/QtIOCompressor.hh
  include/Query.hh
  include/QueryModel.hh
  include/RecentFilesMenu.hh
  include/StatisticsWindow.hh
  include/TreeNode.hh
  include/TreeNodeInspector.hh
  include/ValidityColor.hh
  include/XPathValidator.hh
  include/XSLTransformer.hh
)  

SET(dact_MOC_HDRS
  include/AboutWindow.hh
  include/ArchiveModel.hh
  include/DactApplication.hh
  include/DactColorWell.hh
  include/BracketedDelegate.hh
  include/BracketedColorDelegate.hh
  include/BracketedKeywordInContextDelegate.hh
  include/BracketedSentenceWidget.hh
  include/BracketedVisibilityDelegate.hh
  include/BracketedWindow.hh
  include/CorpusWidget.hh
  include/DependencyTreeWidget.hh
  include/DownloadWindow.hh
  include/DactListView.hh
  include/DactMacrosFile.hh
  include/DactMacrosMenu.hh
  include/DactMacrosModel.hh
  include/DactTableView.hh
  include/DactTool.hh
  include/DactToolMenu.hh
  include/DactToolModel.hh
  include/DactTreeScene.hh
  include/DactTreeView.hh
  include/DelayedLoadFileCallback.hh
  include/HistoryComboBox.hh
  include/QtIOCompressor.hh
  include/QueryModel.hh
  include/FilterModel.hh
  include/GlobalCopyCommand.hh
  include/GlobalCutCommand.hh
  include/GlobalEditCommand.hh
  include/GlobalPasteCommand.hh
  include/MainWindow.hh
  include/PercentageCellDelegate.hh
  include/PreferencesWindow.hh
  include/RecentFilesMenu.hh
  include/StatisticsWindow.hh
  include/TreeNodeInspector.hh
  include/XPathValidator.hh
)

SET(dact_UI
  ui/AboutWindow.ui
  ui/BracketedWindow.ui
  ui/DependencyTreeWidget.ui
  ui/DownloadWindow.ui
  ui/MainWindow.ui
  ui/PreferencesWindow.ui
  ui/StatisticsWindow.ui
  ui/TreeNodeInspector.ui
)

SET(dact_RCS
  resources/dact.qrc
)

if(USE_REMOTE_CORPUS)
  list(APPEND dact_HEADERS
    include/RemoteWindow.hh)
  list(APPEND dact_MOC_HDRS
    include/RemoteWindow.hh)
  list(APPEND dact_SRCS
    src/RemoteWindow.cpp)
  list(APPEND dact_UI
    ui/RemoteWindow.ui)
endif()

if(USE_WEBSERVICE)
  list(APPEND dact_HEADERS
    include/WebserviceWindow.hh)
  list(APPEND dact_MOC_HDRS
    include/WebserviceWindow.hh)
  list(APPEND dact_SRCS
    src/WebserviceWindow.cpp)
  list(APPEND dact_UI
    ui/WebserviceWindow.ui)
endif()

if(WIN32)
  set(dact_SRCS
    ${dact_SRCS}
    resources/dact.rc
  )
endif(WIN32)

set(dact_MACOSX_RESOURCE_FILES
  ${DACT_SOURCE_DIR}/resources/dact-espresso.icns
  ${DACT_SOURCE_DIR}/resources/dact-corpus.icns)

set_source_files_properties(
  ${dact_MACOSX_RESOURCE_FILES}
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
)

qt4_add_resources(dact_SRCS ${dact_RCS})
qt4_wrap_ui(dact_UI_HDRS ${dact_UI})
qt4_wrap_cpp(dact_MOC_SRCS ${dact_MOC_HDRS})

if(ENABLE_BUNDLE)
  add_executable(${PROGNAME} MACOSX_BUNDLE
    ${dact_SRCS}
    ${dact_HDRS}
    ${dact_MOC_SRCS}
    ${dact_UI_HDRS}
    ${dact_MACOSX_RESOURCE_FILES})
else()
  add_executable(${PROGNAME} WIN32
    ${dact_SRCS}
    ${dact_HDRS}
    ${dact_MOC_SRCS}
    ${dact_UI_HDRS}
    ${dact_MACOSX_RESOURCE_FILES})
endif()

target_link_libraries(${PROGNAME} ${QT_LIBRARIES})
target_link_libraries(${PROGNAME} ${ALPINOCORPUS_LIBRARIES})
target_link_libraries(${PROGNAME} ${DBXML_LIBRARY})
target_link_libraries(${PROGNAME} ${ZLIB_LIBRARIES})
#target_link_libraries(${PROGNAME} ${DBCXX_LIBRARY})
#target_link_libraries(${PROGNAME} ${XQILLA_LIBRARY})
#target_link_libraries(${PROGNAME} ${XERCES_LIBRARY})
target_link_libraries(${PROGNAME} ${LIBXML2_LIBRARIES})
target_link_libraries(${PROGNAME} ${LIBXSLT_LIBRARIES})
target_link_libraries(${PROGNAME} ${LIBEXSLT_LIBRARIES})


install(TARGETS ${PROGNAME} 
  BUNDLE DESTINATION . COMPONENT Runtime
  RUNTIME DESTINATION bin COMPONENT Runtime) 

# OS X bundle creation
if(APPLE)
  if(ENABLE_BUNDLE)
    set(APP ${CMAKE_INSTALL_PREFIX}/${PROGNAME}.app)

    get_filename_component(ALPINOCORPUS_LIBRARY_DIR
      ${ALPINOCORPUS_LIBRARIES} PATH)

    set(DIRS
      ${QT_LIBRARY_DIR}
      ${ALPINOCORPUS_LIBRARY_DIR})

    set(MACOSX_BUNDLE_GUI_IDENTIFIER "nl.rug.Dact")
    set(MACOSX_BUNDLE_ICON_FILE "dact-espresso.icns")
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROGNAME} )
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION} )

    set_target_properties(${PROGNAME}
      PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${DACT_SOURCE_DIR}/resources/Info.plist)

    INSTALL(CODE "
      file(WRITE \"${APP}/Contents/Resources/qt.conf\")
      file(MAKE_DIRECTORY \"${APP}/Contents/Resources/qt_menu.nib\")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
        \"${QT_LIBRARY_DIR}/QtGui.framework/Resources/qt_menu.nib\"
        \"${APP}/Contents/Resources/qt_menu.nib\")
      include(BundleUtilities)
      fixup_bundle(\"${APP}\" \"\" \"${DIRS}\")
    " COMPONENT Runtime)

    # XXX - Automatically find frameworks?
    if(ENABLE_CODESIGN)
      INSTALL(CODE "
      execute_process(COMMAND codesign --verbose --verify --sign \"${CODESIGN_IDENTITY}\" \"${APP}/Contents/Frameworks/QtCore.framework/Versions/4/QtCore\")
      execute_process(COMMAND codesign --verbose --verify --sign \"${CODESIGN_IDENTITY}\" \"${APP}/Contents/Frameworks/QtGui.framework/Versions/4/QtGui\")
      execute_process(COMMAND codesign --verbose --verify --sign \"${CODESIGN_IDENTITY}\" \"${APP}/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork\")
      execute_process(COMMAND codesign --verbose --verify --sign \"${CODESIGN_IDENTITY}\" \"${APP}\")
      " COMPONENT Runtime)
    endif(ENABLE_CODESIGN)

    set(CPACK_BINARY_DRAGNDROP ON)
    include(CPack)
  endif(ENABLE_BUNDLE)
endif(APPLE)

# Windows installer
if(WIN32)
  set(CPACK_GENERATOR NSIS)
  set(CPACK_MONOLITHIC_INSTALL TRUE)
  set(CPACK_PACKAGE_NAME "Dact")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Decaffeinated Alpino Corpus Tool")
  set(CPACK_PACKAGE_VENDOR "Daniel de Kok")
  set(CPACK_NSIS_PACKAGE_NAME "Dact")
  set(CPACK_NSIS_DISPLAY_NAME "Dact")
  set(CPACK_PACKAGE_VERSION ${VERSION})
  set(CPACK_NSIS_CONTACT "me@danieldk.eu")
  set(CPACK_PACKAGE_EXECUTABLES ${PROGNAME} ${PROGNAME})
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Dact-${VERSION}")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
  set(CPACK_PACKAGE_ICON "${DACT_SOURCE_DIR}/resources\\\\dact-espresso.bmp")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Decaffeinated Alpino Corpus Tool")

  install(FILES
      "${QT_BINARY_DIR}/QtCore${QT_VERSION_MAJOR}.dll"
      "${QT_BINARY_DIR}/QtGui${QT_VERSION_MAJOR}.dll"
      "${QT_BINARY_DIR}/QtNetwork${QT_VERSION_MAJOR}.dll"
    DESTINATION bin
  )

  # Ok, now things are getting really annoying. We could apply
  # all kinds of fun path manipulations to get the DLLs. But
  # we'll just assume the you've put them in the runtime directory
  # of the source directory.
  #
  # Yes, this makes you long for Linux' package management.
  set(DACT_DEPENDENCY_DIR "${DACT_SOURCE_DIR}/runtime")
  install(FILES
      "${DACT_DEPENDENCY_DIR}/alpino_corpus.dll"
      "${DACT_DEPENDENCY_DIR}/iconv.dll"
      "${DACT_DEPENDENCY_DIR}/libdb48.dll"
      "${DACT_DEPENDENCY_DIR}/libdbxml25.dll"
      "${DACT_DEPENDENCY_DIR}/libexslt.dll"
      "${DACT_DEPENDENCY_DIR}/libxml2.dll"
      "${DACT_DEPENDENCY_DIR}/libxslt.dll"
      "${DACT_DEPENDENCY_DIR}/xerces-c_3_0.dll"
      "${DACT_DEPENDENCY_DIR}/xqilla22.dll"
      "${DACT_DEPENDENCY_DIR}/zlib1.dll"
    DESTINATION bin
  )

  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    !include ${DACT_SOURCE_DIR}/resources/nsis\\\\FileAssociation.nsh
    \\\${registerExtension} \\\"\\\$INSTDIR\\\\bin\\\\dact.exe\\\" \\\".dact\\\" \\\"Dact Corpus\\\"
  ")

  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
    ${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
    "\\\${unregisterExtension} \\\".dact\\\" \\\"Dact Corpus\\\"")

  include(CPack)
endif(WIN32)
