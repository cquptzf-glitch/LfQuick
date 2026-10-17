set(LFQUICK_HAS_SAFE_AREA OFF)
set(LFQUICK_HAS_QRANGE_MODEL OFF)
set(LFQUICK_HAS_QRANGE_MODEL_DRAG_DROP OFF)
set(LFQUICK_HAS_CANVAS_PAINTER OFF)
set(LFQUICK_HAS_QML_PREVIEW_CMAKE OFF)
set(LFQUICK_HAS_STYLEKIT OFF)

if(DEFINED Qt6_VERSION)
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.9.0")
        set(LFQUICK_HAS_SAFE_AREA ON)
    endif()

    # QRangeModel itself is available from Qt 6.10. Qt 6.12 adds the
    # supported drag/drop action properties and DropOperation API.
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.10.0")
        set(LFQUICK_HAS_QRANGE_MODEL ON)
    endif()
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.12.0")
        set(LFQUICK_HAS_QRANGE_MODEL_DRAG_DROP ON)
    endif()

    # QCanvasPainter is available from Qt 6.11. Keep use behind an adapter;
    # the main LfQuick public API remains Qt 6.8 compatible.
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.11.0")
        set(LFQUICK_HAS_CANVAS_PAINTER ON)
    endif()

    # Qt 6.12 adds the qt_add_qml_preview() CMake integration and the
    # technology-preview Qt Labs StyleKit / QStyleKitStyle APIs.
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.12.0")
        set(LFQUICK_HAS_QML_PREVIEW_CMAKE ON)
        set(LFQUICK_HAS_STYLEKIT ON)
    endif()
endif()

function(lfquick_require_qt_feature feature_name feature_value minimum_version)
    if(NOT ${feature_value})
        message(FATAL_ERROR
            "${feature_name} requires Qt ${minimum_version} or newer; detected Qt ${Qt6_VERSION}")
    endif()
endfunction()
