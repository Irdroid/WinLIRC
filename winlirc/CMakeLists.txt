cmake_minimum_required (VERSION 2.8)

project(WinLIRC)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_executable(winlirc WIN32
	config.cpp
	drvdlg.cpp
	globals.cpp
	guicon.cpp
	InputPlugin.cpp
	irconfig.cpp
	irdriver.cpp
	server.cpp
	stdafx.cpp
	trayicon.cpp
	winlirc.cpp

	winlirc.rc

	config.h
	drvdlg.h
	dumpcfg.h
	globals.h
	guicon.h
	InputPlugin.h
	irconfig.h
	irdriver.h
	remote.h
	resource.h
	server.h
	stdafx.h
	trayicon.h
	version.h
	winlirc.h
	)

if (MSVC)
	set_target_properties(winlirc PROPERTIES COMPILE_FLAGS "/Yustdafx.h")
	set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
endif()

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

