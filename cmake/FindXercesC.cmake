# Find the Xerces-C library

find_path(XERCESC_INCLUDE_DIR NAMES xercesc/parsers/SAXParser.hpp)

find_library(XERCESC_LIBRARY NAMES xerces-c)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XERCESC DEFAULT_MSG
  XERCESC_INCLUDE_DIR XERCESC_LIBRARY)

mark_as_advanced(XERCESC_INCLUDE_DIR XERCESC_LIBRARY)
