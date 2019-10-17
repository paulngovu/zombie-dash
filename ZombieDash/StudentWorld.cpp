#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include "Level.h"
#include <cmath>
using namespace std;

GameWorld* createStudentWorld(string assetPath) {
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
	: GameWorld(assetPath) {}

StudentWorld::~StudentWorld() {
	cleanUp();
}

int StudentWorld::init() {
	Level lev(assetPath());

	// get proper level file
	string levelFile = "level0";
	levelFile += to_string(getLevel());
	levelFile += ".txt";

	Level::LoadResult result = lev.loadLevel(levelFile);
	if (result == Level::load_fail_file_not_found || getLevel() == 99) {	// no level found
		cerr << "Level not found" << endl;
		return GWSTATUS_PLAYER_WON;
	}
	else if (result == Level::load_fail_bad_format) {						// level improperly formatted
		cerr << "Your level was improperly formatted" << endl;
		return GWSTATUS_LEVEL_ERROR;
	}
	else if (result == Level::load_success) {								// level found
		cerr << "Successfully loaded level" << endl;

		initializeAllValues();	// initialize all studentworld data members

		for (int y = 0; y < LEVEL_HEIGHT; y++) {		// string rows
			for (int x = 0; x < LEVEL_WIDTH; x++) {		// string cols
				Level::MazeEntry ge = lev.getContentsOf(x, y);	// level_x = 5, level_y = 10
				switch (ge) {									// so x = 80 and y = 160
				case Level::wall:				// creates wall
					cerr << "Wall created at (" << x << ", " << y << ")" << endl;
					addActor(new Wall(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;

				case Level::player:				// creates Penelope
					cerr << "Penelope created at (" << x << ", " << y << ")" << endl;
					m_penelope = new Penelope(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y);
					break;

				case Level::citizen:			// creates citizen
					cerr << "Citizen created at (" << x << ", " << y << ")" << endl;
					addActor(new Citizen(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					m_nCitizens++;
					break;

				case Level::pit:				// creates pit
					cerr << "Pit created at (" << x << ", " << y << ")" << endl;
					addActor(new Pit(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;

				case Level::vaccine_goodie:		// creates vaccine goodie
					cerr << "Vaccine created at (" << x << ", " << y << ")" << endl;
					addActor(new VaccineGoodie(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;

				case Level::gas_can_goodie:		// creates gas can goodie
					cerr << "Gas Can created at (" << x << ", " << y << ")" << endl;
					addActor(new GasCanGoodie(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;

				case Level::landmine_goodie:	// creates landmine goodie
					cerr << "Landmine created at (" << x << ", " << y << ")" << endl;
					addActor(new LandmineGoodie(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;

				case Level::exit:				// creates exit
					cerr << "Exit created at (" << x << ", " << y << ")" << endl;
					addActor(new Exit(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;

				case Level::dumb_zombie:		// creates dumb zombie
					cerr << "Dumb zombie created at (" << x << ", " << y << ")" << endl;
					addActor(new DumbZombie(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;

				case Level::smart_zombie:		// creates smart zombie
					cerr << "Smart zombie created at (" << x << ", " << y << ")" << endl;
					addActor(new SmartZombie(this, SPRITE_WIDTH * x, SPRITE_HEIGHT * y));
					break;
				}
			}
		}
	}

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
	std::vector<Actor*>::iterator i;
	if (!m_penelope->isDead()) {
		// give all actors a chance to do something
		m_penelope->doSomething();
		for (i = m_actors.begin(); i != m_actors.end(); i++) {
			(*i)->doSomething();
		}

		if (m_penelope->isDead()) {		// if Penelope died during this tick
			decLives();
			return GWSTATUS_PLAYER_DIED;
		}

		if (levelFinishedIfAllCitizensGone()) {
			playSound(SOUND_LEVEL_FINISHED);
			return GWSTATUS_FINISHED_LEVEL;
		}
	}

	// Remove newly-dead actors after each tick
	i = m_actors.begin();
	while (i != m_actors.end()) {
		if ((*i)->isDead()) {
			delete *i;
			i = m_actors.erase(i);
		}
		else {
			i++;
		}
	}

	// update the score/lives/level text at screen top
	setDisplayText();

	// the player hasn’t completed the current level and hasn’t died, so
	// continue playing the current level
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
	if (m_penelope != nullptr) {	// only clean up if the actors weren't already deleted
		delete m_penelope;
		m_penelope = nullptr;

		std::vector<Actor*>::iterator it = m_actors.begin();
		while (it != m_actors.end()) {
			delete *it;
			it = m_actors.erase(it);
		}
	}
}

Penelope * StudentWorld::player() {
	return m_penelope;
}

int StudentWorld::nCitizens() const {
	return m_nCitizens;
}

void StudentWorld::decNCitizens() {
	m_nCitizens--;
}

bool StudentWorld::levelFinishedIfAllCitizensGone() const {
	return m_levelFinishedIfAllCitizensGone;
}

void StudentWorld::addActor(Actor * a) {
	m_actors.push_back(a);
}

void StudentWorld::recordCitizenGone() {
	m_nCitizens--;
}

void StudentWorld::recordLevelFinishedIfAllCitizensGone() {
	m_levelFinishedIfAllCitizensGone = true;
}

void StudentWorld::activateOnAppropriateActors(Actor* a) {
	// An object overlaps if Euclidean distance <= 10
	// i.e.: x^2 + y^2 <= 10^2

	// check Penelope
	double deltaX = m_penelope->getX() - a->getX();
	double deltaY = m_penelope->getY() - a->getY();
	if ((deltaX*deltaX) + (deltaY*deltaY) <= 100) {		// if overlaps, activate
		a->activateIfAppropriate(m_penelope);
	}

	// check other actors
	for (std::vector<Actor*>::iterator i = m_actors.begin(); i != m_actors.end(); i++) {
		if (*i != a) {	// make sure we don't act on same actor
			deltaX = (*i)->getX() - a->getX();
			deltaY = (*i)->getY() - a->getY();
			if ((deltaX*deltaX) + (deltaY*deltaY) <= 100) {		// if overlaps, activate
				a->activateIfAppropriate(*i);
			}
		}
	}
}

bool StudentWorld::isAgentMovementBlockedAt(double x, double y) {
	for (std::vector<Actor*>::iterator i = m_actors.begin(); i != m_actors.end(); i++) {
		if ((*i)->blocksMovement() &&	// if actor blocks movement, check coordinates
			x >= (*i)->getX() && x <= (*i)->getX() + SPRITE_WIDTH - 1 &&	// if x is within width of actor
			y >= (*i)->getY() && y <= (*i)->getY() + SPRITE_HEIGHT - 1) {	// if y is within height of actor
			return true;
		}
	}

	// no object is blocking movement
	return false;
}

bool StudentWorld::isFlameBlockedAt(double x, double y) {
	for (std::vector<Actor*>::iterator i = m_actors.begin(); i != m_actors.end(); i++) {
		if ((*i)->blocksFlame() &&	// if actor blocks flame, check coordinates
			x >= (*i)->getX() && x <= (*i)->getX() + SPRITE_WIDTH - 1 &&	// if x is within width of actor
			y >= (*i)->getY() && y <= (*i)->getY() + SPRITE_HEIGHT - 1) {	// if y is within height of actor
			return true;
		}
	}

	// no object is blocking flame
	return false;
}

bool StudentWorld::isZombieVomitTriggerAt(double x, double y) {
	// vomit is triggered with humans (Penelope, citizens)

	// check Penelope first
	if (m_penelope->getX() == x && m_penelope->getY() == x) {
		return true;
	}

	// check citizens
	for (std::vector<Actor*>::iterator i = m_actors.begin(); i != m_actors.end(); i++) {
		if ((*i)->triggersZombieVomit() &&
			(*i)->getX() == x && (*i)->getY() == y) {
			return true;
		}
	}

	return false;
}

void StudentWorld::setDisplayText() {
	int score = getScore();
	int level = getLevel();
	int lives = getLives();
	int vaccines = m_penelope->getNumVaccines();
	int flames = m_penelope->getNumFlameCharges();
	int mines = m_penelope->getNumLandmines();
	int infected = m_penelope->getInfectionDuration();

	// create display string
	string toDisplay = formatDisplayText(score, level, lives, vaccines, flames, mines, infected);

	// display formatted string
	setGameStatText(toDisplay);
}

std::string StudentWorld::formatDisplayText(int score, int level, int lives, int vaccines, int flames, int mines, int infected) const {
	string s = "Score: ";
	s += formatDigit(score, 6, true);

	s += "  Level: ";
	s += formatDigit(level, 2, false);

	s += "  Lives: ";
	s += formatDigit(lives, 1, true);

	s += "  Vaccines: ";
	s += formatDigit(vaccines, 2, false);

	s += "  Flames: ";
	s += formatDigit(flames, 2, false);

	s += "  Mines: ";
	s += formatDigit(mines, 2, false);

	s += "  Infected: ";
	s += formatDigit(infected, 1, false);

	return s;
}

// Format the score to append to the display text
std::string StudentWorld::formatDigit(int input, int totalDigits, bool zeros) const {
	char leading;			// leading char
	if (zeros) {			// if there are zeros
		leading = '0';
	}
	else {					// if not, just put space
		leading = ' ';
	}

	// if input == 0, format it to the totalDigits
	if (input == 0) {
		string s = "0";
		for (int i = 0; i < totalDigits - 1; i++) {
			s = leading + s;
		}

		return s;
	}

	// convert integer to string
	string s = to_string(input);
	for (int i = s.length(); i < totalDigits; i++) {
		s = leading + s;
	}

	return s;
}

void StudentWorld::initializeAllValues() {
	m_nCitizens = 0;
	m_levelFinishedIfAllCitizensGone = false;
}