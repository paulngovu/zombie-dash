#include "Actor.h"
#include "StudentWorld.h"

Actor::Actor(StudentWorld * w, int imageID, double x, double y, int dir, int depth)
	: GraphObject(imageID, x, y, dir, depth) {
	m_world = w;
	m_dead = false;
}

bool Actor::isDead() const {
	return m_dead;
}

void Actor::setDead() {
	m_dead = true;
}

StudentWorld * Actor::world() const {
	return m_world;
}

void Actor::activateIfAppropriate(Actor * a) {}

void Actor::useExitIfAppropriate() {}

void Actor::dieByFallOrBurnIfAppropriate() {}

void Actor::beVomitedOnIfAppropriate() {}

bool Actor::blocksMovement() const {
	return false;
}

bool Actor::blocksFlame() const {
	return false;
}

bool Actor::triggersOnlyActiveLandmines() const {
	return false;
}

bool Actor::triggersZombieVomit() const {
	return false;
}

bool Actor::threatensCitizens() const {
	return false;
}

bool Actor::triggersCitizens() const {
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

Wall::Wall(StudentWorld * w, double x, double y)
	: Actor(w, IID_WALL, x, y, right, 0) {}

void Wall::doSomething() {}

bool Wall::blocksMovement() const {
	return true;
}

bool Wall::blocksFlame() const {
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

ActivatingObject::ActivatingObject(StudentWorld * w, int imageID, double x, double y, int dir, int depth)
	: Actor(w, imageID, x, y, dir, depth) {}

/////////////////////////////////////////////////////////////////////////////////////////

Exit::Exit(StudentWorld* w, double x, double y)
	: ActivatingObject(w, IID_EXIT, x, y, right, 1) {}

void Exit::doSomething() {
	// The exit must determine if it overlaps with a citizen (not Penelope!)

	// if all citizens gone and overlaps with Penelope
	if (world()->nCitizens() <= 0) {
		world()->activateOnAppropriateActors(this);
	}
}

void Exit::activateIfAppropriate(Actor * a) {
	a->useExitIfAppropriate();
}

bool Exit::blocksFlame() const {
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

Pit::Pit(StudentWorld* w, double x, double y)
	: ActivatingObject(w, IID_PIT, x, y, right, 0) {}

void Pit::doSomething() {
	world()->activateOnAppropriateActors(this);
}

void Pit::activateIfAppropriate(Actor * a) {
	a->dieByFallOrBurnIfAppropriate();
}

/////////////////////////////////////////////////////////////////////////////////////////

Flame::Flame(StudentWorld* w, double x, double y, int dir)
	:ActivatingObject(w, IID_FLAME, x, y, dir, 0) {
	m_tick = 0;
}

void Flame::doSomething() {
	if (isDead()) {
		return;
	}

	if (m_tick == 2) {		// flames only last for 2 ticks
		setDead();
		return;
	}

	m_tick++;				// increment tick every time

	// flame will damage any overlapping objects
	world()->activateOnAppropriateActors(this);
}

void Flame::activateIfAppropriate(Actor* a) {
	a->dieByFallOrBurnIfAppropriate();
}

/////////////////////////////////////////////////////////////////////////////////////////

Vomit::Vomit(StudentWorld* w, double x, double y, int dir)
	: ActivatingObject(w, IID_ZOMBIE, x, y, dir, 0) {
	m_tick = 0;
}

void Vomit::doSomething() {
	if (isDead()) {
		return;
	}

	// after 2 ticks, vomit must be set to dead
	if (m_tick == 2) {
		setDead();
		return;
	}

	m_tick++;
	
	// check for overlapping objects
	world()->activateOnAppropriateActors(this);
}

void Vomit::activateIfAppropriate(Actor * a) {
	a->beVomitedOnIfAppropriate();
}

/////////////////////////////////////////////////////////////////////////////////////////

Landmine::Landmine(StudentWorld* w, double x, double y)
	: ActivatingObject(w, IID_LANDMINE, x, y, right, 1) {
	m_tick = 30;
	m_active = false;
}

void Landmine::doSomething() {
	if (isDead()) {
		return;
	}

	m_tick--;
	if (m_tick == 0) {		// a landmine has 30 ticks before becoming active
		m_active = true;
	}

	if (m_active) {		// only damage when active
		// landmine will damage any overlapping objects
		world()->activateOnAppropriateActors(this);
	}
}

void Landmine::activateIfAppropriate(Actor* a) {
	dieByFallOrBurnIfAppropriate();

	// create flames around landmine
	// north
	if (!world()->isFlameBlockedAt(getX(), getY() + SPRITE_HEIGHT) &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX(), getY() + SPRITE_HEIGHT + SPRITE_HEIGHT - 1) &&	// top left corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT) &&	// bottom right corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT + SPRITE_HEIGHT - 1)) {	// top left corner overlap
		world()->addActor(new Flame(world(), getX(), getY() + SPRITE_HEIGHT, up));
	}

	// east
	if (!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH, getY()) &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH + SPRITE_WIDTH - 1, getY()) &&		// bottom right corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT - 1) &&	// top left corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT - 1)) {	// top left corner overlap
		world()->addActor(new Flame(world(), getX() + SPRITE_WIDTH, getY(), up));
	}

	// south
	if (!world()->isFlameBlockedAt(getX(), getY() - SPRITE_HEIGHT) &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH - 1, getY() - SPRITE_HEIGHT)) {	// bottom right corner overlap
		world()->addActor(new Flame(world(), getX(), getY() - SPRITE_HEIGHT, up));
	}

	// west
	if (!world()->isFlameBlockedAt(getX() - SPRITE_WIDTH, getY()) &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT - 1)) {	// bottom right corner overlap
		world()->addActor(new Flame(world(), getX() - SPRITE_WIDTH, getY(), up));
	}

	// northwest
	if (!world()->isFlameBlockedAt(getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT) &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX() - 1, getY() + SPRITE_HEIGHT) &&	// bottom right corner overlap
		!world()->isFlameBlockedAt(getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT + SPRITE_HEIGHT - 1 &&	// top left corner overlap
		!world()->isFlameBlockedAt(getX() - 1, getY() + SPRITE_HEIGHT + SPRITE_HEIGHT - 1))) {	// top right corner overlap
		world()->addActor(new Flame(world(), getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT, up));
	}
	
	// northeast
	if (!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT) &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT) &&		// bottom right corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT + SPRITE_HEIGHT - 1) &&	// top left corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT + SPRITE_HEIGHT - 1)) {	// top right corner overlap
		world()->addActor(new Flame(world(), getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT, up));
	}

	// southeast
	if (!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH, getY() - SPRITE_HEIGHT)  &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX() + SPRITE_WIDTH + SPRITE_WIDTH - 1, getY() - SPRITE_HEIGHT)) {	// bottom right corner overlap
		world()->addActor(new Flame(world(), getX() + SPRITE_WIDTH, getY() - SPRITE_HEIGHT, up));
	}

	// southwest
	if (!world()->isFlameBlockedAt(getX() - SPRITE_WIDTH, getY() - SPRITE_HEIGHT) &&	// bottom left corner overlap
		!world()->isFlameBlockedAt(getX() - 1, getY() - SPRITE_HEIGHT)) {	// bottom right corner overlap
		world()->addActor(new Flame(world(), getX() - SPRITE_WIDTH, getY() - SPRITE_HEIGHT, up));
	}
}

