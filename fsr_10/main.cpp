#include<stdio.h>
#include<stdlib.h>
#include<iostream>

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>

#include <common/shader.hpp>
#include <common/img_util.hpp>

#include <bits/stdc++.h>
#include <chrono>

using namespace std;


GLubyte *textureImage;
vector<GLubyte *> imgs;
vector<vector<int>> wha;
int frame_id = 0;

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if(key == GLFW_KEY_X && action == GLFW_PRESS){
	frame_id = (frame_id+1)%10;
    }
    else if(key == GLFW_KEY_Z && action == GLFW_PRESS){
	frame_id = (frame_id-1+10)%10;

    }

    printf("frame id: %d\n", frame_id);
}


static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

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

    int width  = 1920;
    int height = 1200;
    GLFWwindow *window = glfwCreateWindow(width, height, "FSR 1.0",NULL,NULL);
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

    GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
    glUseProgram(programID);

    static const GLfloat vertices[] = {
        // positions          // colors           // texture coords
        1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
        1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
       -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices [] {0,1,3,1,2,3};

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    unsigned int texture;
    // texture
    // ---------
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    bool hasAlpha;
    for(int i=1;i<=10;++i){
        GLubyte * img;
        int w, h;
        string digit = to_string(i);
        string f = "";
        for(int j=0;j<6-digit.size();++j){
            f.push_back('0');
        }
        f = f + digit;

	string filename = "./input/"+f+".png";
        bool success = loadPngImage(filename.c_str(), w, h, hasAlpha, &img);
 
        imgs.push_back(img);
        wha.push_back({w,h,hasAlpha});
        cout << filename << "  w: " << wha.back()[0] << "   h: " << wha.back()[1] << "   Alpha: " << wha.back()[2] << endl;
        
        if (!success) {
            std::cout << "Unable to load png file" << std::endl;
            return -1;
        }
    
        std::cout << "Image loaded " << w << " " << h  << std::endl;
    }

    GLint loc = glGetUniformLocation(programID, "iResolution");
    printf("loc %d\n", loc);
    glUniform2f(loc, width, height);


    float seconds {0.0};
    GLint t = glGetUniformLocation(programID, "iTime");
    printf("t: %d\n",t);



    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    while(!glfwWindowShouldClose(window)) {
        auto start = chrono::high_resolution_clock::now();

        seconds = glfwGetTime();
        glUniform1f(t, seconds);

	//printf("time: %f\n",seconds);


        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(programID);




	//approach 2: load image from the preloaded array
	textureImage = imgs[frame_id];
        glfwSetWindowSize(window, wha[frame_id][0], wha[frame_id][1]);
        //printf("frame id:%d     w: %d   h: %d\n", frame_id, wh[frame_id].first, wh[frame_id].second);


        if(wha[frame_id][2])
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wha[frame_id][0], wha[frame_id][1], 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wha[frame_id][0], wha[frame_id][1], 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage);

        glGenerateMipmap(GL_TEXTURE_2D);
       
        glUniform1i(glGetUniformLocation(programID, "iChannel0"), 0);



        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Draw the triangle with VAO
        //glDrawArrays(GL_TRIANGLES, 0, 6); // 3 indices starting at 0 -> 1 triangle
        glUseProgram(programID);
        glEnableVertexAttribArray(0);

        // Draw the triangle with EBO
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 3 indices starting at 0 -> 1 triangle



        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

        //cout << duration.count() << " us " << endl;
	//sum+=duration.count();
    }


    // Cleanup VBO
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(programID);


    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    
    return 0;
}
