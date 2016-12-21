#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>       
#include <SDL_mixer.h>

using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

enum GameState { STATE_MAIN_MENU, STATE_LEVEL_MENU, STATE_GAME_LOSE, STATE_GAME_WIN, STATE_LEVEL_ONE, STATE_LEVEL_TWO, STATE_LEVEL_THREE, STATE_GAME_MULTI, STATE_GAME_WINP1, STATE_GAME_WINP2 };
int state = STATE_MAIN_MENU;
int old_state = STATE_LEVEL_ONE;

//Tile Stuff
enum types { GAME_TILE, GAME_BALL, GAME_PLAYER, GAME_WALL };

//Sound Stuff
Mix_Chunk *hitSound;
Mix_Chunk *explodingSound;
Mix_Chunk *hoverSound;
Mix_Chunk *shootSound;
Mix_Music *music;

//MultiPlayer Stuff
bool exploded = false;
bool exploded2 = false;

//Color Stuff
enum colors { BALL_WHITE, BALL_BLUE, BALL_GREEN, BALL_RED, BALL_YELLOW };
vector<string> ballColorImg = { "whiteBall.png", "blueBall.png", "greenBall.png", "redBall.png", "yellowBall.png" };
vector<string> ballColors = { "white", "blue", "green", "red", "yellow" };

class Color {
public:
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a){}
	Color(){}
	float r;
	float g;
	float b;
	float a;
};

Color sWhite(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f);
Color eWhite(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 0.0f);

Color sBlue(0.0f, 97.0f / 255.0f, 255.0f / 255.0f, 1.0f);
Color eBlue(122.0f / 255.0f, 170.0f / 255.0f, 255.0f / 255.0f, 0.0f);

Color sGreen(38.0f / 255.0f, 255.0f / 255.0f, 0.0f, 1.0f);
Color eGreen(160.0f / 255.0f, 255.0f / 255.0f, 140.0f / 255.0f, 0.0f);

Color sRed(255.0f / 255.0f, 0.0f, 0.0f, 1.0f);
Color eRed(255.0f / 255.0f, 130.0f / 255.0f, 130.0f / 255.0f, 0.0f);

Color sYellow(255.0f / 255.0f, 245.0f / 255.0f, 0.0f, 1.0f);
Color eYellow(255.0f / 255.0f, 250.0f / 255.0f, 153.0f / 255.0f, 0.0f);

vector<Color> startColors = { sWhite, sBlue, sGreen, sRed, sYellow };
vector<Color> endColors = { eWhite, eBlue, eGreen, eRed, eYellow };

string life;

SDL_Window* displayWindow;

ShaderProgram* program;
ShaderProgram* program2;

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

//Tile
string tile = "trans";

//Ball
GLuint balltexture;

//Ball Size
float ballWidth = 0.25f;
float ballHeight = 0.25f;
float ballHalfWidth = ballWidth / 2.0f;
float ballHalfHeight = ballHeight / 2.0f;

//Grid Size
int gridX = 15;
int gridY = 14;

//Pics
GLuint hill;
GLuint player;
GLuint line;

float hillV[] = { -0.3, -0.1, 0.3, -0.1, 0.3, 0.5, -0.3, -0.1, 0.3, 0.5, -0.3, 0.5 };
float hillT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float playerV[] = { -0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25 };
float playerT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float lineV[] = { -0.01, 0.0, 0.01, 0.0, 0.01, 0.5, -0.01, 0.0, 0.01, 0.5, -0.01, 0.5 };
float lineT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

GLuint text;

GLuint titleSign;
GLuint playButton;
GLuint multiButton;
GLuint quitButton;
GLuint backButton;
GLuint oneButton;
GLuint twoButton;
GLuint threeButton;
GLuint retryButton;
GLuint menuButton;
GLuint nextButton;
GLuint loseSign;
GLuint winSign;
GLuint P1WinSign;
GLuint P2WinSign;
GLuint mainBack;
GLuint loseBack;
GLuint winBack;


//All Menu
float vtSign[] = { -1.75, 0.5, 1.75, .5, 1.75, 2.0, -1.75, 0.5, 1.75, 2.0, -1.75, 2.0 };
float tctSign[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float vbackR[] = { -3.55, -2.0, 3.55, -2.0, 3.55, 2.0, -3.55, -2.0, 3.55, 2.0, -3.55, 2.0 };
float tbackR[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

//Main Menu
float vpBut[] = { -3.4, -1.0, -1.9, -1.0, -1.9, -0.7, -3.4, -1.0, -1.9, -0.7, -3.4, -0.7 };
float tcpBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float vmBut[] = { -3.4, -1.4, -1.9, -1.4, -1.9, -1.1, -3.4, -1.4, -1.9, -1.1, -3.4, -1.1 };
float tcmBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float vqBut[] = { -3.4, -1.8, -1.9, -1.8, -1.9, -1.5, -3.4, -1.8, -1.9, -1.5, -3.4, -1.5 };
float tcqBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

//LEVEL Menu
float voneBut[] = { -3.4, -0.6, -1.9, -0.6, -1.9, -0.3, -3.4, -0.6, -1.9, -0.3, -3.4, -0.3 };
float tconeBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float vtwoBut[] = { -3.4, -1.0, -1.9, -1.0, -1.9, -0.7, -3.4, -1.0, -1.9, -0.7, -3.4, -0.7 };
float tctwoBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float vthreeBut[] = { -3.4, -1.4, -1.9, -1.4, -1.9, -1.1, -3.4, -1.4, -1.9, -1.1, -3.4, -1.1 };
float tcthreeBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float vbBut[] = { -3.4, -1.8, -1.9, -1.8, -1.9, -1.5, -3.4, -1.8, -1.9, -1.5, -3.4, -1.5 };
float tcbBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

//Retry
float vrBut[] = { -0.9, -0.3, -0.1, -0.3, -0.1, 0.15, -0.9, -0.3, -0.1, 0.15, -0.9, 0.15 };
float tcrBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

//LOSE Menu
float vlBut[] = { 0.1, -0.3, 0.9, -0.3, 0.9, 0.15, 0.1, -0.3, 0.9, 0.15, 0.1, 0.15 };
float tclBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

//WIN Menu
float vnBut[] = { 0.1, -0.3, 0.9, -0.3, 0.9, 0.15, 0.1, -0.3, 0.9, 0.15, 0.1, 0.15 };
float tcnBut[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float unitX;
float unitY;

float lastFrameTicks = 0.0f;
float angle = 0.0f;

float ticks;
float elasped;

float fixedElapsed = elasped;

bool in = false;
bool in1 = false;
bool in2 = false;
bool in3 = false;
bool in4 = false;
bool in5 = false;
bool in6 = false;
bool in7 = false;
bool in8 = false;
bool in9 = false;
bool in10 = false;

SDL_Event event;
bool done = false;

//Support Classes
float lerp(float v0, float v1, float t){
	return (1.0 - t)*v0 + t*v1;
}

class Vector3 {
public:
	Vector3(float x, float y, float z) : x(x), y(y), z(z){}
	Vector3(){}
	float distance(Vector3 &b){
		float squareX = pow((b.x - x), 2);
		float squareY = pow((b.y - y), 2);
		float d = sqrt(squareX + squareY);
		return d;
	}
	float x;
	float y;
	float z;
};

class Vector2{
public:
	Vector2(float x, float y) : x(x), y(y){}
	void normalize(){
		float length = sqrt(x*x + y*y);
		x /= length;
		y /= length;
	}
	float x;
	float y;
};

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

class Particle {
public:
	Particle(Vector3 position, Vector3 velocity, Vector3 origV, float lifetime) : position(position), velocity(velocity), origV(origV), lifetime(lifetime){}
	Vector3 position;
	Vector3 velocity;
	Vector3 origV;
	float lifetime;
};

class ParticleEmitter {
public:
	ParticleEmitter(unsigned int particleCount, Color start, Color end, Vector3 position) : position(position){
		maxLifetime = 2.0;
		srand(time(NULL));
		startColor = start;
		endColor = end;
		velocity = Vector3(0.0f, 0.0f, 0.0f);
		velocityDeviation = Vector3(3.0f, 3.0f, 3.0f);
		for (int i = 0; i < particleCount; i++){
			float random = ((float)rand()) / (float)RAND_MAX;
			float relativeLifetime = random / maxLifetime;
			random *= maxLifetime;
			float random2 = ((float)rand()) / (float)RAND_MAX;
			float relativeLifetime2 = random2 / maxLifetime;
			random2 *= maxLifetime;
			float x;
			if (i % 2 == 0)
				x = lerp(velocity.x, velocityDeviation.x, relativeLifetime);
			else
				x = lerp(velocity.x, -velocityDeviation.x, relativeLifetime);
			float y = lerp(velocity.y, velocityDeviation.y, relativeLifetime2);
			Particle tmp(position, Vector3(x, y, 0.0f), Vector3(x, y, 0.0f), random);
			particles.push_back(tmp);
		}
		for (int i = 0; i < particles.size(); i++) {
			float relativeLifetime = (particles[i].lifetime / maxLifetime);
			particleColors.push_back(lerp(startColor.r, endColor.r, relativeLifetime));
			particleColors.push_back(lerp(startColor.g, endColor.g, relativeLifetime));
			particleColors.push_back(lerp(startColor.b, endColor.b, relativeLifetime));
			particleColors.push_back(lerp(startColor.a, endColor.a, relativeLifetime));
		}
	}
	ParticleEmitter(){
		srand(time(NULL));
		for (int i = 0; i < 25; i++){
			float random = ((float)rand()) / (float)RAND_MAX;
			random *= maxLifetime;
			Particle tmp(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), random);
			particles.push_back(tmp);
		}
	}
	void Update(float elapsed);
	void Render();
	Vector3 position;
	Vector3 gravity = Vector3(0.0f, -1.0f, 0.0f);
	Vector3 velocity;
	Vector3 velocityDeviation;
	Color startColor;
	Color endColor;
	float maxLifetime = 3.0f;
	float timePassed = 0.0f;
	vector<Particle> particles;
	vector<float> particleColors;
};

ParticleEmitter explosions(100, startColors[0], endColors[0], Vector3(3.0f, 3.0f, 0.0f));

void ParticleEmitter::Update(float elasped){
	for (int i = 0; i < particles.size(); i++){
		if (particles[i].lifetime > maxLifetime){
			particles[i].lifetime = fmod(particles[i].lifetime, maxLifetime);
			particles[i].position.x = position.x;
			particles[i].position.y = position.y;
			particles[i].velocity.x = particles[i].origV.x;
			particles[i].velocity.y = particles[i].origV.y;
		}
		else{
			particles[i].lifetime += elasped;
			particles[i].velocity.x += gravity.x * elasped;
			particles[i].velocity.y += gravity.y * elasped;
			particles[i].position.x += particles[i].velocity.x * elasped;
			particles[i].position.y += particles[i].velocity.y * elasped;
		}

		float relativeLifetime = (particles[i].lifetime / maxLifetime);
		particleColors[(i * 4) + 0] = lerp(startColor.r, endColor.r, relativeLifetime);
		particleColors[(i * 4) + 1] = lerp(startColor.g, endColor.g, relativeLifetime);
		particleColors[(i * 4) + 2] = lerp(startColor.b, endColor.b, relativeLifetime);
		particleColors[(i * 4) + 3] = lerp(startColor.a, endColor.a, relativeLifetime);
	}
}

void ParticleEmitter::Render(){
	vector<float> vertices;
	for (int i = 0; i < particles.size(); i++) {
		vertices.push_back(particles[i].position.x);
		vertices.push_back(particles[i].position.y);
	}

	GLuint colorAttribute = glGetAttribLocation(program2->programID, "color");

	glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, false, 0, particleColors.data());
	glEnableVertexAttribArray(colorAttribute);

	glVertexAttribPointer(program2->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program2->positionAttribute);

	glDrawArrays(GL_POINTS, 0, vertices.size() / 2);
}

//Entity
class Entity{
public:
	Entity(Vector3 position, float width, float height, GLuint img, int type) : position(position), width(width), height(height), img(img), type(type){
		top.x = 0.0f;
		top.y = position.y + height / 2;
		top.z = 0.0f;

		vertices.insert(vertices.end(), {
			-0.25f, -0.25f,
			0.25f, -0.25f,
			0.25f, 0.25f,
			-0.25f, -0.25f,
			0.25f, 0.25f,
			-0.25f, 0.25f
		});

		textCoord.insert(textCoord.end(), {
			0.0, 1.0,
			1.0, 1.0,
			1.0, 0.0,
			0.0, 1.0,
			1.0, 0.0,
			0.0, 0.0
		});
		tileUsed = true;
	}
	Entity(Vector3 position, Vector3 velocity, GLuint img, string color, int type) : position(position), velocity(velocity), img(img), color(color), type(type){
		vertices.insert(vertices.end(), {
			position.x - ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y + ballHalfHeight,
			position.x - ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y + ballHalfHeight,
			position.x - ballHalfWidth, position.y + ballHalfHeight
		});

		textCoord.insert(textCoord.end(), {
			0.0, 1.0,
			1.0, 1.0,
			1.0, 0.0,
			0.0, 1.0,
			1.0, 0.0,
			0.0, 0.0
		});
		tileUsed = true;
	}
	Entity(Vector3 position, Vector3 velocity, GLuint img, string color, int type, int row, int gNum) : position(position), velocity(velocity), img(img), color(color), type(type), row(row), gNum(gNum){
		vertices.insert(vertices.end(), {
			position.x - ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y + ballHalfHeight,
			position.x - ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y + ballHalfHeight,
			position.x - ballHalfWidth, position.y + ballHalfHeight
		});

		textCoord.insert(textCoord.end(), {
			0.0, 1.0,
			1.0, 1.0,
			1.0, 0.0,
			0.0, 1.0,
			1.0, 0.0,
			0.0, 0.0
		});
	}
	void Render();
	void Update();
	int snapToGrid();
	bool wallCollision();
	bool ballCollision();
	void cluster(int gridNum);

	int snapToGrid2();
	bool wallCollision2();
	bool ballCollision2();
	void cluster2(int gridNum);

	void floatingCluster(int gridNum);
	Vector3 position;
	Vector3 newposition;
	Vector3 velocity;
	vector<float> vertices;
	vector<float> textCoord;
	float height;
	float width;
	Vector3 top;
	Vector3 newtop;
	float radius = ballHalfHeight;
	GLuint img;
	string color;
	bool tileUsed = false;
	bool processed;
	int type;
	int row;
	int gNum;
	Matrix matrix;
};

void Entity::Render(){
	if (tileUsed == true){
		glBindTexture(GL_TEXTURE_2D, img);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textCoord.data());
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}
}

