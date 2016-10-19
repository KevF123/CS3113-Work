#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h> 

using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_GAME_CLEAR };
int state = STATE_MAIN_MENU;

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int itextureID, float iu, float iv, float iwidth, float iheight, float isize){
		textureID = itextureID;
		u = iu;
		v = iv;
		width = iwidth;
		height = iheight;
		size = isize;
	}
	void Draw(ShaderProgram *program);
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

class Vector3 {
public:
	Vector3(float ix, float iy, float iz){
		x = ix;
		y = iy;
		z = iz;
	}
	float x;
	float y;
	float z;
};

class Vector4 {
public:
	Vector4();
	Vector4(float top, float bottom, float left, float right) : top(top), bottom(bottom), left(left), right(right){}
	float top;
	float bottom;
	float left;
	float right;
};

class Entity{
public:
	Entity(Vector3 position, Vector3 velocity, Vector3 size, float rotation, int life, SheetSprite sprite, bool shoot, int score) : position(position),
		velocity(velocity), size(size), rotation(rotation), lives(life), sprite(sprite), canShoot(shoot), score(score), dimensions(Vector4(0.0f, 0.0f, 0.0f, 0.0f)){
		float aspect = sprite.width / sprite.height;
		float sheight = (-0.5f * sprite.size * aspect) - (0.5f * sprite.size * aspect);
		float swidth = (-0.5f * sprite.size) - (0.5f * sprite.size);
		if (sheight < 0)
			sheight *= -1;
		if (swidth < 0)
			swidth *= -1;
		dimensions.top = position.y + (sheight / 2);
		dimensions.bottom = position.y + ((sheight / 2) * -1);
		dimensions.left = position.x + ((swidth / 2) * -1);
		dimensions.right = position.x + (swidth / 2);
	}
	void Draw(){
		float aspect = sprite.width / sprite.height;
		float sheight = (-0.5f * sprite.size * aspect) - (0.5f * sprite.size * aspect);
		float swidth = (-0.5f * sprite.size) - (0.5f * sprite.size);
		if (sheight < 0)
			sheight *= -1;
		if (swidth < 0)
			swidth *= -1;
		dimensions.top = position.y + (sheight / 2);
		dimensions.bottom = position.y + ((sheight / 2) * -1);
		dimensions.left = position.x + ((swidth / 2) * -1);
		dimensions.right = position.x + (swidth / 2);
	};
	Vector3 position;
	Vector3 velocity;
	Vector3 size;
	Vector4 dimensions;
	float rotation;
	int lives;
	SheetSprite sprite;
	bool usedBullet = false;
	bool canShoot = false;
	bool enemyA = true;
	int score;
};

