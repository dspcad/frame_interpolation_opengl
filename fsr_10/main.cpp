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

#include "fsr_10/sr/fsr.h"

using namespace std;


GLubyte *textureImage;
vector<GLubyte *> imgs;
vector<vector<int>> wha;
int frame_id = 0;
bool FSR_EN=false;
bool GENERATED=false;

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if(key == GLFW_KEY_X && action == GLFW_PRESS){
	frame_id = (frame_id+1)%10;
        GENERATED=false;
        printf("frame id: %d\n", frame_id);
    }
    else if(key == GLFW_KEY_Z && action == GLFW_PRESS){
	frame_id = (frame_id-1+10)%10;
        GENERATED=false;
        printf("frame id: %d\n", frame_id);

    }
    else if(key == GLFW_KEY_F && action == GLFW_PRESS){
        FSR_EN = !FSR_EN;
        printf(FSR_EN ? "Enable FSR 1.0...\n" : "TURN OFF FSR 1.0...\n");
    }
}


static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


uint32_t createOutputImage(struct FSRConstants fsrData) {
    uint32_t outputImage = 0;
    glGenTextures(1, &outputImage);
    glBindTexture(GL_TEXTURE_2D, outputImage);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, fsrData.output_width, fsrData.output_height);
    glBindTexture(GL_TEXTURE_2D, 0);

    return outputImage;
}

static void runFSR(struct FSRConstants fsrData, uint32_t fsrProgramEASU, uint32_t fsrProgramRCAS, uint32_t fsrData_vbo, uint32_t inputImage, uint32_t outputImage) {
    uint32_t displayWidth = fsrData.output_width;
    uint32_t displayHeight = fsrData.output_height;

    static const int threadGroupWorkRegionDim = 16;
    int dispatchX = (displayWidth + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;
    int dispatchY = (displayHeight + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;

    std::cout << "inputImage: " << inputImage << std::endl;
    std::cout << "outputImage: " << outputImage << std::endl;
    std::cout << "GL_TEXTURE0: " << GL_TEXTURE0 << std::endl;

    // binding point constants in the shaders
    const int inFSRDataPos = 0;
    const int inFSRInputTexture = 1;
    const int inFSROutputTexture = 2;

    { // run FSR EASU
        glUseProgram(fsrProgramEASU);

        // connect the input uniform data
        glBindBufferBase(GL_UNIFORM_BUFFER, inFSRDataPos, fsrData_vbo);

        // bind the input image to a texture unit
        glActiveTexture(GL_TEXTURE0 + inFSRInputTexture);
        glBindTexture(GL_TEXTURE_2D, inputImage);

        // connect the output image
        glBindImageTexture(inFSROutputTexture, outputImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(dispatchX, dispatchY, 1);
        glFinish();
    }

    {
        // FSR RCAS
        // connect the input uniform data
        glBindBufferBase(GL_UNIFORM_BUFFER, inFSRDataPos, fsrData_vbo);

        // connect the previous image's output as input
        glActiveTexture(GL_TEXTURE0 + inFSRInputTexture);
        glBindTexture(GL_TEXTURE_2D, outputImage);

        // connect the output image which is the same as the input image
        glBindImageTexture(inFSROutputTexture, outputImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);


        glUseProgram(fsrProgramRCAS);
        glDispatchCompute(dispatchX, dispatchY, 1);
        glFinish();
    }

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




    /*===================
      FSR parameters
      ==================*/
    struct FSRConstants fsrData = {};
    float  resolutionScale      = 2.0f; // Ultra Quality: 1.3
                                        // Quality:       1.5
                                        // Balanced:      1.7
                                        // Performance:   2.0
    float  sharpness            = 0.0f; // sharpness in range [0-2], 0 is sharpest


    const std::string baseDir = "sr/";

    uint32_t fsrProgramEASU = createFSRComputeProgramEAUS(baseDir);
    uint32_t fsrProgramRCAS = createFSRComputeProgramRCAS(baseDir);
    uint32_t bilinearProgram = createBilinearComputeProgram(baseDir);

    // upload the FSR constants, this contains the EASU and RCAS constants in a single uniform
    // TODO destroy the buffer
    unsigned int fsrData_vbo;
    {
        glGenBuffers(1, &fsrData_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, fsrData_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fsrData), &fsrData, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }



    unsigned int inputTexture;
    // texture
    // ---------
    glGenTextures(1, &inputTexture);
    glBindTexture(GL_TEXTURE_2D, inputTexture); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    uint32_t outputImage = createOutputImage(fsrData);

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


    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int i=0;
    while(!glfwWindowShouldClose(window)) {
        auto start = chrono::high_resolution_clock::now();


        glClear(GL_COLOR_BUFFER_BIT);


	//approach 2: load image from the preloaded array
	textureImage = imgs[frame_id];
        glfwSetWindowSize(window, wha[frame_id][0], wha[frame_id][1]);
        //printf("frame id:%d     w: %d   h: %d\n", frame_id, wh[frame_id].first, wh[frame_id].second);


        if(!FSR_EN){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, inputTexture);
            if(wha[frame_id][2]){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wha[frame_id][0], wha[frame_id][1], 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage);
            }
            else{
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wha[frame_id][0], wha[frame_id][1], 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage);
            }

            glGenerateMipmap(GL_TEXTURE_2D);
       
        }
        else{
            fsrData.input_width   = wha[frame_id][0];
            fsrData.input_height  = wha[frame_id][1];
            fsrData.output_width  = fsrData.input_width  * resolutionScale;
            fsrData.output_height = fsrData.input_height * resolutionScale;

            glBindTexture(GL_TEXTURE_2D, outputImage);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, fsrData.output_width, fsrData.output_height);
            glBindTexture(GL_TEXTURE_2D, 0);

            glfwSetWindowSize(window, fsrData.output_width, fsrData.output_height);

            initFSR(&fsrData, sharpness);
            glBindBuffer(GL_ARRAY_BUFFER, fsrData_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(fsrData), &fsrData, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            if(!GENERATED){
                printf("Running FSR\n");
                runFSR(fsrData, fsrProgramEASU, fsrProgramRCAS, fsrData_vbo, inputTexture, outputImage);
                GENERATED=true;
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, outputImage);
            //FSR_EN=~FSR_EN;
        }

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
