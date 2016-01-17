#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include "Actor.h"
using namespace std;

bool isObstacle (char code);

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);
	virtual ~StudentWorld(); //Similar to cleanup

	virtual int init();
	virtual int move();
	virtual void cleanUp();

	void updateDisplayText();
	void removeDeadObjects();
	int loadLevel();
	bool attemptMove(Actor* ptr,int er, int ec);
	int getBonus();
	void setBonus(int bonusVal); 
	void decBonus();
	void incJewelCount();
	void decJewelCount();
	int getJewelCount();
	string formatDisplayText(int score,int level, int lives, int health, int ammo, int bonus);
	Player* getPlayer();

	//Functions to view/modify grid positions.
	void changeGrid(int m_x, int m_y, char toReplace);
	char getGridChar(int m_x, int m_y);
	void changeRobotGrid(int m_x, int m_y, char toReplace);
	char getRobotGrid(int m_x, int m_y);

	void fireBullet(int m_x, int m_y,enum GraphObject::Direction);
	void damageLocation(int m_x, int m_y); 
	void boulderToHole(int cur_x, int cur_y);
	char stealGoodie(int m_x,int m_y);
	bool dropGoodieBack(int d_x,int d_y,char goodieType);

	//factory functions
	int getKleptoCount(int s_x, int s_y);
	void spawnKleptoBot(int d_x, int d_y , bool AngryKlepto);

private:
	Player* m_player;
	vector<Actor*> m_actors; 
	char grid [VIEW_WIDTH][VIEW_HEIGHT][2]; //Third dimension(2nd 2D grid) added for robots
	int m_bonus;
	unsigned int jewelCount;
};

#endif // STUDENTWORLD_H_
