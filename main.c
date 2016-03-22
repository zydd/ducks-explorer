#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include <glad/glad.h>

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

vec2 c = {-0.737253,4.298468};
vec2 p = {-3.119276,0.0};
float zoom = 0.554781;
float escape = 2.0;
int iterations = 152;
int pre = 0;
float C = 0.98136;

vec2 vc = {0}, vp = {0};
float vz = 1;

bool redraw = false;
float mod_shift = 1;

GLuint program;
GLint uniform_zoom, uniform_p, uniform_c, uniform_C, uniform_iterations, uniform_escape, uniform_ratio, uniform_pre;

void draw(SDL_Window *window)
{
    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
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

    SDL_GL_SwapWindow(window);
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

static void poll_keys(void)
{
    const Uint8 *key = SDL_GetKeyboardState(NULL);

    if (key[SDL_SCANCODE_W]) vp[1] += 2e-2 * zoom * mod_shift;
    if (key[SDL_SCANCODE_S]) vp[1] -= 2e-2 * zoom * mod_shift;
    if (key[SDL_SCANCODE_D]) vp[0] += 2e-2 * zoom * mod_shift;
    if (key[SDL_SCANCODE_A]) vp[0] -= 2e-2 * zoom * mod_shift;
    if (key[SDL_SCANCODE_PERIOD]
     || key[SDL_SCANCODE_Z])     vz = 1.0/0.95;
    if (key[SDL_SCANCODE_COMMA]
     || key[SDL_SCANCODE_X])     vz = 0.95;
    if (key[SDL_SCANCODE_I]) vc[1] += 2e-4 * mod_shift;
    if (key[SDL_SCANCODE_K]) vc[1] -= 2e-4 * mod_shift;
    if (key[SDL_SCANCODE_L]) vc[0] += 2e-4 * mod_shift;
    if (key[SDL_SCANCODE_J]) vc[0] -= 2e-4 * mod_shift;
}

static bool key_event(SDL_Keysym ev)
{
    printf("c(%f,%f) p(%f,%f) z(%f)\n",c[0],c[1],p[0],p[1],zoom);

    switch (ev.sym) {
    case SDLK_ESCAPE:
	return false;
    case SDLK_SPACE:
	vc[0] = 0;
	vc[1] = 0;
	vp[0] = 0;
	vp[1] = 0;
	vz = 1;
	break;
    case SDLK_1: iterations += 1; break;
    case SDLK_q: iterations -= 1; break;
    case SDLK_3: escape *= 1.0/0.97; break;
    case SDLK_e: escape *= 0.97; break;
    case SDLK_4: pre += 1; break;
    case SDLK_r: pre -= 1; break;
    case SDLK_5: C *= (1-1e-2); break;
    case SDLK_t: C *= 1.0/(1-1e-2); break;
    }
    redraw = true;
    mod_shift = ev.mod & KMOD_SHIFT ? 0.1 : 1.0;
    if (ev.mod & KMOD_CTRL) mod_shift *= 4.0;
    return true;
}

int main(void)
{
    SDL_Window *window;
    GLuint vertex_buffer, vertex_shader, fragment_shader;
    GLint attribute_pos;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	perror("SDL_Init() failed.");
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(
	"Ducks",
	SDL_WINDOWPOS_CENTERED,
	SDL_WINDOWPOS_CENTERED,
	640, 480,
	SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window) {
	perror("SDL_CreateWindow() failed.");
        return EXIT_FAILURE;
    }

    SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    SDL_GL_SetSwapInterval(1);

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
    uniform_p          = glGetUniformLocation(program, "center");
    uniform_iterations = glGetUniformLocation(program, "iterations");
    uniform_pre        = glGetUniformLocation(program, "pre");
    uniform_escape     = glGetUniformLocation(program, "escape");
    uniform_ratio      = glGetUniformLocation(program, "ratio");

    glEnableVertexAttribArray(attribute_pos);
    glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertice), 0);

    draw(window);

    while (true) {
	SDL_Event event;
	SDL_WaitEvent(&event);
	do {
	    switch(event.type) {
	    case SDL_WINDOWEVENT: redraw = true; break;
	    case SDL_KEYDOWN:
		if(key_event(event.key.keysym))
		    break;
	    case SDL_QUIT: goto cleanup;
	    }
		
	} while (SDL_PollEvent(&event));

	
        poll_keys();
        update_state();
        if (redraw) {
            redraw = false;
            draw(window);
	    SDL_Event user_event = { .type = SDL_USEREVENT };
	    SDL_PushEvent(&user_event);
        }
    }

cleanup:
    SDL_DestroyWindow(window);
    SDL_Quit();

    exit(EXIT_SUCCESS);
}