void Landmine::dieByFallOrBurnIfAppropriate() {
	if (!isDead()) {
		setDead();
		world()->playSound(SOUND_LANDMINE_EXPLODE);
	}
}

Agent::Agent(StudentWorld * w, int imageID, double x, double y, int dir)
	: Actor(w, imageID, x, y, dir, 0) {}

bool Agent::blocksMovement() const {
	return true;
}

bool Agent::triggersOnlyActiveLandmines() const {
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

Human::Human(StudentWorld * w, int imageID, double x, double y)
	: Agent(w, imageID, x, y, right) {
	m_infected = false;
	m_infectionCount = 0;
}

void Human::beVomitedOnIfAppropriate() {
	m_infected = true;
}

bool Human::triggersZombieVomit() const {
	return true;
}

void Human::clearInfection() {
	m_infected = false;
	m_infectionCount = 0;
}

void Human::incInfectionCount() {
	m_infectionCount++;
}

int Human::getInfectionDuration() const {
	return m_infectionCount;
}

bool Human::isInfected() const {
	return m_infected;
}

/////////////////////////////////////////////////////////////////////////////////////////

Penelope::Penelope(StudentWorld * w, double x, double y)
	: Human(w, IID_PLAYER, x, y) {
	m_nVaccines = 0;
	m_nFlameCharges = 0;
	m_nLandmines = 0;
}

void Penelope::doSomething() {
	if (isDead()) {							// if she's dead, don't do anything
		return;
	}

	if (isInfected()) {						// if she's infected, increment by 1
		incInfectionCount();
	}

	if (getInfectionDuration() >= 500) {	// if infection gets to 500
		setDead();
		world()->playSound(SOUND_PLAYER_DIE);
		return;
	}

	int key;	// key input
	if (world()->getKey(key)) {	// user hit a key during this tick
		switch (key) {
		case KEY_PRESS_SPACE:	// flamethrower
			// only do if Penelope has flame charges
			if (getNumFlameCharges() > 0) {
				decreaseFlameCharges();
				world()->playSound(SOUND_PLAYER_FIRE);

				// create 3 flames in a row, if possible
				for (int i = 1; i <= 3; i++) {
					switch (getDirection()) {	// creates flame in direction of Penelope
					case left:
						// check if there is blockable object in the way
						if (world()->isFlameBlockedAt(getX() - (i * SPRITE_WIDTH), getY()) ||
							world()->isFlameBlockedAt(getX() - (i * SPRITE_WIDTH), getY() + SPRITE_HEIGHT - 1)) {
							goto flameBreak;		// exit the loop if blocked
						}

						// otherwise create flame
						world()->addActor(new Flame(world(), getX() - (i * SPRITE_WIDTH), getY(), getDirection()));
						break;

					case right:
						// check if there is blockable object in the way

						// We need all these conditions because object coordinates are oriented
						// as the bottom-left corner, so we need to check an extra block.
						if (world()->isFlameBlockedAt(getX() + (i * SPRITE_WIDTH), getY()) ||
							world()->isFlameBlockedAt(getX() + (i * SPRITE_WIDTH) + SPRITE_WIDTH - 1, getY()) ||
							world()->isFlameBlockedAt(getX() + (i * SPRITE_WIDTH), getY() + SPRITE_HEIGHT - 1) ||
							world()->isFlameBlockedAt(getX() + (i * SPRITE_WIDTH) + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT - 1)) {
							goto flameBreak;		// exit the loop if blocked
						}

						// otherwise create flame
						world()->addActor(new Flame(world(), getX() + (i * SPRITE_WIDTH), getY(), getDirection()));
						break;

					case up:
						// check if there is blockable object in the way

						// We need all these conditions because object coordinates are oriented
						// as the bottom-left corner, so we need to check an extra block.
						if (world()->isFlameBlockedAt(getX(), getY() + (i * SPRITE_HEIGHT)) ||
							world()->isFlameBlockedAt(getX(), getY() + (i * SPRITE_HEIGHT) + SPRITE_HEIGHT - 1) ||
							world()->isFlameBlockedAt(getX() + SPRITE_WIDTH - 1, getY() + (i * SPRITE_HEIGHT)) ||
							world()->isFlameBlockedAt(getX() + SPRITE_WIDTH - 1, getY() + (i * SPRITE_HEIGHT) + SPRITE_HEIGHT - 1)) {
							goto flameBreak;		// exit the loop if blocked
						}

						// otherwise create flame
						world()->addActor(new Flame(world(), getX(), getY() + (i * SPRITE_HEIGHT), getDirection()));
						break;

					case down:
						// check if there is blockable object in the way
						if (world()->isFlameBlockedAt(getX(), getY() - (i * SPRITE_WIDTH)) ||
							world()->isFlameBlockedAt(getX() + SPRITE_WIDTH - 1, getY() - (i * SPRITE_HEIGHT))) {
							goto flameBreak;		// exit the loop if blocked
						}

						// otherwise create flame
						world()->addActor(new Flame(world(), getX(), getY() - (i * SPRITE_HEIGHT), getDirection()));
						break;
					}
				}
			}

		flameBreak:	// go here if flame is stopped
			break;

		case KEY_PRESS_TAB:		// deploy landmine
			// only do if Penelope has landmines
			if (getNumLandmines() > 0) {
				decreaseLandmines();
				world()->addActor(new Landmine(world(), getX(), getY()));
			}
			break;

		case KEY_PRESS_ENTER:	// vaccine
			// only do if Penelope has vaccines
			if (getNumVaccines() > 0) {
				clearInfection();
				decreaseVaccines();
			}
			break;
			
		case KEY_PRESS_LEFT:	// move left
			setDirection(left);
			// check for blocking object
			if (!world()->isAgentMovementBlockedAt(getX() - 4, getY()) &&
				!world()->isAgentMovementBlockedAt(getX() - 4, getY() + SPRITE_HEIGHT - 1)) {
				moveTo(getX() - 4, getY());
			}
			break;

		case KEY_PRESS_RIGHT:	// move right
			setDirection(right);
			// check for blocking object
			if (!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH, getY()) &&
				!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT - 1)) {
				moveTo(getX() + 4, getY());
			}
			break;

		case KEY_PRESS_DOWN:	// move down
			setDirection(down);
			// check for blocking object
			if (!world()->isAgentMovementBlockedAt(getX(), getY() - 4) &&
				!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH - 1, getY() - 4)) {
				moveTo(getX(), getY() - 4);
			}
			break;

		case KEY_PRESS_UP:		// move up
			setDirection(up);
			// check for blocking object
			if (!world()->isAgentMovementBlockedAt(getX(), getY() + SPRITE_HEIGHT) &&
				!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT)) {
				moveTo(getX(), getY() + 4);
			}
			break;
		}
	}
}

