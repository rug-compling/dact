# Find the Oracle Berkeley DB XML library and its requirements

find_library(XQILLA_LIBRARY names xqilla libxqilla.so PATH ${DBXML_PREFIX}/lib)
find_library(XERCES_LIBRARY names xerces-c libxerces-c.so PATH ${DBXML_PREFIX}/lib)
find_library(DBXML_LIBRARY names dbxml libdbxml.so PATH ${DBXML_PREFIX}/lib)
find_library(DBCXX_LIBRARY names db_cxx lib_dbcxx.so PATH ${DBXML_PREFIX}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBXML DEFAULT_MSG DBXML_LIBRARY)
find_package_handle_standard_args(DBCXX DEFAULT_MSG DBCXX_LIBRARY)
find_package_handle_standard_args(XERCES DEFAULT_MSG XERCES_LIBRARY)
find_package_handle_standard_args(XQILLA DEFAULT_MSG XQILLA_LIBRARY)

mark_as_advanced(DBXML_LIBRARIES)
