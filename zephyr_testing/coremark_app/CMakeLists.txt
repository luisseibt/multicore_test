cmake_minimum_required(VERSION 3.20.0)
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../zephyr_modules/custom_uart)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(coremark)

# Die eigentlichen CoreMark-Dxateien
target_sources(app PRIVATE
    ${ZEPHYR_BASE}/../coremark/core_list_join.c
    ${ZEPHYR_BASE}/../coremark/core_main.c
    ${ZEPHYR_BASE}/../coremark/core_matrix.c
    ${ZEPHYR_BASE}/../coremark/core_state.c
    ${ZEPHYR_BASE}/../coremark/core_util.c
    
    # Wir nutzen hier den "barebones" port, weil wir noch kein POSIX/Threading wollen
    ${ZEPHYR_BASE}/../coremark/barebones/core_portme.c
)

target_include_directories(app PRIVATE 
    ${ZEPHYR_BASE}/../coremark
    ${ZEPHYR_BASE}/../coremark/barebones
)

# Compiler-Flags: 1 Thread, ausreichend Iterationen (z.B. 2000)
target_compile_definitions(app PRIVATE
    ITERATIONS=1
    MULTITHREAD=1
)