void Penelope::useExitIfAppropriate() {
	world()->recordLevelFinishedIfAllCitizensGone();
}

void Penelope::dieByFallOrBurnIfAppropriate() {
	world()->playSound(SOUND_PLAYER_DIE);
	setDead();
}

void Penelope::increaseVaccines() {
	// increase vaccine by 1
	m_nVaccines++;
}

void Penelope::decreaseVaccines() {
	m_nVaccines--;
}

void Penelope::increaseFlameCharges() {
	// increase flame charge by 5
	m_nFlameCharges += 5;
}

void Penelope::decreaseFlameCharges() {
	m_nFlameCharges--;
}

void Penelope::increaseLandmines() {
	// increase landmine by 2
	m_nLandmines += 2;
}

void Penelope::decreaseLandmines() {
	m_nLandmines--;
}

int Penelope::getNumVaccines() const {
	return m_nVaccines;
}

int Penelope::getNumFlameCharges() const {
	return m_nFlameCharges;
}

int Penelope::getNumLandmines() const {
	return m_nLandmines;
}

/////////////////////////////////////////////////////////////////////////////////////////

Citizen::Citizen(StudentWorld* w, double x, double y)
	: Human(w, IID_CITIZEN, x, y) {}

void Citizen::doSomething() {
	if (isDead()) {
		return;
	}

	if (isInfected()) {							// if citizen is infected, increment by 1
		incInfectionCount();
	}

	if (getInfectionDuration() >= 500) {		// if infection gets to 500
		setDead();
		world()->playSound(SOUND_ZOMBIE_BORN);	// a zombie is born
		world()->increaseScore(-1000);			// lose 1000 points
		world()->decNCitizens();				// decrease number of citizens in world

		// create new zombie at location: 70% dumb zombie, 30% smart zombie
		int zType = randInt(1, 10);
		switch (zType) {
		case 1:	case 2:	case 3:		// smart zombie
			world()->addActor(new SmartZombie(world(), getX(), getY()));
			break;

		default:					// dumb zombie
			world()->addActor(new DumbZombie(world(), getX(), getY()));
		}

		return;
	}
}

