varying vec2 coord;
uniform vec2 c;
uniform int iterations;
uniform float escape;
uniform int pre;
uniform float C;

vec2 clog(vec2 a)
{
	float b =  atan(a.y,a.x);
	if (b > 0.0) b -= 2.0*3.14159265359;
	return vec2(log(length(a)),b);
}

vec2 cmul(vec2 a, vec2 b) {
	return vec2(a.x*b.x -  a.y*b.y, a.x*b.y + a.y*b.x);
}

void main()
{
	vec2 z = coord;
	int i;
	float mean = 0.0;
	for(i = 0; i < iterations; ++i) {
		z = clog(vec2(z.x,abs(z.y))) + c;
		if (i > pre) mean += length(z);
		if (dot(z,z) > escape && i > pre) break;
	}
	mean/=float(i-1);
	float ci =  1.0 - log2(.5*log2(mean*C));
	gl_FragColor = vec4(0.5+0.5*cos(6.0*ci),
	                    0.5+0.5*cos(6.0*ci+0.4),
	                    0.5+0.5*cos(6.0*ci+0.87931), 1.0);
}