void Entity::Update(){
	if (type == GAME_PLAYER){
		newtop.x = matrix.m[0][0] * top.x + matrix.m[1][0] * top.y + matrix.m[2][0] * 0.0 + matrix.m[3][0] * 1;
		newtop.y = matrix.m[0][1] * top.x + matrix.m[1][1] * top.y + matrix.m[2][1] * 0.0 + matrix.m[3][1] * 1;
		newtop.z = matrix.m[0][2] * top.x + matrix.m[1][2] * top.y + matrix.m[2][2] * 0.0 + matrix.m[3][2] * 1;

		newposition.x = matrix.m[0][0] * position.x + matrix.m[1][0] * position.y + matrix.m[2][0] * 0.0 + matrix.m[3][0] * 1;
		newposition.y = matrix.m[0][1] * position.x + matrix.m[1][1] * position.y + matrix.m[2][1] * 0.0 + matrix.m[3][1] * 1;
		newposition.z = matrix.m[0][2] * position.x + matrix.m[1][2] * position.y + matrix.m[2][2] * 0.0 + matrix.m[3][2] * 1;
	}
	else if (type == GAME_BALL){
		position.x -= velocity.x * elasped;
		position.y -= velocity.y * elasped;

		vector<float> tmpVertices;

		tmpVertices.insert(tmpVertices.end(), {
			position.x - ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y + ballHalfHeight,
			position.x - ballHalfWidth, position.y - ballHalfHeight,
			position.x + ballHalfWidth, position.y + ballHalfHeight,
			position.x - ballHalfWidth, position.y + ballHalfHeight
		});

		vertices = tmpVertices;
	}
}

vector<Entity> tiles;
vector<Entity> balls;

//Win or Lose
bool checkWin(){
	bool win = true;
	for (int i = 0; i < tiles.size(); i++){
		if (tiles[i].tileUsed == true){
			win = false;
			break;
		}
	}
	return win;
}

bool checkLose(){
	bool lose = false;
	int start = (gridY - 1) * gridX;
	if (start < tiles.size()){
		for (int i = start; i < tiles.size(); i++){
			if (tiles[i].tileUsed == true){
				lose = true;
				break;
			}
		}
	}
	return lose;
}

//placeing Tiles

int Entity::snapToGrid(){
	float min = 1000.0f;
	int minIndex = 0;
	Mix_PlayChannel(-1, hitSound, 0);
	for (int i = 0; i < tiles.size(); i++){
		float curr = position.distance(tiles[i].position);
		if (curr < min && tiles[i].tileUsed == false){
			min = curr;
			minIndex = i;
		}
	}
	if (minIndex < tiles.size()){
		tiles[minIndex].img = img;
		tiles[minIndex].color = color;
		tiles[minIndex].tileUsed = true;
		if (balls.size() > 0){
			balls.pop_back();
		}
	}
	return minIndex;
}

void deProcess(){
	for (int i = 0; i < tiles.size(); i++){
		tiles[i].processed = false;
	}
}

//Same color cluster

vector<Entity> getNeighbors(Entity cur){
	vector<Entity> tmpNeigh;
	int x = cur.gNum;
	vector<int> evenRows = { x - 15, x - 14, x - 1, x + 1, x + 15, x + 16 };
	vector<int> oddRows = { x - 16, x - 15, x - 1, x + 1, x + 14, x + 15 };

	if (cur.row % 2 == 0){
		for (int i = 0; i < evenRows.size(); i++){
			if (evenRows[i] >= 0 && evenRows[i] < tiles.size() && tiles[evenRows[i]].processed == false && tiles[evenRows[i]].tileUsed == true){
				if (i == 2 || i == 3){
					if (tiles[evenRows[i]].row == cur.row){
						if (tiles[evenRows[i]].color == cur.color){
							tmpNeigh.push_back(tiles[evenRows[i]]);
						}
					}
				}
				else{
					if (tiles[evenRows[i]].color == cur.color){
						tmpNeigh.push_back(tiles[evenRows[i]]);
					}
				}
			}
		}
	}
	else{
		for (int i = 0; i < oddRows.size(); i++){
			if (oddRows[i] >= 0 && oddRows[i] < tiles.size() && tiles[oddRows[i]].processed == false && tiles[oddRows[i]].tileUsed == true){
				if (i == 2 || i == 3){
					if (tiles[evenRows[i]].row == cur.row){
						if (tiles[oddRows[i]].color == cur.color){
							tmpNeigh.push_back(tiles[oddRows[i]]);
						}
					}
				}
				else{
					if (tiles[oddRows[i]].color == cur.color){
						tmpNeigh.push_back(tiles[oddRows[i]]);
					}
				}
			}
		}
	}
	return tmpNeigh;
}

