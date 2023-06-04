/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include "include/irrKlang.h"
#pragma comment(lib, "irrKlang.lib")

using namespace irrklang;

/*-----------------------------------------------------------
  macros
  -----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

/*-----------------------------------------------------------
  globals
  -----------------------------------------------------------*/
/*
vec2	gPlaneNormal_Left(1.0,0.0);
vec2	gPlaneNormal_Top(0.0,1.0);
vec2	gPlaneNormal_Right(-1.0,0.0);
vec2	gPlaneNormal_Bottom(0.0,-1.0);
*/

table			gTable;
ISoundEngine*	SoundEngine = createIrrKlangDevice();

static const float gRackPositionX[] = {0.0f,0.0f,(BALL_RADIUS*2.0f),(-BALL_RADIUS*2.0f),(BALL_RADIUS*4.0f)}; 
static const float gRackPositionZ[] = {0.5f,0.0f,(-BALL_RADIUS*3.0f),(-BALL_RADIUS*3.0f)}; 

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;
int spacer = 1;

/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//makes a normal for use in collision of the ball with the cushion
	vec2 temp = vertices[1]-vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	//calculate centre point of the cushion
	centre = vertices[0];
	centre += vertices[1];
	centre/=2.0;
}

////old pocket calculations
//void pocket::MakeNormal(void)
//{
//	//can do this in 2d
//	vec2 temp = PocketPosition[1] - PocketPosition[0];
//	PocketNormal(0) = temp(1);
//	PocketNormal(1) = -temp(0);
//	PocketNormal.Normalise();
//}
//void pocket::MakeCentre(void)
//{
//	PocketCentre = PocketPosition[0];
//	PocketCentre += PocketPosition[1];
//	PocketCentre /= 2.0;
//}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;

void ball::Reset(void)
{
	//set velocity and other variable to zero
	velocity = 0.0;
	pocketed = false;

	//reset ball positions based on courseNum and what ball they are
	switch (gTable.courseNum)
	{
		case 0:
			if (index == 0)
			{
				position(1) = 0.5;
				position(0) = 0.7;
				return;
			}
			else
			{
				position(1) = 0.9;
				position(0) = 0.7;
				return;
			}
			break;
		case 1:
			if (index == 0)
			{
				position(1) = 0.7;
				position(0) = 0.1;
				return;
			}
			else
			{
				position(1) = 0.7;
				position(0) = -0.1;
				return;
			}
			break;
		case 2:
			if (index == 0)
			{
				position(1) = 0.7;
				position(0) = 0.9;
				return;
			}
			else
			{
				position(1) = 0.9;
				position(0) = 0.7;
				return;
			}
			break;
		case 3:
			if (index == 0)
			{
				position(1) = 0.8;
				position(0) = 0.8;
				return;
			}
			else
			{
				position(1) = -0.8;
				position(0) = -0.8;
				return;
			}
			break;
	}
}

void ball::ApplyImpulse(vec2 imp)
{
	velocity = imp; //applies ball velocity to the ball
	SoundEngine->play2D("Audio/putter.wav"); //plays ball hitting audio
}

void ball::ApplyFrictionForce(int ms)
{
	if(velocity.Magnitude()<=0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms)/1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if(velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void ball::DoBallCollision(ball &b)
{
	if(HasHitBall(b)) HitBall(b); //ball collision methods
}

void ball::DoPlaneCollision(const cushion &b)
{
	if(HasHitPlane(b)) HitPlane(b); //plane collision methods
}

void ball::DoPocketCollision(const pocket &b)
{
	if (HasHitPocket(b)) HitPocket(b); //pocket collision methods
}

void ball::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms)/1000.0f);
	//set small velocities to zero
	if(velocity.Magnitude()<SMALL_VELOCITY) velocity = 0.0;
}

