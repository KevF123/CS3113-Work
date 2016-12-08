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

using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

SDL_Window* displayWindow;

ShaderProgram* program;
ShaderProgram* program2;

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

GLuint pic1Texture;
GLuint pic2Texture;
GLuint pic3Texture;
GLuint text;

float lastFrameTicks = 0.0f;

float ticks;
float elasped;

float fixedElapsed = elasped;

float spd;

SDL_Event event;
bool done = false;

class Vector3 {
public:
	Vector3(float x, float y, float z) : x(x), y(y), z(z){}
	Vector3(){}
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

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector3> &points1, const std::vector<Vector3> &points2) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	vector<float> e1Projected;
	vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	sort(e1Projected.begin(), e1Projected.end());
	sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];
	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p < 0) {
		return true;
	}
	return false;
}

bool checkSATCollision(const std::vector<Vector3> &e1Points, const std::vector<Vector3> &e2Points) {
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}

		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	return true;
}

class Entity{
public:
	Entity(Vector3 position, Vector3 scale, float angle, vector<Vector3> vertices, float *verR) : position(position), scale(scale), angle(angle), vertices(vertices), verR(verR){
		for (int i = 0; i < vertices.size(); i++){
			bool tmp = true;
			for (int x = 0; x < verticesAct.size(); x++){
				if (vertices[i].x == verticesAct[x].x && vertices[i].y == verticesAct[x].y){
					tmp = false;
					break;
				}
			}
			if (tmp){
				verticesAct.push_back(vertices[i]);
			}
		}
	}
	Entity();
	void Render();
	void Update();
	void checkCollision(Entity &o);
	vector<Vector3> vertices;
	vector<Vector3> verticesAct;
	Vector3 position;
	Vector3 scale;
	float angle;
	float *verR = new float[6];
	Matrix matrix;
};

vector<Entity> entities;

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

void Entity::Render(){
	string life;

	//life = "x,y: " + to_string(verticesPic1[0]) + "," + to_string(verticesPic1[1]) + ", x,y: " + to_string(verticesPic1[2]) + "," + to_string(verticesPic1[3]) + ", x,y: " + to_string(verticesPic1[4]) + "," + to_string(verticesPic1[5]);
	//DrawText(program, text, life, 0.1f, 0.0f);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, verR);
	glEnableVertexAttribArray(program->positionAttribute);
	/*
	float texCoordsPic3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsPic3);
	glEnableVertexAttribArray(program->texCoordAttribute);*/

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisableVertexAttribArray(program->positionAttribute);
}

void Entity::Update(){
	string life;
	for (int i = 0; i < vertices.size(); i++){
		vertices[i].x = matrix.m[0][0] * verR[2*i] + matrix.m[1][0] * verR[2*i+1] + matrix.m[2][0] * 0.0 + matrix.m[3][0] * 1;
		vertices[i].y = matrix.m[0][1] * verR[2*i] + matrix.m[1][1] * verR[2*i+1] + matrix.m[2][1] * 0.0 + matrix.m[3][1] * 1;
		vertices[i].z = matrix.m[0][2] * verR[2 * i] + matrix.m[1][2] * verR[2 * i + 1] + matrix.m[2][2] * 0.0 + matrix.m[3][2] * 1;
	}

	vector<Vector3> tmpV;
	int x = 0;
	for (int i = 0; i < vertices.size(); i++){
		bool tmp = true;
		for (int x = 0; x < tmpV.size(); x++){
			if (vertices[i].x == tmpV[x].x && vertices[i].y == tmpV[x].y){
				tmp = false;
				break;
			}
		}
		if (tmp){
			tmpV.push_back(vertices[i]);
		}
	}
	for (int i = 0; i < verticesAct.size(); i++){
		verticesAct[i] = tmpV[i];
	}
	/*
	modelMatrix.identity();
	modelMatrix.Translate(-3.0,1.5, 0.0);
	program2->setModelMatrix(modelMatrix);
	program2->setProjectionMatrix(projectionMatrix);
	program2->setViewMatrix(viewMatrix);
	life += to_string(verticesAct.size());
	for (int i = 0; i < verticesAct.size(); i++){
		life += "x,y: " + to_string(verticesAct[i].x) + "," + to_string(verticesAct[i].y);
	}
	//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
	//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
	DrawText(program2, text, life, 0.1f, 0.0f);*/
}

