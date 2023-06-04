/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"

/*-----------------------------------------------------------
  Macros
  -----------------------------------------------------------*/
#define TABLE_X					(1.0f) 
#define TABLE_Z					(1.0f)
#define TABLE_Y					(0.1f)
#define BALL_RADIUS				(0.02f)
#define BALL_MASS				(0.1f)
#define TWO_PI					(6.2832f)
#define	SIM_UPDATE_MS			(10)
#define NUM_BALLS				(2)		
#define NUM_CUSHIONS			(14) //max needed
#define NUM_POCKETS				(1)
#define POCKET_RADIUS			(0.1f)
#define NUM_PLAYERS				(2)
/*-----------------------------------------------------------
  plane normals
  -----------------------------------------------------------*/
/*
extern vec2	gPlaneNormal_Left;
extern vec2	gPlaneNormal_Top;
extern vec2	gPlaneNormal_Right;
extern vec2	gPlaneNormal_Bottom;
*/



/*-----------------------------------------------------------
  cushion class
  -----------------------------------------------------------*/
class cushion
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;

	void MakeNormal(void);
	void MakeCentre(void);
};

/*-----------------------------------------------------------
player class
-----------------------------------------------------------*/

class player
{
private:
	int coursePar[4];

public:
	bool	turn;

	void SetPar(int playNum, int val);
	int GetPar(int playerNum);
	void ResetPar(int playerNum);
	void ResetPlayer(int courseNum);
};

/*-----------------------------------------------------------
pocket class
-----------------------------------------------------------*/
class pocket
{
public:
	float	PocketRadius = 0.05f;
	vec2	PocketPosition;

	/*vec2	PocketNormal;
	vec2	PocketCentre;*/

	/*	void MakeNormal(void);
	void MakeCentre(void);*/
};


/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/

class ball
{
	static int ballIndexCnt;
public:
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;
	bool	pocketed;

	ball() : position(0.0), velocity(0.0), radius(BALL_RADIUS),
		mass(BALL_MASS), 
		pocketed(false) {index = ballIndexCnt++; Reset();}
	
	void Reset(void);
	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const cushion &c);
	void DoPocketCollision(const pocket & b);
	void DoBallCollision(ball &b);
	void Update(int ms);
	
	bool HasHitPlane(const cushion &c) const;
	bool HasHitBall(const ball &b) const;
	bool HasHitPocket(const pocket &d) const;

	void HitPocket(const pocket &c);
	void HitPlane(const cushion &c);
	void HitBall(ball &b);
};

/*-----------------------------------------------------------
  table class
  -----------------------------------------------------------*/
class table
{
public:
	ball balls[NUM_BALLS];	
	cushion cushions[NUM_CUSHIONS];
	pocket pockets[NUM_POCKETS];
	player players[NUM_PLAYERS];
	int courseNum;
	void SetupCushions();
	void SetupPockets();
	void SetupPlayers(void);
	void Update(int ms);	
	bool AnyBallsMoving(void) const;
};

/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
extern table gTable;
