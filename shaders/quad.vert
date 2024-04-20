#version 300 es

layout(location = 0) in highp vec2 position;

out highp vec2 texcoords;

void
main()
{
  texcoords = position;

  gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
}