void SheetSprite::Draw(ShaderProgram *program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX, int spriteCountY) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};
	float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f };
	// our regular sprite drawing
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

	GLuint spaceSpriteSheetText = LoadTexture("sheet.png");
	SheetSprite playerSprite = SheetSprite(spaceSpriteSheetText, 112.0f / 1024.0f, 791.0f / 1024.0f, 112.0f / 1024.0f, 75.0f / 1024.0f, 0.25);
	SheetSprite enemyBlue = SheetSprite(spaceSpriteSheetText, 425.0f / 1024.0f, 468.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25);
	SheetSprite enemyGreen = SheetSprite(spaceSpriteSheetText, 425.0f / 1024.0f, 552.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25);
	SheetSprite enemyRed = SheetSprite(spaceSpriteSheetText, 425.0f / 1024.0f, 384.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25);
	SheetSprite bullet = SheetSprite(spaceSpriteSheetText, 856.0f / 1024.0f, 131.0f / 1024.0f, 9.0f / 1024.0f, 37.0f / 1024.0f, 0.25);

	GLuint text = LoadTexture("pixel_font.png");
	string startScreen = "* Press A To Play! *";
	string endScreen = "* You Lose! *";
	string endScreen2 = "* Press Space To Continue! *";
	string endScreen3 = "* You Win! *";
	string life = "Lives: ";
	string score;

	int pS = 0;

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vector<Entity> entities;

	for (int i = 0; i < 49; i++){
		Vector3 enemyStart(-3.0, 1.6, 0.0);
		Vector3 enemySpeed(0.5, 0.05, 0.0);
		float changeX = 0.7;
		float changeY = 0.4;
		int eneS = 5;
		int Ps = 0;
		if (i < 8){
			Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 0), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyGreen, false, eneS);
			entities.push_back(temp);
		}
		else if (i < 16){
			Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 1), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyGreen, false, eneS);
			entities.push_back(temp);
		}
		else if (i < 24){
			Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 2), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyBlue, false, eneS);
			entities.push_back(temp);
		}
		else if (i < 32){
			Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 3), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyBlue, false, eneS);
			entities.push_back(temp);
		}
		else if (i < 40){
			Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 4), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyRed, false, eneS);
			entities.push_back(temp);
		}
		else if (i < 48){
			Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 5), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyRed, true, eneS);
			entities.push_back(temp);
		}
		else if (i < 49){
			Entity temp(Vector3(0.0, -1.8, 0.0), Vector3(2.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), 0.0, 3, playerSprite, true, Ps);
			entities.push_back(temp);
		}
	}

	vector<Entity> bullets;

	for (int i = 0; i < 50; i++){
		Vector3 bulletStart(-3.7, -1.6, 0.0);
		Vector3 bulletSpeed(0.0, 0.0, 0.0);
		float changeX = 0.7;
		float changeY = 0.4;
		int bulS = 2;
		Entity temp(Vector3(bulletStart.x, bulletStart.y, bulletStart.z), Vector3(bulletSpeed.x, bulletSpeed.y, bulletSpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 3, bullet, false, bulS);
		bullets.push_back(temp);
	}
	float lastFrameTicks = 0.0f;
	float angle = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

			const Uint8 *keys = SDL_GetKeyboardState(NULL);

			if (keys[SDL_SCANCODE_UP] && state == STATE_GAME_LEVEL){
				for (int i = 0; i < 50; i++){
					if (bullets[i].usedBullet == false){
						bullets[i].usedBullet = true;
						bullets[i].position.x = entities[48].position.x;
						bullets[i].position.y = entities[48].dimensions.top + ((bullets[i].dimensions.top - bullets[i].dimensions.bottom) / 2);
						bullets[i].velocity.y = 1.0f;
						break;
					}
				}
			}
		}


		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		switch (state) {
		case STATE_MAIN_MENU:
			if (keys[SDL_SCANCODE_A]){
				state = STATE_GAME_LEVEL;
			}
			break;
		case STATE_GAME_LEVEL:
			if (keys[SDL_SCANCODE_LEFT]){
				if (entities[48].dimensions.left > -3.5){
					entities[48].position.x -= elasped * entities[48].velocity.x;
					entities[48].Draw();
				}
			}
			else if (keys[SDL_SCANCODE_RIGHT]){
				if (entities[48].dimensions.right < 3.5){
					entities[48].position.x += elasped * entities[48].velocity.x;
					entities[48].Draw();
				}
			}
			break;
		case STATE_GAME_OVER:
			if (keys[SDL_SCANCODE_SPACE]){
				state = STATE_MAIN_MENU;
			}
			break;
		case STATE_GAME_CLEAR:
			if (keys[SDL_SCANCODE_SPACE]){
				state = STATE_MAIN_MENU;
			}
			break;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		switch (state) {
		case STATE_MAIN_MENU:
			modelMatrix.identity();
			modelMatrix.Translate(-2.3, 0.0, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, text, startScreen, 0.25f, 0.0f);
			pS = 0;
			break;
		case STATE_GAME_LEVEL:
			pS = entities[48].score;
			if (entities[48].lives <= 0)
				state = STATE_GAME_OVER;
			for (int i = 0; i < 49; i++){
				if (i == 48)
					state = STATE_GAME_CLEAR;
				else if (entities[i].enemyA)
					break;
			}
			modelMatrix.identity();
			modelMatrix.Translate(-3.4, 1.9, 0.0);
			program.setModelMatrix(modelMatrix);
			life = "Lives: " + to_string(entities[48].lives) + ", Score: " + to_string(entities[48].score);
			DrawText(&program, text, life, 0.1f, 0.0f);
			for (int i = 0; i < 50; i++){
				for (int x = 0; x < 49; x++){
					if (!(bullets[i].dimensions.bottom > entities[x].dimensions.top) && !(bullets[i].dimensions.top < entities[x].dimensions.bottom) && !(bullets[i].dimensions.left > entities[x].dimensions.right) && !(bullets[i].dimensions.right < entities[x].dimensions.left)){
						if (x == 48 && entities[x].lives > 0){
							bullets[i].usedBullet = false;
							bullets[i].position.x = 3.7f;
							bullets[i].position.y = 1.6f;
							bullets[i].velocity.y = 0.0f;
							entities[x].lives--;
						}
						else{
							bullets[i].usedBullet = false;
							bullets[i].position.x = 3.7f;
							bullets[i].position.y = 1.6f;
							bullets[i].velocity.y = 0.0f;
							entities[x].position.x = 0.0f;
							entities[x].position.y = 3.5f;
							entities[x].velocity.y = 0.0f;
							entities[x].velocity.x = 0.0f;
							entities[x].enemyA = false;
							if (x != 48)
								entities[48].score += entities[x].score;
						}
						if (x < 48 && entities[x].canShoot){
							for (int z = x; z > 0; z -= 8){
								if (entities[z].enemyA){
									entities[z].canShoot = true;
									break;
								}
							}
						}
							
					}
				}
			}
			for (int i = 0; i < 48; i++){
				if (entities[i].dimensions.bottom < -2.0){
					state = STATE_GAME_OVER;
					break;
				}
				if (!(entities[i].dimensions.bottom > entities[48].dimensions.top) && !(entities[i].dimensions.top < entities[48].dimensions.bottom) && !(entities[i].dimensions.left > entities[48].dimensions.right) && !(entities[i].dimensions.right < entities[48].dimensions.left)){
					if (entities[48].lives > 0){
						entities[i].position.x = 0.0f;
						entities[i].position.y = 2.5f;
						entities[i].velocity.y = 0.0f;
						entities[i].velocity.x = 0.0f;
						entities[i].enemyA = false;
						entities[i].lives--;
					}
					else{
						state = STATE_GAME_OVER;
						break;
					}
					if (i < 48 && entities[i].canShoot){
						for (int z = i; z > 0; z -= 8){
							if (entities[z].enemyA){
								entities[z].canShoot = true;
								break;
							}
						}
					}
				}
			}
			for (int i = 0; i < 50; i++){
				for (int x = i + 1; x < 50; x++){
					//if (i != x){
					if (!(bullets[i].dimensions.bottom > bullets[x].dimensions.top) && !(bullets[i].dimensions.top < bullets[x].dimensions.bottom) && !(bullets[i].dimensions.left > bullets[x].dimensions.right) && !(bullets[i].dimensions.right < bullets[x].dimensions.left) && bullets[i].usedBullet && bullets[x].usedBullet && (bullets[i].velocity.y != bullets[x].velocity.y)){
							bullets[i].usedBullet = false;
							bullets[i].position.x = 3.7f;
							bullets[i].position.y = 1.6f;
							bullets[i].velocity.y = 0.0f;
							bullets[x].usedBullet = false;
							bullets[x].position.x = 3.7f;
							bullets[x].position.y = 1.6f;
							bullets[x].velocity.y = 0.0f;
							entities[48].score += bullets[x].score;
						}
					//}
				}
			}
			for (int i = 0; i < 50; i++){
				modelMatrix.identity();
				if (bullets[i].position.y > 2.5 || bullets[i].position.y < -2.5){
					bullets[i].usedBullet = false;
					bullets[i].position.x = 3.7f;
					bullets[i].position.y = 1.6f;
					bullets[i].velocity.y = 0.0f;
				}
				bullets[i].position.y += elasped * bullets[i].velocity.y;
				bullets[i].Draw();
				modelMatrix.Translate(bullets[i].position.x, bullets[i].position.y, bullets[i].position.z);
				program.setModelMatrix(modelMatrix);
				bullets[i].sprite.Draw(&program);
			}
			for (int i = 0; i < 48; i++){
				if (entities[i].dimensions.right > 3.2){
					for (int x = 0; x < 48; x++){
						if (entities[x].enemyA){
							entities[x].velocity.x *= -1;
							entities[x].position.y -= entities[x].velocity.y;
						}
					}
					break;
				}
				if (entities[i].dimensions.left < -3.2){
					for (int x = 0; x < 48; x++){
						if (entities[x].enemyA){
							entities[x].velocity.x *= -1;
							entities[x].position.y -= entities[x].velocity.y;
						}
					}
					break;
				}
			}

			for (int i = 0; i < 49; i++){
				modelMatrix.identity();
				if (i < 48){
					entities[i].position.x += elasped * entities[i].velocity.x;
					entities[i].Draw();

					int srand(time(NULL));

					int chanceShoot = rand() % 1000;

					if (entities[i].canShoot && entities[i].enemyA && chanceShoot < 7){
						for (int x = 0; x < 50; x++){
							if (bullets[x].usedBullet == false){
								bullets[x].usedBullet = true;
								bullets[x].position.x = entities[i].position.x;
								bullets[x].position.y = entities[i].dimensions.bottom - ((bullets[x].dimensions.top - bullets[x].dimensions.bottom) / 2);
								bullets[x].velocity.y = -1.0f;
								break;
							}
						}
					}
				}
				modelMatrix.Translate(entities[i].position.x, entities[i].position.y, entities[i].position.z);
				program.setModelMatrix(modelMatrix);
				entities[i].sprite.Draw(&program);
			}
			break;
		case STATE_GAME_OVER:
			modelMatrix.identity();
			modelMatrix.Translate(-3.0, 0.0, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, text, endScreen, 0.5f, 0.0f);
			modelMatrix.identity();
			modelMatrix.Translate(-3.4, -0.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, text, endScreen2, 0.25f, 0.0f);
			score = "Score: " + to_string(pS);
			modelMatrix.identity();
			modelMatrix.Translate(-1.0, 0.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, text, score, 0.25f, 0.0f);
			entities[48].lives = 3;
			for (int i = 0; i < 49; i++){
				Vector3 enemyStart(-3.0, 1.6, 0.0);
				Vector3 enemySpeed(0.5, 0.05, 0.0);
				float changeX = 0.7;
				float changeY = 0.4;
				int eneS = 5;
				int Ps = 0;
				if (i < 8){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 0), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyGreen, false, eneS);
					entities[i] = temp;
				}
				else if (i < 16){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 1), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyGreen, false, eneS);
					entities[i] = temp;
				}
				else if (i < 24){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 2), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyBlue, false, eneS);
					entities[i] = temp;
				}
				else if (i < 32){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 3), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyBlue, false, eneS);
					entities[i] = temp;
				}
				else if (i < 40){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 4), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyRed, false, eneS);
					entities[i] = temp;
				}
				else if (i < 48){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 5), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyRed, true, eneS);
					entities[i] = temp;
				}
				else if (i < 49){
					Entity temp(Vector3(0.0, -1.8, 0.0), Vector3(2.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), 0.0, 3, playerSprite, true, Ps);
					entities[i] = temp;
				}
			}
			for (int i = 0; i < 50; i++){
				Vector3 bulletStart(-3.7, -1.6, 0.0);
				Vector3 bulletSpeed(0.0, 0.0, 0.0);
				float changeX = 0.7;
				float changeY = 0.4;
				int bulS = 2;
				Entity temp(Vector3(bulletStart.x, bulletStart.y, bulletStart.z), Vector3(bulletSpeed.x, bulletSpeed.y, bulletSpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 3, bullet, false, bulS);
				bullets[i] = temp;
			}
			break;
		case STATE_GAME_CLEAR:
			modelMatrix.identity();
			modelMatrix.Translate(-2.75, 0.0, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, text, endScreen3, 0.5f, 0.0f);
			modelMatrix.identity();
			modelMatrix.Translate(-3.4, -0.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, text, endScreen2, 0.25f, 0.0f);
			score = "Score: " + to_string(pS);
			modelMatrix.identity();
			modelMatrix.Translate(-1.0, 0.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, text, score, 0.25f, 0.0f);
			entities[48].lives = 3;
			for (int i = 0; i < 49; i++){
				Vector3 enemyStart(-3.0, 1.6, 0.0);
				Vector3 enemySpeed(0.5, 0.05, 0.0);
				float changeX = 0.7;
				float changeY = 0.4;
				int eneS = 5;
				int Ps = 0;
				if (i < 8){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 0), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyGreen, false, eneS);
					entities[i] = temp;
				}
				else if (i < 16){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 1), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyGreen, false, eneS);
					entities[i] = temp;
				}
				else if (i < 24){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 2), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyBlue, false, eneS);
					entities[i] = temp;
				}
				else if (i < 32){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 3), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyBlue, false, eneS);
					entities[i] = temp;
				}
				else if (i < 40){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 4), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyRed, false, eneS);
					entities[i] = temp;
				}
				else if (i < 48){
					Entity temp(Vector3(enemyStart.x + (changeX * (i % 8)), enemyStart.y - (changeY * 5), enemyStart.z), Vector3(enemySpeed.x, enemySpeed.y, enemySpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 1, enemyRed, true, eneS);
					entities[i] = temp;
				}
				else if (i < 49){
					Entity temp(Vector3(0.0, -1.8, 0.0), Vector3(2.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), 0.0, 3, playerSprite, true, Ps);
					entities[i] = temp;
				}
			}
			for (int i = 0; i < 50; i++){
				Vector3 bulletStart(-3.7, -1.6, 0.0);
				Vector3 bulletSpeed(0.0, 0.0, 0.0);
				float changeX = 0.7;
				float changeY = 0.4;
				int bulS = 2;
				Entity temp(Vector3(bulletStart.x, bulletStart.y, bulletStart.z), Vector3(bulletSpeed.x, bulletSpeed.y, bulletSpeed.z), Vector3(0.0, 0.0, 0.0), 0.0, 3, bullet, false, bulS);
				bullets[i] = temp;
			}
			break;
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
