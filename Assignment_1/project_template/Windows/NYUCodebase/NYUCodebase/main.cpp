#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint pic1Texture = LoadTexture("hillSnow.png");
	GLuint pic2Texture = LoadTexture("hillGrass.png");
	GLuint pic3Texture = LoadTexture("chipBlackWhite.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 0.0f;
	float angle = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClearColor(0.4f, 0.4f, 0.4f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glBindTexture(GL_TEXTURE_2D, pic1Texture);

		float verticesPic1[] = { -0.75, -0.1818, 0.25, -0.1818, 0.25, 0.1818, -0.75, -0.1818, 0.25, 0.1818, -0.75, 0.1818 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesPic1);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoordsPic1[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsPic1);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		modelMatrix.identity();
		modelMatrix.Rotate(angle);
		angle += elasped;
		program.setModelMatrix(modelMatrix);
		modelMatrix.identity();

		glBindTexture(GL_TEXTURE_2D, pic3Texture);

		float verticesPic3[] = { -0.3, -0.1, 0.3, -0.1, 0.3, 0.5, -0.3, -0.1, 0.3, 0.5, -0.3, 0.5 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesPic3);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoordsPic3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsPic3);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		program.setModelMatrix(modelMatrix);
		modelMatrix.identity();

		glBindTexture(GL_TEXTURE_2D, pic2Texture);

		float verticesPic2[] = { -0.25, -0.1818, 0.75, -0.1818, 0.75, 0.1818, -0.25, -0.1818, 0.75, 0.1818, -0.25, 0.1818 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesPic2);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoordsPic2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsPic2);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
