include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

set(LFQUICK_INSTALL_CMAKEDIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/LfQuick"
    CACHE STRING "Install directory for LfQuick CMake package files")
set(LFQUICK_INSTALL_QMLDIR
    "${CMAKE_INSTALL_LIBDIR}/qt/qml"
    CACHE STRING "Install root for LfQuick QML modules")

function(lfquick_install_target target export_name)
    if(NOT TARGET ${target})
        return()
    endif()

    set_target_properties(${target} PROPERTIES EXPORT_NAME ${export_name})

    get_target_property(_header_sets ${target} HEADER_SETS)
    if(_header_sets AND NOT _header_sets MATCHES "-NOTFOUND$")
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

    get_target_property(_qml_uri ${target} QT_QML_MODULE_URI)
    get_target_property(_qml_output_dir ${target} QT_QML_MODULE_OUTPUT_DIRECTORY)
    get_target_property(_qml_typeinfo ${target} QT_QML_MODULE_TYPEINFO)
    get_target_property(_qml_plugin_target ${target} QT_QML_MODULE_PLUGIN_TARGET)

    set(_has_qml_module FALSE)
    if(_qml_uri AND NOT _qml_uri MATCHES "-NOTFOUND$")
        set(_has_qml_module TRUE)
        string(REPLACE "." "/" _qml_uri_path "${_qml_uri}")
        set(_qml_install_dir "${LFQUICK_INSTALL_QMLDIR}/${_qml_uri_path}")
    endif()

    if(_qml_plugin_target AND NOT _qml_plugin_target MATCHES "-NOTFOUND$"
       AND TARGET ${_qml_plugin_target})
        if(_has_qml_module)
            install(TARGETS ${_qml_plugin_target}
                RUNTIME DESTINATION "${_qml_install_dir}"
                LIBRARY DESTINATION "${_qml_install_dir}"
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        else()
            install(TARGETS ${_qml_plugin_target}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        endif()
    endif()

    if(_has_qml_module
       AND _qml_output_dir AND NOT _qml_output_dir MATCHES "-NOTFOUND$")
        install(FILES "${_qml_output_dir}/qmldir"
            DESTINATION "${_qml_install_dir}"
            OPTIONAL)
        if(_qml_typeinfo AND NOT _qml_typeinfo MATCHES "-NOTFOUND$")
            install(FILES "${_qml_output_dir}/${_qml_typeinfo}"
                DESTINATION "${_qml_install_dir}"
                OPTIONAL)
        endif()
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
