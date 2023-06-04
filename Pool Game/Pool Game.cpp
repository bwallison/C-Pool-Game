// Pool Game.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "stdafx.h"
#include <math.h>
#include "simulation.h"
#include <string>
#include <glut.h>

using namespace std;

//cue variables
float		gCueAngle = 0.0;
float		gCuePower = 0.25;
bool		gCueControl[4] = {false,false,false,false};
float		gCueAngleSpeed = 4.0f; //radians per second
float		gCuePowerSpeed = 1.0f;
float		gCuePowerMax = 0.75;
float		gCuePowerMin = 0.1;
float		gCueBallFactor = 8.0;
bool		gDoCue = true;

//camera variables
vec3		gCamPos(0.0,0.7,2.1);
vec3		gCamLookAt(0.0,0.0,0.0);
bool		gCamRotate			=	true;
float		gCamRotSpeed		=	0.2;
float		gCamMoveSpeed		=	0.5;
bool		gCamL				=	false;
bool		gCamR				=	false;
bool		gCamU				=	false;
bool		gCamD				=	false;
bool		gCamZin				=	false;
bool		gCamZout			=	false;

//colour variables
float		ballColourOne[3]	=	{0.4f, 0.0f, 0.0f};
float		ballColourTwo[3]	=	{1.0f, 1.0f, 0.0f};
float		floorColour[3]		=	{0.0f, 0.15f, 0.0f};
float		wallColour[3]		=	{0.15f, 0.15f, 0.05f};
float		pocketColour[3]		=	{0.2f, 0.0f, 0.0f };

//controls the game
string		par[4]				=	{"3","3","3","3"};
bool		onMenu				=	true;
bool		changeCourse		=	false;
bool		wireMesh			=	true;
bool		gameOver			=	false;

//rendering options
#define DRAW_SOLID	(1)

