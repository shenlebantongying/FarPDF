cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(MuPDF REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        farmainwindow.h farmainwindow.cpp
        document.h document.cpp
        graphicspageitem.h graphicspageitem.cpp
        graphicsview.h graphicsview.cpp
        toctreemodel.h toctreemodel.cpp
        )

qt_add_executable(FarPDF
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

target_link_libraries(FarPDF PRIVATE
        Qt6::Widgets
        ${MUPDF_LIBRARIES})

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