void Entity::cluster(int gridNum){
	vector<Entity> cluster;
	vector<Entity> needProcessing;
	needProcessing.push_back(tiles[gridNum]);
	for (int i = 0; i < needProcessing.size(); i++){
		int a = needProcessing.size();
		if (needProcessing[i].processed == false){
			needProcessing[i].processed = true;
			tiles[needProcessing[i].gNum].processed = true;
			cluster.push_back(needProcessing[i]);
			vector<Entity> tmpVec = getNeighbors(needProcessing[i]);
			for (int x = 0; x < tmpVec.size(); x++){
				needProcessing.push_back(tmpVec[x]);
			}
		}
	}
	deProcess();
	if (cluster.size() >= 3){
		Mix_PlayChannel(-1, explodingSound, 0);
		exploded = true;
		int watColor = -1;
		if (tiles[gridNum].color == ballColors[0])
			watColor = BALL_WHITE;
		else if (tiles[gridNum].color == ballColors[1])
			watColor = BALL_BLUE;
		else if (tiles[gridNum].color == ballColors[2])
			watColor = BALL_GREEN;
		else if (tiles[gridNum].color == ballColors[3])
			watColor = BALL_RED;
		else
			watColor = BALL_YELLOW;

		explosions.startColor = startColors[watColor];
		explosions.endColor = endColors[watColor];
		explosions.position = tiles[gridNum].position;
		explosions.timePassed = 0.0f;

		for (int i = 0; i < cluster.size(); i++){
			GLuint texture = LoadTexture("transparent.png");
			tiles[cluster[i].gNum].img = texture;
			tiles[cluster[i].gNum].color = tile;
			tiles[cluster[i].gNum].tileUsed = false;
		}
	}
}

//Scrolling
float xScroll;
float yScroll;

//Create Level
void buildLevel(int amt){
	int total = 0;
	srand(time(NULL));
	for (int x = 0; x < gridY; x++){
		for (int i = 0; i < gridX; i++){
			float bWidth = ((float)(i)) * ballWidth;
			float bHeight = ((float)(-x)) * ballHeight;
			Vector3 position;
			Vector3 velocity(0.0f, 0.0f, 0.0f);
			if (x % 2 == 0){
				position.x = bWidth + ballHalfWidth;
				position.y = bHeight - ballHalfHeight + 0.02f * x;
				position.z = 0.0f;
			}
			else{
				position.x = bWidth;
				position.y = bHeight - ballHalfHeight + 0.02f * x;
				position.z = 0.0f;
			}
			if (total < amt){
				int color = rand() % 100;
				color = color % 5;
				string tmp = ballColorImg[color];
				char tColor[1024];
				strncpy_s(tColor, tmp.c_str(), sizeof(tColor));
				tColor[sizeof(tColor)-1] = 0;
				GLuint texture = LoadTexture(tColor);
				Entity tile(position, velocity, texture, ballColors[color], GAME_BALL, x, total);
				tile.tileUsed = true;
				tiles.push_back(tile);
			}
			else{
				GLuint texture = LoadTexture("transparent.png");
				Entity tile(position, velocity, texture, tile, GAME_TILE, x, total);
				tiles.push_back(tile);
			}
			total++;
		}
	}
}

