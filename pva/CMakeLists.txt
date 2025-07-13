file(GLOB PVA_SOURCE *.c)

pkg_check_modules(SDL sdl3 REQUIRED)

add_library(PVAudio SHARED ${PVA_SOURCE})

target_link_libraries(PVAudio PUBLIC ${SDL_LINK_LIBRARIES})
target_include_directories(PVAudio SYSTEM AFTER PRIVATE ${SDL_INCLUDEDIR} api ${CMAKE_SOURCE_DIR}/include/api)