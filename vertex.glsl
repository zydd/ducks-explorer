attribute vec2 pos;

uniform float zoom;
uniform float ratio;
uniform vec2 center;

varying vec2 coord;

void main()
{
	gl_Position = vec4(pos, 0.0, 1.0);
	coord = vec2(pos.x*ratio,pos.y)*zoom + center.xy;
}

