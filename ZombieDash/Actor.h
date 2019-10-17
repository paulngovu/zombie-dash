#ifndef ACTOR_INCLUDED
#define ACTOR_INCLUDED

#include "GraphObject.h"

class StudentWorld;
class Goodie;

class Actor : public GraphObject {
public:
    Actor(StudentWorld* w, int imageID, double x, double y, int dir, int depth);

	// Action to perform for each tick.
    virtual void doSomething() = 0;

	// Is this actor dead?
	bool isDead() const;
	
	// Mark this actor as dead.
	void setDead();
	
	// Get this actor's world
	StudentWorld* world() const;
	
	// If this is an activated object, perform its effect on a (e.g., for an
	// Exit have a use the exit).
	virtual void activateIfAppropriate(Actor* a);
	
	// If this object uses exits, use the exit.
	virtual void useExitIfAppropriate();
	
	// If this object can die by falling into a pit or burning, die.
	virtual void dieByFallOrBurnIfAppropriate();
	
	// If this object can be infected by vomit, get infected.
	virtual void beVomitedOnIfAppropriate();
	
	// Does this object block agent movement?
	virtual bool blocksMovement() const;
	
	// Does this object block flames?
	virtual bool blocksFlame() const;
	
	// Does this object trigger landmines only when they're active?
	virtual bool triggersOnlyActiveLandmines() const;
	
	// Can this object cause a zombie to vomit?
	virtual bool triggersZombieVomit() const;
	
	// Is this object a threat to citizens?
	virtual bool threatensCitizens() const;
	
	// Does this object trigger citizens to follow it or flee it?
	virtual bool triggersCitizens() const;
	
private:
	StudentWorld* m_world;
	bool m_dead;
};

class Wall : public Actor {
public:
	Wall(StudentWorld* w, double x, double y);
	
	virtual void doSomething();
	virtual bool blocksMovement() const;
	virtual bool blocksFlame() const;
};

class ActivatingObject : public Actor {
public:
	ActivatingObject(StudentWorld* w, int imageID, double x, double y, int dir, int depth);
};

class Exit : public ActivatingObject {
public:
    Exit(StudentWorld* w, double x, double y);

	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
	virtual bool blocksFlame() const;
};

class Pit : public ActivatingObject {
public:
	Pit(StudentWorld* w, double x, double y);
	
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
};

class Flame : public ActivatingObject {
public:
	Flame(StudentWorld* w, double x, double y, int dir);

    virtual void doSomething();
    virtual void activateIfAppropriate(Actor* a);

private:
	int m_tick;
};

class Vomit : public ActivatingObject {
public:
	Vomit(StudentWorld* w, double x, double y, int dir);

	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);

private:
	int m_tick;
};

class Landmine : public ActivatingObject {
public:
	Landmine(StudentWorld* w, double x, double y);

	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
	virtual void dieByFallOrBurnIfAppropriate();

private:
	int m_tick;
	bool m_active;
};

class Goodie : public ActivatingObject {
public:
	Goodie(StudentWorld* w, int imageID, double x, double y);
	
	virtual void activateIfAppropriate(Actor* a);
	virtual void dieByFallOrBurnIfAppropriate();
	
	// Have Penelope pick up this goodie.
	virtual void increaseGoodieCount() = 0;
};

class VaccineGoodie : public Goodie {
public:
	VaccineGoodie(StudentWorld* w, double x, double y);
	
	virtual void doSomething();
	virtual void increaseGoodieCount();
};

class GasCanGoodie : public Goodie {
public:
    GasCanGoodie(StudentWorld* w, double x, double y);
    virtual void doSomething();
	virtual void increaseGoodieCount();
};

class LandmineGoodie : public Goodie {
public:
    LandmineGoodie(StudentWorld* w, double x, double y);

    virtual void doSomething();
	virtual void increaseGoodieCount();
};

class Agent : public Actor {
public:
	Agent(StudentWorld* w, int imageID, double x, double y, int dir);
	
	virtual bool blocksMovement() const;
	virtual bool triggersOnlyActiveLandmines() const;
};

class Human : public Agent {
public:
	Human(StudentWorld* w, int imageID, double x, double y);
	
	virtual void beVomitedOnIfAppropriate();	// turn human infected
	virtual bool triggersZombieVomit() const;	// all humans can trigger vomit
	
	void clearInfection();				// Make this human uninfected by vomit
	void incInfectionCount();			// increase infection count by 1
	
	int getInfectionDuration() const;	// gets infection count
	bool isInfected() const;			// Is human infected?
	
private:
	int m_infectionCount;
	bool m_infected;
};

class Penelope : public Human {
public:
	Penelope(StudentWorld* w, double x, double y);
	
	virtual void doSomething();
	virtual void useExitIfAppropriate();
	virtual void dieByFallOrBurnIfAppropriate();
	
	void increaseVaccines();		// Increase the number of vaccines the object has.
	void decreaseVaccines();		// Decrease the number of vaccines after usage.
	
	void increaseFlameCharges();	// Increase the number of flame charges the object has.
	void decreaseFlameCharges();	// Decrease the number of flame charges after usage.
	
	void increaseLandmines();		// Increase the number of landmines the object has.
	void decreaseLandmines();		// Decrease the number of landmines after usage.
	
	// How many vaccines does the object have?
	int getNumVaccines() const;
	
	// How many flame charges does the object have?
	int getNumFlameCharges() const;
	
	// How many landmines does the object have?
	int getNumLandmines() const;
	
private:
	int m_nVaccines;
	int m_nFlameCharges;
	int m_nLandmines;
};

class Citizen : public Human {
public:
    Citizen(StudentWorld* w,  double x, double y);

    virtual void doSomething();
    virtual void useExitIfAppropriate();
    virtual void dieByFallOrBurnIfAppropriate();
};

class Zombie : public Agent {
public:
    Zombie(StudentWorld* w, int imageID, double x, double y);

	virtual bool threatensCitizens() const;		// zombies threaten citizens

	int movementPlanDistance() const;			// returns movement plan distance
	void resetMovementPlanDistance(int amt);	// changes to amount passed
	void decMovementPlanDistance();				// decrements by 1

	int tick() const;							// returns tick
	void incTick();								// increase tick by 1
	void resetTick();							// resets tick

	bool isParalyzed() const;					// checks if zombie is paralyzed

	virtual void doSomething();				// pure virtual to create dumb/smart zombies

private:
	int m_movementPlanDistance;
	int m_tick;
};

class DumbZombie : public Zombie {
public:
	DumbZombie(StudentWorld* w,  double x, double y);
	
	virtual void doSomeThing();
	virtual void dieByFallOrBurnIfAppropriate();
};

class SmartZombie : public Zombie {
public:
    SmartZombie(StudentWorld* w,  double x, double y);

    virtual void doSomeThing();
    virtual void dieByFallOrBurnIfAppropriate();
};

#endif // ACTOR_INCLUDED