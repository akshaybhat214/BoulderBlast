#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <cstdlib>

bool passesOver (char code) //Bullets get stopped by walls and factories without doing damage.
{
	return(code==' '||code=='*' || code=='r'|| code=='e'|| code=='a'|| code=='o' ||code=='x');
} 

bool isRobot(char c)
{
	return(c=='h'||c=='v' || c=='$'|| c=='%');
}

bool canDamage (char code) //all positions that can be damaged.
{
	return(code=='@'||code=='h'||code=='v'||code=='b'||code=='$'||code=='%'); //$ is kleptobot, % is angry kleptobot.
}

bool isObstacle (char c) //Obstacle to motion 
{
	return(c=='o'||c=='#'||c=='@'||c=='1'||c=='2'||c=='h'||c=='v'||c=='b'||c=='%'||c=='$');
}

bool isGoodie(char c) //Goodies do not actually include jewels
{
	return(c=='a'||c=='e'||c=='r');
}

Player::Player(StudentWorld* world, int startX, int startY)
		:Actor(world, IID_PLAYER, startX ,startY, right)
{
	m_ammo=20;		//setVisible(true);					
	m_hitPoints=20;	
	m_finished=false; 
}

int Player::getAmmo() const
{
	return m_ammo;
}

int Player::getHealth() const	
{
	return (m_hitPoints*5);
}

void Player::increaseAmmo(int value)
{
	if(value>0)
		m_ammo += value;
}

void Player::doSomething()
{
	if(m_hitPoints<=0 && ( !isDead() ))
	{
		setDead();
		return;
	}
	if (isDead())
		return;
int ch;
if (getWorld()->getKey(ch))
	{
		switch (ch)	// user hit a key this tick!
		{
		case KEY_PRESS_LEFT:
			setDirection(left);
			getWorld()->attemptMove(this,getX()-1,getY());
		break;

		case KEY_PRESS_RIGHT:
			setDirection(right);
			getWorld()->attemptMove(this,getX()+1,getY());
		break;

		case KEY_PRESS_DOWN:
			setDirection(down);
			getWorld()->attemptMove(this,getX(),getY()-1);
		break;

		case KEY_PRESS_UP:
			setDirection(up);
			getWorld()->attemptMove(this, getX(),getY()+1);
		break;

		case KEY_PRESS_ESCAPE:
			setDead();
		break;

		case KEY_PRESS_SPACE:
			if(m_ammo<=0)
				return;
			if (getDirection()==right)
				getWorld()->fireBullet(getX()+1,getY(),right);
			else if (getDirection()==up)
				getWorld()->fireBullet(getX(),getY()+1,up);
			else if (getDirection()==down)
				getWorld()->fireBullet(getX(),getY()-1,down);
			else if (getDirection()==left)
				getWorld()->fireBullet(getX()-1,getY(),left);
			else return;

			m_ammo--;
			getWorld()->playSound(SOUND_PLAYER_FIRE);
			return;
		}
	}
}

void Player::setDead()
{
	Actor::setDead();
	getWorld()->playSound(SOUND_PLAYER_DIE);
}

void Player::restoreFullHealth()
{
	m_hitPoints=20;
}

void Player::finishedLevel()
{
	m_finished=true;
}

bool Player::isFinished() const
{
	return m_finished;
}

bool Player::takeHit()
{
	m_hitPoints-=2;
	if(getHealth()<=0 && !(isDead()))
		setDead();
	else if (getHealth()>0)
		getWorld()->playSound(SOUND_PLAYER_IMPACT);

	return true;
}


Wall::Wall(int startX, int startY)
	:Actor(nullptr, IID_WALL, startX ,startY, none) 
{}
	
void Wall::doSomething()
{
	return; // A wall does not do anything
}


Exit::Exit(StudentWorld* world, int startX, int startY)
	:Actor(world, IID_EXIT, startX ,startY, none)
{
	setVisible(false);
}

