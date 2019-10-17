#ifndef STUDENTWORLD_INCLUDED
#define STUDENTWORLD_INCLUDED

#include "GameWorld.h"
#include <string>
#include <vector>

class Actor;
class Penelope;

class StudentWorld : public GameWorld {
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();
    
    virtual int init();
    virtual int move();
    virtual void cleanUp();

	Penelope* player();
	int nCitizens() const;
	void decNCitizens();		// decrement citizens by 1
	bool levelFinishedIfAllCitizensGone() const;

	// Add an actor to the world.
	void addActor(Actor* a);
	
	// Record that one more citizen on the current level is gone (exited,
	// died, or turned into a zombie).
	void recordCitizenGone();

	// Indicate that the player has finished the level if all citizens
	// are gone.
	void recordLevelFinishedIfAllCitizensGone();

	// For each actor overlapping a, activate a if appropriate.
	void activateOnAppropriateActors(Actor* a);

	// Is an agent blocked from moving to the indicated location?
	bool isAgentMovementBlockedAt(double x, double y);

	// Is creation of a flame blocked at the indicated location?
	bool isFlameBlockedAt(double x, double y);

	// Is there something at the indicated location that might cause a
	// zombie to vomit (i.e., a human)?
	bool isZombieVomitTriggerAt(double x, double y);

//      // Return true if there is a living human, otherwise false.  If true,
//      // otherX, otherY, and distance will be set to the location and distance
//      // of the human nearest to (x,y).
//    bool locateNearestVomitTrigger(double x, double y, double& otherX, double& otherY, double& distance);
//
//      // Return true if there is a living zombie or Penelope, otherwise false.
//      // If true, otherX, otherY, and distance will be set to the location and
//      // distance of the one nearest to (x,y), and isThreat will be set to true
//      // if it's a zombie, false if a Penelope.
//    bool locateNearestCitizenTrigger(double x, double y, double& otherX, double& otherY, double& distance, bool& isThreat) const;
//
//      // Return true if there is a living zombie, false otherwise.  If true,
//      // otherX, otherY and distance will be set to the location and distance
//      // of the one nearest to (x,y).
//    bool locateNearestCitizenThreat(double x, double y, double& otherX, double& otherY, double& distance) const;

private:
	// functions to help display stats
	void setDisplayText();
	std::string formatDisplayText(int score, int level, int lives, int vaccines, int flames, int mines, int infected) const;
	std::string formatDigit(int input, int totalDigits, bool zeros) const;

	void initializeAllValues();		// initializes data members

	Penelope* m_penelope;
	std::vector<Actor*> m_actors;
	int m_nCitizens;
	bool m_levelFinishedIfAllCitizensGone;
};

#endif // STUDENTWORLD_INCLUDED