void printText(int x, int y, string String) //method used in the ui to print the text
{
	//(x,y) is from the bottom left of the window
	glMatrixMode(GL_PROJECTION);
	glPushAttrib(GL_CURRENT_BIT);
	glColor3f(1.0,1.0,1.0);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GLUT_WINDOW_WIDTH, 0, GLUT_WINDOW_HEIGHT, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glRasterPos2i(x, y);
	for (int i = 0; i<String.size(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, String[i]);
	}
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void updateHUD(void) //updates the scores to top left corner based on which players turn it is
{
	string text = "COURSE PAR: " + par[0];
	printText(5, GLUT_WINDOW_HEIGHT - 5, text);
	if (gTable.players[0].turn)
	{
		text = "CURRENT TURN: PLAYER 1";
		printText(5, GLUT_WINDOW_HEIGHT - 10, text);
		text = "CURRENT STROKE: " + to_string(gTable.players[0].GetPar(gTable.courseNum));
		printText(5, GLUT_WINDOW_HEIGHT - 15, text);
	}
	else
	{
		text = "CURRENT TURN: PLAYER 2";
		printText(5, GLUT_WINDOW_HEIGHT - 10, text);
		text = "CURRENT STROKE: " + to_string(gTable.players[1].GetPar(gTable.courseNum));
		printText(5, GLUT_WINDOW_HEIGHT - 15, text);
	}
}

void mainScreen(void) //main screen text printing of the level selection
{
	printText(GLUT_WINDOW_WIDTH/2, GLUT_WINDOW_HEIGHT/2 + 20, "MAIN MENU");
	printText(GLUT_WINDOW_WIDTH / 2 - 5, GLUT_WINDOW_HEIGHT / 2 + 10, "USE 'A' OR 'D' TO SELECT COURSE");

	switch (gTable.courseNum)
	{
		case 0:
			printText(GLUT_WINDOW_WIDTH / 2 - 1, GLUT_WINDOW_HEIGHT / 2, "COURSE: 1  HOLE: 1");
			break;

		case 1:
			printText(GLUT_WINDOW_WIDTH / 2 - 1, GLUT_WINDOW_HEIGHT / 2, "COURSE: 1  HOLE: 2");
			break;

		case 2:
			printText(GLUT_WINDOW_WIDTH / 2 - 1, GLUT_WINDOW_HEIGHT / 2, "COURSE: 2  HOLE: 1");
			break;

		case 3:
			printText(GLUT_WINDOW_WIDTH / 2 - 1, GLUT_WINDOW_HEIGHT / 2, "COURSE: 2  HOLE: 2");
			break; 
	}
}


void endScreen(void) //end screen text when courses are completed
{
	printText(GLUT_WINDOW_WIDTH / 2, GLUT_WINDOW_HEIGHT / 2 + 20, "GAME OVER");
	printText(GLUT_WINDOW_WIDTH / 2 - 5, GLUT_WINDOW_HEIGHT / 2 + 10, "PRESS ESC TO GO BACK TO MAIN MENU");

	switch (gTable.courseNum) //outputs results based of what course it is
	{
	case 1:
		printText(GLUT_WINDOW_WIDTH / 2 - 6, GLUT_WINDOW_HEIGHT / 2, "HOLE 1:   PLAYER 1: " + to_string(gTable.players[0].GetPar(0)) + "      PLAYER 2: " + to_string(gTable.players[1].GetPar(0)));
		printText(GLUT_WINDOW_WIDTH / 2 - 6, GLUT_WINDOW_HEIGHT / 2 - 10, "HOLE 2:   PLAYER 1: " + to_string(gTable.players[0].GetPar(1)) + "      PLAYER 2: " + to_string(gTable.players[1].GetPar(1)));
		if((gTable.players[0].GetPar(0) + gTable.players[0].GetPar(1)) < (gTable.players[0].GetPar(1) + gTable.players[1].GetPar(1)))
			printText(GLUT_WINDOW_WIDTH / 2, GLUT_WINDOW_HEIGHT / 2 - 20, "PLAYER 1 WINS");
		else if ((gTable.players[0].GetPar(0) + gTable.players[0].GetPar(1)) > (gTable.players[0].GetPar(1) + gTable.players[1].GetPar(1)))
			printText(GLUT_WINDOW_WIDTH / 2, GLUT_WINDOW_HEIGHT / 2 - 20, "PLAYER 2 WINS");
		else
			printText(GLUT_WINDOW_WIDTH / 2, GLUT_WINDOW_HEIGHT / 2 - 20, "DRAW");
		break;

	case 3:
		printText(GLUT_WINDOW_WIDTH / 2 - 6, GLUT_WINDOW_HEIGHT / 2, "HOLE 1:   PLAYER 1: " + to_string(gTable.players[0].GetPar(3)) + "      PLAYER 2: " + to_string(gTable.players[1].GetPar(3)));
		printText(GLUT_WINDOW_WIDTH / 2 - 6, GLUT_WINDOW_HEIGHT / 2 - 10, "HOLE 2:   PLAYER 1: " + to_string(gTable.players[0].GetPar(4)) + "      PLAYER 2: " + to_string(gTable.players[1].GetPar(4)));
		if ((gTable.players[0].GetPar(3) + gTable.players[0].GetPar(4)) < (gTable.players[0].GetPar(3) + gTable.players[1].GetPar(4)))
			printText(GLUT_WINDOW_WIDTH / 2, GLUT_WINDOW_HEIGHT / 2 - 20, "PLAYER 1 WINS");
		else if((gTable.players[0].GetPar(3) + gTable.players[0].GetPar(4)) > (gTable.players[0].GetPar(3) + gTable.players[1].GetPar(4)))
			printText(GLUT_WINDOW_WIDTH / 2, GLUT_WINDOW_HEIGHT / 2 - 20, "PLAYER 2 WINS");
		else
			printText(GLUT_WINDOW_WIDTH / 2, GLUT_WINDOW_HEIGHT / 2 - 20, "DRAW");
		break;
	}
}

void DoCamera(int ms) //applies camera controls to move the camera position
{
	static const vec3 up(0.0,1.0,0.0);

	if(gCamRotate)
	{
		if(gCamL)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localL = up.Cross(camDir);
			vec3 inc = (localL* ((gCamRotSpeed*ms)/100.0) );
			gCamLookAt = gCamPos + camDir + inc;
		}
		if(gCamR)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = up.Cross(camDir);
			vec3 inc = (localR* ((gCamRotSpeed*ms)/100.0) );
			gCamLookAt = gCamPos + camDir - inc;
		}
		if(gCamU)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp* ((gCamMoveSpeed*ms)/500.0) );
			gCamLookAt = gCamPos + camDir + inc;
		}
		if(gCamD)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp* ((gCamMoveSpeed*ms)/500.0) );
			gCamLookAt = gCamPos + camDir - inc;
		}		
	}
	else
	{
		if(gCamL)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localL = up.Cross(camDir);
			vec3 inc = (localL* ((gCamMoveSpeed*ms)/ 100.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamR)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 inc = (localR* ((gCamMoveSpeed*ms)/ 100.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamU)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp* ((gCamMoveSpeed*ms)/ 500.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamD)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localDown = camDir.Cross(localR);
			vec3 inc = (localDown* ((gCamMoveSpeed*ms)/ 500.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
	}

	if(gCamZin)
	{
		vec3 camDir = (gCamLookAt - gCamPos).Normalised();
		vec3 inc = (camDir* ((gCamMoveSpeed*ms)/100.0) );
		gCamPos += inc;
		gCamLookAt += inc;
	}
	if(gCamZout)
	{
		vec3 camDir = (gCamLookAt - gCamPos).Normalised();
		vec3 inc = (camDir* ((gCamMoveSpeed*ms)/100.0) );
		gCamPos -= inc;
		gCamLookAt -= inc;
	}
}

void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (gTable.balls[0].pocketed == true && gTable.balls[1].pocketed == true) //if hole is completed
	{
		if (changeCourse == false) //if not changing course
		{
			if (gTable.courseNum == 1 || gTable.courseNum == 3) //if end level, end game
			{
				gameOver = true;
			}
			else //if not move to next course
			{ 
				gTable.courseNum += 1;
				changeCourse = true;
				gTable.balls[0].Reset();
				gTable.balls[1].Reset();
			}
		}
	}

	if (gameOver) //if game end, go to end screen
	{
		endScreen();
	}
	else if (onMenu == false && gameOver == false) // if not on gameOver or onMenu render the game
	{
		vec3 ballPosition;

		//set camera
		glLoadIdentity();
		if (gTable.players[0].turn)
		{
			ballPosition = (gTable.balls[0].position(0), gTable.balls[0].position(1), 1);
			gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);
		}
		else
		{
			ballPosition = (gTable.balls[1].position, 1);
			gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);
		}

		//gluLookAt(gCamPos(0),gCamPos(1),gCamPos(2),gCamLookAt(0),gCamLookAt(1),gCamLookAt(2),0.0f,1.0f,0.0f);

		//draw the ball
		for (int i = 0; i < NUM_BALLS; i++) //if balls are not pocketed, renders the ball
		{
			if (gTable.balls[i].pocketed == false)
			{
				glPushMatrix();
				glTranslatef(gTable.balls[i].position(0), (BALL_RADIUS), gTable.balls[i].position(1));
				#if DRAW_SOLID
				glutSolidSphere(gTable.balls[i].radius, 32, 32);
				#else
				glutWireSphere(gTable.balls[i].radius, 12, 12);
				#endif
			}
			if (i == 0) //renders the colours
			{
				glMaterialfv(GL_FRONT, GL_AMBIENT, ballColourOne);
				gTable.balls[i].index = 0;
			}
			else
			{
				glMaterialfv(GL_FRONT, GL_AMBIENT, ballColourTwo);
				gTable.balls[i].index = 1;
			}
			glPopMatrix();
		}

		for (int i = 0; i < NUM_CUSHIONS; i++) //renders the walls
		{
			if (wireMesh == false) //if wireMesh is not selected render solid
			{
				glBegin(GL_POLYGON);
				glMaterialfv(GL_FRONT, GL_AMBIENT, wallColour);
			}
			else
				glBegin(GL_LINE_LOOP);

			glVertex3f(gTable.cushions[i].vertices[0](0), 0.0, gTable.cushions[i].vertices[0](1));
			glVertex3f(gTable.cushions[i].vertices[0](0), 0.1, gTable.cushions[i].vertices[0](1));
			glVertex3f(gTable.cushions[i].vertices[1](0), 0.1, gTable.cushions[i].vertices[1](1));
			glVertex3f(gTable.cushions[i].vertices[1](0), 0.0, gTable.cushions[i].vertices[1](1));
			glEnd();

			/*	glBegin(GL_LINES);
			glVertex3f(gTable.cushions[i].centre(0), 0.05, gTable.cushions[i].centre(1));
			glVertex3f(gTable.cushions[i].normal(0)/2+ gTable.cushions[i].centre(0), 0.05, gTable.cushions[i].normal(1)/2+ gTable.cushions[i].centre(1));
			glEnd();*/
		}

		/*for (int i = 0; i < NUM_POCKETS; i++)
		{
			glBegin(GL_POLYGON);
			glMaterialfv(GL_FRONT, GL_AMBIENT, pocketColour);
			glVertex3f(gTable.pockets[i].PocketPosition[0](0), 0.0, gTable.pockets[i].PocketPosition[0](1));
			glVertex3f(gTable.pockets[i].PocketPosition[0](0), 0.1, gTable.pockets[i].PocketPosition[0](1));
			glVertex3f(gTable.pockets[i].PocketPosition[1](0), 0.1, gTable.pockets[i].PocketPosition[1](1));
			glVertex3f(gTable.pockets[i].PocketPosition[1](0), 0.0, gTable.pockets[i].PocketPosition[1](1));
			glEnd();

			glBegin(GL_LINES);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(gTable.pockets[i].PocketCentre(0), 0.05, gTable.pockets[i].PocketCentre(1));
			glVertex3f(gTable.pockets[i].PocketNormal(0) / 2 + gTable.pockets[i].PocketCentre(0), 0.05, gTable.pockets[i].PocketNormal(1) / 2 + gTable.pockets[i].PocketCentre(1));
			glEnd();
		}*/

		//draw floor

		if(wireMesh == false) //render floor if non-mesh and use colour
		{
			glBegin(GL_POLYGON);
			glMaterialfv(GL_FRONT, GL_AMBIENT, floorColour);
			glVertex3f(TABLE_X, 0.0, -TABLE_Z);
			glVertex3f(-TABLE_X, 0.0, -TABLE_Z);
			glVertex3f(-TABLE_X, 0.0, TABLE_Z);
			glVertex3f(TABLE_X, 0.0, TABLE_Z);
			glEnd();
		}

	
		glBegin(GL_LINE_LOOP); //render the pockets as a 2d circle
		glMaterialfv(GL_FRONT, GL_AMBIENT, pocketColour);
		for (int i = 0; i < 30; i++)
		{
			float theta = 2.0f * 3.1415926f * float(i) / float(30);//get the current angle 

			float x = gTable.pockets[0].PocketRadius * cosf(theta);//calculate the x component 
			float y = gTable.pockets[0].PocketRadius * sinf(theta);//calculate the y component 

			glVertex3f(x + gTable.pockets[0].PocketPosition.elem[0], 0.0, y + gTable.pockets[0].PocketPosition.elem[1]);//output vertex 
		}
		glEnd();

		if (gDoCue) //draw the cue
		{
			glBegin(GL_POLYGON);
			float cuex = sin(gCueAngle) * gCuePower;
			float cuez = cos(gCueAngle) * gCuePower;

			glMaterialfv(GL_FRONT, GL_AMBIENT, pocketColour);

			if (gTable.players[0].turn) //draws based on the player turn and that players ball position
			{
				if (gTable.players[0].turn)
				{
					glVertex3f(gTable.balls[0].position(0), (BALL_RADIUS), gTable.balls[0].position(1));
					glVertex3f((gTable.balls[0].position(0) + cuex - 0.01), (BALL_RADIUS), (gTable.balls[0].position(1) + cuez));
					glVertex3f((gTable.balls[0].position(0) + cuex + 0.01), (BALL_RADIUS), (gTable.balls[0].position(1) + cuez));
					glEnd();
				}
			}
			else
			{
				if (gTable.players[1].turn)
				{
					glVertex3f(gTable.balls[1].position(0), (BALL_RADIUS), gTable.balls[1].position(1));
					glVertex3f((gTable.balls[1].position(0) + cuex - 0.01), (BALL_RADIUS), (gTable.balls[1].position(1) + cuez));
					glVertex3f((gTable.balls[1].position(0) + cuex + 0.01), (BALL_RADIUS), (gTable.balls[1].position(1) + cuez));
					glEnd();
				}
			}
		}
		glPopAttrib();
		updateHUD();
	}
	else
		mainScreen(); //go to mainscreen if not

	glFlush();
	glutSwapBuffers();
}