void Entity::checkCollision(Entity &o){
	int maxChecks = 100;
	int op = 0;
	while (checkSATCollision(verticesAct, o.verticesAct)){
		Vector2 responseVector = Vector2(position.x - o.position.x, position.y - o.position.y);

		responseVector.normalize();

		position.x += responseVector.x * 0.002;
		position.y += responseVector.y * 0.002;
		matrix.Translate(position.x, position.y, 0.0);
		Update();

		o.position.x -= responseVector.x * 0.002;
		o.position.y -= responseVector.y * 0.002;
		o.matrix.Translate(o.position.x, o.position.y, 0.0);
		o.Update();

		op++;

		string life = "";
		modelMatrix.identity();
		modelMatrix.Translate(-3.0, 1.4, 0.0);
		program2->setModelMatrix(modelMatrix);
		program2->setProjectionMatrix(projectionMatrix);
		program2->setViewMatrix(viewMatrix);
		life = "true " + to_string(o.position.x) + " " + to_string(o.position.y) + " " + to_string(op);
		DrawText(program2, text, life, 0.1f, 0.0f);

		//maxChecks--;
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
	program = new ShaderProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	program2 = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program->programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	Setup();

	text = LoadTexture("pixel_font.png");

	vector<Vector3> tri = { Vector3(-0.25, -0.1818, 0.0), Vector3(0.25, -0.1818, 0.0), Vector3(0.0, -0.1818, 0.0) };
	float triVer[] = { -0.25, -0.1818, 0.25, -0.1818, 0.0, 0.1818 };
	Entity temp(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), 0.0f, tri, triVer);
	entities.push_back(temp);

	vector<Vector3> sq = { Vector3(-0.25, -0.25, 0.0), Vector3(0.25, -0.25, 0.0), Vector3(0.25, 0.25, 0.0), Vector3(-0.25, -0.25, 0.0), Vector3(0.25, 0.25, 0.0), Vector3(-0.25, 0.25, 0.0) };
	float sqVer[] = { -0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25 };
	Entity temp2(Vector3(2.0, 0.0, 0.0), Vector3(2.0, 1.0, 1.0), 45.0f, sq, sqVer);
	entities.push_back(temp2);

	vector<Vector3> sq2 = { Vector3(-0.25, -0.25, 0.0), Vector3(0.25, -0.25, 0.0), Vector3(0.25, 0.25, 0.0), Vector3(-0.25, -0.25, 0.0), Vector3(0.25, 0.25, 0.0), Vector3(-0.25, 0.25, 0.0) };
	float sq2Ver[] = { -0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25 };
	Entity temp3(Vector3(-2.0, 0.0, 0.0), Vector3(2.0, 2.0, 1.0), 45.0f, sq2, sq2Ver);
	entities.push_back(temp3);

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClearColor(0.4f, 0.4f, 0.4f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		program->setModelMatrix(modelMatrix);
		program->setProjectionMatrix(projectionMatrix);
		program->setViewMatrix(viewMatrix);

		ticks = (float)SDL_GetTicks() / 1000.0f;
		elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		spd = 0.25;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		if (keys[SDL_SCANCODE_UP]){
			entities[0].checkCollision(entities[1]);
			entities[0].checkCollision(entities[2]);
			entities[0].position.y += elasped * spd;
		}
		if (keys[SDL_SCANCODE_DOWN]){
			entities[0].checkCollision(entities[1]);
			entities[0].checkCollision(entities[2]);
			entities[0].position.y -= elasped * spd;
		}
		if (keys[SDL_SCANCODE_RIGHT]){
			entities[0].checkCollision(entities[1]);
			entities[0].checkCollision(entities[2]);
			entities[0].position.x += elasped * spd;
		}
		if (keys[SDL_SCANCODE_LEFT]){
			entities[0].checkCollision(entities[1]);
			entities[0].checkCollision(entities[2]);
			entities[0].position.x -= elasped * spd;
		}

		entities[0].checkCollision(entities[1]);
		entities[0].checkCollision(entities[2]);

		entities[0].matrix.identity();
		entities[0].matrix.Translate(entities[0].position.x, entities[0].position.y, 0.0);
		entities[0].Update();
		program->setModelMatrix(entities[0].matrix);
		entities[0].Render();

		entities[1].matrix.identity();
		entities[1].matrix.Translate(entities[1].position.x, entities[1].position.y, entities[1].position.z);
		entities[1].matrix.Scale(entities[1].scale.x, entities[1].scale.y, entities[1].scale.z);
		entities[1].matrix.Rotate(entities[1].angle * (3.1415926 / 180.0));
		entities[1].Update();
		program->setModelMatrix(entities[1].matrix);
		entities[1].Render();

		entities[2].matrix.identity();
		entities[2].matrix.Translate(entities[2].position.x, entities[2].position.y, entities[2].position.z);
		entities[2].matrix.Scale(entities[2].scale.x, entities[2].scale.y, entities[2].scale.z);
		entities[2].matrix.Rotate(entities[2].angle * (3.1415926 / 180.0));
		entities[2].Update();
		program->setModelMatrix(entities[2].matrix);
		entities[2].Render();

		string life = "";
		modelMatrix.identity();
		modelMatrix.Translate(-3.0, 1.5, 0.0);
		program2->setModelMatrix(modelMatrix);
		program2->setProjectionMatrix(projectionMatrix);
		program2->setViewMatrix(viewMatrix);
		life += to_string(entities[0].verticesAct.size());
		for (int i = 0; i < entities[0].verticesAct.size(); i++){
			life += "x,y: " + to_string(entities[0].verticesAct[i].x) + "," + to_string(entities[0].verticesAct[i].y);
		}
		//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
		//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
		DrawText(program2, text, life, 0.1f, 0.0f); 

		string life2 = "";
		modelMatrix.identity();
		modelMatrix.Translate(-3.0, 1.6, 0.0);
		program2->setModelMatrix(modelMatrix);
		program2->setProjectionMatrix(projectionMatrix);
		program2->setViewMatrix(viewMatrix);
		life2 += to_string(entities[1].position.x);
		for (int i = 0; i < entities[1].verticesAct.size(); i++){
			life2 += "x,y: " + to_string(entities[1].verticesAct[i].x) + "," + to_string(entities[1].verticesAct[i].y);
		}
		//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
		//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
		DrawText(program2, text, life2, 0.1f, 0.0f);

		string life3 = "";
		modelMatrix.identity();
		modelMatrix.Translate(-3.0, 1.7, 0.0);
		program2->setModelMatrix(modelMatrix);
		program2->setProjectionMatrix(projectionMatrix);
		program2->setViewMatrix(viewMatrix);
		life3 += to_string(entities[2].position.x);
		for (int i = 0; i < entities[2].verticesAct.size(); i++){
			life3 += "x,y: " + to_string(entities[2].verticesAct[i].x) + "," + to_string(entities[2].verticesAct[i].y);
		}
		//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
		//life = "x,y: " + to_string(vertices[0].x) + "," + to_string(vertices[0].y) + ", x,y: " + to_string(vertices[1].x) + "," + to_string(vertices[1].y) + ", x,y: " + to_string(vertices[2].x) + "," + to_string(vertices[2].y);
		DrawText(program2, text, life3, 0.1f, 0.0f);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
