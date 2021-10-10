cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(CrossTemplate VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions -Xlinker --no-demangle")
set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
    add_definitions(-DCC_TARGET_OS_MAC)
    include_directories(
        Cacao/
        Cacao/include
        Cacao/include/cocos2dx
        Cacao/include/cocos2dx/include
        Cacao/include/cocos2dx/kazmath/include
        Cacao/include/cocos2dx/platform/mac
        Cacao/include/cocos2dx/platform/third_party/mac
        Cacao/include/cocos2dx/platform/third_party/mac/OGLES
        Cacao/include/cocos2dext
        Cacao/include/cocos2dx/custom/Sprites/CCMenuItemSpriteExtra
    )
    add_subdirectory(Cacao)
else() 
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Out")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
    add_definitions(-DCC_TARGET_OS_WIN32)
    include_directories(
        gd.h/cocos2dx
        gd.h/include
        gd.h/
        cocos-headers/
        cocos-headers/extensions
        cocos-headers/cocos2dx
        cocos-headers/cocos2dx/include
        cocos-headers/cocos2dx/kazmath/include
        cocos-headers/cocos2dx/platform/win32
        cocos-headers/cocos2dx/platform/third_party/win32
        cocos-headers/cocos2dx/platform/third_party/win32/OGLES
        minhook/include
    )
    add_subdirectory(minhook EXCLUDE_FROM_ALL)
    set(CMAKE_SIZEOF_VOID_P 4)
    set(CMAKE_SYSTEM_PROCESSOR "i686")
    set(CMAKE_GENERATOR_PLATFORM x86)
endif()

add_library(${PROJECT_NAME} SHARED 
    main.cpp
)

if(APPLE)
    target_link_directories(${PROJECT_NAME} PRIVATE Cacao)
    target_link_libraries(${PROJECT_NAME} Cacao)

    add_custom_command(
        TARGET  ${PROJECT_NAME}
        COMMAND sudo osxinj \"Geometry Dash\" *.dylib || true
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Injecting"
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        minhook
        "${CMAKE_SOURCE_DIR}/cocos-headers/cocos2dx/libcocos2d.lib"
        "${CMAKE_SOURCE_DIR}/cocos-headers/extensions/libExtensions.lib"
    )

    install(
        TARGETS ${PROJECT_NAME} 
        DESTINATION ${CMAKE_BINARY_DIR}/out/
    )
endif()