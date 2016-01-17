#include "StudentWorld.h"
#include <string>
#include "Level.h"
#include <iostream>
#include <sstream>
#include <iomanip>  // defines the manipulator setw
#include <algorithm>
using namespace std;


GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(std::string assetDir)
	: GameWorld(assetDir)
{}

int StudentWorld::init()
{
		jewelCount=0;
		setBonus(1000);
		return loadLevel();
}

void StudentWorld::cleanUp()
{
    delete m_player;

	vector<Actor*>::iterator it;
	for(it=m_actors.begin();it!=m_actors.end(); it++)
	{
			delete *it; 		
	}
	m_actors.erase(m_actors.begin(),m_actors.end());
}

StudentWorld::~StudentWorld()
{
	delete m_player;
	vector<Actor*>::iterator it;
	for(it=m_actors.begin();it!=m_actors.end(); it++)
	{
			delete *it; 
	}
	//cerr<<"~StudentWorld(): "<<m_actors.size();
}

int StudentWorld::move()
{
	updateDisplayText();

	m_player->doSomething();
	for(unsigned int i=0;i<m_actors.size(); i++)
	{
		if(m_actors[i]->isDead()== false) //If actor is alive, call doSomething()
			m_actors[i]->doSomething();		

		if(m_player->isDead())
			{
				//////////////*DEBUGGING*///////////
				for(int y=VIEW_HEIGHT-1;y>=0;y--) //rows
					{
						for(int x=0;x<VIEW_WIDTH;x++) //cols
							cerr<<grid[x][y][0];
						for(int x=0;x<VIEW_WIDTH;x++) //cols
							cerr<<grid[x][y][1];
						cerr<<endl;
					}
					cerr<<"----------------------"<<endl;
				//////////////*DEBUGGING*///////////
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}	 

		if(m_player->isFinished())
			{
				int scoreIncrease= 2000+ getBonus();
				increaseScore(scoreIncrease);
				return GWSTATUS_FINISHED_LEVEL; 
			}
	}

	removeDeadObjects(); //Remove Dead Actors
	decBonus(); //Every tick, bonus should reduce by 1 until 0.

	if(m_player->isDead())
	{
			decLives();
			return GWSTATUS_PLAYER_DIED;
	}

	if(m_player->isFinished())
		{
			int scoreIncrease= 2000+ getBonus();
			increaseScore(scoreIncrease);
			return GWSTATUS_FINISHED_LEVEL; 
		}
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::updateDisplayText()
{
	int score = getScore();
	int level = getLevel();
	unsigned int bonus = getBonus();
	int livesLeft = getLives();
	int health= m_player->getHealth();
	int ammo= m_player->getAmmo();
	string s = formatDisplayText(score, level, livesLeft, health, ammo, bonus);  
	setGameStatText(s); 
}

string StudentWorld::formatDisplayText(int score,int level, int lives, int health, int ammo, int bonus)
{
	string output="Score: ";
	ostringstream s; s.fill('0');
	s<<setw(7)<<score;
		output += s.str();
		output +="  Level: ";
	ostringstream lev; lev.fill('0');
	lev<<setw(2)<<level;
		output += lev.str();
		output += "  Lives: ";
	ostringstream liv; liv.fill(' ');
	liv<<setw(2)<<lives;
		output += liv.str();
		output+="  Health: ";
	ostringstream h; h.fill(' ');
		h<<setw(2)<<health;
		output += h.str(); output+="%  Ammo: ";
	ostringstream am; am.fill(' ');
	am<<setw(3)<<ammo; 
		output+=am.str(); output+="  Bonus: ";
	ostringstream bon; bon.fill(' ');
	bon<<setw(4)<<bonus;
		output += bon.str();
	return output;
}

void StudentWorld::removeDeadObjects()
{
	vector<Actor*>::iterator itr;
	for(itr=m_actors.begin(); itr!= m_actors.end(); )
		if((*itr)->isDead())
		{
			delete (*itr);
			itr=m_actors.erase(itr); 
		}
		else
			itr++;
}

int StudentWorld::loadLevel()
{
	if(getLevel()==100)
		return GWSTATUS_PLAYER_WON;

	Level lev(assetDirectory());
	ostringstream lev_format; lev_format.fill('0');
	lev_format<<setw(2)<<getLevel();
	string levelFormat= "level" + lev_format.str() + ".dat";
	Level::LoadResult result = lev.loadLevel(levelFormat);
	//Level::LoadResult result = lev.loadLevel("level04.dat"); 

	if (result == Level::load_fail_file_not_found){
		cerr << "Could not find "<<levelFormat<<" data file\n";
		return GWSTATUS_PLAYER_WON;
	}
	else if (result == Level:: load_fail_bad_format){
		cerr << "Your level was improperly formatted\n";
		return GWSTATUS_LEVEL_ERROR;
	}
	else if (result == Level:: load_success){
		cerr << "Successfully loaded level\n";

	for(int y=0;y<VIEW_HEIGHT;y++) //rows
		for(int x=0;x<VIEW_WIDTH;x++) //cols
		{
			grid[x][y][0]=' ';
			grid[x][y][1]=' ';
		}

	for(int y=0;y<VIEW_HEIGHT;y++) //rows
		for(int x=0;x<VIEW_WIDTH;x++) //cols
		{
			Level::MazeEntry ge = lev.getContentsOf(x,y); 
			switch (ge)
			{
				case Level::empty:
				break;
				case Level::exit:
					m_actors.push_back(new Exit(this,x,y));
					grid[x][y][0]='x';
				break;
				case Level::player:
					m_player= new Player(this,x,y);
					grid[x][y][0]='@';
				break;
				case Level::horiz_snarlbot:
					m_actors.push_back(new SnarlBot(this,x,y,GraphObject::Direction::right));
					grid[x][y][1]='h';
				break;
				case Level::vert_snarlbot:
					m_actors.push_back(new SnarlBot(this,x,y,GraphObject::Direction::down));
					grid[x][y][1]='v';
				break;
				case Level::kleptobot_factory: 
					m_actors.push_back(new RobotFactory(this,x,y,false));
					grid[x][y][0]='1';
				break;
				case Level::angry_kleptobot_factory: //Difference between angry and normal is boolean
					m_actors.push_back(new RobotFactory(this,x,y,true));
					grid[x][y][0]='2';
				break;
				case Level::wall:
					m_actors.push_back(new Wall(x,y));
					grid[x][y][0]='#';
					//grid[x][y][1]='#';
				break;
				case Level::jewel:
					m_actors.push_back(new Jewel(this,x,y));
					grid[x][y][0]='*';
					incJewelCount();
				break;
				case Level::hole:
					m_actors.push_back(new Hole(this,x,y));
					grid[x][y][0]='o';
				break;
				case Level::boulder:
					m_actors.push_back(new Boulder(this,x,y));
					grid[x][y][0]='b';
				break;
				case Level::ammo:
					m_actors.push_back(new AmmoGoodie(this,x,y));
					grid[x][y][0]='a';
				break;
				case Level::restore_health:
					m_actors.push_back(new RestoreHealthGoodie(this,x,y));
					grid[x][y][0]='r';
				break;
				case Level::extra_life:
					m_actors.push_back(new ExtraLifeGoodie(this,x,y));
					grid[x][y][0]='e';
				break;
			}
		}
	}
	return GWSTATUS_CONTINUE_GAME;
}

bool StudentWorld::attemptMove(Actor* ptr , int ex, int ey)
{
	int sx=ptr->getX();
	int sy=ptr->getY();

	if(grid[sx][sy][0]=='@' && grid[ex][ey][0]=='b') //NO EXTRA CHECKING NEEDED HERE
	{    
		for(unsigned int i=0;i<m_actors.size();i++)
			if( m_actors[i]->getX()==ex && m_actors[i]->getY()==ey)
			{
				Actor* ap =m_actors[i];
				if (ap != nullptr)
				{
					Boulder* wp = dynamic_cast<Boulder*>(ap);
					if (wp != nullptr)
					{
						return (wp->push(ptr->getDirection() ));
					}
				}
			}

		return false;//push was never called
	}
	else if(( !isObstacle(grid[ex][ey][0]) ) && (grid[ex][ey][1]==' ')  ) // NO ROBOTS
	{
		grid[ex][ey][0]=grid[sx][sy][0];
		grid[sx][sy][0]=' ';
		ptr->moveTo(ex,ey);
		return true;
	}
	else
		return false;
}

int StudentWorld::getBonus()
{
	return m_bonus;
}

void StudentWorld::setBonus(int bonusVal)
{
	m_bonus=bonusVal;
}

void StudentWorld::decBonus()
{
	if(getBonus()>0)
		m_bonus--;
}

void StudentWorld::incJewelCount()
{
	jewelCount++;
}

void StudentWorld::decJewelCount()
{
	if(jewelCount>0)
		jewelCount--;
}

int StudentWorld::getJewelCount()
{
	return jewelCount;
}

Player* StudentWorld::getPlayer()
{
	return m_player;
}

void StudentWorld::changeGrid(int m_x, int m_y, char toReplace) //NOT THE ROBOTS ARRAY
{
	grid[m_x][m_y][0]=toReplace;
		return;
}

char StudentWorld::getGridChar(int m_x, int m_y)
{
	if(m_x<VIEW_WIDTH && m_y<VIEW_HEIGHT)
		return grid[m_x][m_y][0];
	else 
		return '0'; //Error
}

void StudentWorld::changeRobotGrid(int m_x, int m_y, char toReplace) //Added just to maintain location of Robots
{
	grid[m_x][m_y][1]=toReplace;
		return;
}

char StudentWorld::getRobotGrid(int m_x, int m_y)
{
	if(m_x<VIEW_WIDTH && m_y<VIEW_HEIGHT)
		return grid[m_x][m_y][1];
	else 
		return '0'; //Error
}

void StudentWorld::fireBullet(int m_x, int m_y,enum GraphObject::Direction direct)
{
	m_actors.push_back(new Bullet(this,m_x,m_y,direct));
}

void StudentWorld::damageLocation(int m_x, int m_y) 
{
	if(m_player->getX()==m_x && m_player->getY()==m_y)
		{
			m_player->takeHit();
			return;
		}

		for(unsigned int i=0;i<m_actors.size();i++)
			if( m_actors[i]->getX()==m_x && m_actors[i]->getY()==m_y)
				m_actors[i]->takeHit();
}

void StudentWorld::boulderToHole(int cur_x, int cur_y)
{
	for(unsigned int i=0;i< m_actors.size();i++)
		if( m_actors[i]->getX()==cur_x && m_actors[i]->getY()==cur_y)
		{
			Actor* ap = m_actors[i];
			Hole* killMe = dynamic_cast<Hole*>(ap);
			if (killMe != nullptr)
			{ 
				killMe->setDead();
				grid[cur_x][cur_y][0]=' ';
				return;
			}
		}
	return;
}

char StudentWorld::stealGoodie(int m_x,int m_y)
{
	char tempChar= grid[m_x][m_y][0];
	if (tempChar=='*') //ensuring that jewels arent accidentally counted.
		return ' ';

	for(unsigned int i=0;i< m_actors.size();i++)
		if( m_actors[i]->getX()==m_x && m_actors[i]->getY()==m_y)
		{
			Actor* ap = m_actors[i];
			Goodie* killMe = dynamic_cast<Goodie*>(ap);
			if (killMe != nullptr)
			{ 
				killMe->setDead();
				grid[m_x][m_y][0]=' ';
				return tempChar;
			}
		}
	return ' '; //Means that the function did not destroy any goodie.
}

bool StudentWorld::dropGoodieBack(int d_x,int d_y,char goodieType)
{
	if (grid[d_x][d_y][0]!=' ')
		return false;
	if(goodieType=='r')
		m_actors.push_back(new RestoreHealthGoodie(this,d_x,d_y));
	else if(goodieType=='a')
		m_actors.push_back(new AmmoGoodie(this,d_x,d_y));
	else if(goodieType=='e')
		m_actors.push_back(new ExtraLifeGoodie(this,d_x,d_y));
	else 
		return false;

	(grid[d_x][d_y][0]=goodieType);
	return true;
}

int StudentWorld::getKleptoCount(int s_x, int s_y)
{
	int kleptoBotCount=0;
	int x_west= max(0,s_x-3); 	int y_north= min(14,s_y+3);
	int x_east= min(14,s_x+3); 	int y_south= max(0,s_y-3);

	for(int i=y_south;i<=y_north;i++)
		for(int j=x_west;j<=x_east;j++)
			if(grid[j][i][1]=='$' || grid[j][i][1]=='%')
				kleptoBotCount++;

	return kleptoBotCount;
}

void StudentWorld::spawnKleptoBot(int d_x, int d_y , bool AngryKlepto)
{
	if (AngryKlepto)
	{
		m_actors.push_back(new AngryKleptoBot(this,d_x,d_y,IID_ANGRY_KLEPTOBOT));
		grid[d_x][d_y][1]='%';
	}
	else //Normal kleptobot
	{
		m_actors.push_back(new KleptoBot(this,d_x,d_y,IID_KLEPTOBOT));
		grid[d_x][d_y][1]='$';
	}
	playSound(SOUND_ROBOT_BORN);
}

/* DEBUGGING
		for(int y=VIEW_HEIGHT-1;y>=0;y--) //rows
			{
				for(int x=0;x<VIEW_WIDTH;x++) //cols
					cerr<<grid[x][y][0];
				for(int x=0;x<VIEW_WIDTH;x++) //cols
					cerr<<grid[x][y][1];
				cerr<<endl;
			}
			cerr<<"----------------------"<<endl;
*/