void SpecKeyboardFunc(int key, int x, int y) //cue controls
{
	switch(key)
	{
		case GLUT_KEY_LEFT:
		{
			gCueControl[0] = true;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			gCueControl[1] = true;
			break;
		}
		case GLUT_KEY_UP:
		{
			gCueControl[2] = true;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			gCueControl[3] = true;
			break;
		}
	}
}

void SpecKeyboardUpFunc(int key, int x, int y) //cue controls
{
	switch(key)
	{
		case GLUT_KEY_LEFT:
		{
			gCueControl[0] = false;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			gCueControl[1] = false;
			break;
		}
		case GLUT_KEY_UP:
		{
			gCueControl[2] = false;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			gCueControl[3] = false;
			break;
		}
	}
}

void KeyboardFunc(unsigned char key, int x, int y) 
{
	switch(key)
	{
		case(13):
		{
			if (onMenu) //if enter is pressed go to course
			{
				onMenu = false;
				changeCourse = true;
			}
			else
			{
				if (gDoCue)
				{
					vec2 imp((-sin(gCueAngle) * gCuePower * gCueBallFactor),
						(-cos(gCueAngle) * gCuePower * gCueBallFactor));

					if (gTable.players[0].turn) //applies impulse if it's that players turn then changes turn
					{
						gTable.balls[0].ApplyImpulse(imp);
						gTable.players[0].SetPar(gTable.courseNum, 1); //adds one to the shot count
						if (gTable.balls[1].pocketed == false) //swaps turn
						{
							gTable.players[0].turn = false;
							gTable.players[1].turn = true;
						}
					}
					else
					{
						gTable.balls[1].ApplyImpulse(imp);
						gTable.players[1].SetPar(gTable.courseNum, 1); //adds one to the shot count
						if (gTable.balls[0].pocketed == false) //swaps turn
						{
							gTable.players[0].turn = true;
							gTable.players[1].turn = false;
						}
					}
				}
			}
			break;
		}

		case(27):
		{
			for(int i=0;i<NUM_BALLS;i++) //if esc is pressed reset balls
			{
				gTable.balls[i].Reset();
			}
			if (gameOver) //if on game over reset and go to main menu
			{
				gameOver = false;
				onMenu = true;
				gTable.players[0].ResetPar(0), gTable.players[0].ResetPar(1), gTable.players[0].ResetPar(3), gTable.players[0].ResetPar(4);
				gTable.players[1].ResetPar(0), gTable.players[1].ResetPar(1), gTable.players[1].ResetPar(3), gTable.players[1].ResetPar(4);
			}
			break;
		}

		case(32): //caps stops camera rotation
		{
			gCamRotate = false; 
			break;
		}

		case('a'): //cycle through the menu left or do camera
		{
			if (onMenu)
			{
				if (gTable.courseNum == 0)
				{
					gTable.courseNum = 3;
				}
				else
					gTable.courseNum = gTable.courseNum - 1;
			}
			else
			{
				gCamL = true;
			}
			break;
		}

		case('d'): //cycle through the menu right or do camera
		{
			if (onMenu)
			{
				if (gTable.courseNum == 3)
				{
					gTable.courseNum = 0;
				}
				else
					gTable.courseNum = gTable.courseNum + 1;
			}
			else
			{
				gCamR = true;
			}
			break;
		}

		case('w'): //other camera controls
		{
			gCamU = true;
			break;
		}

		case('s'):
		{
			gCamD = true;
			break;
		}

		case('r'):
		{
			gCamZin = true;
			break;
		}

		case('f'):
		{
			gCamZout = true;
			break;
		}
	}
}

