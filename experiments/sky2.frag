STRINGIFY(

#version 300 es

precision highp float;

uniform vec3 cameraPosition;
uniform float yaw;
uniform float pitch;

const int numStars = 100;

in vec2 fragCoord;

out vec4 fragColor;

// Function to generate evenly distributed points on the unit sphere
vec3 generateSpherePoint(float u, float v) {
    float phi = 2.0 * 3.14159265359 * u;
    float theta = 3.14159265359 * v;
    float x = cos(phi) * sin(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(theta);
    return vec3(x, y, z);
}

void main() {
    // Calculate the normalized screen coordinates
    vec2 normalizedCoord = fragCoord / vec2(800.0, 600.0); // Adjust the resolution as needed

    // Create an array of star positions
    vec3 starPositions[numStars];

    // Populate starPositions with evenly distributed points on the unit sphere
    for (int i = 0; i < numStars; i++) {
        float u = float(i) / float(numStars);
        float v = fract(float(i) * 0.61803398875); // Golden ratio to make distribution more uniform
        starPositions[i] = generateSpherePoint(u, v);
    }

    // Transform star positions based on camera's orientation
    mat2 transformationMatrix = mat2(
        cos(yaw), -sin(yaw),
        sin(yaw), cos(yaw)
    ) * mat2(
        cos(pitch), -sin(pitch),
        sin(pitch), cos(pitch)
    );

    for (int i = 0; i < numStars; i++) {
        starPositions[i] = transformationMatrix * starPositions[i];
    }

    // Check if the current fragment is inside any of the star positions
    bool isStar = false;
    for (int i = 0; i < numStars; i++) {
        vec3 starDirection = normalize(starPositions[i]);
        vec3 viewDirection = normalize(cameraPosition - starDirection);
        if (acos(dot(viewDirection, starDirection)) < 0.01) {
            isStar = true;
            break;
        }
    }

    // Output color based on whether it's a star or not
    vec3 color = isStar ? vec3(1.0) : vec3(0.0);

    fragColor = vec4(color, 1.0);
}

)