void renderLevel(){
	for (int i = 0; i < tiles.size(); i++){
		tiles[i].Render();
	}
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
	program2 = new ShaderProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program->programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void scroll(){
	viewMatrix.identity();
	xScroll = (tiles[tiles.size() - 1].position.x - tiles[0].position.x) / 2.0 + ballHalfWidth;
	yScroll = (tiles[tiles.size() - 1].position.y - tiles[0].position.y) / 2.0 - ballHeight;
	viewMatrix.Translate(-xScroll, -yScroll, 0.0f);
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


//Walls

float tWallt = 0.0 + ballHeight + 0.1;
float tWallr = ((float)gridX * ballWidth) + 0.25;
float tWalll = 0.0 - ballHalfWidth - 0.25;
float tWallb = 0.0;

float topWallV[] = { tWalll, tWallb, tWallr, tWallb, tWallr, tWallt, tWalll, tWallb, tWallr, tWallt, tWalll, tWallt };
float topWallT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float lWallt = 0.0 + ballHeight;
float lWallr = 0.0 - ballHalfWidth;
float lWalll = lWallr - 0.25;
float lWallb = -3.375f - 2 * ballHeight;

float leftWallV[] = { lWalll, lWallb, lWallr, lWallb, lWallr, lWallt, lWalll, lWallb, lWallr, lWallt, lWalll, lWallt };
float leftWallT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float rWallt = 0.0 + ballHeight;
float rWallr = tWallr;
float rWalll = tWallr - 0.25;
float rWallb = -3.375f - 2 * ballHeight;

float rightWallV[] = { rWalll, rWallb, rWallr, rWallb, rWallr, rWallt, rWalll, rWallb, rWallr, rWallt, rWalll, rWallt };
float rightWallT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

GLuint topWall;
GLuint leftWall;
GLuint rightWall;

void renderWalls(){
	//left

	glBindTexture(GL_TEXTURE_2D, leftWall);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, leftWallV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, leftWallT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

	//right

	glBindTexture(GL_TEXTURE_2D, rightWall);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, rightWallV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, rightWallT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

	//top
	glBindTexture(GL_TEXTURE_2D, topWall);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, topWallV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, topWallT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//Wall-Collisions
bool Entity::wallCollision(){
	if ((position.x - ballHalfWidth) < lWallr){
		position.x = lWallr + ballHalfWidth + .0001;
		velocity.x *= -1;
		Mix_PlayChannel(-1, hitSound, 0);
	}
	if ((position.x + ballHalfWidth) > rWalll){
		position.x = rWalll - ballHalfWidth - .0001;
		velocity.x *= -1;
		Mix_PlayChannel(-1, hitSound, 0);
	}
	if ((position.y + ballHalfHeight) > tWallb){
		position.y = tWallb - ballHalfHeight - .0001;
		velocity.x = 0.0;
		velocity.y = 0.0;
		int snap = snapToGrid();
		cluster(snap);
		return true;
	}
	return false;
}

//ballCollision
bool Entity::ballCollision(){
	for (int i = 0; i < tiles.size(); i++){
		if (tiles[i].tileUsed){
			float d = position.distance(tiles[i].position);
			if (d < 2 * radius){
				velocity.x = 0.0;
				velocity.y = 0.0;
				int snap = snapToGrid();
				cluster(snap);
				return true;
			}
		}
	}
	return false;
}

//renderLine
void renderLine(){
	glBindTexture(GL_TEXTURE_2D, line);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, lineV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, lineT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//backGroundMap
GLuint backgroundMap;

float bMapt = 0.0 + ballHeight;
float bMapr = tWallr - 0.1;
float bMapl = lWallr - 0.15;
float bMapb = -3.375f - 2 * ballHeight;

float bMapV[] = { bMapl, bMapb, bMapr, bMapb, bMapr, bMapt, bMapl, bMapb, bMapr, bMapt, bMapl, bMapt };
float bMapT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

void renderBackgroundMap(){
	glBindTexture(GL_TEXTURE_2D, backgroundMap);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, bMapV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, bMapT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
//lose line
GLuint loseLine;

float llt = -3.2f;
float llr = tWallr - 0.1;
float lll = lWallr - 0.15;
float llb = llt - 0.05;

float lloseV[] = { lll, llb, llr, llb, llr, llt, lll, llb, llr, llt, lll, llt };
float lloseT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

void renderLoseLine(){
	glBindTexture(GL_TEXTURE_2D, loseLine);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, lloseV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, lloseT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//MultiPlayer

GLuint player2;
GLuint line2;

GLuint topWall2;
GLuint leftWall2;
GLuint rightWall2;

GLuint backgroundMap2;
GLuint loseLine2;

GLuint balltexture2;

GLuint Line2;

float startX = (llr - lll) + 0.2f;
float angle2 = 0.0f;

ParticleEmitter explosions2(100, startColors[0], endColors[0], Vector3(3.0f, 3.0f, 0.0f));

vector<Entity> tiles2;
vector<Entity> balls2;

//Win or Lose 2
bool checkWin2(){
	bool win = true;
	for (int i = 0; i < tiles2.size(); i++){
		if (tiles2[i].tileUsed == true){
			win = false;
			break;
		}
	}
	return win;
}

bool checkLose2(){
	bool lose = false;
	int start = (gridY - 1) * gridX;
	if (start < tiles2.size()){
		for (int i = start; i < tiles2.size(); i++){
			if (tiles2[i].tileUsed == true){
				lose = true;
				break;
			}
		}
	}
	return lose;
}

//placeing Tiles 2

int Entity::snapToGrid2(){
	float min = 1000.0f;
	int minIndex = 0;
	Mix_PlayChannel(-1, hitSound, 0);
	for (int i = 0; i < tiles2.size(); i++){
		float curr = position.distance(tiles2[i].position);
		if (curr < min && tiles2[i].tileUsed == false){
			min = curr;
			minIndex = i;
		}
	}
	if (minIndex < tiles2.size()){
		tiles2[minIndex].img = img;
		tiles2[minIndex].color = color;
		tiles2[minIndex].tileUsed = true;
		if (balls2.size() > 0){
			balls2.pop_back();
		}
	}
	return minIndex;
}


void deProcess2(){
	for (int i = 0; i < tiles2.size(); i++){
		tiles2[i].processed = false;
	}
}

//Same color cluster 2

vector<Entity> getNeighbors2(Entity cur){
	vector<Entity> tmpNeigh;
	int x = cur.gNum;
	vector<int> evenRows = { x - 15, x - 14, x - 1, x + 1, x + 15, x + 16 };
	vector<int> oddRows = { x - 16, x - 15, x - 1, x + 1, x + 14, x + 15 };

	if (cur.row % 2 == 0){
		for (int i = 0; i < evenRows.size(); i++){
			if (evenRows[i] >= 0 && evenRows[i] < tiles2.size() && tiles2[evenRows[i]].processed == false && tiles2[evenRows[i]].tileUsed == true){
				if (i == 2 || i == 3){
					if (tiles2[evenRows[i]].row == cur.row){
						if (tiles2[evenRows[i]].color == cur.color){
							tmpNeigh.push_back(tiles2[evenRows[i]]);
						}
					}
				}
				else{
					if (tiles2[evenRows[i]].color == cur.color){
						tmpNeigh.push_back(tiles2[evenRows[i]]);
					}
				}
			}
		}
	}
	else{
		for (int i = 0; i < oddRows.size(); i++){
			if (oddRows[i] >= 0 && oddRows[i] < tiles2.size() && tiles2[oddRows[i]].processed == false && tiles2[oddRows[i]].tileUsed == true){
				if (i == 2 || i == 3){
					if (tiles2[evenRows[i]].row == cur.row){
						if (tiles2[oddRows[i]].color == cur.color){
							tmpNeigh.push_back(tiles2[oddRows[i]]);
						}
					}
				}
				else{
					if (tiles2[oddRows[i]].color == cur.color){
						tmpNeigh.push_back(tiles2[oddRows[i]]);
					}
				}
			}
		}
	}
	return tmpNeigh;
}

void Entity::cluster2(int gridNum){
	vector<Entity> cluster;
	vector<Entity> needProcessing;
	needProcessing.push_back(tiles2[gridNum]);
	for (int i = 0; i < needProcessing.size(); i++){
		int a = needProcessing.size();
		if (needProcessing[i].processed == false){
			needProcessing[i].processed = true;
			tiles2[needProcessing[i].gNum].processed = true;
			cluster.push_back(needProcessing[i]);
			vector<Entity> tmpVec = getNeighbors2(needProcessing[i]);
			for (int x = 0; x < tmpVec.size(); x++){
				needProcessing.push_back(tmpVec[x]);
			}
		}
	}
	deProcess2();
	if (cluster.size() >= 3){
		Mix_PlayChannel(-1, explodingSound, 0);
		exploded2 = true;
		int watColor = -1;
		if (tiles2[gridNum].color == ballColors[0])
			watColor = BALL_WHITE;
		else if (tiles2[gridNum].color == ballColors[1])
			watColor = BALL_BLUE;
		else if (tiles2[gridNum].color == ballColors[2])
			watColor = BALL_GREEN;
		else if (tiles2[gridNum].color == ballColors[3])
			watColor = BALL_RED;
		else
			watColor = BALL_YELLOW;

		explosions2.startColor = startColors[watColor];
		explosions2.endColor = endColors[watColor];
		explosions2.position = tiles2[gridNum].position;
		explosions2.timePassed = 0.0f;

		for (int i = 0; i < cluster.size(); i++){
			GLuint texture = LoadTexture("transparent.png");
			tiles2[cluster[i].gNum].img = texture;
			tiles2[cluster[i].gNum].color = tile;
			tiles2[cluster[i].gNum].tileUsed = false;
		}
	}
}

void buildLevel2(int amt){
	int total = 0;
	srand(time(NULL));
	for (int x = 0; x < gridY; x++){
		for (int i = 0; i < gridX; i++){
			float bWidth = ((float)(i)) * ballWidth;
			float bHeight = ((float)(-x)) * ballHeight;
			Vector3 position;
			Vector3 velocity(0.0f, 0.0f, 0.0f);
			if (x % 2 == 0){
				position.x = -startX + bWidth + ballHalfWidth;
				position.y = bHeight - ballHalfHeight + 0.02f * x;
				position.z = 0.0f;
			}
			else{
				position.x = -startX + bWidth;
				position.y = bHeight - ballHalfHeight + 0.02f * x;
				position.z = 0.0f;
			}
			if (total < amt){
				int color = rand() % 100;
				color = color % 5;
				string tmp = ballColorImg[color];
				char tColor[1024];
				strncpy_s(tColor, tmp.c_str(), sizeof(tColor));
				tColor[sizeof(tColor)-1] = 0;
				GLuint texture = LoadTexture(tColor);
				Entity tile(position, velocity, texture, ballColors[color], GAME_BALL, x, total);
				tile.tileUsed = true;
				tiles2.push_back(tile);
			}
			else{
				GLuint texture = LoadTexture("transparent.png");
				Entity tile(position, velocity, texture, tile, GAME_TILE, x, total);
				tiles2.push_back(tile);
			}
			total++;
		}
	}
}

void renderLevel2(){
	for (int i = 0; i < tiles2.size(); i++){
		tiles2[i].Render();
	}
}

//Walls2
float tWallt2 = 0.0 + ballHeight + 0.1;
float tWallr2 = -startX + ((float)gridX * ballWidth) + 0.25;
float tWalll2 = -startX + 0.0 - ballHalfWidth - 0.25;
float tWallb2 = 0.0;

float topWallV2[] = { tWalll2, tWallb2, tWallr2, tWallb2, tWallr2, tWallt2, tWalll2, tWallb2, tWallr2, tWallt2, tWalll2, tWallt2 };
float topWallT2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float lWallt2 = 0.0 + ballHeight;
float lWallr2 = -startX + 0.0 - ballHalfWidth;
float lWalll2 = -startX + lWallr - 0.25;
float lWallb2 = -3.375f - 2 * ballHeight;

float leftWallV2[] = { lWalll2, lWallb2, lWallr2, lWallb2, lWallr2, lWallt2, lWalll2, lWallb2, lWallr2, lWallt2, lWalll2, lWallt2 };
float leftWallT2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float rWallt2 = 0.0 + ballHeight;
float rWallr2 = -startX + tWallr;
float rWalll2 = -startX + tWallr - 0.25;
float rWallb2 = -3.375f - 2 * ballHeight;

float rightWallV2[] = { rWalll2, rWallb2, rWallr2, rWallb2, rWallr2, rWallt2, rWalll2, rWallb2, rWallr2, rWallt2, rWalll2, rWallt2 };
float rightWallT2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

void renderWalls2(){
	//left

	glBindTexture(GL_TEXTURE_2D, leftWall2);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, leftWallV2);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, leftWallT2);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

	//right

	glBindTexture(GL_TEXTURE_2D, rightWall2);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, rightWallV2);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, rightWallT2);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

	//top
	glBindTexture(GL_TEXTURE_2D, topWall2);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, topWallV2);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, topWallT2);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//Wall-Collisions2
bool Entity::wallCollision2(){
	if ((position.x - ballHalfWidth) < lWallr2){
		position.x = lWallr2 + ballHalfWidth + .0001;
		velocity.x *= -1;
		Mix_PlayChannel(-1, hitSound, 0);
	}
	if ((position.x + ballHalfWidth) > rWalll2){
		position.x = rWalll2 - ballHalfWidth - .0001;
		velocity.x *= -1;
		Mix_PlayChannel(-1, hitSound, 0);
	}
	if ((position.y + ballHalfHeight) > tWallb){
		position.y = tWallb - ballHalfHeight - .0001;
		velocity.x = 0.0;
		velocity.y = 0.0;
		int snap = snapToGrid2();
		cluster2(snap);
		return true;
	}
	return false;
}

//ballCollision2
bool Entity::ballCollision2(){
	for (int i = 0; i < tiles2.size(); i++){
		if (tiles2[i].tileUsed){
			float d = position.distance(tiles2[i].position);
			if (d < 2 * radius){
				velocity.x = 0.0;
				velocity.y = 0.0;
				int snap = snapToGrid2();
				cluster2(snap);
				return true;
			}
		}
	}
	return false;
}

//Line 2
void renderLine2(){
	glBindTexture(GL_TEXTURE_2D, line2);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, lineV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, lineT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//backGroundMap
float bMapt2 = 0.0 + ballHeight;
float bMapr2 = -startX + tWallr - 0.1;
float bMapl2 = -startX + lWallr - 0.15;
float bMapb2 = -3.375f - 2 * ballHeight;

float bMapV2[] = { bMapl2, bMapb2, bMapr2, bMapb2, bMapr2, bMapt2, bMapl2, bMapb2, bMapr2, bMapt2, bMapl2, bMapt2 };
float bMapT2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

void renderBackgroundMap2(){
	glBindTexture(GL_TEXTURE_2D, backgroundMap2);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, bMapV2);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, bMapT2);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//Lose Line 2
float llt2 = -3.2f;
float llr2 = -startX + tWallr - 0.1;
float lll2 = -startX + lWallr - 0.15;
float llb2 = llt - 0.05;

float lloseV2[] = { lll2, llb2, llr2, llb2, llr2, llt2, lll2, llb2, llr2, llt2, lll2, llt2 };
float lloseT2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

void renderLoseLine2(){
	glBindTexture(GL_TEXTURE_2D, loseLine2);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, lloseV2);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, lloseT2);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void scroll2(){
	viewMatrix.identity();
	//xScroll = (tiles[tiles.size() - 1].position.x - tiles[0].position.x) / 2.0 + ballHalfWidth;
	xScroll = llr - lll2;
	xScroll /= 2.0f;
	xScroll = llr - xScroll;
	yScroll = (tiles[tiles.size() - 1].position.y - tiles[0].position.y) / 2.0 - ballHeight;
	yScroll += 0.35f;
	viewMatrix.Translate(-xScroll, -yScroll, 0.0f);
}

//MultiPlayer
GLuint tPlayer;
GLuint tPlayer2;

float tPlayert = 0.0 + ballHeight + 0.1f + 0.75f;
float tPlayerr = ((float)gridX * ballWidth) - 1.25f;
float tPlayerl = 0.0 - ballHalfWidth + 1.25f;
float tPlayerb = 0.0 + ballHeight + 0.1;

float topPlayerV[] = { tPlayerl, tPlayerb, tPlayerr, tPlayerb, tPlayerr, tPlayert, tPlayerl, tPlayerb, tPlayerr, tPlayert, tPlayerl, tPlayert };
float topPlayerT[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

float tPlayert2 = 0.0 + ballHeight + 0.1f + 0.75f;
float tPlayerr2 = tWallr2 - 1.25f;
float tPlayerl2 = tWalll2 + 1.25f;
float tPlayerb2 = 0.0 + ballHeight + 0.1;

float topPlayerV2[] = { tPlayerl2, tPlayerb2, tPlayerr2, tPlayerb2, tPlayerr2, tPlayert2, tPlayerl2, tPlayerb2, tPlayerr2, tPlayert2, tPlayerl2, tPlayert2 };
float topPlayerT2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

void renderSigns(){
	//P1
	glBindTexture(GL_TEXTURE_2D, tPlayer);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, topPlayerV2);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, topPlayerT2);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
	//P2
	glBindTexture(GL_TEXTURE_2D, tPlayer2);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, topPlayerV);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, topPlayerT);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

