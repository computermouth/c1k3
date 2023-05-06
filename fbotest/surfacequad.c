#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <GLES3/gl3.h>

TTF_Font * font = NULL;

SDL_Surface* create_checkerboard_surface(int width, int height) {
    font = TTF_OpenFont("RobotoMono-Thin.ttf", 72);
	if ( !font ) {
		printf("Error loading font: %s", TTF_GetError());
	}
    SDL_Color c = {.r = 255, .g = 255, .b = 255, .a = 255};
    SDL_Surface* burf = TTF_RenderText_Solid(font, "ok", c);    
    
    SDL_Surface* surface = SDL_CreateRGBSurface(0, burf->w, burf->h, 32, 0, 0, 0, 0);
    
    SDL_BlitSurface(burf, NULL, surface, NULL);

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create SDL surface: %s", SDL_GetError());
        return NULL;
    }
    return surface;

    const int check_size = 16;
    Uint32* pixels = (Uint32*)surface->pixels;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int tile_x = x / check_size;
            int tile_y = y / check_size;
            Uint32 color = (tile_x + tile_y) % 2 == 0 ? 0xFFFFFFFF : 0xFF000000;
            pixels[x + y * width] = color;
        }
    }

    return surface;
}

GLuint create_shader_from_source(char* source, GLenum type) {
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

int main(int argc, char* argv[]) {
    SDL_Window* window;
    SDL_GLContext context;
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow("SDL2 + GLES3", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    // Create a checkerboard surface
    SDL_Surface* surface = create_checkerboard_surface(256, 256);

    // Create a GL texture from the surface
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    GLuint program = create_program_from_shaders(vertex_shader, fragment_shader);
    glUseProgram(program);
    GLuint position_location = glGetAttribLocation(program, "position");
    GLuint texcoord_location = glGetAttribLocation(program, "texcoord");
    GLuint texture_location = glGetUniformLocation(program, "tex");

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
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
        // Enable the vertex attributes and set the pointers
        glEnableVertexAttribArray(position_location);
        glEnableVertexAttribArray(texcoord_location);
        glVertexAttribPointer(position_location, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
        glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (GLvoid*)(sizeof(GLfloat) * 2));

        // Bind the texture to texture unit 0 and set the uniform
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(texture_location, 0);

        // Set the clear color and clear the screen
        glClearColor(1.0f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the triangle
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(vertices[0]) / 4);

        // Swap the buffers
        SDL_GL_SwapWindow(window);
    }

quit:

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    glDeleteProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    SDL_FreeSurface(surface);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

}