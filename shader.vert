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
