add_executable(TextureTest
    main.cpp)

target_include_directories(TextureTest PRIVATE ${CMAKE_SOURCE_DIR}/include/
                            PRIVATE ${CMAKE_SOURCE_DIR}/include/managers)
target_link_libraries(TextureTest PRIVATE Texture
    PRIVATE glad_gl_core_33
    PRIVATE OpenGL::GL
    PRIVATE LogManager
    PRIVATE WindowManager)
    
set_target_properties(TextureTest PROPERTIES FOLDER "Tests")