#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

GLuint gProgram;
int gWidth, gHeight;

GLuint gVertexAttribBuffer, gIndexBuffer;

GLint modelingMatrixLoc;
GLint viewingMatrixLoc;
GLint projectionMatrixLoc;
GLint eyePosLoc;
GLint samplesLoc;
GLint coordMultiplierLoc;
GLint lightCountLoc;
GLint lightsPositionLoc[5];
GLint lightsIntensityLoc[5];
GLint bezierLoc;
GLint scaleLoc;
GLint horizCountLoc;
GLint vertiCountLoc;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 2);
glm::vec3 lookAt(0, 0, -1);
glm::vec3 upVec(0, 1, 0);

struct Light {
    glm::vec3 position;
    glm::vec3 intensity;

    Light(GLint xPosIn, GLint yPosIn, GLint zPosIn, \
          GLfloat rIntensityIn, GLfloat gIntensityIn, GLfloat bIntensityIn)
    {
        position = glm::vec3(xPosIn, yPosIn, zPosIn);
        intensity = glm::vec3(rIntensityIn, gIntensityIn, bIntensityIn);
    }
};

int light_count;
std::vector<Light> lights;

int vertical, horizontal;
GLfloat scale;
GLint horizCount;
GLint vertiCount;
std::vector<glm::mat4x4> surfaces;

GLint samples = 10;
GLfloat coordMultiplier = 1.0;
GLfloat rotationAngle = -30.0;

GLdouble lastTime;
GLdouble currentTime;
GLint frameCount;

bool ReadDataFromFile(const std::string& fileName, std::string& data)
{
	std::fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		std::string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	std::string shaderSource;

	std::string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		std::cout << "Cannot find file name: " + filename << std::endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	std::string shaderSource;

	std::string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		std::cout << "Cannot find file name: " + filename << std::endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void parse_input(const std::string& fileName)
{
    std::fstream input_file;
    std::stringstream ss;
	input_file.open(fileName.c_str(), std::ios::in);

    if (input_file.is_open()) 
    {
        std::string curLine;

        getline(input_file, curLine);
        ss.str(curLine);

        ss >> light_count;

        for (int i = 0; i < light_count; i ++) 
        {
            GLint xPosTemp;
            GLint yPosTemp;
            GLint zPosTemp;

            GLfloat rIntensityTemp;
            GLfloat gIntensityTemp;
            GLfloat bIntensityTemp;
            
            getline(input_file, curLine);
            ss.str(curLine);

            ss >> xPosTemp >> yPosTemp >> zPosTemp \
               >> rIntensityTemp >> gIntensityTemp >> bIntensityTemp;

            lights.push_back(Light(xPosTemp, yPosTemp, zPosTemp, \
                                   rIntensityTemp, gIntensityTemp, bIntensityTemp));
        }

        getline(input_file, curLine);
        ss.str(curLine);

        ss >> vertical >> horizontal;

        vertiCount = vertical / 4;
        horizCount = horizontal / 4;
        surfaces = std::vector<glm::mat4x4>(vertiCount * horizCount);
        scale = 1.0 / glm::max(vertiCount, horizCount);

        for (int i = 0; i < vertical; i += 4)
        {
            getline(input_file, curLine);
            ss.str(curLine);

            for (int j = 0; j < horizontal; j += 4) 
            {
                glm::mat4x4& temp = surfaces[(i/4) * (horizontal/4) + j/4];
                ss >> temp[0][0] >> temp[1][0] >> temp[2][0] >> temp[3][0];
            }

            getline(input_file, curLine);
            ss.str(curLine);

            for (int j = 0; j < horizontal; j += 4) 
            {
                glm::mat4x4& temp = surfaces[(i/4) * (horizontal/4) + j/4];
                ss >> temp[0][1] >> temp[1][1] >> temp[2][1] >> temp[3][1];
            }

            getline(input_file, curLine);
            ss.str(curLine);

            for (int j = 0; j < horizontal; j += 4) 
            {
                glm::mat4x4& temp = surfaces[(i/4) * (horizontal/4) + j/4];
                ss >> temp[0][2] >> temp[1][2] >> temp[2][2] >> temp[3][2];
            }

            getline(input_file, curLine);
            ss.str(curLine);

            for (int j = 0; j < horizontal; j += 4) 
            {
                glm::mat4x4& temp = surfaces[(i/4) * (horizontal/4) + j/4];
                ss >> temp[0][3] >> temp[1][3] >> temp[2][3] >> temp[3][3];
            }
        }

        input_file.close();
    }
    else 
    {
        std::cout << "ERROR: Input file could not be opened." << std::endl;
        exit(-1);
    }
}

void init_shaders(void)
{
    gProgram = glCreateProgram();

    GLuint vs = createVS("vert.glsl");
	GLuint fs = createFS("frag.glsl");

    glAttachShader(gProgram, vs);
	glAttachShader(gProgram, fs);

    glLinkProgram(gProgram);
	GLint status;
	glGetProgramiv(gProgram, GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		std::cout << "ERROR: Program link failed." << std::endl;
		exit(-1);
	}

    modelingMatrixLoc = glGetUniformLocation(gProgram, "modelingMatrix");
    viewingMatrixLoc = glGetUniformLocation(gProgram, "viewingMatrix");
    projectionMatrixLoc = glGetUniformLocation(gProgram, "projectionMatrix");
    eyePosLoc = glGetUniformLocation(gProgram, "eyePos");
    samplesLoc = glGetUniformLocation(gProgram, "samples");
    coordMultiplierLoc = glGetUniformLocation(gProgram, "coordMultiplier");
    lightCountLoc = glGetUniformLocation(gProgram, "lightCount");
    
    lightsPositionLoc[0] = glGetUniformLocation(gProgram, "lights[0].position");
    lightsIntensityLoc[0] = glGetUniformLocation(gProgram, "lights[0].intensity");
    lightsPositionLoc[1] = glGetUniformLocation(gProgram, "lights[1].position");
    lightsIntensityLoc[1] = glGetUniformLocation(gProgram, "lights[1].intensity");
    lightsPositionLoc[2] = glGetUniformLocation(gProgram, "lights[2].position");
    lightsIntensityLoc[2] = glGetUniformLocation(gProgram, "lights[2].intensity");
    lightsPositionLoc[3] = glGetUniformLocation(gProgram, "lights[3].position");
    lightsIntensityLoc[3] = glGetUniformLocation(gProgram, "lights[3].intensity");
    lightsPositionLoc[4] = glGetUniformLocation(gProgram, "lights[4].position");
    lightsIntensityLoc[4] = glGetUniformLocation(gProgram, "lights[4].intensity");

    bezierLoc = glGetUniformLocation(gProgram, "bezier");
    scaleLoc = glGetUniformLocation(gProgram, "scale");
    horizCountLoc = glGetUniformLocation(gProgram, "horizCount");
    vertiCountLoc = glGetUniformLocation(gProgram, "vertiCount");
}

void triangulation(void)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    int facesCount = (samples - 1) * (samples - 1) * 2;
	int indexDataSizeInBytes = facesCount * 3 * sizeof(GLuint);
	GLuint* indexData = new GLuint[facesCount * 3];

    for (int i = 0; i < samples - 1; i ++)
    {
        for (int j = 0; j < samples - 1; j ++)
        {
            indexData[2 * 3 * (samples - 1) * i + 2 * 3 * j + 0] = (samples) * i + j;
            indexData[2 * 3 * (samples - 1) * i + 2 * 3 * j + 1] = (samples) * (i + 1) + j;
            indexData[2 * 3 * (samples - 1) * i + 2 * 3 * j + 2] = (samples) * i + j + 1;

            indexData[2 * 3 * (samples - 1) * i + 2 * 3 * j + 3] = (samples) * (i + 1) + j;
            indexData[2 * 3 * (samples - 1) * i + 2 * 3 * j + 4] = (samples) * (i + 1) + j + 1;
            indexData[2 * 3 * (samples - 1) * i + 2 * 3 * j + 5] = (samples) * i + j + 1;
        }
    }

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	delete[] indexData;
}

