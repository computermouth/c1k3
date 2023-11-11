#include <SDL2/SDL.h>
#include <GLES3/gl3.h>

#define WINDOW_W 640
#define WINDOW_H 480

GLuint create_shader_from_source(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);

    if (!shader) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create GL shader");
        return 0;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        char* log = malloc(log_length);
        glGetShaderInfoLog(shader, log_length, &log_length, log);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile shader: %s", log);
        free(log);

        glDeleteShader(shader);
        return 0;
    }

        GLenum gerror = glGetError();
        while (gerror != GL_NO_ERROR) {
            printf("create_shader_from_source: %x\n", gerror);
            gerror = glGetError();
        }

    return shader;
}

GLuint create_program_from_shaders(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint program = glCreateProgram();

    if (!program) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create GL program");
        return 0;
    }

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        char* log = malloc(log_length);
        glGetProgramInfoLog(program, log_length, &log_length, log);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to link program: %s", log);
        free(log);

        glDeleteProgram(program);
        return 0;
    }

        GLenum gerror = glGetError();
        while (gerror != GL_NO_ERROR) {
            printf("create_program_from_shaders: %x\n", gerror);
            gerror = glGetError();
        }

    return program;
}

GLuint quad_program;
GLuint quad_position;
GLuint quad_vbo;
void init_quad(){

    // Create a shader program and get the attribute and uniform locations
    GLuint vertex_shader = create_shader_from_source(
                            "#version 300 es\n"
                            "precision mediump float; \n"
                            "in vec4 position; \n"
                            "out vec2 screen_pos; \n"
                            "void main() \n"
                            "{ \n"
                            " screen_pos = position.xy * 0.5 + 0.5;\n"
                            " gl_Position = position; \n"
                            "} \n",
                           GL_VERTEX_SHADER);
    GLuint fragment_shader = create_shader_from_source(
                            "#version 300 es\n"
                            "precision mediump float; \n"
                            "in vec2 screen_pos;\n"
                            "out vec4 color;\n"
                            "void main() \n"
                            "{ \n"
                            " color = vec4(screen_pos.x, screen_pos.y, 1.0, 1.0); \n"
                            "} \n",
                             GL_FRAGMENT_SHADER);
    
    quad_program = create_program_from_shaders(vertex_shader, fragment_shader);
    glUseProgram(quad_program);
    quad_position = glGetAttribLocation(quad_program, "position");

    GLfloat vertices[] = {
        // bottom right
        // Position       Texture coordinates
        1.0f, -1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 1.0f,
        1.0f,  1.0f,     1.0f, 0.0f,
        // top left
        // Position       Texture coordinates
        -1.0f,  1.0f,     0.0f, 0.0f,
        1.0f,  1.0f,     1.0f, 0.0f,
        -1.0f, -1.0f,     0.0f, 1.0f,
    };
    
    // Create a VBO and upload the vertex data
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
}

void draw_quad(){
    glUseProgram(quad_program);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        
    glEnableVertexAttribArray(quad_position);
    glVertexAttribPointer(quad_position, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    // Requires at least OpenGL ES 2.0
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window* window = SDL_CreateWindow("gradient_quad",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_W, WINDOW_H,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_CreateContext(window);
    
    glEnable(GL_BLEND);
    
    init_quad();

    while (1) {

        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                goto quit;
        }
        
        glClear(GL_COLOR_BUFFER_BIT);

        draw_quad();
        
        SDL_GL_SwapWindow(window);
        
        const char * serror = NULL;
        while ( strcmp((serror = SDL_GetError()), "") ) {
            fprintf(stderr, "sdlerror: %s\n", serror);
            SDL_ClearError();
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