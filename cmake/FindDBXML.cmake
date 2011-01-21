# Find the Oracle Berkeley DB XML library

find_path(DBXML_INCLUDE_DIR NAMES dbxml/DbXml.hpp)

find_library(DBXML_LIBRARY NAMES dbxml)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBXML DEFAULT_MSG
                                  DBXML_INCLUDE_DIR DBXML_LIBRARY)

mark_as_advanced(DBXML_INCLUDE_DIR DBXML_LIBRARY)
