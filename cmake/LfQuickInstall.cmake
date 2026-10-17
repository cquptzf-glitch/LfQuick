include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

set(LFQUICK_INSTALL_CMAKEDIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/LfQuick"
    CACHE STRING "Install directory for LfQuick CMake package files")

function(lfquick_install_target target export_name)
    if(NOT TARGET ${target})
        return()
    endif()

    set_target_properties(${target} PROPERTIES EXPORT_NAME ${export_name})

    get_target_property(_header_sets ${target} HEADER_SETS)
    if(_header_sets)
        install(TARGETS ${target}
            EXPORT LfQuickTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    else()
        install(TARGETS ${target}
            EXPORT LfQuickTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()

    get_target_property(_qml_plugin_target ${target} QT_QML_MODULE_PLUGIN_TARGET)
    if(_qml_plugin_target AND TARGET ${_qml_plugin_target})
        install(TARGETS ${_qml_plugin_target}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()
endfunction()

function(lfquick_install_package)
    set(_config_in "${PROJECT_SOURCE_DIR}/cmake/LfQuickConfig.cmake.in")
    set(_config_out "${PROJECT_BINARY_DIR}/LfQuickConfig.cmake")
    set(_version_out "${PROJECT_BINARY_DIR}/LfQuickConfigVersion.cmake")

    configure_package_config_file(
        "${_config_in}"
        "${_config_out}"
        INSTALL_DESTINATION "${LFQUICK_INSTALL_CMAKEDIR}"
    )

    write_basic_package_version_file(
        "${_version_out}"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
    )

    install(EXPORT LfQuickTargets
        FILE LfQuickTargets.cmake
        NAMESPACE LfQuick::
        DESTINATION "${LFQUICK_INSTALL_CMAKEDIR}"
    )

    install(FILES
        "${_config_out}"
        "${_version_out}"
        DESTINATION "${LFQUICK_INSTALL_CMAKEDIR}"
    )
endfunction()
