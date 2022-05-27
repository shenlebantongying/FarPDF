if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(TARGETS FarPDF DESTINATION bin)
    install(FILES ${CMAKE_SOURCE_DIR}/os_linux/org.slbtty.farpdf.desktop DESTINATION share/applications)
endif()