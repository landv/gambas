
set(GB_NET_SRC_LIST
    main.c tools.c
    CDnsClient.c
    CSocket.c CServerSocket.c CUdpSocket.c
    CSerialPort.c
    CNet.c
)

gb_component(NATIVE gb.net
    SOURCE_LIST ${GB_NET_SRC_LIST}
    SUBCOMPONENTS curl gb.net.smtp gb.net.pop3)
