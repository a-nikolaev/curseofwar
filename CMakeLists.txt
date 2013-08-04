cmake_minimum_required(VERSION 2.8)

PROJECT( curseofwar )

find_package(SDL)
find_package(Curses)

option(CW_NCURSE_FRONTEND "Compile the NCurses frontend" ON)

if(SDL_FOUND)
    option(CW_SDL_FRONTEND "Compile the SDL frontend" ON)
    option(CW_SDL_MULTIPLAYER "Include multiplayer mode support in the SDL frontend" ON)
endif(SDL_FOUND)



SET (SOURCE_COMMON     grid.c 
                       state.c 
                       king.c 
                       output-common.c 
                       path.c 
                       main-common.c)

SET (SOURCE_SDL     output-sdl.c
                    main-sdl.c)

SET (SOURCE_NCURSES output.c
                    main.c)


SET (SOURCE_NETWORK network.c
                    client.c
                    server.c)

SET (SOURCES ${SOURCE_COMMON} ${SOURCE_SDL} ${SOURCE_NCURSES} ${SOURCE_NETWORK})                    
set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )                    

if(WIN32)
    # Windows includes the math lib by default, no need to add them
    SET(COMMON_LIBS)
else(WIN32)
    SET(COMMON_LIBS m)
endif(WIN32)

if(CW_NCURSE_FRONTEND)
    ADD_EXECUTABLE(curseofwar ${SOURCE_COMMON} ${SOURCE_NCURSES} ${SOURCE_NETWORK})
    TARGET_LINK_LIBRARIES( curseofwar ${COMMON_LIBS} ncurses )
endif(CW_NCURSE_FRONTEND)

if(CW_SDL_FRONTEND)
    include_directories(  ${SDL_INCLUDE_DIR}
                          ${INCLUDE_DIRECTORIES}  )
    
    if(CW_SDL_MULTIPLAYER)
        SET(SOURCE_OPTIONAL ${SOURCE_NETWORK})
    endif(CW_SDL_MULTIPLAYER)

    ADD_EXECUTABLE(curseofwar-sdl ${SOURCE_COMMON} ${SOURCE_OPTIONAL} ${SOURCE_SDL})
    if(WIN32)
        # SDL under windows is more comfortable in a console application instead of a windowed one
        set_target_properties(curseofwar-sdl PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif(WIN32)    
    TARGET_LINK_LIBRARIES(curseofwar-sdl ${COMMON_LIBS} ${SDL_LIBRARY} )
endif(CW_SDL_FRONTEND)

