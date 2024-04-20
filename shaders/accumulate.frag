#version 300 es

uniform sampler2D r_texture;

uniform sampler2D g_texture;

uniform sampler2D b_texture;

uniform sampler2D previous;

in highp vec2 texcoords;

out highp vec4 hdr_output;

void
main()
{
  highp float r = texture(r_texture, texcoords).a;
  highp float g = texture(g_texture, texcoords).a;
  highp float b = texture(b_texture, texcoords).a;
  hdr_output = vec4(texture(previous, texcoords).rgb + vec3(r, g, b), 1.0);
}
