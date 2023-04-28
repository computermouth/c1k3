
#include <GLES2/gl2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>

#define WINDOW_W 960
#define WINDOW_H 540

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader(const GLchar *shaderSrc, GLenum type)
{
    GLuint shader;
    GLint compiled;

// Create the shader object
    shader = glCreateShader(type);
    if(shader == 0)
        return 0;
// Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

// Compile the shader
    glCompileShader(shader);
// Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char* infoLog = malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("Error compiling shader:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
///
// Initialize the shader and program object
//
GLuint Init()
{
    GLchar vShaderStr[] =
        "attribute vec4 vPosition; \n"
        "void main() \n"
        "{ \n"
        " gl_Position = vPosition; \n"
        "} \n";

    GLchar fShaderStr[] =
        "precision mediump float; \n"
        "void main() \n"
        "{ \n"
        " gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
        "} \n";
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

// Load the vertex/fragment shaders
    vertexShader = LoadShader(vShaderStr, GL_VERTEX_SHADER);
    fragmentShader = LoadShader(fShaderStr, GL_FRAGMENT_SHADER);
// Create the program object
    programObject = glCreateProgram();
    if(programObject == 0)
        return 0;
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
// Bind vPosition to attribute 0
    glBindAttribLocation(programObject, 0, "vPosition");
// Link the program
    glLinkProgram(programObject);
// Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char* infoLog = malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            printf("Error linking program:\n%s\n", infoLog);

            free(infoLog);
        }
        glDeleteProgram(programObject);
        return 0;
    }
// Store the program object
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    return programObject;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(GLuint program)
{

    GLfloat vVertices[] = {0.0f, 0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,
                           0.5f, -0.5f, 0.0f
                          };

// Set the viewport
    glViewport(0, 0, WINDOW_W, WINDOW_H);

// Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
// Use the program object
    glUseProgram(program);
// Load the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    // Requires at least OpenGL ES 2.0
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window* window = SDL_CreateWindow("sdl2 gles2 triangle",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          960, 540,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_CreateContext(window);
    GLuint program = Init();

    while (1) {

        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                goto quit;
        }

        Draw(program);
        SDL_GL_SwapWindow(window);

        const char * serror = SDL_GetError();
        while ( strcmp(serror, "") ) {
            printf("sdlerror: %s\n", serror);
            serror = SDL_GetError();
        }

        GLenum gerror = glGetError();
        while (gerror != GL_NO_ERROR) {
            printf("glerror: %x\n", gerror);
            gerror = glGetError();
        }
    }

quit:

    return 0;
}