void Exit::doSomething()
{
	int jewels=getWorld()->getJewelCount();
	if (jewels==0 && isVisible()==false)
	 {
		setVisible(true);
		getWorld()->playSound(SOUND_REVEAL_EXIT);
	 }
	if (isVisible() &&(getWorld()->getPlayer()->getX()==getX() && getWorld()->getPlayer()->getY()==getY()) ) //Visible and player on same spot.
	 {
		getWorld()->playSound(SOUND_FINISHED_LEVEL);
		getWorld()->getPlayer()->finishedLevel();
	 }
}


void Goodie::doSomething() 
{
	if (isDead())
		return;

	else if ( samePosition() ) //as player
		{
			setDead();
			setVisible(false);
			goodiePrize(); //Function to give player whatever the goodie/jewel should.
			getWorld()->playSound(SOUND_GOT_GOODIE);
		}
	else 
		return;
}

bool Goodie::samePosition() const //Checks whether player is in same position as the goodie.
{
 return (getWorld()->getPlayer()->getX()==getX() && getWorld()->getPlayer()->getY()==getY() );
}

Jewel::Jewel(StudentWorld* world, int startX, int startY) //This is not actually a goodie, but displays many of the same characteristics.
	:Goodie(world, startX ,startY, IID_JEWEL)
{}

void Jewel::goodiePrize()
{
		getWorld()->decJewelCount();
		getWorld()->increaseScore(50);
}


ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* world, int startX, int startY)
	:Goodie(world, startX ,startY, IID_EXTRA_LIFE)
{} 	

void ExtraLifeGoodie::goodiePrize()
{
		getWorld()->incLives();
		getWorld()->increaseScore(1000);
}

RestoreHealthGoodie::RestoreHealthGoodie(StudentWorld* world, int startX, int startY)
	:Goodie(world, startX ,startY,IID_RESTORE_HEALTH)
{}	

void RestoreHealthGoodie::goodiePrize()
{
		getWorld()->getPlayer()->restoreFullHealth();
		getWorld()->increaseScore(500);
}

AmmoGoodie::AmmoGoodie(StudentWorld* world, int startX, int startY)
	:Goodie(world, startX ,startY, IID_AMMO)
{}

void AmmoGoodie::goodiePrize()
{
		getWorld()->getPlayer()->increaseAmmo(20);
		getWorld()->increaseScore(100);
}

Boulder::Boulder(StudentWorld* world, int startX, int startY)
	:Actor(world, IID_BOULDER, startX ,startY, none), m_hitPoints(10)
{
	setVisible(true);
}

void Boulder::doSomething()
{
	if (isDead())
		return;
	else if (m_hitPoints<=0)
	{
		setDead();
		getWorld()->changeGrid(getX(), getY(), ' '); //Empties the spot which boulder was on.
	}
	else 
		return;//Do nothing
}

bool Boulder::takeHit()
{
	m_hitPoints-=2;
	if(m_hitPoints<=0 && (!isDead()))
	{
		setDead();
		getWorld()->changeGrid(getX(), getY(), ' '); //Also in doSomething() function.
	}
	return true;
}

bool Boulder::push(enum GraphObject::Direction dir) 
{
	bool movePlayer=false;
	int sx=getX(); //Current coords of boulder
	int sy=getY();

	int attemptX=0;//Where boulder will try to go
	int attemptY=0;

		switch(dir)
		{
			case left:
				attemptX= sx-1; attemptY=sy;
				break;
			case up:
				attemptX= sx; attemptY=sy+1;
				break;
			case right:
				attemptX= sx+1; attemptY=sy;
				break;
			case down:
				attemptX= sx; attemptY=sy-1;
				break;
			default:
				break;
		}

	if((getWorld()->getGridChar(attemptX,attemptY)==' ') && (getWorld()->getRobotGrid(attemptX,attemptY)==' '))
		{
			moveTo(attemptX,attemptY);
			getWorld()->changeGrid(attemptX,attemptY,'b'); 
			movePlayer=true;  //tells the function to move the player in the final step
		}
	else if(getWorld()->getGridChar(attemptX,attemptY)=='o') //DID NOT(need to)CHECK FOR ROBOTS
		{
			moveTo(attemptX,attemptY);
			getWorld()->boulderToHole(attemptX,attemptY);
			setDead();
			movePlayer=true;
		}

	if (movePlayer==true) //Moving the player only if the boulder has moved
	{
		getWorld()->changeGrid(sx,sy,' ');
		return(getWorld()->attemptMove(getWorld()->getPlayer(),sx,sy));
	}
	return false;
} 