bool ball::HasHitPlane(const cushion &c) const
{
	//if moving away from plane, cannot hit
	if(velocity.Dot(c.normal) >= 0.0) return false;
	
	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if(sep > radius) return false; 
	else

	//if not within confines of the cushion wall return false
	if (position(0) + (radius) < fmin(c.vertices[0](0), c.vertices[1](0)) || position(0) - (radius) > fmax(c.vertices[0](0), c.vertices[1](0)))
		return false;
	if (position(1) + (radius) < fmin(c.vertices[0](1), c.vertices[1](1)) || position(1) - (radius) > fmax(c.vertices[0](1), c.vertices[1](1)))
		return false;

	return true;
}

bool ball::HasHitBall(const ball &b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float) relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if(relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if(dist > (radius+b.radius)) return false;
	return true;
}

bool ball::HasHitPocket(const pocket &d) const
{
	vec2 relPos = position - d.PocketPosition;
	double sep = relPos.Magnitude();
	if (sep > d.PocketRadius) 
		return false;
	else 
		return true;

	////if moving away from plane, cannot hit
	///*if (velocity.Dot(d.PocketNormal) >= 0.0) return false;
	//
	//vec2 relPos = position - d.PocketPosition[0];
	//double sep = relPos.Dot(d.PocketNormal);*/
	//if (sep > radius) return false; 
	//else

	//if (position(0) + (radius) < fmin(d.PocketPosition[0](0), d.PocketPosition[1](0)) || position(0) - (radius) > fmax(d.PocketPosition[0](0), d.PocketPosition[1](0)))
	//	return false;
	//if (position(1) + (radius) < fmin(d.PocketPosition[0](1), d.PocketPosition[1](1)) || position(1) - (radius) > fmax(d.PocketPosition[0](1), d.PocketPosition[1](1)))
	//	return false;
	//return true;
	return false;
}

void ball::HitPocket(const pocket &c)
{
	//hits pockets move it out of the course, and set pocketed to true
	velocity = 0;
	if (index == 0)
	{
		position(1) = 2.5;
		position(0) = 2.5;
		pocketed = true;
	}
	else
	{
		position(1) = 2.5*spacer;
		position(0) = 2.5*spacer;
		spacer += 1;
		pocketed = true;
	}
}

void ball::HitPlane(const cushion &c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0+gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta; 

/*
	//assume elastic collision
	//find plane normal
	vec2 planeNorm = gPlaneNormal_Left;
	//split velocity into 2 components:
	//find velocity component perpendicular to plane
	vec2 perp = planeNorm*(velocity.Dot(planeNorm));
	//find velocity component parallel to plane
	vec2 parallel = velocity - perp;
	//reverse perpendicular component
	//parallel component is unchanged
	velocity = parallel + (-perp)*gCoeffRestitution;
*/
}

