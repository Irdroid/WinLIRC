cmake_minimum_required(VERSION 2.8)

# Not using MFC
set(CMAKE_MFC_FLAG 0)


add_library(Serial SHARED 
	Globals.cpp
	hardware.cpp
	irdriver.cpp
	Serial.cpp
	SerialDialog.cpp
	Settings.cpp
	stdafx.cpp
	Transmit.cpp

	winlirc.rc

	Globals.h
	irdriver.h
	resource.h
	SerialDialog.h
	Settings.h
	stdafx.h
	Transmit.h
	)

if (MSVC)
	set_target_properties(Serial PROPERTIES COMPILE_FLAGS "/Yustdafx.h /Zi")
	set_target_properties(Serial PROPERTIES LINK_FLAGS "/DEBUG")
	set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
endif()

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

# TODO: create a cmake module to find wtl.
include_directories(
	${CMAKE_SOURCE_DIR}/wtl/include
	)

target_link_libraries(Serial
	Common
	)

set_target_properties(Serial PROPERTIES FOLDER "DLL")
