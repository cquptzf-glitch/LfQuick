foreach(_required
        LFQUICK_BUILD_DIR
        LFQUICK_INSTALL_PREFIX
        LFQUICK_CONSUMER_SOURCE_DIR
        LFQUICK_CONSUMER_BUILD_DIR
        LFQUICK_QT6_DIR)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable: ${_required}")
    endif()
endforeach()

file(REMOVE_RECURSE
    "${LFQUICK_INSTALL_PREFIX}"
    "${LFQUICK_CONSUMER_BUILD_DIR}"
)

set(_install_command
    "${CMAKE_COMMAND}"
    --install "${LFQUICK_BUILD_DIR}"
    --prefix "${LFQUICK_INSTALL_PREFIX}"
)
if(DEFINED LFQUICK_CONFIG AND NOT "${LFQUICK_CONFIG}" STREQUAL "")
    list(APPEND _install_command --config "${LFQUICK_CONFIG}")
endif()

execute_process(
    COMMAND ${_install_command}
    RESULT_VARIABLE _install_result
    OUTPUT_VARIABLE _install_stdout
    ERROR_VARIABLE _install_stderr
)
if(NOT _install_result EQUAL 0)
    message(FATAL_ERROR
        "LfQuick package smoke install failed (${_install_result})\n"
        "stdout:\n${_install_stdout}\n"
        "stderr:\n${_install_stderr}")
endif()

set(_configure_command
    "${CMAKE_COMMAND}"
    -S "${LFQUICK_CONSUMER_SOURCE_DIR}"
    -B "${LFQUICK_CONSUMER_BUILD_DIR}"
    "-DCMAKE_PREFIX_PATH=${LFQUICK_INSTALL_PREFIX}"
    "-DQt6_DIR=${LFQUICK_QT6_DIR}"
)

if(DEFINED LFQUICK_GENERATOR AND NOT "${LFQUICK_GENERATOR}" STREQUAL "")
    list(APPEND _configure_command -G "${LFQUICK_GENERATOR}")
endif()
if(DEFINED LFQUICK_GENERATOR_PLATFORM AND NOT "${LFQUICK_GENERATOR_PLATFORM}" STREQUAL "")
    list(APPEND _configure_command -A "${LFQUICK_GENERATOR_PLATFORM}")
endif()
if(DEFINED LFQUICK_GENERATOR_TOOLSET AND NOT "${LFQUICK_GENERATOR_TOOLSET}" STREQUAL "")
    list(APPEND _configure_command -T "${LFQUICK_GENERATOR_TOOLSET}")
endif()

execute_process(
    COMMAND ${_configure_command}
    RESULT_VARIABLE _configure_result
    OUTPUT_VARIABLE _configure_stdout
    ERROR_VARIABLE _configure_stderr
)
if(NOT _configure_result EQUAL 0)
    message(FATAL_ERROR
        "LfQuick downstream find_package smoke configure failed (${_configure_result})\n"
        "stdout:\n${_configure_stdout}\n"
        "stderr:\n${_configure_stderr}")
endif()

message(STATUS "LfQuick installed-package downstream smoke test passed")
