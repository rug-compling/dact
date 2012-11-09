# Find the XQilla library

find_path(XQILLA_INCLUDE_DIR NAMES xqilla/xqilla-simple.hpp)

find_library(XQILLA_LIBRARY NAMES xqilla)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XQILLA DEFAULT_MSG
  XQILLA_INCLUDE_DIR XQILLA_LIBRARY)

mark_as_advanced(XQILLA_INCLUDE_DIR XQILLA_LIBRARY)