Bullet::Bullet(StudentWorld* world, int startX, int startY,enum GraphObject::Direction dir)
	:Actor(world, IID_BULLET, startX ,startY,dir)
{
	setDirection(dir); 	
}

void Bullet::doSomething()
{
	if (isDead())
		return;

	//In initial position
	if( (passesOver(getWorld()->getGridChar(getX(),getY())) ) && (getWorld()->getRobotGrid(getX(),getY())==' '))
	{ 		//cerr<<getX()<<" , "<<getY()<<endl;
		switch(getDirection())
		{
			case left:
				moveTo(getX()-1,getY());
				break;
			case up:
				moveTo(getX(),getY()+1);
				break;
			case right:
				moveTo(getX()+1,getY());
				break;
			case down:
				moveTo(getX(),getY()-1);
				break;
			default:
				setDead();
				break;
		}	
	}
																	
	else if( (canDamage(getWorld()->getGridChar(getX(),getY())) )|| (getWorld()->getRobotGrid(getX(),getY())!=' '))
	{
		setDead();
		getWorld()->damageLocation(getX(),getY()); //function in StudentWorld to attempt to damage anything in that location.
		return; //Shouldnt do anything else this tick.
	}
	else //Hit a wall or factory
	{
		setDead();
		return;
	}

	//AFTER CHECKING
	// repeat for next position.
	 if( (canDamage(getWorld()->getGridChar(getX(),getY())) )|| (getWorld()->getRobotGrid(getX(),getY())!=' '))
	 {
		setDead();
		getWorld()->damageLocation(getX(),getY());
	 }
	 else if(!passesOver(getWorld()->getGridChar(getX(),getY()))  ) //if there is some other obstacle
	 {
		 setDead();
	 }

	return;
}


Hole::Hole(StudentWorld* world, int startX, int startY)
	:Actor(world,IID_HOLE, startX ,startY)
{}

void Hole::doSomething()
{
	return;
}

Robot::Robot(StudentWorld* world, int startX, int startY,int imageId)
		:Actor(world, imageId, startX ,startY, right)
{
		m_ticks = ((28- getWorld()->getLevel())/4);
		if (m_ticks<3)
			m_ticks=3;

		tickCount=0;
}

bool Robot::takeHit()
{
	m_hitPoints-=2;
	if(m_hitPoints<=0 && (!isDead()))
		{
			setDead();
		}
	else if (m_hitPoints>0)
		getWorld()->playSound(SOUND_ROBOT_IMPACT);	//plays sound only if it hasnt died yet

	return true;
}

void Robot::setDead()
{
	Actor::setDead();
	getWorld()->changeRobotGrid(getX(), getY(), ' '); //Empties the spot in the robot grid
	getWorld()->playSound(SOUND_ROBOT_DIE);
	getWorld()->increaseScore(pointsPerKill);
}

void Robot::doSomething()
{
	if (isDead())
		return;
	if(!(tickCount % m_ticks==0)) 
	{
		tickCount++;
		return;
	}
	else // if nth tick
	{
		robotAction();
		tickCount++;
		return;
	}
}

