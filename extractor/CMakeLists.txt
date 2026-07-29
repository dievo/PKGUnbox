project(PkgExtractor)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Build Qt6 GUI" ON)

find_package(ZLIB 1.3 MODULE)

include_directories(../src)
include_directories(../externals)

add_definitions(-DNTDDI_VERSION=0x0A000006 -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00)

# Qt6 (optional)
if(BUILD_GUI)
	find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
endif()

# Shared sources
set(EXTRACTOR_SOURCES
	../src/core/loader.cpp
	../src/core/file_format/pkg.cpp
	../src/core/file_format/psf.cpp
	../src/core/file_format/pkg_type.cpp
	../src/core/crypto/crypto.cpp
	../src/common/io_file.cpp
	../src/common/assert.cpp
	../src/common/error.cpp
	../src/common/path_util.cpp
	../src/common/string_util.cpp
)

# Libraries
add_library(libcryptopp STATIC IMPORTED)
set_property(TARGET libcryptopp PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/cryptopp/cryptopp/libcryptopp.a")

# === CLI target (always built) ===
add_executable(pkgunbox main.cpp ${EXTRACTOR_SOURCES})
target_link_libraries(pkgunbox PRIVATE libcryptopp ZLIB::ZLIB)

# === GUI target (optional) ===
if(BUILD_GUI)
	set(GUI_SOURCES
		gui/main.cpp
		gui/main_window.cpp
		gui/extract_worker.cpp
		gui/settings_manager.cpp
		gui/resources/resources.qrc
	)

	# Translation files
	set(TS_FILES
		gui/translations/pkgunbox_pt_BR.ts
		gui/translations/pkgunbox_en.ts
		gui/translations/pkgunbox_es.ts
	)

	add_executable(pkgunbox-gui ${GUI_SOURCES} ${EXTRACTOR_SOURCES} ${TS_FILES})
	target_link_libraries(pkgunbox-gui PRIVATE
		Qt6::Widgets Qt6::Core Qt6::Gui
		libcryptopp ZLIB::ZLIB
	)

	# Qt6 translations support (requires qt6-l10n-tools)
	find_program(QT_LUPDATE_EXECUTABLE NAMES lupdate HINTS /usr/lib/qt6/bin)
	find_program(QT_LRELEASE_EXECUTABLE NAMES lrelease HINTS /usr/lib/qt6/bin)

	if(QT_LUPDATE_EXECUTABLE AND QT_LRELEASE_EXECUTABLE)
		# Compile .ts → .qm files
		set(QM_FILES)
		foreach(TS_FILE ${TS_FILES})
			get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
			set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/translations/${TS_NAME}.qm")
			add_custom_command(
				OUTPUT ${QM_FILE}
				COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/translations"
				COMMAND ${QT_LRELEASE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${TS_FILE} -qm ${QM_FILE}
				DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TS_FILE}
				COMMENT "Compiling ${TS_NAME}.ts → .qm"
			)
			list(APPEND QM_FILES ${QM_FILE})
		endforeach()

		add_custom_target(translations DEPENDS ${QM_FILES})
		add_dependencies(pkgunbox-gui translations)

		# Install .qm files
		install(FILES ${QM_FILES} DESTINATION share/pkgunbox/translations)

		message(STATUS "Translation support enabled (lupdate + lrelease found)")
	else()
		message(STATUS "Translation tools not found - .ts files included but not compiled to .qm")
		message(STATUS "Install qt6-l10n-tools for full translation support")
	endif()
endif()