if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/weft-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package weft)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT weft_Development
)

install(
    TARGETS weft_weft
    EXPORT weftTargets
    RUNTIME #
    COMPONENT weft_Runtime
    LIBRARY #
    COMPONENT weft_Runtime
    NAMELINK_COMPONENT weft_Development
    ARCHIVE #
    COMPONENT weft_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    weft_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE weft_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(weft_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${weft_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT weft_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${weft_INSTALL_CMAKEDIR}"
    COMPONENT weft_Development
)

install(
    EXPORT weftTargets
    NAMESPACE weft::
    DESTINATION "${weft_INSTALL_CMAKEDIR}"
    COMPONENT weft_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()

install(
    FILES "${PROJECT_BINARY_DIR}/weft.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    COMPONENT weft_Development
)

install(
    DIRECTORY "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT weft_Development
)
