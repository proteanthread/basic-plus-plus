# =====================================================================
# BASIC++ CMake Package Configuration File (basicppConfig.cmake)
# Usage: find_package(basicpp REQUIRED)
#        target_link_libraries(my_app PRIVATE basicpp::baspp)
# =====================================================================

@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

if(NOT TARGET basicpp::baspp)
    add_library(basicpp::baspp SHARED IMPORTED)
    set_target_properties(basicpp::baspp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/include"
        IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/lib/basicpp.dll"
        IMPORTED_IMPLIB "${PACKAGE_PREFIX_DIR}/lib/basicpp.lib"
    )
endif()

check_required_components(basicpp)
