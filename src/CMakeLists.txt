cmake_minimum_required(VERSION 3.25)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (LINUX)
    message(STATUS "Build using mupdf.pc")
    set(ENV{PKG_CONFIG_PATH} "${CMAKE_SOURCE_DIR}")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PKG_mupdf REQUIRED IMPORTED_TARGET mupdf)
else ()
    find_package(MuPDF REQUIRED)
endif ()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        farmainwindow.h farmainwindow.cpp
        document.h document.cpp
        graphicspageitem.h graphicspageitem.cpp
        graphicsview.h graphicsview.cpp
        toctreemodel.h toctreemodel.cpp
        popupmenu.h popupmenu.cpp
        )

qt_add_executable(FarPDF
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

target_link_libraries(FarPDF PRIVATE Qt6::Widgets)
if (LINUX)
    target_link_libraries(FarPDF PRIVATE PkgConfig::PKG_mupdf)
else ()
    target_link_libraries(FarPDF PRIVATE ${MUPDF_LIBRARIES})
endif ()

# ALL WARNING IS ERROR FOR GCC
target_compile_options(FarPDF PRIVATE -Wall -Wextra -Werror)

set_target_properties(FarPDF PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        )

qt_finalize_executable(FarPDF)

# Doxygen
find_package(Doxygen
        REQUIRED dot
        OPTIONAL_COMPONENTS mscgen dia)

if(NOT DOXYGEN_FOUND)
    message(WARNING "Doxygen not found - Documentation will not be generated!")
else()
    # output -> build/html
    set(DOXYGEN_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/doxygen")


    doxygen_add_docs(src_doc
        ${PROJECT_SOURCES}
        )
endif()