void Citizen::useExitIfAppropriate() {
	setDead();
}

void Citizen::dieByFallOrBurnIfAppropriate() {
	setDead();
	world()->playSound(SOUND_CITIZEN_DIE);
	world()->increaseScore(-1000);
	world()->decNCitizens();
}

/////////////////////////////////////////////////////////////////////////////////////////

Zombie::Zombie(StudentWorld* w, int imageID, double x, double y)
	: Agent(w, imageID, x, y, right) {
	m_movementPlanDistance = 10;
	m_tick = 1;
}

bool Zombie::threatensCitizens() const {
	return true;
}

int Zombie::movementPlanDistance() const {
	return m_movementPlanDistance;
}

void Zombie::resetMovementPlanDistance(int amt) {
	m_movementPlanDistance = amt;
}

void Zombie::decMovementPlanDistance() {
	m_movementPlanDistance--;
}

int Zombie::tick() const {
	return m_tick;
}

void Zombie::incTick() {
	m_tick++;
}

void Zombie::resetTick() {
	m_tick = 1;
}

bool Zombie::isParalyzed() const {
	return m_tick % 2 == 0;
}

void Zombie::doSomething() {
	if (isDead()) {
		return;
	}

	if (isParalyzed()) {	// zombies are paralyzed on every even tick (2, 4, 6, etc.)
		resetTick();
		return;
	}

	incTick();				// increment tick so zombie can be paralyzed

	// zombie checks if there are citizens/Penelope in front of it
	switch (getDirection()) {
	case up:
		if (world()->isZombieVomitTriggerAt(getX(), getY() + SPRITE_HEIGHT)) {
			world()->addActor(new Vomit(world(), getX(), getY() + SPRITE_HEIGHT, getDirection()));
			return;
		}
		break;

	case down:
		if (world()->isZombieVomitTriggerAt(getX(), getY() - SPRITE_HEIGHT)) {
			world()->addActor(new Vomit(world(), getX(), getY() - SPRITE_HEIGHT, getDirection()));
			return;
		}
		break;

	case left:
		if (world()->isZombieVomitTriggerAt(getX() - SPRITE_WIDTH, getY())) {
			world()->addActor(new Vomit(world(), getX() - SPRITE_WIDTH, getY(), getDirection()));
			return;
		}
		break;

	case right:
		if (world()->isZombieVomitTriggerAt(getX() + SPRITE_WIDTH, getY())) {
			world()->addActor(new Vomit(world(), getX() + SPRITE_WIDTH, getY(), getDirection()));
			return;
		}
		break;
	}

	// zombie checks if it needs new movement plan
	if (movementPlanDistance() == 0) {
		int rand = randInt(3, 10);			// picks a number between 3 and 10
		resetMovementPlanDistance(rand);

		int randDir = randInt(1, 4);		// picks random direction
		switch (randDir) {
		case 1:		setDirection(up);		break;
		case 2:		setDirection(down);		break;
		case 3:		setDirection(left);		break;
		case 4:		setDirection(right);
		}
	}

	// zombie determines new destination coordinate
	switch (getDirection()) {
	case up:
		// check for blocking object
		if (!world()->isAgentMovementBlockedAt(getX(), getY() + SPRITE_HEIGHT) &&
			!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH - 1, getY() + SPRITE_HEIGHT)) {
			moveTo(getX(), getY() + 1);
			decMovementPlanDistance();
		}
		else {		// can't move in this direction, so set movement plan to 0
			resetMovementPlanDistance(0);
		}
		break;

	case down:
		// check for blocking object
		if (!world()->isAgentMovementBlockedAt(getX(), getY() - 1) &&
			!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH - 1, getY() - 1)) {
			moveTo(getX(), getY() - 1);
			decMovementPlanDistance();
		}
		else {		// can't move in this direction, so set movement plan to 0
			resetMovementPlanDistance(0);
		}
		break;

	case left:
		// check for blocking object
		if (!world()->isAgentMovementBlockedAt(getX() - 1, getY()) &&
			!world()->isAgentMovementBlockedAt(getX() - 1, getY() + SPRITE_HEIGHT - 1)) {
			moveTo(getX() - 1, getY());
			decMovementPlanDistance();
		}
		else {		// can't move in this direction, so set movement plan to 0
			resetMovementPlanDistance(0);
		}
		break;

	case right:
		// check for blocking object
		if (!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH, getY()) &&
			!world()->isAgentMovementBlockedAt(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT - 1)) {
			moveTo(getX() + 1, getY());
			decMovementPlanDistance();
		}
		else {		// can't move in this direction, so set movement plan to 0
			resetMovementPlanDistance(0);
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

DumbZombie::DumbZombie(StudentWorld* w, double x, double y)
	: Zombie(w, IID_ZOMBIE, x, y) {}

void DumbZombie::doSomeThing() {
	


}

void DumbZombie::dieByFallOrBurnIfAppropriate() {
	setDead();
	world()->playSound(SOUND_ZOMBIE_DIE);
	world()->increaseScore(1000);

	// 1 in 10 dumb zombies are carrying vaccines that will drop when they die
	int chance = randInt(1, 10);
	if (chance == 1) {
		world()->addActor(new VaccineGoodie(world(), getX(), getY()));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

SmartZombie::SmartZombie(StudentWorld* w, double x, double y)
	: Zombie(w, IID_ZOMBIE, x, y) {}

void SmartZombie::doSomeThing() {}

void SmartZombie::dieByFallOrBurnIfAppropriate() {
	setDead();
	world()->playSound(SOUND_ZOMBIE_DIE);
	world()->increaseScore(2000);
}

/////////////////////////////////////////////////////////////////////////////////////////

Goodie::Goodie(StudentWorld * w, int imageID, double x, double y)
	: ActivatingObject(w, imageID, x, y, right, 1) {}

void Goodie::activateIfAppropriate(Actor * a) {
	world()->increaseScore(50);		// if player obtains goodie, gain 50 points
	setDead();
	world()->playSound(SOUND_GOT_GOODIE);
	
	increaseGoodieCount();
}

void Goodie::dieByFallOrBurnIfAppropriate() {
	setDead();
}

VaccineGoodie::VaccineGoodie(StudentWorld * w, double x, double y)
	: Goodie(w, IID_VACCINE_GOODIE, x, y) {}

void VaccineGoodie::doSomething() {
	if (isDead()) {
		return;
	}

	// if vaccine overlaps with Penelope
	world()->activateOnAppropriateActors(this);
}

void VaccineGoodie::increaseGoodieCount() {
	world()->player()->increaseVaccines();
}

/////////////////////////////////////////////////////////////////////////////////////////

GasCanGoodie::GasCanGoodie(StudentWorld * w, double x, double y)
	: Goodie(w, IID_GAS_CAN_GOODIE, x, y) {}

void GasCanGoodie::doSomething() {
	if (isDead()) {
		return;
	}

	// if gas can overlaps with Penelope
	world()->activateOnAppropriateActors(this);
}

void GasCanGoodie::increaseGoodieCount() {
	world()->player()->increaseFlameCharges();
}

/////////////////////////////////////////////////////////////////////////////////////////

LandmineGoodie::LandmineGoodie(StudentWorld * w, double x, double y)
	: Goodie(w, IID_LANDMINE_GOODIE, x, y) {}

void LandmineGoodie::doSomething() {
	if (isDead()) {
		return;
	}

	// if landmine goodie overlaps with Penelope
	world()->activateOnAppropriateActors(this);
}

void LandmineGoodie::increaseGoodieCount() {
	world()->player()->increaseLandmines();
}