project(PkgExtractor)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Construir GUI Qt6" ON)

find_package(ZLIB 1.3 MODULE)

include_directories(../src)
include_directories(../externals)

add_definitions(-DNTDDI_VERSION=0x0A000006 -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00)

# Qt6 (condicional)
if(BUILD_GUI)
	find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
endif()

# Sources compartilhados
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

# Bibliotecas
add_library(libcryptopp STATIC IMPORTED)
set_property(TARGET libcryptopp PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/cryptopp/cryptopp/libcryptopp.a")

# === Alvo CLI (sempre construído) ===
add_executable(pkg_extractor main.cpp ${EXTRACTOR_SOURCES})
target_link_libraries(pkg_extractor PRIVATE libcryptopp ZLIB::ZLIB)

# === Alvo GUI (condicional) ===
if(BUILD_GUI)
	set(GUI_SOURCES
		gui/main.cpp
		gui/main_window.cpp
		gui/extract_worker.cpp
		gui/settings_manager.cpp
		gui/resources/resources.qrc
	)

	add_executable(pkg_extractor_gui ${GUI_SOURCES} ${EXTRACTOR_SOURCES})
	target_link_libraries(pkg_extractor_gui PRIVATE
		Qt6::Widgets Qt6::Core Qt6::Gui
		libcryptopp ZLIB::ZLIB
	)
endif()