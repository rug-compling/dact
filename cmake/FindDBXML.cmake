# Find the Oracle Berkeley DB XML library and its requirements

find_library(XQILLA_LIBRARY NAMES xqilla)
find_library(XERCES_LIBRARY NAMES xerces-c)
find_library(DBXML_LIBRARY NAMES dbxml)
find_library(DBCXX_LIBRARY NAMES db_cxx)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBXML DEFAULT_MSG DBXML_LIBRARY)
find_package_handle_standard_args(DBCXX DEFAULT_MSG DBCXX_LIBRARY)
find_package_handle_standard_args(XERCES DEFAULT_MSG XERCES_LIBRARY)
find_package_handle_standard_args(XQILLA DEFAULT_MSG XQILLA_LIBRARY)

mark_as_advanced(DBXML_LIBRARIES)
