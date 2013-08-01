cmake_minimum_required(VERSION 2.8)

PROJECT( curseofwar )

find_package(SDL)

SET (SOURCE_COMMON     grid.c 
                       state.c 
                       king.c 
                       output-common.c 
                       main-common.c)

SET (SOURCE_SDL     output-sdl.c
                    main-sdl.c)

SET (SOURCE_NCURSES output.c
                    main.c)

SET (SOURCE_NETWORK network.c
                    client.c
                    server.c)

ADD_EXECUTABLE(curseofwar ${SOURCE_COMMON} ${SOURCE_NCURSES} ${SOURCE_NETWORK})
TARGET_LINK_LIBRARIES( curseofwar m ncurses )

if(SDL_FOUND)
    include_directories(  ${SDL_INCLUDE_DIR}
                          ${INCLUDE_DIRECTORIES}  )
    ADD_EXECUTABLE(curseofwar-sdl ${SOURCE_COMMON} ${SOURCE_SDL} ${SOURCE_NETWORK})
    TARGET_LINK_LIBRARIES(curseofwar-sdl m ${SDL_LIBRARY} )
endif(SDL_FOUND)

