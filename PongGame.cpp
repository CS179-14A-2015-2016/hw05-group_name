// PongGame.cpp : Pong game using c++ and openGL *sigh*.

#include "stdafx.h"

#include <string>
#include <windows.h>
#include <iostream>
#include <conio.h>
#include <sstream>
#include <math.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "GL/freeglut.h"
#pragma comment(lib, "OpenGL32.lib") //specify linker options

//key define
#define VK_W 0x57
#define VK_S 0x53

using namespace std;

//window size and refresh rate
int width = 600;
int height = 300;
int refresh = 1000 / 60;

//scoring
int p1score = 0;
int p2score = 0;

//paddles
int paddle_width = 8;
int paddle_height = 60;
int paddle_speed = 6;

//paddle positions
float leftpaddle_x = 10;
float leftpaddle_y = 125;

float rightpaddle_x = width - paddle_width - 20;
float rightpaddle_y = 125;

//ball shenanigans
//ball1
float ball_posx = width / 2 + 10;
float ball_posy = height / 2;
float ball_dirx = -1.0;
float ball_diry = 1.0;

//ball2
float ball2_posx = width / 2 - 10;
float ball2_posy = height / 2;
float ball2_dirx = 1.0;
float ball2_diry = -1.0;

float ball_speed = 5;
float ball_radius = 5;
int ball_segments = 8;

//function functions

//int to string function
string inttostr(int x) {
	//converts an int to a string using sstream lib
	stringstream convs;
	convs << x;
	return convs.str();
}

/*
//clamp function
float clamp(float n, float lower, float upper) {
	n = (n > lower) * n + !(n > lower) * lower;
	return (n < upper) * n + !(n < upper) * upper;
}
*/

//void functions
//keyboard controls
void keyboard() {
	//left paddle
	if (GetAsyncKeyState(VK_W)) leftpaddle_y += paddle_speed;
	if (GetAsyncKeyState(VK_S)) leftpaddle_y -= paddle_speed;

	//right paddle
	if (GetAsyncKeyState(VK_UP)) rightpaddle_y += paddle_speed;
	if (GetAsyncKeyState(VK_DOWN)) rightpaddle_y -= paddle_speed;
}

//makes gl recognized 2d usage
void use2D(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//prints text on screen
void textDraw(float x, float y, string text) {
	glRasterPos2f(x, y);
	glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*)text.c_str());
}

//draw paddles
void paddleDraw(float x, float y, float width, float height) {
	glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x + width, y);
		glVertex2f(x + width, y + height);
		glVertex2f(x, y + height);
	glEnd();
}

//draw the ball
void ballDraw(float cx, float cy, float r, int segments) {
	float theta = 2 * 3.1415926 / float(segments);
	//sin and cos calculation
	float cos = cosf(theta); 
	float sin = sinf(theta);
	float t;

	float varx = r; //start at angle 0
	float vary = 0;

	glBegin(GL_LINE_LOOP);
		for (int i = 0; i < segments; i++) {
			glVertex2f(varx + cx, vary + cy); //outputs vertex

			//apply rotation matrix
			t = varx;
			varx = cos * varx - sin * vary;
			vary = sin * t + cos * vary;
		}
	glEnd();
}

//normalize vectors
void vecnorm(float &x, float &y) {
	float length = sqrt((x * x) + (y * y));
	if (length != 0.0f) {
		length = 1.0f / length;
		x *= length;
		y *= length;
	}
}

//ball1 movement
void ballMove() {
	ball_posx += ball_dirx * ball_speed;
	ball_posy += ball_diry * ball_speed;

	//ball to ball collision
	float D = sqrt((ball2_posx - ball_posx)*(ball2_posx - ball_posx) + (ball2_posy - ball_posy)*(ball2_posy - ball_posy));
	//if collision
	if (D <= 2 * ball_radius) {
		float b = ((ball_posy - ball2_posy) / (2 * ball_radius)) - 0.5f;
		ball_dirx = -ball_dirx;
		ball_diry = b;
	}

	//hit by left paddle
	if ((ball_posx <= leftpaddle_x + paddle_width) &&
		(ball_posx >= leftpaddle_x) &&
		(ball_posy <= leftpaddle_y + paddle_height) &&
		(ball_posy >= leftpaddle_y)) {
		// set fly direction depending on where it hit the racket
		// (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
		float t = ((ball_posy - leftpaddle_y) / paddle_height) - 0.5f;
		ball_dirx = fabs(ball_dirx); // force it to be positive
		ball_diry = t;
	}

	//hit by right paddle
	if ((ball_posx >= rightpaddle_x) &&
		(ball_posx <= rightpaddle_x + paddle_width) &&
		(ball_posy <= rightpaddle_y + paddle_height) &&
		(ball_posy >= rightpaddle_y)) {
		// set fly direction depending on where it hit the racket
		// (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
		float t = ((ball_posy - rightpaddle_y) / paddle_height) - 0.5f;
		ball_dirx = -fabs(ball_dirx); // force it to be negative
		ball_diry = t;
	}

	//left wall collision
	if (ball_posx < 0) {
		++p2score;
		ball_posx = width / 2;
		ball_posy = height / 2;
		ball_dirx = fabs(ball_dirx); // force it to be positive
		ball_diry = 0;
	}

	//right wall collision
	if (ball_posx > width) {
		++p1score;
		ball_posx = width / 2;
		ball_posy = height / 2;
		ball_dirx = -fabs(ball_dirx); // force it to be negative
		ball_diry = 0;
	}

	//top wall collision
	if (ball_posy > height-20) {
		ball_diry = -fabs(ball_diry); // force it to be negative
	}

	//bottom wall collision
	if (ball_posy < 10) {
		ball_diry = fabs(ball_diry); // force it to be positive
	}

	// make sure that length of dir stays at 1
	vecnorm(ball_dirx, ball_diry);
}

