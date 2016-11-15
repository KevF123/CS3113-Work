#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
#define TILE_SIZE 0.5f

enum GameState { PLATFORMER, LOSE};
int state = PLATFORMER;

SDL_Window* displayWindow;

ShaderProgram* program;

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

GLuint pic1Texture;
GLuint pic2Texture;
GLuint pic3Texture;

float lastFrameTicks = 0.0f;
float angle = 0.0f;

float ticks;
float elasped;

float fixedElapsed = elasped;

SDL_Event event;
bool done = false;

unsigned char **levelData;

int mapWidth;
int mapHeight;

float leftedge = 0.0f;
float rightedge;

float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

class Vector3 {
public:
	Vector3(float x, float y, float z) : x(x), y(y), z(z){}
	Vector3(){}
	float x;
	float y;
	float z;
};

class Vector4 {
public:
	Vector4(float top, float bottom, float left, float right) : top(top), bottom(bottom), left(left), right(right){}
	Vector4(){}
	float top;
	float bottom;
	float left;
	float right;
};

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width),
	height(height), size(size){}
	void Draw();
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

enum EntityType {
	ENTITY_PLAYER, ENTITY_BLOCK, ENTITY_BACKGROUND, ENTITY_ENEMY, ENTITY_KEY
};

class Entity {
public:
	Entity(SheetSprite sprite, Vector3 position,Vector3 velocity, Vector3 acceleration, Vector3 friction, Vector3 gravity,
		bool isStatic, EntityType entityType) : sprite(sprite), position(position), velocity(velocity), acceleration(acceleration),
		friction(friction), gravity(gravity), isStatic(isStatic), entityType(entityType){
		dimensions.top = position.y;
		dimensions.bottom = position.y - +TILE_SIZE;
		dimensions.left = position.x;
		dimensions.right = position.x + +TILE_SIZE;
	}
	void calDim(){
		dimensions.top = position.y;
		dimensions.bottom = position.y - +TILE_SIZE;
		dimensions.left = position.x;
		dimensions.right = position.x + +TILE_SIZE;
	}
	void allF(){
		collidedTop = false;
		collidedBottom = false;
		collidedLeft = false;
		collidedRight = false;
	}
	void Update(float elapsed);
	void Render();
	bool collidesWith(Entity *entity);
	SheetSprite sprite;
	Vector3 position;
	Vector4 dimensions;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 friction;
	Vector3 gravity;
	float height = TILE_SIZE/2;
	float width = TILE_SIZE/2;
	bool isStatic;
	EntityType entityType;
	bool collidedTop = false;
	bool collidedBottom = true;
	bool collidedLeft = false;
	bool collidedRight = false;
	int sign = 1;
};

vector<Entity> entities;
vector<Entity> enemies;
Entity* Player;
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