bool Robot::robotFire()
{
	bool inLineOfSight=false;
	int s_x=getX(); int s_y=getY();
	int bullet_x = getX();int bullet_y=getY(); //Coords that bullet will start from

	switch(getDirection()){
		case right:
			bullet_x=getX()+1;
			if(getWorld()->getGridChar(s_x+1,s_y)=='@'){
				inLineOfSight=true;
				break;
			}
			while( (passesOver(getWorld()->getGridChar(s_x+1,s_y))) && (getWorld()->getRobotGrid(s_x+1,s_y)==' ') )
			{
				if (getWorld()->getGridChar(s_x+2,s_y)=='@'){
					inLineOfSight=true; 
					break;
				}
				else
					s_x++;
			}
		break;

		case down:
			bullet_y= getY()-1;
			if(getWorld()->getGridChar(s_x,s_y-1)=='@'){
					inLineOfSight=true;
					break;
				}
			while((passesOver(getWorld()->getGridChar(s_x,s_y-1))) && (getWorld()->getRobotGrid(s_x,s_y-1)==' ') )
			{
				if (getWorld()->getGridChar(s_x,s_y-2)=='@'){
					inLineOfSight=true; 
					break;
				}
				else
					s_y--;	
			}
		break;

		case left: 	
			bullet_x = getX()-1;
			if(getWorld()->getGridChar(s_x-1,s_y)=='@'){
					inLineOfSight=true;
					break;
				}
			while((passesOver(getWorld()->getGridChar(s_x-1,s_y)))  && (getWorld()->getRobotGrid(s_x-1,s_y)==' ') )
			{
				if (getWorld()->getGridChar(s_x-2,s_y)=='@'){
					inLineOfSight=true; 
					break;
				}
				else
					s_x--;	
			}
		break;

		case up:
			bullet_y = getY()+1;
			if(getWorld()->getGridChar(s_x,s_y+1)=='@'){
					inLineOfSight=true;
					break;
				}
			while((passesOver(getWorld()->getGridChar(s_x,s_y+1))) && (getWorld()->getRobotGrid(s_x,s_y+1)==' ') )
			{
				if (getWorld()->getGridChar(s_x,s_y+2)=='@'){
					inLineOfSight=true; 
					break;
				}
				else
					s_y++;	
			}
		break;

	}

	if(inLineOfSight==true) //Fires a bullet only if the robot detected a player
	{
		getWorld()->fireBullet(bullet_x,bullet_y, getDirection());
		getWorld()->playSound(SOUND_ENEMY_FIRE);
		return true;
	}
	return false;
}

bool Robot::attemptRobotMove(int m_x, int m_y)
{
		int sx=getX(); int sy=getY();
		if( (!isObstacle(getWorld()->getGridChar(m_x,m_y)) ) && (getWorld()->getRobotGrid(m_x,m_y)==' ') )
		{
			getWorld()->changeRobotGrid(m_x,m_y, getWorld()->getRobotGrid(sx,sy)); 	
			getWorld()->changeRobotGrid(sx,sy,' ');									
			moveTo(m_x,m_y);
			return true;
		}
		else 
			return false;
}

SnarlBot::SnarlBot(StudentWorld* world, int startX, int startY, GraphObject::Direction direct)
		:Robot(world,startX,startY,IID_SNARLBOT)
{ 
		setDirection(direct);
		setMaxHitPoints(10);
		setPointsPerKill(100);
}

void SnarlBot::robotAction()
{
	if (robotFire()) //Moves only if it cant shoot
		return;

	int sx=getX(); int sy=getY();
	switch(getDirection())
	{
		case right:
			if (!attemptRobotMove(sx+1,sy))
				setDirection(left);
		break;

		case down:
			if (!attemptRobotMove(sx,sy-1))
				setDirection(up);
		break;

		case left:
			if (!attemptRobotMove(sx-1,sy))
				setDirection(right);
		break;

		case up:
			if (!attemptRobotMove(sx,sy+1))
				setDirection(down);
		break;
	}

return; 
}

KleptoBot::KleptoBot(StudentWorld* world, int startX, int startY, int imId )
		:Robot(world,startX,startY,imId), goodieStolen(' ')
{ 
		distanceToTurn= rand() % 6 +1;
		setDirection(right);
		setMaxHitPoints(5);
		setPointsPerKill(10);
}

