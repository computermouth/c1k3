STRINGIFY(

#version 300 es

precision highp float;

uniform vec3 cameraPosition;
uniform float yaw;
uniform float pitch;

const int numStars = 100;

in vec2 fragCoord;

out vec4 fragColor;

void main() {
    // Calculate the normalized screen coordinates
    vec2 normalizedCoord = fragCoord / vec2(800.0, 600.0); // Adjust the resolution as needed

    // Create a fixed array of star positions (you can specify these)
    vec2 starPositions[numStars];

    // Assign fixed positions to stars (you can adjust these values)
    for (int i = 0; i < numStars; i++) {
        starPositions[i] = vec2(0.0, 0.0); // Set the positions of the stars here
    }

    // Transform star positions based on camera's yaw and pitch
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
        if (length(normalizedCoord - (starPositions[i] * 0.5 + 0.5)) < 0.01) {
            isStar = true;
            break;
        }
    }

    // Output color based on whether it's a star or not
    vec3 color = isStar ? vec3(1.0) : vec3(0.0);

    fragColor = vec4(color, 1.0);
}

)