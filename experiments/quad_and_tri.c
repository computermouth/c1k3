#include <GLES2/gl2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GLES3/gl3.h>

#define WINDOW_W 640
#define WINDOW_H 480

TTF_Font * font = NULL;

SDL_Surface* create_surface() {
    // font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 42);
    font = TTF_OpenFont("/home/computermouth/Downloads/TerminessNerdFontMono-Bold.ttf", 32);
	if ( !font ) {
		printf("Error loading font: %s", TTF_GetError());
	}
    SDL_Color c = {.r = 0, .g = 0, .b = 0, .a = 255};
    SDL_Surface* ammo = TTF_RenderUTF8_Solid(font, "ammo: ∞", c);
    SDL_Surface* health = TTF_RenderUTF8_Solid(font, "♥: 50", c);
    SDL_Surface* c1k3 = TTF_RenderUTF8_Solid(font, "C1K3", c);
    
    int width = WINDOW_W;
    int height = WINDOW_H;
    
    // todo, endianness
    SDL_Surface* surface =SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ABGR8888);
    SDL_FillRect(surface, 0, 0x00000000);
    
    SDL_BlitSurface(ammo, NULL, surface, &(SDL_Rect){
        .x = (width / 4) - ammo->w / 2,
        .y = height - ammo->h - 20,
    });
    
    SDL_BlitSurface(health, NULL, surface, &(SDL_Rect){
        .x = (width / 4 * 3) - health->w / 2,
        .y = height - health->h - 20,
    });
    
    // cant scale-blit 8-bit surface to 32-bit surface, so we upgrade first
    SDL_Surface* doink =SDL_CreateRGBSurfaceWithFormat(0, c1k3->w, c1k3->h, 32, SDL_PIXELFORMAT_ABGR8888);
    SDL_FillRect(doink, 0, 0x00000000);
    SDL_BlitSurface(c1k3, NULL, doink, &(SDL_Rect){
        .x = 0,
        .y = 0,
    });
    // then scale blit
    int scale = 4;
    SDL_BlitScaled(doink, NULL, surface, &(SDL_Rect){
        .x = (surface->w / 2) - doink->w * scale / 2,
        .y = 20,
        .w = doink->w * scale,
        .h = doink->h * scale
    });

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create SDL surface: %s", SDL_GetError());
        return NULL;
    }
    return surface;
}

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

    return program;
}

GLuint quad_program;
GLuint quad_position;
GLuint quad_texcoord;
GLuint quad_tex;
GLuint quad_vbo;
GLuint quad_texture;
void init_quad(){

    // Create a shader program and get the attribute and uniform locations
    GLuint vertex_shader = create_shader_from_source("#version 300 es\n"
                           "in vec4 position;\n"
                           "in vec2 texcoord;\n"
                           "out vec2 v_texcoord;\n"
                           "void main()\n"
                           "{\n"
                           "    gl_Position = position;\n"
                           "    v_texcoord = texcoord;\n"
                           "}\n",
                           GL_VERTEX_SHADER);
    GLuint fragment_shader = create_shader_from_source("#version 300 es\n"
                             "precision mediump float;\n"
                             "in vec2 v_texcoord;\n"
                             "uniform sampler2D tex;\n"
                             "out vec4 color;\n"
                             "void main()\n"
                             "{\n"
                             "    color = texture(tex, v_texcoord);\n"
                             "}\n",
                             GL_FRAGMENT_SHADER);
    
    quad_program = create_program_from_shaders(vertex_shader, fragment_shader);
    glUseProgram(quad_program);
    quad_position = glGetAttribLocation(quad_program, "position");
    quad_texcoord = glGetAttribLocation(quad_program, "texcoord");
    quad_tex = glGetUniformLocation(quad_program, "tex");

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
    
    SDL_Surface* surface = create_surface();
    // Create a GL texture from the surface
    glGenTextures(1, &quad_texture);
    glBindTexture(GL_TEXTURE_2D, quad_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
}

void draw_quad(){
    glUseProgram(quad_program);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        
    glEnableVertexAttribArray(quad_position);
    glEnableVertexAttribArray(quad_texcoord);
    glVertexAttribPointer(quad_position, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
    glVertexAttribPointer(quad_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (GLvoid*)(sizeof(GLfloat) * 2));
        
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, quad_texture);
    glUniform1i(quad_tex, 0);
    //glDrawArrays(GL_TRIANGLES, 0, sizeof(vVertices)/sizeof(vVertices[0]) / 4);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint triangle_program;
GLuint triangle_position;
GLuint triangle_vbo;
void init_triangle(){

    // Create a shader program and get the attribute and uniform locations
    GLuint vertex_shader = create_shader_from_source(
        "attribute vec4 vPosition; \n"
        "void main() \n"
        "{ \n"
        " gl_Position = vPosition; \n"
        "} \n",
        GL_VERTEX_SHADER);
    GLuint fragment_shader = create_shader_from_source(
        "precision mediump float; \n"
        "void main() \n"
        "{ \n"
        " gl_FragColor = vec4(0.5, 0.5, 1.0, 1.0); \n"
        "} \n",
        GL_FRAGMENT_SHADER);
    
    triangle_program = create_program_from_shaders(vertex_shader, fragment_shader);
    glUseProgram(triangle_program);
    triangle_position = glGetAttribLocation(triangle_program, "vPosition");
    
    GLfloat vVertices[] = {0.0f, 0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,
                           0.5f, -0.5f, 0.0f
                          };
    
    // Create a VBO and upload the vertex data
    glGenBuffers(1, &triangle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);
    
}

void draw_triangle(){
    glUseProgram(triangle_program);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glVertexAttribPointer(triangle_position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
    glEnableVertexAttribArray(triangle_position);
    //glDrawArrays(GL_TRIANGLES, 0, sizeof(vVertices)/sizeof(vVertices[0]) / 4);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    TTF_Init();
    // Requires at least OpenGL ES 2.0
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window* window = SDL_CreateWindow("sdl2 gles2 triangle",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_W, WINDOW_H,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_CreateContext(window);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    
    init_quad();
    init_triangle();

    while (1) {

        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                goto quit;
        }
        
        glClear(GL_COLOR_BUFFER_BIT);

        draw_triangle();
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