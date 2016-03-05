if(CMAKE_CROSSCOMPILING)
    find_program(R2PKG_VAR r2pkg
            HINTS
                ${PROJECT_SOURCE_DIR}
                ${PROJECT_SOURCE_DIR}/build
                ${PROJECT_SOURCE_DIR}/local
                ${REAPING2_DEPS_INSTALL_DIR}
            PATH_SUFFIXES
                bin
                tools/r2pkg)
    if(NOT R2PKG_VAR)
        message(STATUS "R2PKG not found! Data package will not be generated.")
    else(NOT R2PKG_VAR)
        message(STATUS "R2PKG found! Data package will be generated.")
    endif(NOT R2PKG_VAR)
else(CMAKE_CROSSCOMPILING)
    set(R2PKG_VAR r2pkg)
endif(CMAKE_CROSSCOMPILING)

if(R2PKG_VAR)

    file(GLOB_RECURSE data_src "data/*")

    if(WIN32 AND NOT CMAKE_CROSSCOMPILING)
        # Ensure that r2pkg can find its dependent DLLs
        set(SETENV_FOR_UPDATE_DATA
            ${CMAKE_COMMAND} -E env
            "PATH=${REAPING2_DEPS_INSTALL_DIR}/bin$<SEMICOLON>$<TARGET_FILE_DIR:r2pkg>")
    endif(WIN32 AND NOT CMAKE_CROSSCOMPILING)

    add_custom_command(OUTPUT data.pkg
        COMMAND
            ${SETENV_FOR_UPDATE_DATA}
            ${R2PKG_VAR} -c ${CMAKE_CURRENT_SOURCE_DIR}/data
        DEPENDS ${data_src})

    add_custom_target(update_data ALL
        DEPENDS data.pkg
        SOURCES ${data_src})

endif(R2PKG_VAR)