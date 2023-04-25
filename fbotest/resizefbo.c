
#include <GLES3/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_video.h>

#define WINDOW_W 320
#define WINDOW_H 180

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
int Init()
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
        return -1;
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
        return -1;
    }
    // Store the program object
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Set the viewport
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    // Use the program object
    glUseProgram(programObject);
    return 0;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw()
{

    GLfloat vVertices[] = {0.0f, 0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,
                           0.5f, -0.5f, 0.0f
                          };

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
    // Load the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    // Requires at least OpenGL ES 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window* window = SDL_CreateWindow("sdl2 gles2 triangle",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_W, WINDOW_H,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_CreateContext(window);
    // SDL_GL_MakeCurrent??

    if (Init() != 0) {
        printf("failed to init\n");
        goto quit;
    }

    GLint default_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_fbo);
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_W, WINDOW_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    Draw();

    unsigned char * pixbuf = calloc(sizeof(char), WINDOW_W * WINDOW_H * 4);

    glReadPixels(
        0, 0,
        WINDOW_W, WINDOW_H,
        GL_RGBA, GL_UNSIGNED_BYTE,
        pixbuf
    );

    // for(int i = 0; i < WINDOW_W * WINDOW_H * 4; i++) {
    //     printf("0x%x ", pixbuf[i]);
    // }
    
    while (1) {

        SDL_Event e;
        int window_change = 0;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                goto quit;
            if(e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                window_change = 1;
        }
        if (window_change) {
            continue;
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        float fw = w;
        float fh = h;
        float dw = WINDOW_W;
        float dh = WINDOW_H;
        int padx = 0;
        int pady = 0;
        float ratio;
        if (fw / dw >= fh / dh){
            ratio = fh / dh;
            padx = (fw - (dw * ratio)) / 2.0f;
        } else {
            ratio = fw / dw;
            pady = (fh - (dh * ratio)) / 2.0f;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, default_fbo);
        // glViewport(0, 0, w, h);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBlitFramebuffer(0, 0, WINDOW_W, WINDOW_H, padx, pady, w - padx, h - pady, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
        GLenum gerror = glGetError();
        while (gerror != GL_NO_ERROR) {
            printf("glerror: %x\n", gerror);
            gerror = glGetError();
        }
    
        SDL_GL_SwapWindow(window);
        const char * serror = SDL_GetError();
        while ( strcmp(serror, "") ) {
            printf("sdlerror: %s\n", serror);
            serror = SDL_GetError();
        }
    }

quit:

    return 0;
}