void Setup(){
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program->programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
	int spriteCountY) {
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
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void SheetSprite::Draw() {
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

bool Entity::collidesWith(Entity *entity){
	float penetration = 0.0f;
	float yDist = (dimensions.top - (TILE_SIZE / 2)) - (entity->dimensions.top - (TILE_SIZE / 2));
	float xDist = (dimensions.left + (TILE_SIZE / 2)) - (entity->dimensions.left + (TILE_SIZE / 2));
	if (!(entity->dimensions.bottom > dimensions.top) && !(entity->dimensions.top < dimensions.bottom) && !(entity->dimensions.left > dimensions.right) && !(entity->dimensions.right < dimensions.left)){
		if (dimensions.bottom <= entity->dimensions.top){
			if (position.y < 0){
				collidedBottom = true;
				penetration = fabs(position.y - entity->position.y - (height / 2) - (entity->height / 2));
				position.y += penetration + 0.0001f;
				velocity.y = 0.0f;
			}
		}
		else if (dimensions.top >= entity->dimensions.bottom){
			if (position.y >(-TILE_SIZE * mapHeight)){
				collidedTop = true;
				penetration = fabs(entity->position.y - position.y - (entity->height / 2) - (height / 2));
				position.y -= penetration + 0.0001f;
			}
		}
	}
	calDim();
	if (!(entity->dimensions.bottom > dimensions.top) && !(entity->dimensions.top < dimensions.bottom) && !(entity->dimensions.left > dimensions.right) && !(entity->dimensions.right < dimensions.left)){
		if (dimensions.left <= entity->dimensions.right){
			if (position.x < rightedge){
				collidedLeft = true;
				penetration = fabs(position.x - entity->position.x - (width / 2) - (entity->width / 2));
				position.x += penetration + 0.0001f;
				velocity.x = 0.0f;
			}
		}
		else if (dimensions.right >= entity->dimensions.left){
			if (position.x > 0){
				collidedRight = true;
				penetration = fabs(entity->position.x - position.x - (entity->width / 2) - (width / 2));
				position.x -= penetration + 0.0001f;
				velocity.x = 0.0f;
			}
		}
		calDim();
	}
	return collidedBottom || collidedLeft || collidedRight || collidedTop;
}

void Entity::Update(float elapsed){
	velocity.x = lerp(velocity.x, 0.0f, elasped * friction.x);
	velocity.y = lerp(velocity.y, 0.0f, elasped * friction.y);

	velocity.x += acceleration.x * elasped;
	velocity.y += acceleration.y * elasped;

	velocity.x += gravity.x * elasped;
	velocity.y += gravity.y * elasped;

	for (int i = 0; i < enemies.size(); i++){
		enemies[i].Render();
	}
	Player->Render();
}

void Entity::Render(){
	GLfloat texCoords[] = {
		sprite.u, sprite.v,
		sprite.u, sprite.v + sprite.height,
		sprite.u + sprite.width, sprite.v + sprite.height,
		sprite.u, sprite.v,
		sprite.u + sprite.width, sprite.v + sprite.height,
		sprite.u + sprite.width, sprite.v
	};
	float vertices[] = {
		position.x, position.y,
		position.x, (position.y) - TILE_SIZE,
		(position.x) + TILE_SIZE, (position.y) - TILE_SIZE,
		position.x, position.y,
		(position.x) + TILE_SIZE, (position.y) - TILE_SIZE,
		(position.x) + TILE_SIZE, position.y };

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void scrolling(){
	viewMatrix.identity();
	if (Player->position.x > (TILE_SIZE * 7.1f) && Player->position.x < (rightedge - (TILE_SIZE * 7.055f))){
		viewMatrix.Translate(-(Player->position.x), -(Player->position.y), 0.0f);
	}
	else if (Player->position.x >(rightedge - (TILE_SIZE * 7.055f))){
		viewMatrix.Translate(-(rightedge - (TILE_SIZE * 7.055f)), -(Player->position.y), 0.0f);
	}
	else{
		viewMatrix.Translate(-(TILE_SIZE * 7.1f), -(Player->position.y), 0.0f);
	}
	program->setViewMatrix(viewMatrix);
}

// Creating Level
// Creating Level
void placeEntity(string type, float placeX, float placeY){
	if (type == "Player"){
		float u = (float)(((int)98) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)(((int)98) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
		float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
		SheetSprite tempS = SheetSprite(pic2Texture, u, v, spriteWidth, spriteHeight, TILE_SIZE);
		Player = new Entity(tempS, Vector3(placeX, placeY, 0.0f), Vector3(0.5f, 0.5f, 0.5f), Vector3(0.5f, 0.5f, 0.5f), Vector3(0.5f, 0.5f, 0.5f),
			Vector3(1.0f, 1.0f, 1.0f), false, ENTITY_PLAYER);
		Player->Render();
	}
	else if (type == "Enemy"){
		float u = (float)(((int)80) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)(((int)80) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
		float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
		SheetSprite tempS = SheetSprite(pic2Texture, u, v, spriteWidth, spriteHeight, TILE_SIZE);
		Entity tempE(tempS, Vector3(placeX, placeY, 0.0f), Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f),
			Vector3(1.0f, 1.0f, 1.0f), false, ENTITY_ENEMY);
		enemies.push_back(tempE);
		tempE.Render();
	}
	else if (type == "Key"){
		float u = (float)(((int)85) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)(((int)85) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
		float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
		SheetSprite tempS = SheetSprite(pic2Texture, u, v, spriteWidth, spriteHeight, TILE_SIZE);
		Entity tempE(tempS, Vector3(placeX, placeY, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 0.0f), false, ENTITY_ENEMY);
		entities.push_back(tempE);
		tempE.Render();
	}
}

bool readHeader(std::ifstream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
			rightedge = mapWidth * TILE_SIZE;
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool readLayerData(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

bool readEntityData(std::ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str())*TILE_SIZE;
			float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

void readLevel(){
	ifstream infile("SimplePlat.txt");
	string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[Object Layer 1]") {
			readEntityData(infile);
		}
	}
}

void buildMap(){
	vector<float> vertexData;
	vector<float> texCoordData;
	int tiles = 0;
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (levelData[y][x] != 0) {
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				SheetSprite tempS = SheetSprite(pic2Texture, u, v, spriteWidth, spriteHeight, TILE_SIZE);
				if (levelData[y][x] < 36){
					Entity tempE(tempS, Vector3((TILE_SIZE * x) + (TILE_SIZE / 2.0f), (-TILE_SIZE * y) - (TILE_SIZE / 2.0f), 0.0f),
						Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f),
						true, ENTITY_BLOCK);
					entities.push_back(tempE);
				}
				else{
					Entity tempE(tempS, Vector3((TILE_SIZE * x) + (TILE_SIZE / 2.0f), (-TILE_SIZE * y) - (TILE_SIZE / 2.0f), 0.0f),
						Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f),
						true, ENTITY_BACKGROUND);
					entities.push_back(tempE);
				}
				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});
				texCoordData.insert(texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
				});
				tiles++;
			}
		}
	}
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (levelData[y][x] != 0) {
				glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
				glEnableVertexAttribArray(program->positionAttribute);

				glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
				glEnableVertexAttribArray(program->texCoordAttribute);

				glDrawArrays(GL_TRIANGLES, 0, tiles * 6);

				glDisableVertexAttribArray(program->positionAttribute);
				glDisableVertexAttribArray(program->texCoordAttribute);
			}
		}
	}
}