void KleptoBot::setDead()
{
	Robot::setDead();
	if (goodieStolen==' ')//No goodie stolen.
		return;
	else if (isGoodie(goodieStolen)) //Stole some goody(jewels not included)
		getWorld()->dropGoodieBack(getX(),getY(),goodieStolen);
}

void KleptoBot::robotAction()
{
	int sx=getX(); int sy=getY();
	if ( (isGoodie(getWorld()->getGridChar(sx,sy)) ) && (goodieStolen==' ') && (rand()%10 == 2)) //Standing on a goodie, not holding a goodie, 10% probability
	{
		goodieStolen=getWorld()->stealGoodie(sx,sy); 
		getWorld()->playSound(SOUND_ROBOT_MUNCH);
		return;
	}
	if( distanceToTurn>0 )
	{
		switch(getDirection())
			{
			case right:
				if (attemptRobotMove(sx+1,sy)){
					distanceToTurn--;	return; 
				}
			break;

			case down:
				if (attemptRobotMove(sx,sy-1)){
					distanceToTurn--;	return; 
				}
			break;

			case left:
				if (attemptRobotMove(sx-1,sy)){
					distanceToTurn--;	return; 
				}
			break;

			case up:
				if (attemptRobotMove(sx,sy+1)){
					distanceToTurn--;	return; 
				}
			break;
			}
	}
	//Either distanceToTurn is 0, or we got blocked.

		distanceToTurn= rand() % 6 +1;
		enum GraphObject::Direction newDirection= static_cast<Direction>(rand() % 4 +1); //randomly select new direction
		setDirection(newDirection);
		switch (newDirection) //selecting new direction for kleptobot
		{
			case right:
				if (attemptRobotMove(sx+1,sy))
					return;
				break;
			case down:
				if (attemptRobotMove(sx,sy-1))
					return;
				break;
			case left:
				if (attemptRobotMove(sx-1,sy))
					return;
				break;
			case up:
				if (attemptRobotMove(sx,sy+1))
					return;
				break;
		}		

		//If we reached this point, the randomly selected direction was blocked
			if (newDirection!=right)	//try every remaining direction 
				if (attemptRobotMove(sx+1,sy)){
					setDirection(right);
					return;
				}
			if (newDirection!=left)
				if (attemptRobotMove(sx-1,sy)){
					setDirection(left);
					return;
				}
			if (newDirection!=up)
				if (attemptRobotMove(sx,sy+1)){
					setDirection(up);
					return;
				}
			if (newDirection!=down)
				if (attemptRobotMove(sx,sy-1)){
					setDirection(down);
					return;
				}
//If we reached this point, the kleptobot is blocked from all 4 directions and does nothing else.
return;
}

AngryKleptoBot::AngryKleptoBot(StudentWorld* world, int startX, int startY, int imId)
		:KleptoBot(world,startX,startY, imId)
{ 
		setMaxHitPoints(8);
		setPointsPerKill(20);
}

void AngryKleptoBot::robotAction()
{
	if(Robot::robotFire())
		return;
	KleptoBot::robotAction(); //Calls kleptobot function only if not firing at player
}

RobotFactory::RobotFactory(StudentWorld* world, int startX, int startY,bool Angry)
	:Actor(world, IID_ROBOT_FACTORY, startX ,startY, none),isAngry(Angry), kleptoCount(0)
{}

void RobotFactory::doSomething()
{
	kleptoCount=getWorld()->getKleptoCount(getX(),getY());

	//Less than 3 kleptobots at location, and robot grid is empty and randomly generated number is 3.
	if(kleptoCount<3 && getWorld()->getRobotGrid(getX(),getY())==' ' && (rand() %50 == 3) )	
	{                                                                                       
		getWorld()->spawnKleptoBot(getX(),getY(),isAngry);
	}
	return;
}