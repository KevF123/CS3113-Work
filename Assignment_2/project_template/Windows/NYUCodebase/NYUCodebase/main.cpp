#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "ShaderProgram.h"
#include "Matrix.h"

using namespace std;

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

void DrawText(ShaderProgram *program, int fontTexture, string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

int main(int argc, char *argv[])
{
	cout << "test" << endl;
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	ShaderProgram program2(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint score = LoadTexture("pixel_font.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-5.325, 5.325, -3.0f, 3.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 0.0f;
	float angle = 0.0f;
	float leftPlayer = 0.0f;
	float rightPlayer = 0.0f;
	float ballX = 0.0f;
	float ballY = 0.0f;
	float ballSpeed = 1.7f;
	float paddleSd = 1.9f;
	float degree = 45.0f;
	float negX = 1.0f;
	float negY = 1.0f;
	float rightPlayerTop = rightPlayer + 0.5;
	float rightPlayerBot = rightPlayer - 0.5;
	float leftPlayerTop = leftPlayer + 0.5;
	float leftPlayerBot = leftPlayer - 0.5;
	float ballTop = ballY + 0.15;
	float ballBot = ballY - 0.15;
	float ballLeft = ballX - 0.15;
	float ballRight = ballX + 0.15;
	int lScore = 0;
	int rScore = 0;
	bool start = false;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		float rPlayer[] = { 4.75, -0.5, 5.05, -0.5, 5.05, 0.5, 4.75, -0.5, 5.05, 0.5, 4.75, 0.5 };
		float lPlayer[] = { -5.05, -0.5, -4.75, -0.5, -4.75, 0.5, -5.05, -0.5, -4.75, 0.5, -5.05, 0.5 };
		float ball[] = { -0.15, -0.15, 0.15, -0.15, 0.15, 0.15, -0.15, -0.15, 0.15, 0.15, -0.15, 0.15 };
		float topWall[] = { -5.325, 2.75, 5.325, 2.75, 5.325, 3.00, -5.325, 2.75, 5.325, 3.00, -5.325, 3.00 };
		float botWall[] = { -5.325, -2.75, 5.325, -2.75, 5.325, -3.00, -5.325, -2.75, 5.325, -3.00, -5.325, -3.00 };
		float midWall[] = { -0.05, -2.75, 0.05, -2.75, 0.05, 2.75, -0.05, -2.75, 0.05, 2.75, -0.05, 2.75 };

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		if (keys[SDL_SCANCODE_UP]){
			if (rightPlayer < 2.25f)
				rightPlayer += elasped * paddleSd;
		}
		else if (keys[SDL_SCANCODE_DOWN]){
			if (rightPlayer > -2.25f)
				rightPlayer -= elasped * paddleSd;
		}
		if (keys[SDL_SCANCODE_W]){
			if (leftPlayer < 2.25f)
				leftPlayer += elasped * paddleSd;
		}
		else if (keys[SDL_SCANCODE_S]){
			if (leftPlayer > -2.25f)
				leftPlayer -= elasped * paddleSd;
		}
		
		rightPlayerTop = rightPlayer + 0.5;
		rightPlayerBot = rightPlayer - 0.5;
		leftPlayerTop = leftPlayer + 0.5;
		leftPlayerBot = leftPlayer - 0.5;
		ballTop = ballY + 0.15;
		ballBot = ballY - 0.15;
		ballLeft = ballX - 0.15;
		ballRight = ballX + 0.15;

		if (ballY <= -2.6f){
			negY *= -1.0;
			ballY = -2.59f;
			ballSpeed += 0.05f;
		}
		if (ballY >= 2.6f){
			negY *= -1.0;
			ballY = 2.59f;
			ballSpeed += 0.05f;
		}
		if (!(ballBot > rightPlayerTop) && !(ballTop < rightPlayerBot) && !(ballLeft > rPlayer[2]) && !(ballRight < rPlayer[0])){
			negX *= -1;
			ballSpeed += 0.1f;
		}
		if (!(leftPlayerBot > ballTop) && !(leftPlayerTop < ballBot) && !(lPlayer[0] > ballRight) && !(lPlayer[2] < ballLeft)){
			negX *= -1;
			ballSpeed += 0.1f;
		}
		if (start){
			ballX += cos(degree) * elasped * ballSpeed * negX;
			ballY += sin(degree) * elasped * ballSpeed * negY;
		}
		else
			start = true;
		if (!(ballBot > rightPlayerTop) && !(ballTop < rightPlayerBot) && !(ballLeft > rPlayer[2]) && !(ballRight < rPlayer[0]))
			ballX = rPlayer[0] - 0.16;
		if (!(leftPlayerBot > ballTop) && !(leftPlayerTop < ballBot) && !(lPlayer[0] > ballRight) && !(lPlayer[2] < ballLeft))
			ballX = lPlayer[2] + 0.16;

		//glClearColor(0.4f, 0.4f, 0.4f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT);

		modelMatrix.identity();
		modelMatrix.Translate(0.0, rightPlayer, 0.0);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rPlayer);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		modelMatrix.identity();
		modelMatrix.Translate(0.0, leftPlayer, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, lPlayer);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		modelMatrix.identity();
		modelMatrix.Translate(ballX, ballY, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ball);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		modelMatrix.identity();
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, topWall);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		modelMatrix.identity();
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, botWall);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		modelMatrix.identity();
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, midWall);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		modelMatrix.identity();
		modelMatrix.Translate(-1.0, 2.25, 0.0);
		program2.setModelMatrix(modelMatrix);
		program2.setProjectionMatrix(projectionMatrix);
		program2.setViewMatrix(viewMatrix);
		string test = "";
		test = to_string(lScore) + "-" + to_string(rScore);
		DrawText(&program2, score, test, 0.5f, 0.5f);

		if (ballRight <= -5.325f || ballLeft >= 5.325f){
			if (ballRight <= -5.325f)
				rScore++;
			else if (ballLeft >= 5.325f)
				lScore++;
			ballX = 0.0f;
			ballY = 0.0f;
			negX *= -1.0f;
			int srand(time(NULL));
			int iSecret = rand() % 130 + 20;
			if (iSecret % 2 == 0)
				degree = iSecret;
			else
				degree = iSecret * -1;
			ballSpeed = 1.7f;
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
	
}