int main(int argc, char *argv[])
{
	Setup();

	text = LoadTexture("pixel_font.png");

	titleSign = LoadTexture("logo2.png");
	playButton = LoadTexture("Play.png");
	multiButton = LoadTexture("2Player.png");
	quitButton = LoadTexture("quit.png");
	backButton = LoadTexture("back.png");
	oneButton = LoadTexture("level1.png");
	twoButton = LoadTexture("level2.png");
	threeButton = LoadTexture("level3.png");
	retryButton = LoadTexture("retry.png");
	menuButton = LoadTexture("menu.png");
	nextButton = LoadTexture("next.png");
	loseSign = LoadTexture("lose.png");
	winSign = LoadTexture("win.png");
	P1WinSign = LoadTexture("P1win.png");
	P2WinSign = LoadTexture("P2win.png");
	mainBack = LoadTexture("menuScreen.png");
	loseBack = LoadTexture("loseScreen.png");
	winBack = LoadTexture("heaven.png");
	//Game

	player = LoadTexture("player.png");
	line = LoadTexture("line.png");

	topWall = LoadTexture("line.png");
	leftWall = LoadTexture("line.png");
	rightWall = LoadTexture("line.png");

	backgroundMap = LoadTexture("background.png");
	loseLine = LoadTexture("loseLine.png");

	srand(time(NULL));

	int bcolor = rand() % 100;
	bcolor = bcolor % 5;
	string tmp = ballColorImg[bcolor];
	char tbColor[1024];
	strncpy_s(tbColor, tmp.c_str(), sizeof(tbColor));
	tbColor[sizeof(tbColor)-1] = 0;
	balltexture = LoadTexture(tbColor);

	Entity Player(Vector3(0.0, 0.0, 0.0), 0.6, 0.6, player, GAME_PLAYER);

	//Player 2

	player2 = LoadTexture("player2.png");
	line2 = LoadTexture("line2.png");

	topWall2 = LoadTexture("line2.png");
	leftWall2 = LoadTexture("line2.png");
	rightWall2 = LoadTexture("line2.png");

	backgroundMap2 = LoadTexture("background.png");
	loseLine2 = LoadTexture("loseLine.png");

	int bcolor2 = rand() % 100;
	bcolor2 = bcolor2 % 5;
	string tmp2 = ballColorImg[bcolor2];
	char tbColor2[1024];
	strncpy_s(tbColor2, tmp.c_str(), sizeof(tbColor2));
	tbColor2[sizeof(tbColor2)-1] = 0;
	balltexture2 = LoadTexture(tbColor2);

	Entity Player2(Vector3(0.0, 0.0, 0.0), 0.6, 0.6, player2, GAME_PLAYER);

	//Multi
	tPlayer = LoadTexture("P1.png");
	tPlayer2 = LoadTexture("P2.png");

	//sound
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	hitSound = Mix_LoadWAV("hit.wav");
	explodingSound = Mix_LoadWAV("explode.wav");
	hoverSound = Mix_LoadWAV("hover.wav");
	shootSound = Mix_LoadWAV("shoot.wav");
	music = Mix_LoadMUS("music.mp3");
	Mix_PlayMusic(music, -1);

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			if (event.type == SDL_MOUSEMOTION){
				unitX = (((float)event.motion.x / 640.0f) * 7.1f) - 3.55f;
				unitY = (((float)(360 - event.motion.y) / 360.0f) * 4.0f) - 2.0f;
				switch (state) {
				case STATE_MAIN_MENU:
					if (unitX < -1.9  && unitX > -3.4 && unitY < -0.7 && unitY > -1.0){
						float tmparr[] = { -3.45, -1.05, -1.95, -1.05, -1.95, -0.65, -3.45, -1.05, -1.95, -0.65, -3.45, -0.65 };
						copy(tmparr, tmparr + 12, vpBut);
						if (in == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in = true;
						}
					}
					else{
						float tmparr[] = { -3.4, -1.0, -1.9, -1.0, -1.9, -0.7, -3.4, -1.0, -1.9, -0.7, -3.4, -0.7 };
						copy(tmparr, tmparr + 12, vpBut);
						if (in == true)
							in = false;
					}
					if (unitX < -1.9  && unitX > -3.4 && unitY < -1.1 && unitY > -1.4){
						float tmparr[] = { -3.45, -1.45, -1.95, -1.45, -1.95, -1.05, -3.45, -1.45, -1.95, -1.05, -3.45, -1.05 };
						copy(tmparr, tmparr + 12, vmBut);
						if (in1 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in1 = true;
						}
					}
					else{
						float tmparr[] = { -3.4, -1.4, -1.9, -1.4, -1.9, -1.1, -3.4, -1.4, -1.9, -1.1, -3.4, -1.1 };
						copy(tmparr, tmparr + 12, vmBut);
						if (in1 == true)
							in1 = false;
					}
					if (unitX < -1.9  && unitX > -3.4 && unitY < -1.5 && unitY > -1.8){
						float tmparr[] = { -3.45, -1.85, -1.95, -1.85, -1.95, -1.45, -3.45, -1.85, -1.95, -1.45, -3.45, -1.45 };
						copy(tmparr, tmparr + 12, vqBut);
						if (in2 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in2 = true;
						}
					}
					else{
						float tmparr[] = { -3.4, -1.8, -1.9, -1.8, -1.9, -1.5, -3.4, -1.8, -1.9, -1.5, -3.4, -1.5 };
						copy(tmparr, tmparr + 12, vqBut);
						if (in2 == true)
							in2 = false;
					}
					break;
				case STATE_LEVEL_MENU:
					if (unitX < -1.9  && unitX > -3.4 && unitY < -0.3 && unitY > -0.6){
						float tmparr[] = { -3.45, -0.65, -1.95, -0.65, -1.95, -0.25, -3.45, -0.65, -1.95, -0.25, -3.45, -0.25 };
						copy(tmparr, tmparr + 12, voneBut);
						if (in3 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in3 = true;
						}
					}
					else{
						float tmparr[] = { -3.4, -0.6, -1.9, -0.6, -1.9, -0.3, -3.4, -0.6, -1.9, -0.3, -3.4, -0.3 };
						copy(tmparr, tmparr + 12, voneBut);
						if (in3 == true)
							in3 = false;
					}
					if (unitX < -1.9  && unitX > -3.4 && unitY < -0.7 && unitY > -1.0){
						float tmparr[] = { -3.45, -1.05, -1.95, -1.05, -1.95, -0.65, -3.45, -1.05, -1.95, -0.65, -3.45, -0.65 };
						copy(tmparr, tmparr + 12, vtwoBut);
						if (in4 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in4 = true;
						}
					}
					else{
						float tmparr[] = { -3.4, -1.0, -1.9, -1.0, -1.9, -0.7, -3.4, -1.0, -1.9, -0.7, -3.4, -0.7 };
						copy(tmparr, tmparr + 12, vtwoBut);
						if (in4 == true)
							in4 = false;
					}
					if (unitX < -1.9  && unitX > -3.4 && unitY < -1.1 && unitY > -1.4){
						float tmparr[] = { -3.45, -1.45, -1.95, -1.45, -1.95, -1.05, -3.45, -1.45, -1.95, -1.05, -3.45, -1.05 };
						copy(tmparr, tmparr + 12, vthreeBut);
						if (in5 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in5 = true;
						}
					}
					else{
						float tmparr[] = { -3.4, -1.4, -1.9, -1.4, -1.9, -1.1, -3.4, -1.4, -1.9, -1.1, -3.4, -1.1 };
						copy(tmparr, tmparr + 12, vthreeBut);
						if (in5 == true)
							in5 = false;
					}
					if (unitX < -1.9  && unitX > -3.4 && unitY < -1.5 && unitY > -1.8){
						float tmparr[] = { -3.45, -1.85, -1.95, -1.85, -1.95, -1.45, -3.45, -1.85, -1.95, -1.45, -3.4, -1.45 };
						copy(tmparr, tmparr + 12, vbBut);
						if (in6 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in6 = true;
						}
					}
					else{
						float tmparr[] = { -3.4, -1.8, -1.9, -1.8, -1.9, -1.5, -3.4, -1.8, -1.9, -1.5, -3.4, -1.5 };
						copy(tmparr, tmparr + 12, vbBut);
						if (in6 == true)
							in6 = false;
					}
					break;
				case STATE_GAME_LOSE:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { -0.95, -0.35, -0.15, -0.35, -0.15, 0.10, -0.95, -0.35, -0.15, 0.10, -0.95, 0.10 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in7 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in7 = true;
						}
					}
					else{
						float tmparr[] = { -0.9, -0.3, -0.1, -0.3, -0.1, 0.15, -0.9, -0.3, -0.1, 0.15, -0.9, 0.15 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in7 == true)
							in7 = false;
					}
					if (unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { 0.15, -0.35, 0.95, -0.35, 0.95, 0.10, 0.15, -0.35, 0.95, 0.10, 0.15, 0.10 };
						copy(tmparr, tmparr + 12, vlBut);
						if (in8 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in8 = true;
						}
					}
					else{
						float tmparr[] = { 0.1, -0.3, 0.9, -0.3, 0.9, 0.15, 0.1, -0.3, 0.9, 0.15, 0.1, 0.15 };
						copy(tmparr, tmparr + 12, vlBut);
						if (in8 == true)
							in8 = false;
					}
					break;
				case STATE_GAME_WIN:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { -0.95, -0.35, -0.15, -0.35, -0.15, 0.10, -0.95, -0.35, -0.15, 0.10, -0.95, 0.10 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in9 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in9 = true;
						}
					}
					else{
						float tmparr[] = { -0.9, -0.3, -0.1, -0.3, -0.1, 0.15, -0.9, -0.3, -0.1, 0.15, -0.9, 0.15 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in9 == true)
							in9 = false;
					}
					if (unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { 0.15, -0.35, 0.95, -0.35, 0.95, 0.10, 0.15, -0.35, 0.95, 0.10, 0.15, 0.10 };
						copy(tmparr, tmparr + 12, vnBut);
						if (in10 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in10 = true;
						}
					}
					else{
						float tmparr[] = { 0.1, -0.3, 0.9, -0.3, 0.9, 0.15, 0.1, -0.3, 0.9, 0.15, 0.1, 0.15 };
						copy(tmparr, tmparr + 12, vnBut);
						if (in10 == true)
							in10 = false;
					}
					break;
				case STATE_GAME_WINP1:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { -0.95, -0.35, -0.15, -0.35, -0.15, 0.10, -0.95, -0.35, -0.15, 0.10, -0.95, 0.10 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in9 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in9 = true;
						}
					}
					else{
						float tmparr[] = { -0.9, -0.3, -0.1, -0.3, -0.1, 0.15, -0.9, -0.3, -0.1, 0.15, -0.9, 0.15 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in9 == true)
							in9 = false;
					}
					if (unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { 0.15, -0.35, 0.95, -0.35, 0.95, 0.10, 0.15, -0.35, 0.95, 0.10, 0.15, 0.10 };
						copy(tmparr, tmparr + 12, vnBut);
						if (in10 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in10 = true;
						}
					}
					else{
						float tmparr[] = { 0.1, -0.3, 0.9, -0.3, 0.9, 0.15, 0.1, -0.3, 0.9, 0.15, 0.1, 0.15 };
						copy(tmparr, tmparr + 12, vnBut);
						if (in10 == true)
							in10 = false;
					}
					break;
				case STATE_GAME_WINP2:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { -0.95, -0.35, -0.15, -0.35, -0.15, 0.10, -0.95, -0.35, -0.15, 0.10, -0.95, 0.10 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in9 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in9 = true;
						}
					}
					else{
						float tmparr[] = { -0.9, -0.3, -0.1, -0.3, -0.1, 0.15, -0.9, -0.3, -0.1, 0.15, -0.9, 0.15 };
						copy(tmparr, tmparr + 12, vrBut);
						if (in9 == true)
							in9 = false;
					}
					if (unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						float tmparr[] = { 0.15, -0.35, 0.95, -0.35, 0.95, 0.10, 0.15, -0.35, 0.95, 0.10, 0.15, 0.10 };
						copy(tmparr, tmparr + 12, vnBut);
						if (in10 == false){
							Mix_PlayChannel(-1, hoverSound, 0);
							in10 = true;
						}
					}
					else{
						float tmparr[] = { 0.1, -0.3, 0.9, -0.3, 0.9, 0.15, 0.1, -0.3, 0.9, 0.15, 0.1, 0.15 };
						copy(tmparr, tmparr + 12, vnBut);
						if (in10 == true)
							in10 = false;
					}
					break;
				}
			}
			if(event.type == SDL_MOUSEBUTTONDOWN){
				unitX = (((float)event.button.x / 640.0f) * 7.1f) - 3.55f;
				unitY = (((float)(360 - event.button.y) / 360.0f) * 4.0f) - 2.0f;

				explosions.position.x = 3.0f;
				explosions.position.y = 3.0f;
				explosions2.position.x = 3.0f;
				explosions2.position.y = 3.0f;
				switch (state) {
				case STATE_MAIN_MENU:
					if (unitX < -1.9  && unitX > -3.4 && unitY < -0.7 && unitY > -1.0){
						state = STATE_LEVEL_MENU;
					}
					else if (unitX < -1.9  && unitX > -3.4 && unitY < -1.1 && unitY > -1.4){
						state = STATE_GAME_MULTI;
						vector<Entity> tmpTiles;
						tiles = tmpTiles;
						tiles2 = tmpTiles;
						buildLevel(45);
						buildLevel2(45);
					}
					else if (unitX < -1.9  && unitX > -3.4 && unitY < -1.5 && unitY > -1.8){
						done = true;
					}
					break;
				case STATE_LEVEL_MENU:
				{
										 if (unitX < -1.9  && unitX > -3.4 && unitY < -0.3 && unitY > -0.6){
											 state = STATE_LEVEL_ONE;
											 vector<Entity> tmpTiles;
											 tiles = tmpTiles;
											 buildLevel(45);
										 }
										 else if (unitX < -1.9  && unitX > -3.4 && unitY < -0.7 && unitY > -1.0){
											 state = STATE_LEVEL_TWO;
											 vector<Entity> tmpTiles;
											 tiles = tmpTiles;
											 buildLevel(90);
										 }
										 else if (unitX < -1.9  && unitX > -3.4 && unitY < -1.1 && unitY > -1.4){
											 state = STATE_LEVEL_THREE;
											 vector<Entity> tmpTiles;
											 tiles = tmpTiles;
											 buildLevel(135);
										 }
										 else if (unitX < -1.9  && unitX > -3.4 && unitY < -1.5 && unitY > -1.8){
											 state = STATE_MAIN_MENU;
										 }
										 break;
				}
				case STATE_GAME_LOSE:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						switch (old_state){
						case STATE_LEVEL_ONE:{
												 state = STATE_LEVEL_ONE;
												 vector<Entity> tmpTiles;
												 tiles = tmpTiles;
												 buildLevel(45);
												 break;
						}
						case STATE_LEVEL_TWO:{
												 state = STATE_LEVEL_TWO;
												 vector<Entity> tmpTiles;
												 tiles = tmpTiles;
												 buildLevel(90);
												 break;
						}
						case STATE_LEVEL_THREE:{
												   state = STATE_LEVEL_THREE;
												   vector<Entity> tmpTiles;
												   tiles = tmpTiles;
												   buildLevel(135);
												   break;
						}
						}
					}
					else if(unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						state = STATE_LEVEL_MENU;
					}
					break;
				case STATE_GAME_WIN:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						switch (old_state){
						case STATE_LEVEL_ONE:{
												 state = STATE_LEVEL_ONE;
												 vector<Entity> tmpTiles;
												 tiles = tmpTiles;
												 buildLevel(45);
												 break;
						}
						case STATE_LEVEL_TWO:{
												 state = STATE_LEVEL_TWO;
												 vector<Entity> tmpTiles;
												 tiles = tmpTiles;
												 buildLevel(90);
												 break;
						}
						case STATE_LEVEL_THREE:{
												   state = STATE_LEVEL_THREE;
												   vector<Entity> tmpTiles;
												   tiles = tmpTiles;
												   buildLevel(135);
												   break;
						}
						}
					}
					else if (unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						if (old_state == STATE_LEVEL_ONE){
							state = STATE_LEVEL_TWO;
							vector<Entity> tmpTiles;
							tiles = tmpTiles;
							buildLevel(90);
						}
						else if (old_state == STATE_LEVEL_TWO){
							state = STATE_LEVEL_THREE;
							vector<Entity> tmpTiles;
							tiles = tmpTiles;
							buildLevel(135);
						}
						else if (old_state == STATE_LEVEL_THREE){
							state = STATE_LEVEL_MENU;
						}
					}
					break;
				case STATE_GAME_WINP1:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						state = STATE_GAME_MULTI;
						vector<Entity> tmpTiles;
						tiles = tmpTiles;
						tiles2 = tmpTiles;
						buildLevel(45);
						buildLevel2(45);
					}
					else if (unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						state = STATE_MAIN_MENU;
					}
					break;
				case STATE_GAME_WINP2:
					if (unitX < -0.1  && unitX > -0.9 && unitY < 0.15 && unitY > -0.3){
						state = STATE_GAME_MULTI;
						vector<Entity> tmpTiles;
						tiles = tmpTiles;
						tiles2 = tmpTiles;
						buildLevel(45);
						buildLevel2(45);
					}
					else if (unitX < 0.9  && unitX > 0.1 && unitY < 0.15 && unitY > -0.3){
						state = STATE_MAIN_MENU;
					}
					break;
				}
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		
		program->setModelMatrix(modelMatrix);
		program->setProjectionMatrix(projectionMatrix);
		program->setViewMatrix(viewMatrix);

		ticks = (float)SDL_GetTicks() / 1000.0f;
		elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_ESCAPE]){
			done = true;
		}
		if (keys[SDL_SCANCODE_E]){
			state = STATE_GAME_WIN;
		}
		if (keys[SDL_SCANCODE_Q]){
			state = STATE_LEVEL_MENU;
		}
		if (state == STATE_LEVEL_ONE || state == STATE_LEVEL_TWO || state == STATE_LEVEL_THREE){
			if (keys[SDL_SCANCODE_LEFT]){
				if (angle < 1.50f)
					angle += 0.01f;
			}
			else if (keys[SDL_SCANCODE_RIGHT]){
				if (angle > -1.50f)
					angle -= 0.01f;
			}
			else if (keys[SDL_SCANCODE_UP]){
				if (balls.size() < 1){
					Mix_PlayChannel(-1, shootSound, 0);
					Vector2 responseVector = Vector2(Player.newposition.x - Player.newtop.x, Player.newposition.y - Player.newtop.y);
					responseVector.normalize();
					Vector3 p(Player.newtop.x, Player.newtop.y, Player.newtop.z);
					Vector3 v(responseVector.x * 2, responseVector.y * 2, 0.0f);
					Entity ball(p, v, balltexture, ballColors[bcolor], GAME_BALL);
					balls.push_back(ball);
					bcolor = rand() % 100;
					bcolor = bcolor % 5;
					tmp = ballColorImg[bcolor];
					memset(tbColor, 0, sizeof(tbColor));
					strncpy_s(tbColor, tmp.c_str(), sizeof(tbColor));
					tbColor[sizeof(tbColor)-1] = 0;
					balltexture = LoadTexture(tbColor);
				}
			}
		}

		if (lineV[5] != 0.5){
			lineV[5] = 0.5;
			lineV[9] = 0.5;
			lineV[11] = 0.5;
		}

		switch (state) {
		case STATE_MAIN_MENU:
			projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
			program->setProjectionMatrix(projectionMatrix);

			viewMatrix.identity();

			modelMatrix.identity();
			program->setModelMatrix(modelMatrix);
			//Background
			glBindTexture(GL_TEXTURE_2D, mainBack);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vbackR);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tbackR);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Title
			glBindTexture(GL_TEXTURE_2D, titleSign);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vtSign);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tctSign);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Play Button
			glBindTexture(GL_TEXTURE_2D, playButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vpBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcpBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//MultiPlayer Button
			glBindTexture(GL_TEXTURE_2D, multiButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vmBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcmBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Quit Button
			glBindTexture(GL_TEXTURE_2D, quitButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vqBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcqBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, -1.65, 0.0);
			program->setModelMatrix(modelMatrix);
			life = "Press Esc at anytime to Quit!";
			DrawText(program, text, life, 0.1f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, -0.85, 0.0);
			program->setModelMatrix(modelMatrix);
			life = "Press Q at anytime to return to Level Menu!";
			DrawText(program, text, life, 0.1f, 0.0f);

			break;
		case STATE_LEVEL_MENU:
			projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
			program->setProjectionMatrix(projectionMatrix);

			viewMatrix.identity();

			modelMatrix.identity();
			program->setModelMatrix(modelMatrix);
			//Background
			glBindTexture(GL_TEXTURE_2D, mainBack);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vbackR);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tbackR);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Title
			glBindTexture(GL_TEXTURE_2D, titleSign);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vtSign);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tctSign);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Level 1 Button
			glBindTexture(GL_TEXTURE_2D, oneButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, voneBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tconeBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Level 2 Button
			glBindTexture(GL_TEXTURE_2D, twoButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vtwoBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tctwoBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Level 3 Button
			glBindTexture(GL_TEXTURE_2D, threeButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vthreeBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcthreeBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//back Button
			glBindTexture(GL_TEXTURE_2D, backButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vbBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcbBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);
			break;
		case STATE_GAME_LOSE:
			projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
			program->setProjectionMatrix(projectionMatrix);

			viewMatrix.identity();

			modelMatrix.identity();
			program->setModelMatrix(modelMatrix);
			//Background
			glBindTexture(GL_TEXTURE_2D, loseBack);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vbackR);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tbackR);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//LOSE IMG
			glBindTexture(GL_TEXTURE_2D, loseSign);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vtSign);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tctSign);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Retry Button
			glBindTexture(GL_TEXTURE_2D, retryButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vrBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcrBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Level Menu Button
			glBindTexture(GL_TEXTURE_2D, menuButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vlBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tclBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);
			break;
		case STATE_GAME_WIN:
			projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
			program->setProjectionMatrix(projectionMatrix);

			viewMatrix.identity();

			modelMatrix.identity();
			program->setModelMatrix(modelMatrix);
			//Background
			glBindTexture(GL_TEXTURE_2D, winBack);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vbackR);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tbackR);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//WIN IMG
			glBindTexture(GL_TEXTURE_2D, winSign);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vtSign);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tctSign);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Retry Button
			glBindTexture(GL_TEXTURE_2D, retryButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vrBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcrBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Next Level Button
			glBindTexture(GL_TEXTURE_2D, nextButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vnBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcnBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);
			break;
		case STATE_GAME_WINP1:
			projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
			program->setProjectionMatrix(projectionMatrix);

			viewMatrix.identity();

			modelMatrix.identity();
			program->setModelMatrix(modelMatrix);
			//Background
			glBindTexture(GL_TEXTURE_2D, winBack);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vbackR);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tbackR);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//WIN IMG
			glBindTexture(GL_TEXTURE_2D, P1WinSign);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vtSign);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tctSign);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Retry Button
			glBindTexture(GL_TEXTURE_2D, retryButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vrBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcrBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Menu Button
			glBindTexture(GL_TEXTURE_2D, menuButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vnBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcnBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);
			break;
		case STATE_GAME_WINP2:
			projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
			program->setProjectionMatrix(projectionMatrix);

			viewMatrix.identity();

			modelMatrix.identity();
			program->setModelMatrix(modelMatrix);
			//Background
			glBindTexture(GL_TEXTURE_2D, winBack);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vbackR);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tbackR);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//WIN IMG
			glBindTexture(GL_TEXTURE_2D, P2WinSign);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vtSign);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tctSign);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//Retry Button
			glBindTexture(GL_TEXTURE_2D, retryButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vrBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcrBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			//menu Button
			glBindTexture(GL_TEXTURE_2D, menuButton);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vnBut);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tcnBut);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);
			break;
		case STATE_LEVEL_ONE:{
								 old_state = STATE_LEVEL_ONE;
								 projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
								 program->setProjectionMatrix(projectionMatrix);

								 modelMatrix.identity();
								 program->setModelMatrix(modelMatrix);
								 renderBackgroundMap();
								 renderLoseLine();
								 renderWalls();
								 renderLevel();

								 scroll();

								 for (int i = 0; i < balls.size(); i++){
									 modelMatrix.identity();
									 program->setModelMatrix(modelMatrix);
									 if (!balls[i].wallCollision() && !balls[i].ballCollision()){
										 balls[i].Update();
										 balls[i].Render();
									 }
								 }

								 glPointSize(5.0);
								 program2->setModelMatrix(modelMatrix);
								 program2->setProjectionMatrix(projectionMatrix);
								 program2->setViewMatrix(viewMatrix);

								 if (explosions.timePassed > 3.0f){
									 explosions.position.x = 3.0f;
									 explosions.position.y = 3.0f;
								 }
								 else{
									 explosions.Update(elasped);
									 explosions.Render();
									 explosions.timePassed += elasped;
								 }

								 program->setModelMatrix(modelMatrix);
								 program->setProjectionMatrix(projectionMatrix);
								 program->setViewMatrix(viewMatrix);

								 Player.matrix.identity();
								 Player.matrix.Translate(xScroll, -3.375f - ballHeight, 0.0f);
								 Player.matrix.Rotate(angle);
								 Player.Update();
								 program->setModelMatrix(Player.matrix);
								 Player.Render();
								 Vector3 tmpp(0.0f, 0.0f, 0.0f);
								 Vector3 tmpv(0.0f, 0.0f, 0.0f);
								 Entity tmpball(tmpp, tmpv, balltexture, ballColors[bcolor], GAME_BALL);
								 tmpball.Render();
								 lineV[5] = 2.0;
								 lineV[9] = 2.0;
								 lineV[11] = 2.0;
								 renderLine();

								 if (checkWin()){
									 state = STATE_GAME_WIN;
								 }
								 if (checkLose()){
									 state = STATE_GAME_LOSE;
								 }
								 break;
		}
		case STATE_LEVEL_TWO:{
								 old_state = STATE_LEVEL_TWO;
								 projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
								 program->setProjectionMatrix(projectionMatrix);

								 modelMatrix.identity();
								 program->setModelMatrix(modelMatrix);
								 renderBackgroundMap();
								 renderLoseLine();
								 renderWalls();
								 renderLevel();

								 scroll();

								 for (int i = 0; i < balls.size(); i++){
									 modelMatrix.identity();
									 program->setModelMatrix(modelMatrix);
									 if (!balls[i].wallCollision() && !balls[i].ballCollision()){
										 balls[i].Update();
										 balls[i].Render();
									 }
								 }

								 glPointSize(5.0);
								 program2->setModelMatrix(modelMatrix);
								 program2->setProjectionMatrix(projectionMatrix);
								 program2->setViewMatrix(viewMatrix);

								 if (explosions.timePassed > 3.0f){
									 explosions.position.x = 3.0f;
									 explosions.position.y = 3.0f;
								 }
								 else{
									 explosions.Update(elasped);
									 explosions.Render();
									 explosions.timePassed += elasped;
								 }

								 program->setModelMatrix(modelMatrix);
								 program->setProjectionMatrix(projectionMatrix);
								 program->setViewMatrix(viewMatrix);

								 Player.matrix.identity();
								 Player.matrix.Translate(xScroll, -3.375f - ballHeight, 0.0f);
								 Player.matrix.Rotate(angle);
								 Player.Update();
								 program->setModelMatrix(Player.matrix);
								 Player.Render();
								 Vector3 tmpp(0.0f, 0.0f, 0.0f);
								 Vector3 tmpv(0.0f, 0.0f, 0.0f);
								 Entity tmpball(tmpp, tmpv, balltexture, ballColors[bcolor], GAME_BALL);
								 tmpball.Render();
								 lineV[5] = 1.0;
								 lineV[9] = 1.0;
								 lineV[11] = 1.0;
								 renderLine();

								 if (checkWin()){
									 state = STATE_GAME_WIN;
								 }
								 if (checkLose()){
									 state = STATE_GAME_LOSE;
								 }
								
								 break;
		}
		case STATE_LEVEL_THREE:{
								   old_state = STATE_LEVEL_THREE;

								   projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
								   program->setProjectionMatrix(projectionMatrix);

								   modelMatrix.identity();
								   program->setModelMatrix(modelMatrix);
								   renderBackgroundMap();
								   renderLoseLine();
								   renderWalls();
								   renderLevel();

								   scroll();

								   for (int i = 0; i < balls.size(); i++){
									   modelMatrix.identity();
									   program->setModelMatrix(modelMatrix);
									   if (!balls[i].wallCollision() && !balls[i].ballCollision()){
										   balls[i].Update();
										   balls[i].Render();
									   }
								   }

								   glPointSize(5.0);
								   program2->setModelMatrix(modelMatrix);
								   program2->setProjectionMatrix(projectionMatrix);
								   program2->setViewMatrix(viewMatrix);

								   if (explosions.timePassed > 3.0f){
									   explosions.position.x = 3.0f;
									   explosions.position.y = 3.0f;
								   }
								   else{
									   explosions.Update(elasped);
									   explosions.Render();
									   explosions.timePassed += elasped;
								   }

								   program->setModelMatrix(modelMatrix);
								   program->setProjectionMatrix(projectionMatrix);
								   program->setViewMatrix(viewMatrix);

								   Player.matrix.identity();
								   Player.matrix.Translate(xScroll, -3.375f - ballHeight, 0.0f);
								   Player.matrix.Rotate(angle);
								   Player.Update();
								   program->setModelMatrix(Player.matrix);
								   Player.Render();
								   Vector3 tmpp(0.0f, 0.0f, 0.0f);
								   Vector3 tmpv(0.0f, 0.0f, 0.0f);
								   Entity tmpball(tmpp, tmpv, balltexture, ballColors[bcolor], GAME_BALL);
								   tmpball.Render();
								   renderLine();

								   if (checkWin()){
									   state = STATE_GAME_WIN;
								   }
								   if (checkLose()){
									   state = STATE_GAME_LOSE;
								   }
								   break;
		}
		case STATE_GAME_MULTI:{
								  projectionMatrix.setOrthoProjection(-4.34875f, 4.34875f, -2.45f, 2.45f, -1.0f, 1.0f);
								  modelMatrix.identity();
								  program->setModelMatrix(modelMatrix);
								  program->setProjectionMatrix(projectionMatrix);
								  program->setViewMatrix(viewMatrix);
								  renderBackgroundMap();
								  renderLoseLine();
								  renderWalls();
								  renderLevel();

								  const Uint8 *keys = SDL_GetKeyboardState(NULL);

								  if (exploded2){
									  bcolor = 0;
									  tmp = ballColorImg[bcolor];
									  memset(tbColor, 0, sizeof(tbColor));
									  strncpy_s(tbColor, tmp.c_str(), sizeof(tbColor));
									  tbColor[sizeof(tbColor)-1] = 0;
									  balltexture = LoadTexture(tbColor);
									  exploded2 = false;
								  }

								  if (keys[SDL_SCANCODE_LEFT]){
									  if (angle < 1.50f)
										  angle += 0.01f;
								  }
								  else if (keys[SDL_SCANCODE_RIGHT]){
									  if (angle > -1.50f)
										  angle -= 0.01f;
								  }
								  else if (keys[SDL_SCANCODE_UP]){
									  if (balls.size() < 1){
										  Mix_PlayChannel(-1, shootSound, 0);
										  Vector2 responseVector = Vector2(Player.newposition.x - Player.newtop.x, Player.newposition.y - Player.newtop.y);
										  responseVector.normalize();
										  Vector3 p(Player.newtop.x, Player.newtop.y, Player.newtop.z);
										  Vector3 v(responseVector.x * 2, responseVector.y * 2, 0.0f);
										  Entity ball(p, v, balltexture, ballColors[bcolor], GAME_BALL);
										  balls.push_back(ball);
										  bcolor = rand() % 100;
										  bcolor = bcolor % 5;
										  tmp = ballColorImg[bcolor];
										  memset(tbColor, 0, sizeof(tbColor));
										  strncpy_s(tbColor, tmp.c_str(), sizeof(tbColor));
										  tbColor[sizeof(tbColor)-1] = 0;
										  balltexture = LoadTexture(tbColor);
									  }
								  }

								  //Play Button

								  for (int i = 0; i < balls.size(); i++){
									  modelMatrix.identity();
									  program->setModelMatrix(modelMatrix);
									  if (!balls[i].wallCollision() && !balls[i].ballCollision()){
										  balls[i].Update();
										  balls[i].Render();
									  }
								  }

								  glPointSize(5.0);
								  program2->setModelMatrix(modelMatrix);
								  program2->setProjectionMatrix(projectionMatrix);
								  program2->setViewMatrix(viewMatrix);

								  if (explosions.timePassed > 3.0f){
									  explosions.position.x = 3.0f;
									  explosions.position.y = 3.0f;
								  }
								  else{
									  explosions.Update(elasped);
									  explosions.Render();
									  explosions.timePassed += elasped;
								  }

								  program->setModelMatrix(modelMatrix);
								  program->setProjectionMatrix(projectionMatrix);
								  program->setViewMatrix(viewMatrix);

								  Player.matrix.identity();
								  float playerX = (tiles[tiles.size() - 1].position.x - tiles[0].position.x) / 2.0 + ballHalfWidth;
								  Player.matrix.Translate(playerX, -3.375f - ballHeight, 0.0f);
								  Player.matrix.Rotate(angle);
								  Player.Update();
								  program->setModelMatrix(Player.matrix);
								  Player.Render();
								  Vector3 tmpp(0.0f, 0.0f, 0.0f);
								  Vector3 tmpv(0.0f, 0.0f, 0.0f);
								  Entity tmpball(tmpp, tmpv, balltexture, ballColors[bcolor], GAME_BALL);
								  tmpball.Render();
								  renderLine();

								  scroll2();

								  modelMatrix.identity();
								  modelMatrix.Translate(-3.4, 1.9, 0.0);
								  program->setModelMatrix(modelMatrix);
								  life = "ball x,x,y,y: " + to_string(tiles[0].position.x) + " " + to_string(tiles[tiles.size() - 1].position.x) + "," + to_string(tiles[0].position.y) + " " + to_string(tiles[tiles.size() - 1].position.y);
								  DrawText(program, text, life, 0.1f, 0.0f);

								  //Player 2
								  modelMatrix.identity();
								  program->setModelMatrix(modelMatrix);
								  renderBackgroundMap2();
								  renderLoseLine2();
								  renderWalls2();
								  renderLevel2();

								  if (exploded){
									  bcolor2 = 0;
									  tmp2 = ballColorImg[bcolor2];
									  memset(tbColor2, 0, sizeof(tbColor2));
									  strncpy_s(tbColor2, tmp2.c_str(), sizeof(tbColor2));
									  tbColor2[sizeof(tbColor2)-1] = 0;
									  balltexture2 = LoadTexture(tbColor2);
									  exploded = false;
								  }

								  if (keys[SDL_SCANCODE_A]){
									  if (angle2 < 1.50f)
										  angle2 += 0.01f;
								  }
								  else if (keys[SDL_SCANCODE_D]){
									  if (angle2 > -1.50f)
										  angle2 -= 0.01f;
								  }
								  else if (keys[SDL_SCANCODE_W]){
									  if (balls2.size() < 1){
										  Mix_PlayChannel(-1, shootSound, 0);
										  Vector2 responseVector = Vector2(Player2.newposition.x - Player2.newtop.x, Player2.newposition.y - Player2.newtop.y);
										  responseVector.normalize();
										  Vector3 p(Player2.newtop.x, Player2.newtop.y, Player2.newtop.z);
										  Vector3 v(responseVector.x * 2, responseVector.y * 2, 0.0f);
										  Entity ball(p, v, balltexture2, ballColors[bcolor2], GAME_BALL);
										  balls2.push_back(ball);
										  bcolor2 = rand() % 100;
										  bcolor2 = bcolor2 % 5;
										  tmp2 = ballColorImg[bcolor2];
										  memset(tbColor2, 0, sizeof(tbColor2));
										  strncpy_s(tbColor2, tmp2.c_str(), sizeof(tbColor2));
										  tbColor2[sizeof(tbColor2)-1] = 0;
										  balltexture2 = LoadTexture(tbColor2);
									  }
								  }

								  //Play Button

								  for (int i = 0; i < balls2.size(); i++){
									  modelMatrix.identity();
									  program->setModelMatrix(modelMatrix);
									  if (!balls2[i].wallCollision2() && !balls2[i].ballCollision2()){
										  balls2[i].Update();
										  balls2[i].Render();
									  }
								  }

								  glPointSize(5.0);
								  program2->setModelMatrix(modelMatrix);
								  program2->setProjectionMatrix(projectionMatrix);
								  program2->setViewMatrix(viewMatrix);

								  if (explosions2.timePassed > 3.0f){
									  explosions2.position.x = 3.0f;
									  explosions2.position.y = 3.0f;
								  }
								  else{
									  explosions2.Update(elasped);
									  explosions2.Render();
									  explosions2.timePassed += elasped;
								  }

								  program->setModelMatrix(modelMatrix);
								  program->setProjectionMatrix(projectionMatrix);
								  program->setViewMatrix(viewMatrix);

								  Player2.matrix.identity();
								  Player2.matrix.Translate(-startX + playerX, -3.375f - ballHeight, 0.0f);
								  Player2.matrix.Rotate(angle2);
								  Player2.Update();
								  program->setModelMatrix(Player2.matrix);
								  Player2.Render();
								  Entity tmpball2(tmpp, tmpv, balltexture2, ballColors[bcolor2], GAME_BALL);
								  tmpball2.Render();
								  renderLine2();


								  modelMatrix.identity();
								  program->setModelMatrix(modelMatrix);
								  program->setProjectionMatrix(projectionMatrix);
								  program->setViewMatrix(viewMatrix);

								  renderSigns();

								  if (checkWin()){
									  state = STATE_GAME_WINP2;
								  }
								  if (checkLose()){
									  state = STATE_GAME_WINP1;
								  }
								  if (checkWin2()){
									  state = STATE_GAME_WINP1;
								  }
								  if (checkLose2()){
									  state = STATE_GAME_WINP2;
								  }
								  break;
		}
		}
		SDL_GL_SwapWindow(displayWindow);
	}
	Mix_FreeChunk(hitSound);
	Mix_FreeChunk(explodingSound);
	Mix_FreeChunk(hoverSound);
	Mix_FreeChunk(shootSound);
	Mix_FreeMusic(music);

	SDL_Quit();
	return 0;
}
