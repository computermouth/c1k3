#include <SDL2/SDL.h>
#include <GLES3/gl3.h>

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

const char * sky_frag =
"#version 300 es"
"precision highp float;"
"uniform vec3 cameraPosition;"
"uniform float yaw;"
"uniform float pitch;"
"const int numStars = 100;"
"in vec2 fragCoord;"
"out vec4 fragColor;"
"void main() {"
"    // Calculate the normalized screen coordinates"
"    vec2 normalizedCoord = fragCoord / vec2(800.0, 600.0); // Adjust the resolution as needed"
"    // Create a fixed array of star positions (you can specify these)"
"    vec2 starPositions[numStars];"
"    // Assign fixed positions to stars (you can adjust these values)"
"    for (int i = 0; i < numStars; i++) {"
"        starPositions[i] = vec2(0.0, 0.0); // Set the positions of the stars here"
"    }"
"    // Transform star positions based on camera's yaw and pitch"
"    mat2 transformationMatrix = mat2("
"        cos(yaw), -sin(yaw),"
"        sin(yaw), cos(yaw)"
"    ) * mat2("
"        cos(pitch), -sin(pitch),"
"        sin(pitch), cos(pitch)"
"    );"
"    for (int i = 0; i < numStars; i++) {"
"        starPositions[i] = transformationMatrix * starPositions[i];"
"    }"
"    // Check if the current fragment is inside any of the star positions"
"    bool isStar = false;"
"    for (int i = 0; i < numStars; i++) {"
"        if (length(normalizedCoord - (starPositions[i] * 0.5 + 0.5)) < 0.01) {"
"            isStar = true;"
"            break;"
"        }"
"    }"
"    // Output color based on whether it's a star or not"
"    vec3 color = isStar ? vec3(1.0) : vec3(0.0);"
"    fragColor = vec4(color, 1.0);"
"}";

const char * sky_vert =
"#version 300 es"
"in vec2 inPosition;"
"out vec2 fragCoord;"
"void main() {"
"    gl_Position = vec4(inPosition, 0.0, 1.0);"
"    fragCoord = inPosition;"
"}";

#define WINDOW_W 960
#define WINDOW_H 540

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window* window = SDL_CreateWindow("sky",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          800, 600,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_CreateContext(window);

    // build shader
    GLuint sfrags = create_shader_from_source(sky_frag, GL_FRAGMENT_SHADER);
    GLuint sverts = create_shader_from_source(sky_vert, GL_VERTEX_SHADER);
    GLuint sprogs  = create_program_from_shaders(sverts, sfrags);

    // Access shader uniforms (you should implement functions for this)
    GLint yawUniform = glGetUniformLocation(sprogs, "yaw");
    GLint pitchUniform = glGetUniformLocation(sprogs, "pitch");
    GLint cameraPositionUniform = glGetUniformLocation(sprogs, "cameraPosition");

    float cameraYaw = 0;
    float cameraPitch = 0;
    float cameraPositionX = 0;
    float cameraPositionY = 0;
    float cameraPositionZ = 0;

    while (1) {

        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                goto quit;
        }


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program
        glUseProgram(sprogs);

        // Set shader uniforms (yaw, pitch)
        glUniform1f(yawUniform, cameraYaw);
        glUniform1f(pitchUniform, cameraPitch);
	    glUniform3f(cameraPositionUniform, cameraPositionX, cameraPositionY, cameraPositionZ);

        // Render a fullscreen quad
        glBegin(GL_TRIANGLES);

        glVertex2f(-1.0, -1.0); // bottom-left
        glVertex2f( 1.0, -1.0); // bottom-right
        glVertex2f(-1.0,  1.0); // top-left

        glVertex2f(-1.0,  1.0); // top-left
        glVertex2f( 1.0, -1.0); // bottom-right
        glVertex2f( 1.0,  1.0); // top-right

        glEnd();


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