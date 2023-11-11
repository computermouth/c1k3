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
GLint quad_yaw;
GLint quad_pitch;

void init_quad(){

    // Create a shader program and get the attribute and uniform locations
    GLuint vertex_shader = create_shader_from_source(
                            "#version 300 es\n"
                            "precision mediump float; \n"
                            "in vec2 position; \n"
                            "out vec2 frag_pos; \n"
                            "void main() \n"
                            "{ \n"
                            " frag_pos = position;\n"
                            " gl_Position = vec4(position, 0.0, 1.0); \n"
                            "} \n",
                           GL_VERTEX_SHADER);
    GLuint fragment_shader = create_shader_from_source(
                            "#version 300 es\n"
                            "precision mediump float; \n"
                            "uniform float yaw;\n"
                            "uniform float pitch;\n"
                            "in vec2 frag_pos;\n"
                            "out vec4 color;\n"
                            "void main() \n"
                            "{ \n"
                            " mat2 trans_mat = mat2(cos(yaw), -sin(yaw), sin(yaw), cos(yaw));"
                            " vec2 c_pos = (frag_pos.xy * trans_mat + vec2(1.,1.)) / vec2(2.,2.) * vec2(640.,480.);"
                            " int norm_pos[2];\n"
                            " norm_pos[0] = int(c_pos.x);\n"
                            " norm_pos[1] = int(c_pos.y += pitch);\n"
                            " vec3 oc = vec3(0.1, 0.1, 0.1);"
                            " if(norm_pos[0] % 20 == 0 && norm_pos[1] % 20 == 0)"
                            "   oc = vec3(0.7, 0.7, 0.7);"
                            " color = vec4(oc, 1.0); \n"
                            "} \n",
                             GL_FRAGMENT_SHADER);
    
// GLuint fragment_shader = create_shader_from_source(
//                             "#version 300 es\n"
// "precision mediump float;                                                                          \n"
//                             "out vec4 color;\n"
// "                                                                                                  \n"
// "uniform float pitch;  // Camera pitch angle                                                       \n"
// "uniform float yaw;    // Camera yaw angle                                                         \n"
// "                                                                                                  \n"
// "void main() {                                                                                     \n"
// "    // Normalize frag_position to the range [0, 1]                                                \n"
// "    vec2 normalizedPosition = gl_FragCoord.xy / vec2(800.0, 600.0);                               \n"
// "                                                                                                  \n"
// "    // Map normalized position to [-1, 1] range                                                   \n"
// "    vec2 centeredPosition = normalizedPosition * 2.0 - 1.0;                                       \n"
// "                                                                                                  \n"
// "    // Apply pitch and yaw transformations                                                        \n"
// "    float cosYaw = cos(yaw);                                                                      \n"
// "    float sinYaw = sin(yaw);                                                                      \n"
// "    float cosPitch = cos(pitch);                                                                  \n"
// "    float sinPitch = sin(pitch);                                                                  \n"
// "                                                                                                  \n"
// "    vec3 viewDir = normalize(vec3(cosYaw * cosPitch, sinPitch, sinYaw * cosPitch));               \n"
// "    vec3 rightDir = normalize(cross(viewDir, vec3(0.0, 1.0, 0.0)));                               \n"
// "    vec3 upDir = cross(rightDir, viewDir);                                                        \n"
// "                                                                                                  \n"
// "    vec3 rotatedPosition = centeredPosition.x * rightDir + centeredPosition.y * upDir + viewDir;  \n"
// "                                                                                                  \n"
// "    // Apply field of view (FOV) scaling                                                          \n"
// "    rotatedPosition.xy *= 1.0 / tan(radians(45.0) / 2.0);                                         \n"
// "                                                                                                  \n"
// "    // Output black for the sky background                                                        \n"
// "    vec3 c = vec3(0.0);                                                                       \n"
// "                                                                                                  \n"
// "    // Add white pixels to simulate stars                                                         \n"
// "    float starSize = 0.001;                                                                       \n"
// "    float intensity = smoothstep(starSize, starSize + 0.1, 1.0 - length(rotatedPosition.xy));   \n"
// "                                                                                                  \n"
// "    c += vec3(intensity);                                                                     \n"
// "                                                                                                  \n"
// "    color = vec4(c, 1.0);                                                              \n"
// "}                                                                                                 \n",
// GL_FRAGMENT_SHADER);


    quad_program = create_program_from_shaders(vertex_shader, fragment_shader);
    glUseProgram(quad_program);
    quad_position = glGetAttribLocation(quad_program, "position");
    quad_yaw = glGetUniformLocation(quad_program, "yaw");
    quad_pitch = glGetUniformLocation(quad_program, "pitch");

    GLfloat vertices[] = {
        // bottom right
        // Position
        1.0f, -1.0f,
        -1.0f, -1.0f,
        1.0f,  1.0f,
        // top left
        // Position
        -1.0f,  1.0f,
        1.0f,  1.0f,
        -1.0f, -1.0f
    };
    
    // Create a VBO and upload the vertex data
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
}

typedef struct {
    float yaw;
    float pitch;
} player_t;

void draw_quad(player_t p){
    glUseProgram(quad_program);

    glUniform1f(quad_yaw, p.yaw);
    glUniform1f(quad_pitch, p.pitch);

    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        
    glEnableVertexAttribArray(quad_position);
    glVertexAttribPointer(quad_position, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);

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
    SDL_SetRelativeMouseMode(1);
    
    glEnable(GL_BLEND);
    
    init_quad();

    player_t p = { 0 };

    while (1) {

        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                goto quit;
            if(e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    SDL_SetRelativeMouseMode(0);
                    break;
                }
            }
            if(e.type == SDL_MOUSEMOTION) {
                p.yaw   = fmodf( p.yaw + e.motion.xrel * 0.0015, 3.14159 * 2);
                p.pitch = fmodf( p.pitch + e.motion.yrel * 0.0015, 3.14159 * 2);;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);

        draw_quad(p);
        
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