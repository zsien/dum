set(BIN_NAME deepin-update-manager)

add_executable(${BIN_NAME}
    main.cpp
    dbusx/bus.cpp
    dbusx/bus.h
    dbusx/bus_private.cpp
    dbusx/bus_private.h
    dbusx/error.h
    dbusx/interface.h
    dbusx/message.cpp
    dbusx/message.h
    dbusx/message_private.cpp
    dbusx/message_private.h
    dbusx/method.h
    dbusx/property.h
    dbusx/signal.h
    dbusx/typedef.h
    dbusx/type.h
    dbusx/utils.h
    dbusx/vtable.h
)

target_link_libraries(${BIN_NAME} PRIVATE
    PkgConfig::libsystemd
    PkgConfig::libuv
    spdlog
    uvw::uvw
)

install(
    TARGETS ${BIN_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/dde-dialog
)
