find_path(XiApi_INCLUDE_DIR
  NAMES m3api/xiApi.h
  PATHS /opt/XIMEA/include
  )

find_library(XiApi_LIBRARY
  NAMES m3api
  )

mark_as_advanced(XiApi_INCLUDE_DIR xiApi_LIBRARY)

find_package_handle_standard_args(XiApi
  DEFAULT_MSG
  XiApi_LIBRARY
  XiApi_INCLUDE_DIR
  )
