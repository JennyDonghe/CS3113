attribute vec4 position;
attribute vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

varying vec2 tex_coord_var;
varying vec2 pix_position;

void main()
{
    vec4 world_position = modelMatrix * position;
    pix_position = vec2(world_position.x, world_position.y);  // for spotlight calc
    tex_coord_var = texCoord;
    gl_Position = projectionMatrix * viewMatrix * world_position;
}