//Testing
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
	Setup();
	readLevel();
	pic2Texture = LoadTexture("sheet.png");
	GLuint text = LoadTexture("pixel_font.png");
	string test = "";

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			const Uint8 *keys = SDL_GetKeyboardState(NULL);

			if (keys[SDL_SCANCODE_LEFT]){
				test = "Pleft: " + to_string(Player->dimensions.left) + ", left: " + to_string(leftedge);
				DrawText(program, text, test, 0.1f, 0.0f);
				if (Player->dimensions.left > leftedge){
					Player->position.x -= Player->velocity.x;
					Player->calDim();
					Player->collidedBottom = false;
				}
			}
			else if (keys[SDL_SCANCODE_RIGHT]){
				test = "PR: " + to_string(Player->dimensions.right) + ", R: " + to_string(rightedge);
				DrawText(program, text, test, 0.1f, 0.0f);
				if (Player->dimensions.right < rightedge){
					Player->position.x += Player->velocity.x;
					Player->calDim();
					Player->collidedBottom = false;
				}
			}
			else if (keys[SDL_SCANCODE_UP] && Player->collidedBottom == true){
				test = "PU: " + to_string(Player->dimensions.bottom);
				DrawText(program, text, test, 0.1f, 0.0f);
				Player->velocity.y = 1.0f;
				Player->position.y += Player->velocity.y;
				Player->calDim();
				Player->collidedBottom = false;
			}
		}
		
		switch (state) {
		case PLATFORMER:
			glClearColor(0.4f, 1.0f, 1.0f, 0.5f);
			glClear(GL_COLOR_BUFFER_BIT);

			program->setModelMatrix(modelMatrix);
			program->setProjectionMatrix(projectionMatrix);
			program->setViewMatrix(viewMatrix);

			ticks = (float)SDL_GetTicks() / 1000.0f;
			elasped = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			fixedElapsed = elasped;
			if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
				fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
			}
			while (fixedElapsed >= FIXED_TIMESTEP) {
				fixedElapsed -= FIXED_TIMESTEP;
				Player->Update(FIXED_TIMESTEP);
				for (int i = 0; i < enemies.size(); i++){
					enemies[i].Update(FIXED_TIMESTEP);
				}
			}
			Player->Update(fixedElapsed);
			for (int i = 0; i < enemies.size(); i++){
				enemies[i].Update(fixedElapsed);
			}

			glBindTexture(GL_TEXTURE_2D, pic2Texture);
			buildMap();

			if (Player->dimensions.right >= rightedge){
				test = "TL";
				DrawText(program, text, test, 0.1f, 0.0f);
				Player->position.x = rightedge - TILE_SIZE - 0.0001f;
				Player->calDim();
			}
			else if (Player->dimensions.left <= leftedge){
				test = "TR";
				DrawText(program, text, test, 0.1f, 0.0f);
				Player->position.x = 0.0001f;
				Player->calDim();
				test = "TR : " + to_string(Player->position.x);
				DrawText(program, text, test, 0.1f, 0.0f);
			}
			for (int i = 0; i < entities.size(); i++){
				if (entities[i].entityType == ENTITY_BLOCK){
					Player->collidesWith(&entities[i]);
				}
			}
			if (Player->collidedBottom == false){
				Player->collidedTop = false;
				Player->position.y -= elasped * Player->velocity.y;
				Player->calDim();
			}
			for (int i = 0; i < enemies.size(); i++){
				if (Player->collidesWith(&enemies[i])){
					//state = LOSE;
					//break;
				}
				if (enemies[i].position.x > 0 && enemies[i].position.x < rightedge){
					enemies[i].position.x += 0.1f * enemies[i].sign;
				}
				else{
					enemies[i].sign *= -1;
				}
			}
			scrolling();
			for (int i = 0; i < enemies.size(); i++){
				enemies[i].Render();
			}
			Player->Render();
			break;
		case LOSE:
			glClear(GL_COLOR_BUFFER_BIT);

			program->setModelMatrix(modelMatrix);
			program->setProjectionMatrix(projectionMatrix);
			program->setViewMatrix(viewMatrix);

			test = "YOU LOSE";
			DrawText(program, text, test, 0.1f, 0.0f);
			break;
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
