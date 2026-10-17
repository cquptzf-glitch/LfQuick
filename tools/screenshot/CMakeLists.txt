qt_add_executable(lfquick-screenshot
    main.cpp
)

target_link_libraries(lfquick-screenshot
    PRIVATE
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

install(TARGETS lfquick-screenshot
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tools
)