void init_vbo(void)
{
    GLuint vao;
	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

	glGenBuffers(1, &gIndexBuffer);

	triangulation();
}

void display()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;

	float angleRad = (float)(rotationAngle / 180.0) * M_PI;

	// Compute the modeling matrix
	modelingMatrix = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(1.0, 0.0, 0.0));

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram);
	glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc, 1, glm::value_ptr(eyePos));
    glUniform1i(samplesLoc, samples);
    glUniform1i(horizCountLoc, horizCount);
    glUniform1i(vertiCountLoc, vertiCount);
    glUniform1f(coordMultiplierLoc, coordMultiplier);
    glUniform1f(scaleLoc, scale);
    glUniform1i(lightCountLoc, light_count);
    glUniformMatrix4fv(bezierLoc, surfaces.size(), GL_FALSE, glm::value_ptr(surfaces[0][0]));

    for (int i = 0; i < light_count; i ++) {
        glUniform3fv(lightsPositionLoc[i], 1, glm::value_ptr(lights[i].position));
        glUniform3fv(lightsIntensityLoc[i], 1, glm::value_ptr(lights[i].intensity));
    }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glDrawElementsInstanced(GL_TRIANGLES, (samples - 1) * (samples - 1) * 2 * 3, GL_UNSIGNED_INT, 0, vertiCount * horizCount);
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);
    
    projectionMatrix = glm::perspective(glm::radians(45.0f), w/(float) h, 1.0f, 100.0f);

    viewingMatrix = glm::lookAt(eyePos, lookAt, upVec);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        if (samples < 80)
        {
            samples += 2;
            triangulation();
        }
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        if (samples > 2)
        {
            samples -= 2;
            triangulation();
        }
    }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        coordMultiplier += 0.1;
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        if (coordMultiplier > 0.15)
        {
            coordMultiplier -= 0.1;
        }
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        rotationAngle += 10;
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        rotationAngle -= 10;
    }
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS) //Wireframe mode
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS) //Standard mode
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 800, height = 600;
    window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

    if (!window)
    {
        std::cout << "ERROR: Window creation failed." << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = { 0 };
    strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    parse_input(argv[1]);
    glEnable(GL_DEPTH_TEST);
	init_shaders();
    init_vbo();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, width, height);

    lastTime = glfwGetTime();
    frameCount = 0;
    while (!glfwWindowShouldClose(window))
	{
        currentTime = glfwGetTime();
        frameCount ++;
        if ( currentTime - lastTime >= 1.0 ){
            printf("Samples: %d | FPS: %d\n", samples, frameCount);
            frameCount = 0;
            lastTime += 1.0;
        }
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
