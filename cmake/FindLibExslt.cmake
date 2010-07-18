# - Try to find the LibXslt library
# Once done this will define
#
#  LIBEXSLT_FOUND - system has LibXslt
#  LIBEXSLT_INCLUDE_DIR - the LibXslt include directory
#  LIBEXSLT_LIBRARIES - Link these to LibXslt
#  LIBEXSLT_DEFINITIONS - Compiler switches required for using LibXslt

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_LIBEXSLT libexslt)
SET(LIBEXSLT_DEFINITIONS ${PC_LIBEXSLT_CFLAGS_OTHER})

FIND_PATH(LIBEXSLT_INCLUDE_DIR NAMES libexslt/exslt.h
    HINTS
   ${PC_LIBEXSLT_INCLUDEDIR}
   ${PC_LIBEXSLT_INCLUDE_DIRS}
  )

FIND_LIBRARY(LIBEXSLT_LIBRARIES NAMES exslt elibxslt
    HINTS
   ${PC_LIBEXSLT_LIBDIR}
   ${PC_LIBEXSLT_LIBRARY_DIRS}
  )

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibExslt DEFAULT_MSG LIBEXSLT_LIBRARIES LIBEXSLT_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBEXSLT_INCLUDE_DIR LIBEXSLT_LIBRARIES)

