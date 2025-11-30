add_executable(MeshTest
    main.cpp)

target_include_directories(MeshTest PRIVATE ${CMAKE_SOURCE_DIR}/include/
                            PRIVATE ${CMAKE_SOURCE_DIR}/include/managers)
target_link_libraries(MeshTest PRIVATE ShaderProgram
    PRIVATE glad_gl_core_33
    PRIVATE OpenGL::GL
    PRIVATE LogManager
    PRIVATE WindowManager
    PRIVATE Texture)
    
set_target_properties(MeshTest PROPERTIES FOLDER "Tests")