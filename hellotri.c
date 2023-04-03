// gcc hello_triangle.cpp -lSDL2 -lGLESv2 && ./a.out
#include <stdbool.h>
#include <SDL2/SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

//~ // Shader sources
//~ const GLchar* vertexSource =
    //~ "attribute vec4 position;    \n"
    //~ "void main()                  \n"
    //~ "{                            \n"
    //~ "   gl_Position = vec4(position.xyz, 1.0);  \n"
    //~ "}                            \n";
//~ const GLchar* fragmentSource =
    //~ "precision mediump float;\n"
    //~ "void main()                                  \n"
    //~ "{                                            \n"
    //~ "  gl_FragColor = vec4 (0.0, 0.0, 1.0, 1.0 );\n"
    //~ "}";

const GLchar* vertexSource =
	"precision highp float;"

	// Vertex positions, normals and uv coords for the fragment shader
	"varying vec3 vp,vn;"
	"varying vec2 vt;"

	// Input vertex positions & normals and blend vertex positions & normals
	"attribute vec3 p,n,p2,n2;"

	// Input UV coords
	"attribute vec2 t;"		

	// Camera position (x, y, z) and aspect ratio (w)
	"uniform vec4 c;" 

	// Model position (x, y, z)
	"uniform vec3 mp;" 

	// Model rotation (yaw, pitch)
	"uniform vec2 mr;" 		

	// Mouse rotation yaw (x), pitch (y)
	"uniform vec2 m;"

	// Blend factor between the two vertex positions
	"uniform float f;"

	// Generate a rotation Matrix around the x,y,z axis;
	// Used for model rotation and camera yaw
	"mat4 rx(float r){"
		"return mat4("
			"1,0,0,0,"
			"0,cos(r),sin(r),0,"
			"0,-sin(r),cos(r),0,"
			"0,0,0,1"
		");"
	"}"

	"mat4 ry(float r){"
		"return mat4("
			"cos(r),0,-sin(r),0,"
			"0,1,0,0,"
			"sin(r),0,cos(r),0,"
			"0,0,0,1"
		");"
	"}"

	"mat4 rz(float r){"
		"return mat4("
			"cos(r),sin(r),0,0,"
			"-sin(r),cos(r),0,0,"
			"0,0,1,0,"
			"0,0,0,1"
		");"
	"}"

	"void main(void){"
		// Rotation Matrixes for model rotation
		"mat4 "
			"mry=ry(mr.x),"
			"mrz=rz(mr.y);"

		// Mix vertex positions, rotate and add the model position
		"vp=(mry*mrz*vec4(mix(p,p2,f),1.)).xyz+mp;"

		// Mix normals
		"vn=(mry*mrz*vec4(mix(n,n2,f),1.)).xyz;"

		// UV coords are handed over to the fragment shader as is
		"vt=t;"

		// Final vertex position is transformed by the projection matrix,
		// rotated around mouse yaw/pitch and offset by the camera position
		// We use a FOV of 90, so the matrix[0] and [5] are conveniently 1.
		// (1 / Math.tan((90/180) * Math.PI / 2) === 1)
		"gl_Position="
			"mat4("
				"1,0,0,0,"
				"0,c.w,0,0,"
				"0,0,1,1,"
				"0,0,-2,0"
			")*" // projection
			"rx(-m.y)*ry(-m.x)*"
			"vec4(vp-c.xyz,1.);"
	"}"
;
const GLchar* fragmentSource =
	"precision highp float;"

	// Vertex positions, normals and uv coords
	"varying vec3 vp,vn;"
	"varying vec2 vt;"

	"uniform sampler2D s;"

	// Lights [(x,y,z), [r,g,b], ...]
	// R_MAX_LIGHT_V3
	"uniform vec3 l[64];"

	"void main(void){"
		"gl_FragColor=texture2D(s,vt);"

		// Debug: no textures
		// "gl_FragColor=vec4(1.0,1.0,1.0,1.0);" 

		// Calculate all lights
		"vec3 vl;"
	    // R_MAX_LIGHT_V3
		"for(int i=0;i<64;i+=2) {"
			"vl+="
				// Angle to normal
				"max("
					"dot("
						"vn, normalize(l[i]-vp)"
					")"
				",0.)*" 
				"(1./pow(length(l[i]-vp),2.))" // Inverse distance squared
				"*l[i+1];" // Light color/intensity
		"}"

		// Debug: full bright lights
		// "vl = vec3(2,2,2);"

		"gl_FragColor.rgb=floor("
			"gl_FragColor.rgb*pow(vl,vec3(0.75))" // Light, Gamma
			"*16.0+0.5"
		")/16.0;" // Reduce final output color for some extra dirty looks
	"}"
;

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    // The OpenGL ES renderer backend in TGUI requires at least OpenGL ES 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // TGUI requires a window created with the SDL_WINDOW_OPENGL flag and an OpenGL context
    SDL_Window* window = SDL_CreateWindow("TGUI example - SDL_GLES2 backend",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          800, 600,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
	glClearColor(0.8f, 0.8f, 0.8f, 1.f);
	
    // Create Vertex Array Object
    //~ GLuint vao;
    //~ glGenVertexArraysOES(1, &vao);
    //~ glBindVertexArrayOES(vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    //~ GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    //~ glEnableVertexAttribArray(posAttrib);
    //~ glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    int a = 1;
    a = a + 1;
    // a = 2
    while(true){
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) exit(0);
        }


        // Clear the screen to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle from the 3 vertices
        glDrawArrays(GL_TRIANGLES, 0, 3);

	GLenum error;
	while (error = glGetError()){
		printf("glerror: %x\n", error);
	}

        SDL_GL_SwapWindow(window);
    };

    return 0;
}