//ball2 movement
void ball2Move() {
	ball2_posx += ball2_dirx * ball_speed;
	ball2_posy += ball2_diry * ball_speed;

	//ball to ball collision
	float D = sqrt((ball2_posx - ball_posx)*(ball2_posx - ball_posx) + (ball2_posy - ball_posy)*(ball2_posy - ball_posy));
	//if collision
	if (D <= 2 * ball_radius) {
		float b = ((ball2_posy - ball_posy) / (2 * ball_radius)) - 0.5f;
		ball2_dirx = -ball2_dirx;
		ball2_diry = b;
	}

	//hit by left paddle
	if ((ball2_posx <= leftpaddle_x + paddle_width) &&
		(ball2_posx >= leftpaddle_x) &&
		(ball2_posy <= leftpaddle_y + paddle_height) &&
		(ball2_posy >= leftpaddle_y)) {
		// set fly direction depending on where it hit the racket
		// (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
		float t = ((ball2_posy - leftpaddle_y) / paddle_height) - 0.5f;
		ball2_dirx = fabs(ball2_dirx); // force it to be positive
		ball2_diry = t;
	}

	//hit by right paddle
	if ((ball2_posx >= rightpaddle_x) &&
		(ball2_posx <= rightpaddle_x + paddle_width) &&
		(ball2_posy <= rightpaddle_y + paddle_height) &&
		(ball2_posy >= rightpaddle_y)) {
		// set fly direction depending on where it hit the racket
		// (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
		float t = ((ball2_posy - rightpaddle_y) / paddle_height) - 0.5f;
		ball2_dirx = -fabs(ball2_dirx); // force it to be negative
		ball2_diry = t;
	}

	//left wall collision
	if (ball2_posx < 0) {
		++p2score;
		ball2_posx = width / 2;
		ball2_posy = height / 2;
		ball2_dirx = fabs(ball2_dirx); // force it to be positive
		ball2_diry = 0;
	}

	//right wall collision
	if (ball2_posx > width) {
		++p1score;
		ball2_posx = width / 2;
		ball2_posy = height / 2;
		ball2_dirx = -fabs(ball2_dirx); // force it to be negative
		ball2_diry = 0;
	}

	//top wall collision
	if (ball2_posy > height - 20) {
		ball2_diry = -fabs(ball2_diry); // force it to be negative
	}

	//bottom wall collision
	if (ball2_posy < 10) {
		ball2_diry = fabs(ball2_diry); // force it to be positive
	}

	// make sure that length of dir stays at 1
	vecnorm(ball2_dirx, ball2_diry);
}

//draw on screen
void draw() {
	//clearing the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//put draw codes below

	//ball display
	ballDraw(ball_posx, ball_posy, ball_radius, ball_segments);
	ballDraw(ball2_posx, ball2_posy, ball_radius, ball_segments);

	//paddle display
	paddleDraw(leftpaddle_x, leftpaddle_y, paddle_width, paddle_height);
	paddleDraw(rightpaddle_x, rightpaddle_y, paddle_width, paddle_height);

	//score display
	textDraw(width / 2 - 30, height - 30, inttostr(p1score) + " : " + inttostr(p2score));

	//swapping buffers
	glutSwapBuffers();
}

//screen update handler
void update(int upvalue) {
	//input
	keyboard();

	//ball movement
	ballMove();
	ball2Move();

	//calls update in millisecs
	glutTimerFunc(refresh, update, 0);

	//Redisplay Frame
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	//initiliaze opengl using glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Le Pong");

	//uses the void functions
	glutDisplayFunc(draw);
	glutTimerFunc(refresh, update, 0);

	//setup the 2d and set draw color to black
	use2D(width, height);
	glColor3f(1.0f, 1.0f, 1.0f);

	//program loop
	glutMainLoop();

    return 0;
}

