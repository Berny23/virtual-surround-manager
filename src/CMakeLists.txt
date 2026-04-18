add_executable(virtual-surround-manager main.cpp)

target_include_directories(virtual-surround-manager PUBLIC ${PIPEWIRE_INCLUDE_DIRS})
target_compile_options(virtual-surround-manager PUBLIC ${PIPEWIRE_CFLAGS_OTHER})
target_link_libraries(virtual-surround-manager PUBLIC ${PIPEWIRE_LIBRARIES})