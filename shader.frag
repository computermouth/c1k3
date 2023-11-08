	"precision highp float;"

	// Vertex positions, normals and uv coords
	"varying vec3 vp,vn;"
	"varying vec2 vt;"

	"uniform sampler2D s;"

	// count of (the vectors) of lights (2* num_lights)
	"uniform int light_count;"

	// Lights [(x,y,z), [r,g,b], ...]
	"uniform vec3 l["STR(R_MAX_LIGHT_V3)"];"

	"void main(void){"
		"gl_FragColor=texture2D(s,vt);"

		// Debug: no textures
		// "gl_FragColor=vec4(.5,.5,.5,1.0);" 

		// Calculate all lights
		"vec3 vl = vec3(0,0,0);"
		"for(int i=0;i<light_count;i+=2) {"
			"vec3 livp = l[i]-vp;"
			"float c_len = length(livp);"
			"float c_dot = dot(vn, normalize(livp));"
			"if ( c_len > 1024. || c_dot < 0.) { continue; }"
			"vl+= c_dot *" 
				"(1./pow(c_len,2.))" // Inverse distance squared
				"*l[i+1];" // Light color/intensity
		"}"

		// Debug: full bright lights
		// "vl = vec3(2,2,2);"

		"gl_FragColor.rgb=floor("
			"gl_FragColor.rgb*pow(vl,vec3(0.75))" // Light, Gamma
			"*16.0+0.5"
		")/16.0;" // Reduce final output color for some extra dirty looks
	"}"
