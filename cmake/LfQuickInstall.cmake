include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

set(LFQUICK_INSTALL_CMAKEDIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/LfQuick"
    CACHE STRING "Install directory for LfQuick CMake package files")
set(LFQUICK_INSTALL_QMLDIR
    "${CMAKE_INSTALL_LIBDIR}/qt/qml"
    CACHE STRING "Install root for LfQuick QML modules")
set(LFQUICK_INSTALL_OBJECTDIR
    "${CMAKE_INSTALL_LIBDIR}/lfquick/objects"
    CACHE STRING "Install directory for generated static QML object targets")

function(lfquick_effective_qt_min_version out_var)
    set(_version "${LFQUICK_QT_MIN_VERSION}")

    if(TARGET LfQuickSafeAreaAdapter)
        set(_version "6.9.0")
    endif()
    if(TARGET LfQuickRangeModelAdapter)
        set(_version "6.10.0")
    endif()
    if(TARGET LfQuickCanvasPainterAdapter OR TARGET LfQuickStyleKit)
        set(_version "6.12.0")
    endif()

    set(${out_var} "${_version}" PARENT_SCOPE)
endfunction()

# qt_add_qml_module() can create extra resource/plugin-init targets for static
# backing libraries. Qt documents those OUTPUT_TARGETS as part of the installed
# module's link requirements, so remember them on the public backing target and
# export them together with the SDK.
function(lfquick_register_qml_output_targets target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR
            "lfquick_register_qml_output_targets: unknown target '${target}'")
    endif()

    if(ARGN)
        set_property(TARGET ${target}
            PROPERTY LFQUICK_QML_OUTPUT_TARGETS "${ARGN}")
    endif()
endfunction()

function(lfquick_assert_library_linkage target)
    get_target_property(_type ${target} TYPE)
    if(BUILD_SHARED_LIBS)
        set(_expected_type SHARED_LIBRARY)
    else()
        set(_expected_type STATIC_LIBRARY)
    endif()

    if(NOT _type STREQUAL _expected_type)
        message(FATAL_ERROR
            "${target} has type '${_type}', but BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} "
            "requires '${_expected_type}'. Check Qt policy QTP0003 and target creation.")
    endif()
endfunction()

function(lfquick_install_target target export_name)
    if(NOT TARGET ${target})
        return()
    endif()

    lfquick_assert_library_linkage(${target})
    set_target_properties(${target} PROPERTIES EXPORT_NAME ${export_name})

    # Directory-level LFQUICK_STATIC is sufficient while building LfQuick,
    # but it is not part of an exported target's usage requirements. Static
    # SDK consumers must see the same define or the public headers use
    # Q_DECL_IMPORT on Windows for symbols that actually live in static libs.
    if(NOT BUILD_SHARED_LIBS)
        target_compile_definitions(${target} INTERFACE LFQUICK_STATIC)
    endif()

    get_target_property(_header_sets ${target} HEADER_SETS)
    if(_header_sets AND NOT _header_sets MATCHES "-NOTFOUND$")
        install(TARGETS ${target}
            EXPORT LfQuickTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            OBJECTS DESTINATION ${LFQUICK_INSTALL_OBJECTDIR}
            FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    else()
        install(TARGETS ${target}
            EXPORT LfQuickTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            OBJECTS DESTINATION ${LFQUICK_INSTALL_OBJECTDIR}
        )
    endif()

    get_target_property(_qml_uri ${target} QT_QML_MODULE_URI)
    get_target_property(_qml_output_dir ${target} QT_QML_MODULE_OUTPUT_DIRECTORY)
    get_target_property(_qml_typeinfo ${target} QT_QML_MODULE_TYPEINFO)
    get_target_property(_qml_plugin_target ${target} QT_QML_MODULE_PLUGIN_TARGET)
    get_target_property(_qml_output_targets ${target} LFQUICK_QML_OUTPUT_TARGETS)

    set(_has_qml_module FALSE)
    if(_qml_uri AND NOT _qml_uri MATCHES "-NOTFOUND$")
        set(_has_qml_module TRUE)
        string(REPLACE "." "/" _qml_uri_path "${_qml_uri}")
        set(_qml_install_dir "${LFQUICK_INSTALL_QMLDIR}/${_qml_uri_path}")
    endif()

    # Be defensive if a future Qt version starts returning the generated plugin
    # through OUTPUT_TARGETS as well. It is installed separately below because
    # its runtime destination differs for shared QML modules.
    if(_qml_plugin_target AND NOT _qml_plugin_target MATCHES "-NOTFOUND$")
        list(REMOVE_ITEM _qml_output_targets "${_qml_plugin_target}")
    endif()

    if(_qml_output_targets AND NOT _qml_output_targets MATCHES "-NOTFOUND$")
        list(REMOVE_DUPLICATES _qml_output_targets)
        install(TARGETS ${_qml_output_targets}
            EXPORT LfQuickTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            OBJECTS DESTINATION ${LFQUICK_INSTALL_OBJECTDIR}
        )
    endif()

    if(_qml_plugin_target AND NOT _qml_plugin_target MATCHES "-NOTFOUND$"
       AND TARGET ${_qml_plugin_target})
        if(NOT BUILD_SHARED_LIBS)
            # Static QML plugins are link-time SDK targets. Give them a stable
            # exported name instead of leaking the generated build target name.
            set_target_properties(${_qml_plugin_target}
                PROPERTIES EXPORT_NAME "${export_name}QmlPlugin")
        endif()

        if(_has_qml_module)
            if(BUILD_SHARED_LIBS)
                install(TARGETS ${_qml_plugin_target}
                    RUNTIME DESTINATION "${_qml_install_dir}"
                    LIBRARY DESTINATION "${_qml_install_dir}"
                    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    OBJECTS DESTINATION ${LFQUICK_INSTALL_OBJECTDIR}
                )
            else()
                install(TARGETS ${_qml_plugin_target}
                    EXPORT LfQuickTargets
                    RUNTIME DESTINATION "${_qml_install_dir}"
                    LIBRARY DESTINATION "${_qml_install_dir}"
                    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    OBJECTS DESTINATION ${LFQUICK_INSTALL_OBJECTDIR}
                )
            endif()
        else()
            if(BUILD_SHARED_LIBS)
                install(TARGETS ${_qml_plugin_target}
                    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    OBJECTS DESTINATION ${LFQUICK_INSTALL_OBJECTDIR}
                )
            else()
                install(TARGETS ${_qml_plugin_target}
                    EXPORT LfQuickTargets
                    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    OBJECTS DESTINATION ${LFQUICK_INSTALL_OBJECTDIR}
                )
            endif()
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

    lfquick_effective_qt_min_version(LFQUICK_PACKAGE_QT_MIN_VERSION)

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
