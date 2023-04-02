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
