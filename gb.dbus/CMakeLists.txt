gb_component(NATIVE gb.dbus GAMBAS gb.dbus
    REQUIRED_PACKAGES DBus
    SOURCE_LIST c_dbus.c  c_dbusconnection.c  c_dbusobserver.c  c_dbusvariant.c
    dbus_print_message.c  helper.c  main.c)
