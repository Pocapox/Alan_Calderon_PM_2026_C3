cmake_minimum_required(VERSION 3.16)
project(PrestamosVehiculos C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cJSON
)

set(CORE_SOURCES
    src/linked_list.c
    src/loan_service.c
    src/json_store.c
    src/report.c
    third_party/cJSON/cJSON.c
)

add_executable(prestamos WIN32
    src/main.c
    src/gui.c
    ${CORE_SOURCES}
)
target_link_libraries(prestamos comctl32 gdi32 m)

enable_testing()
add_executable(test_core
    tests/test_core.c
    ${CORE_SOURCES}
)
target_link_libraries(test_core m)
add_test(NAME CoreTests COMMAND test_core)
