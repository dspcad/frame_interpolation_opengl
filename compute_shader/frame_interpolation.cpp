#include<stdio.h>
#include<stdlib.h>
#include<iostream>

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>

#include <common/shader.hpp>

#include <png.h>
#include <cstring>
#include <bits/stdc++.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <chrono>
#include <cmath>
#include <unistd.h>

using namespace std;


int width;
int height;


int main(){
    if(!glfwInit()){
        fprintf(stderr, "Failed to initialize GLFW!\n");
        getchar();
        return -1;
    }


    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    width  = 1782;
    height = 810;
    GLFWwindow *window = glfwCreateWindow(width, height, "Frame Interpolation",NULL,NULL);
    if(window==NULL){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if(glewInit()!=GLEW_OK){
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window,GLFW_STICKY_KEYS,GL_TRUE);

    glClearColor(0.0f,0.0f,0.4f,0.0f);

 
    GLuint programID = LoadComputeShaders("compute.shader" );
    glUseProgram(programID);


    GLuint out_tex;
    glGenTextures( 1, &out_tex );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, out_tex );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glm::uvec2 work_size( 10, 1 );
    // create empty texture
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, work_size.x, work_size.y, 0, GL_RED, GL_FLOAT, NULL );
    glBindImageTexture( 0, out_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F );
    
    //initialize compute stuff
    glUseProgram( programID );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, out_tex );

    float values[ 10 ] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, work_size.x, work_size.y, 0, GL_RED, GL_FLOAT, values );



     
    while(glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0){
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        glUseProgram( programID );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, out_tex );

        // just keep it simple, 2d work group
        glDispatchCompute( work_size.x, work_size.y, 1 );
        glMemoryBarrier( GL_ALL_BARRIER_BITS );

        unsigned int collection_size = work_size.x * work_size.y;
        std::vector<float> compute_data( collection_size );
        glGetTexImage( GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, compute_data.data() );

        for ( auto d : compute_data ) {
            std::cout << d << " ";
        }
        std::cout << std::endl;


    }

    glDeleteProgram(programID);


    // Close OpenGL window and terminate GLFW
    glfwTerminate();

   
    return 0;
}