void KeyboardUpFunc(unsigned char key, int x, int y) 
{
	switch(key) //camera controls
	{
	case(32):
		{
			gCamRotate = true;
			break;
		}
	case('a'):
		{
			gCamL = false;
			break;
		}
	case('d'):
		{
			gCamR = false;
			break;
		}
	case('w'):
		{
			gCamU = false;
			break;
		}
	case('s'):
		{
			gCamD = false;
			break;
		}
	case('r'):
		{
			gCamZin = false;
			break;
		}
	case('f'):
		{
			gCamZout = false;
			break;
		}
	}
}

void ChangeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0) h = 1;
	float ratio = 1.0* w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,0.2,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0.0,0.7,2.1, 0.0,0.0,0.0, 0.0f,1.0f,0.0f);
	gluLookAt(gCamPos(0),gCamPos(1),gCamPos(2),gCamLookAt(0),gCamLookAt(1),gCamLookAt(2),0.0f,1.0f,0.0f);
}

void InitLights(void)
{
	GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 0.5 };
	GLfloat mat_shininess[] = { 10.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	GLfloat light_ambient[] = { 2.0, 2.0, 2.0, 1.0 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
}

void UpdateScene(int ms) 
{
	if(!onMenu) //do camera control but not if on menu
	{
		if (gTable.AnyBallsMoving() == false) gDoCue = true; //if not balls are moving cue can be done
		else gDoCue = false;

		if (gDoCue)
		{
			if (gCueControl[0]) gCueAngle -= ((gCueAngleSpeed * ms) / 1000);
			if (gCueControl[1]) gCueAngle += ((gCueAngleSpeed * ms) / 1000);
			if (gCueAngle < 0.0) gCueAngle += TWO_PI;
			if (gCueAngle > TWO_PI) gCueAngle -= TWO_PI;

			if (gCueControl[2]) gCuePower += ((gCuePowerSpeed * ms) / 1000);
			if (gCueControl[3]) gCuePower -= ((gCuePowerSpeed * ms) / 1000);
			if (gCuePower > gCuePowerMax) gCuePower = gCuePowerMax;
			if (gCuePower < gCuePowerMin) gCuePower = gCuePowerMin;
		}

		DoCamera(ms);

		gTable.Update(ms);
	}

	if (changeCourse) //if course changes, re-assign cushions, pockets, players, and ball positions
	{
		gTable.SetupCushions();
		gTable.SetupPockets();
		gTable.SetupPlayers();
		gTable.balls[0].Reset();
		gTable.balls[1].Reset();
		changeCourse = false;
	}

	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutPostRedisplay();
}

int _tmain(int argc, _TCHAR* argv[])
{
	glutInit(&argc, ((char **)argv));

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE| GLUT_RGBA );

	glutInitWindowPosition(0,0);
	glutInitWindowSize(1920,1080);
	//glutFullScreen();
	glutCreateWindow("CMP9133M Advanced Programming - 14468387");
	#if DRAW_SOLID
	InitLights();
	#endif
	glutDisplayFunc(RenderScene);
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutReshapeFunc(ChangeSize);
	glutIdleFunc(RenderScene);
	
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(KeyboardFunc);
	glutKeyboardUpFunc(KeyboardUpFunc);
	glutSpecialFunc(SpecKeyboardFunc);
	glutSpecialUpFunc(SpecKeyboardUpFunc);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}
