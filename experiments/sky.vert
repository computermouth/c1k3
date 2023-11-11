
#version 300 es

in vec2 inPosition;

out vec2 fragCoord;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragCoord = inPosition;
}
