cmake_minimum_required (VERSION 2.8)

project(WinLIRC)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_definitions(
	-DUNICODE
	-D_UNICODE
    -DMONGOOSE_NO_AUTH
    -DMONGOOSE_NO_CGI
    -DMONGOOSE_NO_DAV
    -DMONGOOSE_NO_DIRECTORY_LISTING
    -DMONGOOSE_NO_SSI
    -DMONGOOSE_NO_WEBSOCKET
	)

add_executable(winlirc WIN32
	config.cpp
	drvdlg.cpp
	globals.cpp
	guicon.cpp
    httpserver.cpp
	InputPlugin.cpp
	irconfig.cpp
	irdriver.cpp
    mongoose/mongoose.c
	server.cpp
	trayicon.cpp
	winlirc.cpp

    winlirc.rc

    config.h
    drvdlg.h
    dumpcfg.h
    globals.h
    guicon.h
    httpserver.h
    InputPlugin.h
    irconfig.h
    irdriver.h
    mongoose/mongoose.h
    remote.h
    resource.h
    server.h
    stdafx.h
    trayicon.h
    version.h
    winlirc.h
	)

# TODO: add unit tests
# add_subdirectory(tests)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( winlirc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG} )
endforeach()
