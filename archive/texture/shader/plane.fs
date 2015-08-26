#version 330

in vec2 tex_coord;
uniform sampler2D tex_sampler;
out vec4 color;

void main()
{
    color = texture(tex_sampler, tex_coord);
}