void ball::HitBall(ball &b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity-(relDir*perpV);
	vec2 parallelV2 = b.velocity-(relDir*perpV2);
	
	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV*(mass-b.mass))/sumMass) + (float)((perpV2*(2.0*b.mass))/sumMass);
	float perpVNew2 = (float)((perpV2*(b.mass-mass))/sumMass) + (float)((perpV*(2.0*mass))/sumMass);
	
	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir*perpVNew);
	b.velocity = parallelV2 + (relDir*perpVNew2);
	SoundEngine->play2D("Audio/putter.wav");
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
void table::SetupCushions(void)
{
	switch (courseNum)
	{
	case 0: //COURSE 1
		//TOP RIGHT CORNER
		cushions[0].vertices[0](0) = TABLE_X;
		cushions[0].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[0].vertices[1](0) = TABLE_X - 0.2;
		cushions[0].vertices[1](1) = TABLE_Z - 2.0;

		//TOP LEFT CORNER
		cushions[1].vertices[0](0) = -TABLE_X + 0.2;
		cushions[1].vertices[0](1) = TABLE_Z - 2;
		cushions[1].vertices[1](0) = -TABLE_X;
		cushions[1].vertices[1](1) = -TABLE_Z + 0.2;

		//BOTTOM LEFT CORNER
		cushions[2].vertices[0](0) = -TABLE_X;
		cushions[2].vertices[0](1) = -TABLE_Z + 1.8;
		cushions[2].vertices[1](0) = -TABLE_X + 0.2;
		cushions[2].vertices[1](1) = TABLE_Z;

		//BOTTOM RIGHT CORNER
		cushions[3].vertices[0](0) = -TABLE_X + 1.8;
		cushions[3].vertices[0](1) = TABLE_Z;
		cushions[3].vertices[1](0) = TABLE_X;
		cushions[3].vertices[1](1) = TABLE_Z - 0.2;

		//MIDDLE TOP WALL
		cushions[4].vertices[0](0) = TABLE_X - 1.5;
		cushions[4].vertices[0](1) = -TABLE_Z + 0.7;
		cushions[4].vertices[1](0) = TABLE_X;
		cushions[4].vertices[1](1) = -TABLE_Z + 0.6;

		//MIDDLE MIDDLE WALL
		cushions[5].vertices[0](0) = TABLE_X - 1.5;
		cushions[5].vertices[0](1) = -TABLE_Z + 1.3;
		cushions[5].vertices[1](0) = TABLE_X - 1.5;
		cushions[5].vertices[1](1) = -TABLE_Z + 0.7;

		//MIDDLE BOTTOM WALL
		cushions[6].vertices[0](0) = TABLE_X;
		cushions[6].vertices[0](1) = TABLE_Z - 0.6;
		cushions[6].vertices[1](0) = TABLE_X - 1.5;
		cushions[6].vertices[1](1) = -TABLE_Z + 1.3;

		//BOTTOM WALL
		cushions[7].vertices[0](0) = -TABLE_X + 0.2;
		cushions[7].vertices[0](1) = TABLE_Z;
		cushions[7].vertices[1](0) = TABLE_X - 0.2;
		cushions[7].vertices[1](1) = TABLE_Z;

		//TOP WALL
		cushions[8].vertices[0](0) = TABLE_X - 0.2;
		cushions[8].vertices[0](1) = -TABLE_Z;
		cushions[8].vertices[1](0) = -TABLE_X + 0.2;
		cushions[8].vertices[1](1) = -TABLE_Z;

		//LEFT WALL
		cushions[9].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[9].vertices[0](0) = -TABLE_X;
		cushions[9].vertices[1](0) = -TABLE_X;
		cushions[9].vertices[1](1) = TABLE_Z - 0.2;

		//RIGHT BOTTOM WALL
		cushions[10].vertices[0](0) = TABLE_X;
		cushions[10].vertices[0](1) = TABLE_Z - 0.2;
		cushions[10].vertices[1](0) = TABLE_X;
		cushions[10].vertices[1](1) = -TABLE_Z + 1.4;

		//RIGHT TOP WALL
		cushions[11].vertices[0](0) = TABLE_X;
		cushions[11].vertices[0](1) = TABLE_Z - 1.4;
		cushions[11].vertices[1](0) = TABLE_X;
		cushions[11].vertices[1](1) = -TABLE_Z + 0.2;

		//BLANK
		cushions[12].vertices[0](0) = 0;
		cushions[12].vertices[0](1) = 0;
		cushions[12].vertices[1](0) = 0;
		cushions[12].vertices[1](1) = 0;

		//BLANK						  
		cushions[13].vertices[0](0) = 0;
		cushions[13].vertices[0](1) = 0;
		cushions[13].vertices[1](0) = 0;
		cushions[13].vertices[1](1) = 0;
		break;

	case 1: //COURSE 2
		//TOP RIGHT CORNER
		cushions[0].vertices[0](0) = TABLE_X;
		cushions[0].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[0].vertices[1](0) = TABLE_X - 0.2;
		cushions[0].vertices[1](1) = TABLE_Z - 2.0;

		//TOP LEFT CORNER
		cushions[1].vertices[0](0) = -TABLE_X + 0.2;
		cushions[1].vertices[0](1) = TABLE_Z - 2;
		cushions[1].vertices[1](0) = -TABLE_X;
		cushions[1].vertices[1](1) = -TABLE_Z + 0.2;

		//BOTTOM LEFT CORNER
		cushions[2].vertices[0](0) = -TABLE_X;
		cushions[2].vertices[0](1) = -TABLE_Z + 1.8;
		cushions[2].vertices[1](0) = -TABLE_X + 0.2;
		cushions[2].vertices[1](1) = TABLE_Z;

		//BOTTOM RIGHT CORNER
		cushions[3].vertices[0](0) = -TABLE_X + 1.8;
		cushions[3].vertices[0](1) = TABLE_Z;
		cushions[3].vertices[1](0) = TABLE_X;
		cushions[3].vertices[1](1) = TABLE_Z - 0.2;

		//BOTTOM WALL
		cushions[4].vertices[0](0) = -TABLE_X + 0.2;
		cushions[4].vertices[0](1) = TABLE_Z;
		cushions[4].vertices[1](0) = TABLE_X - 0.2;
		cushions[4].vertices[1](1) = TABLE_Z;

		//TOP WALL
		cushions[5].vertices[0](0) = TABLE_X - 0.2;
		cushions[5].vertices[0](1) = -TABLE_Z;
		cushions[5].vertices[1](0) = -TABLE_X + 0.2;
		cushions[5].vertices[1](1) = -TABLE_Z;

		//LEFT WALL
		cushions[6].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[6].vertices[0](0) = -TABLE_X;
		cushions[6].vertices[1](0) = -TABLE_X;
		cushions[6].vertices[1](1) = TABLE_Z - 0.2;

		//RIGHT WALL
		cushions[7].vertices[0](0) = TABLE_X;
		cushions[7].vertices[0](1) = TABLE_Z - 0.2;
		cushions[7].vertices[1](0) = TABLE_X;
		cushions[7].vertices[1](1) = -TABLE_Z + 0.2;

		//SQUARE RIGHT WALL
		cushions[8].vertices[0](0) = TABLE_X - 0.5; 
		cushions[8].vertices[0](1) = -TABLE_Z + 0.5;
		cushions[8].vertices[1](0) = TABLE_X - 0.5;
		cushions[8].vertices[1](1) = TABLE_Z - 0.5;

		//SQUARE LEFT WALL
		cushions[9].vertices[0](0) = TABLE_Z - 1.5; 
		cushions[9].vertices[0](1) = -TABLE_Z + 1.5; 
		cushions[9].vertices[1](0) = TABLE_X - 1.5;
		cushions[9].vertices[1](1) = TABLE_X - 1.5;

		//SQUARE TOP WALL
		cushions[10].vertices[0](0) = TABLE_Z - 1.5;
		cushions[10].vertices[0](1) = TABLE_X - 1.5;
		cushions[10].vertices[1](0) = -TABLE_Z + 1.5;
		cushions[10].vertices[1](1) = TABLE_X - 1.5; 

		//SQUARE BOTTOM WALL
		cushions[11].vertices[0](0) = TABLE_X - 0.5; 
		cushions[11].vertices[0](1) = TABLE_Z - 0.5;
		cushions[11].vertices[1](0) = -TABLE_Z + 0.5;
		cushions[11].vertices[1](1) = TABLE_X - 0.5;

		//BLANK
		cushions[12].vertices[0](0) = 0;
		cushions[12].vertices[0](1) = 0;
		cushions[12].vertices[1](0) = 0;
		cushions[12].vertices[1](1) = 0;

		//BLANK						  
		cushions[13].vertices[0](0) = 0;
		cushions[13].vertices[0](1) = 0;
		cushions[13].vertices[1](0) = 0;
		cushions[13].vertices[1](1) = 0;

		break;

	case 2: //COURSE 3
		//TOP RIGHT CORNER
		cushions[0].vertices[0](0) = TABLE_X;
		cushions[0].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[0].vertices[1](0) = TABLE_X - 0.2;
		cushions[0].vertices[1](1) = TABLE_Z - 2.0;

		//TOP LEFT CORNER
		cushions[1].vertices[0](0) = -TABLE_X + 0.2;
		cushions[1].vertices[0](1) = TABLE_Z - 2;
		cushions[1].vertices[1](0) = -TABLE_X;
		cushions[1].vertices[1](1) = -TABLE_Z + 0.2;

		//BOTTOM LEFT CORNER
		cushions[2].vertices[0](0) = -TABLE_X;
		cushions[2].vertices[0](1) = -TABLE_Z + 1.8;
		cushions[2].vertices[1](0) = -TABLE_X + 0.2;
		cushions[2].vertices[1](1) = TABLE_Z;

		//BOTTOM RIGHT CORNER
		cushions[3].vertices[0](0) = -TABLE_X + 1.8;
		cushions[3].vertices[0](1) = TABLE_Z;
		cushions[3].vertices[1](0) = TABLE_X;
		cushions[3].vertices[1](1) = TABLE_Z - 0.2;

		//BOTTOM WALL
		cushions[4].vertices[0](0) = -TABLE_X + 0.2;
		cushions[4].vertices[0](1) = TABLE_Z;
		cushions[4].vertices[1](0) = TABLE_X - 0.2;
		cushions[4].vertices[1](1) = TABLE_Z;

		//TOP WALL
		cushions[5].vertices[0](0) = TABLE_X - 0.2;
		cushions[5].vertices[0](1) = -TABLE_Z;
		cushions[5].vertices[1](0) = -TABLE_X + 0.2;
		cushions[5].vertices[1](1) = -TABLE_Z;

		//LEFT WALL
		cushions[6].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[6].vertices[0](0) = -TABLE_X;
		cushions[6].vertices[1](0) = -TABLE_X;
		cushions[6].vertices[1](1) = TABLE_Z - 0.2;

		//RIGHT WALL
		cushions[7].vertices[0](0) = TABLE_X;
		cushions[7].vertices[0](1) = TABLE_Z - 0.2;
		cushions[7].vertices[1](0) = TABLE_X;
		cushions[7].vertices[1](1) = -TABLE_Z + 0.2;

		//TOP TRIANGLE LEFT WALL
		cushions[8].vertices[0](0) = TABLE_Z - 1.7;
		cushions[8].vertices[0](1) = -TABLE_Z + 1.7;
		cushions[8].vertices[1](0) = TABLE_X - 1.7;
		cushions[8].vertices[1](1) = TABLE_X - 1.7;

		//TOP TRIANGLE RIGHT WALL
		cushions[9].vertices[0](0) = TABLE_Z - 1.7;
		cushions[9].vertices[0](1) = TABLE_X - 1.7;
		cushions[9].vertices[1](0) = -TABLE_Z + 1.7;
		cushions[9].vertices[1](1) = TABLE_X - 1.7;

		//BOTTOM TRIANGLE HYPOTENUSE WALL
		cushions[10].vertices[0](0) = TABLE_X - 0.3;
		cushions[10].vertices[0](1) = TABLE_X - 1.7;
		cushions[10].vertices[1](0) = TABLE_Z - 1.7;
		cushions[10].vertices[1](1) = -TABLE_Z + 1.7;

		//BLANK
		cushions[11].vertices[0](0) = 0;
		cushions[11].vertices[0](1) = 0;
		cushions[11].vertices[1](0) = 0;
		cushions[11].vertices[1](1) = 0;

		//BLANK
		cushions[12].vertices[0](0) = 0;
		cushions[12].vertices[0](1) = 0;
		cushions[12].vertices[1](0) = 0;
		cushions[12].vertices[1](1) = 0;

		//BLANK						  
		cushions[13].vertices[0](0) = 0;
		cushions[13].vertices[0](1) = 0;
		cushions[13].vertices[1](0) = 0;
		cushions[13].vertices[1](1) = 0;
		break;

	case 3: //COURSE 4
		//TOP RIGHT CORNER
		cushions[0].vertices[0](0) = TABLE_X;
		cushions[0].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[0].vertices[1](0) = TABLE_X - 0.2;
		cushions[0].vertices[1](1) = TABLE_Z - 2.0;

		//TOP LEFT CORNER
		cushions[1].vertices[0](0) = -TABLE_X + 0.2;
		cushions[1].vertices[0](1) = TABLE_Z - 2;
		cushions[1].vertices[1](0) = -TABLE_X;
		cushions[1].vertices[1](1) = -TABLE_Z + 0.2;

		//BOTTOM LEFT CORNER
		cushions[2].vertices[0](0) = -TABLE_X;
		cushions[2].vertices[0](1) = -TABLE_Z + 1.8;
		cushions[2].vertices[1](0) = -TABLE_X + 0.2;
		cushions[2].vertices[1](1) = TABLE_Z;

		//BOTTOM RIGHT CORNER
		cushions[3].vertices[0](0) = -TABLE_X + 1.8;
		cushions[3].vertices[0](1) = TABLE_Z;
		cushions[3].vertices[1](0) = TABLE_X;
		cushions[3].vertices[1](1) = TABLE_Z - 0.2;

		//BOTTOM WALL
		cushions[4].vertices[0](0) = -TABLE_X + 0.2;
		cushions[4].vertices[0](1) = TABLE_Z;
		cushions[4].vertices[1](0) = TABLE_X - 0.2;
		cushions[4].vertices[1](1) = TABLE_Z;

		//TOP WALL
		cushions[5].vertices[0](0) = TABLE_X - 0.2;
		cushions[5].vertices[0](1) = -TABLE_Z;
		cushions[5].vertices[1](0) = -TABLE_X + 0.2;
		cushions[5].vertices[1](1) = -TABLE_Z;

		//LEFT WALL
		cushions[6].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[6].vertices[0](0) = -TABLE_X;
		cushions[6].vertices[1](0) = -TABLE_X;
		cushions[6].vertices[1](1) = TABLE_Z - 0.2;

		//RIGHT WALL
		cushions[7].vertices[0](0) = TABLE_X;
		cushions[7].vertices[0](1) = TABLE_Z - 0.2;
		cushions[7].vertices[1](0) = TABLE_X;
		cushions[7].vertices[1](1) = -TABLE_Z + 0.2;

		//BOTTOM TRIANGLE RIGHT  WALL
		cushions[8].vertices[0](0) = TABLE_X - 0.3;
		cushions[8].vertices[0](1) = -TABLE_Z + 0.6;
		cushions[8].vertices[1](0) = TABLE_X - 0.3;
		cushions[8].vertices[1](1) = TABLE_Z - 0.3;

		//TOP TRIANGLE LEFT WALL
		cushions[9].vertices[0](0) = TABLE_Z - 1.7;
		cushions[9].vertices[0](1) = -TABLE_Z + 1.4;
		cushions[9].vertices[1](0) = TABLE_X - 1.7;
		cushions[9].vertices[1](1) = TABLE_X - 1.7;

		//TOP TRIANGLE RIGHT WALL
		cushions[10].vertices[0](0) = TABLE_Z - 1.7;
		cushions[10].vertices[0](1) = TABLE_X - 1.7;
		cushions[10].vertices[1](0) = -TABLE_Z + 1.4;
		cushions[10].vertices[1](1) = TABLE_X - 1.7;

		//BOTTOM TRIANGLE LEFT WALL
		cushions[11].vertices[0](0) = TABLE_X - 0.3;
		cushions[11].vertices[0](1) = TABLE_Z - 0.3;
		cushions[11].vertices[1](0) = -TABLE_Z + 0.6;
		cushions[11].vertices[1](1) = TABLE_X - 0.3;

		//BOTTOM TRIANGLE HYPOTENUSE WALL
		cushions[12].vertices[0](0) = TABLE_Z - 1.4;
		cushions[12].vertices[0](1) = TABLE_X - 0.3;
		cushions[12].vertices[1](0) = TABLE_X - 0.3;
		cushions[12].vertices[1](1) = -TABLE_Z + 0.6;

		//TOP TRIANGLE HYPOTENUSE WALL
		cushions[13].vertices[0](0) = TABLE_X - 0.6;
		cushions[13].vertices[0](1) = TABLE_X - 1.7;
		cushions[13].vertices[1](0) = TABLE_Z - 1.7;
		cushions[13].vertices[1](1) = -TABLE_Z + 1.4;
		break;
	}

	//MAKE CUSHIONS AND NORMALS FOR CUSHIONS
	for (int i = 0; i<NUM_CUSHIONS; i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
	}
}

