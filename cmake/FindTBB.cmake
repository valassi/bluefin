# - Locate tbblibrary
# Defines:

#  TBB_FOUND
#  TBB_LIBRARIES
#  TBB_LIBRARY_DIRS (not cached)

find_library(TBB_LIBRARIES NAMES tbb)

get_filename_component(TBB_LIBRARY_DIRS ${TBB_LIBRARIES} PATH)

# handle the QUIETLY and REQUIRED arguments and set TBB_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TBB DEFAULT_MSG TBB_LIBRARIES)

mark_as_advanced(TBB_FOUND TBB_LIBRARIES)
