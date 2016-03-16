#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"


static const struct vertice {
	float x, y;
} vertices[] = {
	{ 1, 1},
	{-1, 1},
	{-1,-1},
	{ 1,-1},
};

GLubyte indices[] = {0, 1, 2, 3};

void compile_shader(const GLuint shader, const char *file)
{
	FILE *f = fopen(file, "rb");
	if (!f) return;
	fseek(f, 0, SEEK_END);
	GLint size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(1 + size * sizeof(char));
	if (!buffer) return;
	fread(buffer, sizeof(char), size, f);
	buffer[size] = '\0';

	fclose(f);

	const GLchar *src[1] = {buffer};
	glShaderSource(shader, 1, src, &size);
	
	free(buffer);

	glCompileShader(shader);
	
	GLint length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	if (length < 5) return;
	
	char *log = malloc(length * sizeof(char));
	glGetShaderInfoLog(shader, length, &length, log);

	puts(file);
	fwrite(log, sizeof(char), length, stdout);
	fflush(stdout);
	putchar('\n');

	free(log);
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

/*
static void mousepos_callback(GLFWwindow *window, double x, double y)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	c[0] = 4 * (x / width - 0.5);
	c[1] = 2 * (0.5 - y / height);
}*/

vec2 c = {-0.737253,4.298468};
vec2 p = {-3.119276,0.0};
float zoom = 0.554781;
float escape = 4.0;
int iterations = 152;
int pre = 1;
float C = 0.98136;

vec2 vc = {0}, vp = {0};
float vz = 1;

bool redraw = false;
float mod_shift = 1;

GLuint program;
GLint uniform_zoom, uniform_p, uniform_c, uniform_C, uniform_iterations, uniform_escape, uniform_ratio, uniform_pre;

void draw(GLFWwindow *window)
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float ratio = (float) width / height;

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);

	glUniform2fv(uniform_c, 1, c);
	glUniform2fv(uniform_p, 1, p);
	glUniform1f(uniform_zoom, zoom);
	glUniform1f(uniform_C, C);
	glUniform1f(uniform_ratio, ratio);
	glUniform1i(uniform_iterations, iterations);
	glUniform1i(uniform_pre, pre);
	glUniform1f(uniform_escape, pow(10.0,escape));

	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indices);

	glfwSwapBuffers(window);
}

static void update_state()
{
	if (vec2_mul_inner(vc,vc) > 1e-10) {
		redraw = true;
		vec2_add(c,c,vc);
		vec2_scale(vc,vc,0.8);
	} else {
		vc[0] = 0;
		vc[1] = 0;
	}

	if (vec2_len(vp) > 1e-2 * 2e-2 * zoom) {
		redraw = true;
		vec2_add(p,p,vp);
		vec2_scale(vp,vp,0.7);
	} else {
		vp[0] = 0;
		vp[1] = 0;
	}

	if (fabs(1-fabs(vz)) > 0.01) {
		redraw = true;
		zoom *= vz;
		vz += 0.5-0.5*vz;
	}
}

static void poll_keys(GLFWwindow *window)
{
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)) vp[1] += 2e-2 * zoom * mod_shift;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)) vp[1] -= 2e-2 * zoom * mod_shift;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)) vp[0] += 2e-2 * zoom * mod_shift;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)) vp[0] -= 2e-2 * zoom * mod_shift;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_PERIOD)
	 || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Z))     vz = 1.0/0.95;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_COMMA)
	 || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_X))     vz = 0.95;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_I)) vc[1] += 2e-4 * mod_shift;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_K)) vc[1] -= 2e-4 * mod_shift;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_L)) vc[0] += 2e-4 * mod_shift;
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_J)) vc[0] -= 2e-4 * mod_shift;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	printf("c(%f,%f) p(%f,%f) z(%f)\n",c[0],c[1],p[0],p[1],zoom);
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			break;
		case GLFW_KEY_SPACE:
			vc[0] = 0;
			vc[1] = 0;
			vp[0] = 0;
			vp[1] = 0;
			vz = 1;
			break;
		case GLFW_KEY_1: iterations += 1; break;
		case GLFW_KEY_Q: iterations -= 1; break;
		case GLFW_KEY_3: escape *= 1.0/0.94; break;
		case GLFW_KEY_E: escape *= 0.94; break;
		case GLFW_KEY_4: pre += 1; break;
		case GLFW_KEY_R: pre -= 1; break;
		case GLFW_KEY_5: C *= (1-1e-2); break;
		case GLFW_KEY_T: C /= (1-1e-2); break;
		}
	redraw = true;
	mod_shift = mods & GLFW_MOD_SHIFT ? 0.1 : 1.0;
	if (mods & GLFW_MOD_CONTROL) mod_shift *= 2.0;
}

int main(void)
{
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader;
	GLint attribute_pos;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Ducks", NULL, NULL);
	
	glfwSetWindowRefreshCallback(window, draw);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
//	glfwSetCursorPosCallback(window, mousepos_callback);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	compile_shader(vertex_shader, "vertex.glsl");

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	compile_shader(fragment_shader, "fragment.glsl");

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	attribute_pos  = glGetAttribLocation(program, "pos");

	uniform_c          = glGetUniformLocation(program, "c");
	uniform_C          = glGetUniformLocation(program, "C");
	uniform_zoom       = glGetUniformLocation(program, "zoom");
	uniform_p     = glGetUniformLocation(program, "center");
	uniform_iterations = glGetUniformLocation(program, "iterations");
	uniform_pre        = glGetUniformLocation(program, "pre");
	uniform_escape     = glGetUniformLocation(program, "escape");
	uniform_ratio      = glGetUniformLocation(program, "ratio");

	glEnableVertexAttribArray(attribute_pos);
	glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertice), 0);

//	glVertexPointer(2, GL_FLOAT, sizeof(struct vertice), (void *) 0);

	while (!glfwWindowShouldClose(window))
	{
		poll_keys(window);
		update_state();
		if (redraw) {
			redraw = false;
			draw(window);
		}
		glfwWaitEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//! [code]

