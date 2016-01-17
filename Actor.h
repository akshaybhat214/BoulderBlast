#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <iostream>
using namespace std;
class StudentWorld;

class Actor: public GraphObject
{
	public:
		Actor(StudentWorld* world,int imageID, int startX, int startY, Direction dir = none)
			:GraphObject(imageID,startX,startY,dir), m_dead(false), m_world(world)
		{
			setVisible(true);
		}

		virtual void setDead() 
		{
			m_dead=true;
			setVisible(false);
		}

		bool isDead() const
		{
			return m_dead;
		}
		virtual void doSomething()=0; //Actor is ABC, but all actors will have to call doSomething()
		virtual bool takeHit()	//All actors that can stop bullets will call this function, and it will just return in most cases
		{
			return false;
		}
		virtual ~Actor()	
		 {}

		StudentWorld* getWorld() const
		{
			return m_world;
		}

	private:
		bool m_dead;
		StudentWorld* m_world;
};

class Player: public Actor
{
	public:
		Player(StudentWorld* world, int startX, int startY);

		virtual void doSomething(); 
		virtual void setDead();
		int getAmmo() const;		
		void increaseAmmo(int value);	
		int getHealth() const;
		void restoreFullHealth();
		void finishedLevel();
		bool isFinished() const;
		virtual bool takeHit();

	private:
		int m_hitPoints;
		int m_ammo;
		bool m_finished; //means the actor finished the level.
};

class Wall: public Actor
{
	public:
		Wall(int startX, int startY);
		void doSomething();
};

class Exit: public Actor
{
	public:
		Exit(StudentWorld* world, int startX, int startY);
		void doSomething();
};

class Goodie: public Actor
{
	public:
		Goodie(StudentWorld* world, int startX, int startY,int imageId)
			:Actor(world, imageId, startX ,startY, none)
		{}
		virtual void doSomething();
		bool samePosition() const; //All Goodies check position same way
		virtual void goodiePrize()=0; //ABC doesn't do anything
};

class Jewel: public Goodie //not a goodie, but derived from  goodie base class for similar characteristics.
{
	public:
		Jewel(StudentWorld* world, int startX, int startY);
		 void goodiePrize();
};

class ExtraLifeGoodie: public Goodie
{
	public:
		ExtraLifeGoodie(StudentWorld* world, int startX, int startY);
		 void goodiePrize();
};

class RestoreHealthGoodie: public Goodie
{
	public:
		RestoreHealthGoodie(StudentWorld* world, int startX, int startY);
		void goodiePrize();
};

class AmmoGoodie: public Goodie
{
	public:
		AmmoGoodie(StudentWorld* world, int startX, int startY);
		void goodiePrize();
};

class Boulder: public Actor
{
	public:
		Boulder(StudentWorld* world, int startX, int startY);
		virtual void doSomething();
		bool takeHit();
		bool push(enum GraphObject::Direction dir); //To be implemented
	private:
		int m_hitPoints;
};

class Bullet: public Actor
{
	public:
		Bullet(StudentWorld* world, int startX, int startY,enum GraphObject::Direction dir);
		void doSomething();
};

class Hole: public Actor
{
	public:
		Hole(StudentWorld* world, int startX, int startY);
		void doSomething();
};

class Robot: public Actor
{
public:
	Robot(StudentWorld* world, int startX, int startY,int imageId);

    bool takeHit();
	virtual void setDead(); //Virtual so that kleptobots can drop goodies
	virtual void doSomething();
	bool robotFire();
	virtual void robotAction()=0; //ABC does not actually do anything
	bool attemptRobotMove(int m_x,int m_y);
	void setMaxHitPoints(int points)
	{   m_hitPoints=points;     }
	void setPointsPerKill(int points)
	{   pointsPerKill=points;    }

private:
	int m_hitPoints;
	int m_ticks;	//Robot moves every m_ticks
	int tickCount; //Total number of ticks robot is alive for
	int pointsPerKill; //Points player gets for killing the robot.
};

class SnarlBot: public Robot
{
	public:
		SnarlBot(StudentWorld* world, int startX, int startY, GraphObject::Direction dir);
		virtual void robotAction();
};

class KleptoBot: public Robot
{
	public:
		KleptoBot(StudentWorld* world, int startX, int startY, int imId);
		virtual void robotAction();
		virtual void setDead();

	private:
		char goodieStolen;
		int distanceToTurn; 	
};

class AngryKleptoBot: public KleptoBot
{
	public:
		AngryKleptoBot(StudentWorld* world, int startX, int startY,int imId);
		virtual void robotAction();
};

class RobotFactory: public Actor
{
	public:
		RobotFactory(StudentWorld* world, int startX, int startY,bool Angry);
		virtual void doSomething();

	private:
		bool isAngry;
		int kleptoCount;
};

#endif // ACTOR_H_