void table::SetupPockets()
{
	switch(courseNum)
	{
	case 0:
		//pocket of course 1
		pockets[0].PocketPosition.elem[0] = -TABLE_X + 1.5;
		pockets[0].PocketPosition.elem[1] = -TABLE_Z + 0.3;
		break;
	case 1:
		//pocket of course 2
		pockets[0].PocketPosition.elem[0] = -TABLE_X + 1.0;
		pockets[0].PocketPosition.elem[1] = -TABLE_Z + 0.2;
		break;
	case 2:
		//pocket of course 3
		pockets[0].PocketPosition.elem[0] = -TABLE_X + 0.2;
		pockets[0].PocketPosition.elem[1] = -TABLE_Z + 0.2;
		break;
	case 3:
		//pocket of course 4
		pockets[0].PocketPosition.elem[0] = -TABLE_X + 1.0;
		pockets[0].PocketPosition.elem[1] = -TABLE_Z + 1.0;
		break;
	}

	//for (int i = 0; i<NUM_POCKETS; i++)
	//{
	//	pockets[i].MakeCentre();
	//	pockets[i].MakeNormal();
	//}
}

void table::SetupPlayers(void) //sets up player at start of a course
{
	players[0].turn = true;
	players[1].turn = false;
}

void table::Update(int ms)
{
	//check for collisions for each ball
	for(int i=0;i<NUM_BALLS;i++) 
	{
		for (int j = 0; j < NUM_POCKETS; j++) //checks for collision with pockets
		{
			balls[i].DoPocketCollision(pockets[j]);
		}

		for (int j = 0; j < NUM_CUSHIONS; j++) //checks for collision with cushions
		{
			balls[i].DoPlaneCollision(cushions[j]);
		}

		for(int j=(i+1);j<NUM_BALLS;j++) //checks for collision with balls
		{
			balls[i].DoBallCollision(balls[j]);
		}
	}
	
	//update all balls
	for(int i=0;i<NUM_BALLS;i++) balls[i].Update(ms);
}

bool table::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for(int i=0;i<NUM_BALLS;i++) 
	{
		if(balls[i].velocity(0)!=0.0) return true;
		if(balls[i].velocity(1)!=0.0) return true;
	}
	return false;
}

/*-----------------------------------------------------------
player class members
-----------------------------------------------------------*/

void player::SetPar(int playNum, int val)
{
	coursePar[playNum] += val;
}

int player::GetPar(int playerNum)
{
	return coursePar[playerNum];
}

void player::ResetPar(int playerNum)
{
	coursePar[playerNum] = 0;
}

void player::ResetPlayer(int courseNum)
{
	coursePar[courseNum] = 0;
	turn = 0;
}
