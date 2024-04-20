#version 300 es

uniform sampler2D hdr;

uniform highp float scale;

in highp vec2 texcoords;

highp vec4 hdr_output;

void
main()
{
  /* TODO : tone mapping function */
  hdr_output = texture2D(hdr, texcoords) * scale;
}
