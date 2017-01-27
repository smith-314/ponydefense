#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <functional>
#include <errno.h>
#include <math.h>
#include <limits>

#include <SOIL/SOIL.h>
#include <GL/glut.h>
#include <FTGL/ftgl.h>

const char *VERSION = "V0.32";

#include "texture.cpp"
#include "draw.cpp"
#include "menu.cpp"
#include "grid.cpp"
#include "stats.cpp"
#include "pony.cpp"
#include "effects.cpp"
#include "tower.cpp"
#include "airstrike.cpp"
#include "wave.cpp"


// forward declarations
class backgroundPonies *bgPonies;
void drawMainMenu();
class menu *currentMenu = NULL;

// structure to save stats of running game
struct sgSettings {
	unsigned int mapid, wave;
	double lives, score, money;
};

// structure to save towers of running game
struct sgTower {
	unsigned int x, y, type, upgrade;
	double health;
};

// check if savegame file has loadable game
char *savegameInfo() {
	FILE *fp = fopen(stats::path, "rb");
	if(fp == NULL) return NULL;
	if(fseek(fp, sizeof(double)+sizeof(stats::maps), SEEK_SET) == -1) {
		fclose(fp);
		return NULL;
	}
	struct sgSettings s;
	if(fread((void*)&s, sizeof(s), 1, fp) != 1) {
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	char *buffer = new char[256];
	snprintf(buffer, 256, "Wave: %d", s.wave);
	return buffer;
}

// load saved game
void loadGame() {
	// open savegame file
	FILE *fp = fopen(stats::path, "rb");
	if(fp == NULL) {
		fprintf(stderr, "Error: can't open savegame file:\n%s\n", stats::path);
		perror(NULL);
		exit(1);
	}
	if(fseek(fp, sizeof(double)+sizeof(stats::maps), SEEK_SET) == -1) {
		fprintf(stderr, "Error: savegame is damaged - wrong size.\n");
		perror(NULL);
		exit(1);
	}
	// load stats
	struct sgSettings s;
	if(fread((void*)&s, sizeof(s), 1, fp) != 1) {
		fprintf(stderr, "Error: can't read savegame file:\n");
		perror(NULL);
		exit(1);
	}

	// initialize game
	delete bgPonies;
	grid::init((MAP)s.mapid);
	if(s.mapid < MAP1 || s.mapid > MAP5) {
		fprintf(stderr, "Error: savegame is damaged - unknown map.\n");
		exit(1);
	}
	grid::wv = new wave();
	stats::t = 0;
	if(s.mapid == MAP4 || s.mapid == MAP5)
		draw::setBackground(tex::BG_SNOW);
	else draw::setBackground(tex::BG_DESERT);

	// load towers
	struct sgTower t;
	while(fread((void*)&t, sizeof(t), 1, fp) == 1)  {
		if(grid::map[t.x][t.y].type == NONE) {
			class tower *tw = new tower(t.x, t.y, (TOWERTYPE)t.type);
			for(unsigned int i = 0; i < t.upgrade; i++) tw->doUpgrade();
			tw->setHealth(t.health);
		}
		else if(grid::map[t.x][t.y].type == WAY) {
			class supTower *tw = new supTower(t.x, t.y, (SUPTWTYPE)t.type);
			for(unsigned int i = 0; i < t.upgrade; i++) tw->doUpgrade();
			tw->setHealth(t.health);
		}
	}

	// set stats
	grid::lives = s.lives;
	grid::score = s.score;
	grid::money = s.money;
	grid::wave = s.wave;

	fclose(fp);
}

// save running game
void saveGame(FILE *fp) {
	// save stats
	struct sgSettings s;
	s.lives = grid::lives;
	s.score = grid::score;
	s.wave = grid::wave;
	s.money = grid::money;
	s.mapid = grid::mapid;
	fwrite((void*)&s, sizeof(s), 1, fp); 

	// save towers
	for(unsigned int x = 0; x < grid::size; x++) {
		for(unsigned int y = 0; y < grid::size; y++) {
			if(grid::map[x][y].tw != NULL) {
				if(grid::map[x][y].type == NONE) {
					struct sgTower t;
					t.x = x; t.y = y;
					t.type = grid::map[x][y].tw->getType();
					t.upgrade = grid::map[x][y].tw->getUpgrade();
					t.health = grid::map[x][y].tw->getHealth();
					fwrite((void*)&t, sizeof(t), 1, fp); 
				}
				else if(grid::map[x][y].type == WAY) {
					struct sgTower t;
					t.x = x; t.y = y;
					t.type = grid::map[x][y].ts->getType();
					t.upgrade = grid::map[x][y].ts->getUpgrade();
					t.health = grid::map[x][y].ts->getHealth();
					fwrite((void*)&t, sizeof(t), 1, fp); 
				}
			}
		}
	}
}

// cleanup after quit/gameover
void cleanup() {
	// delete ponies
	while(ponyList != NULL) {
		delete ponyList->py;
	}
	// delete towers
	for(unsigned int x = 0; x < grid::size; x++) {
		for(unsigned int y = 0; y < grid::size; y++) {
			if(grid::map[x][y].type == NONE) {
				if(grid::map[x][y].tw != NULL)
					delete grid::map[x][y].tw;
			}
			else if(grid::map[x][y].type == WAY) {
				if(grid::map[x][y].ts != NULL)
					delete grid::map[x][y].ts;
			}
		}
	}
	// delete cell array
	grid::cleanup();
}

// refresh mouse cell position
void mouseCallback(int x, int y) {
	xF = grid::pfx(draw::relX(x));
	yF = grid::pfy(draw::relY(y));
}

// callback for exit message
void menuCallbackExit(int res) {
	if(res == 1) { // back to main menu
		stats::save();
		cleanup();
		delete grid::wv;
		draw::speed = 1;
		drawMainMenu();
		bgPonies = new backgroundPonies();
	}
	else if(res == -1) { // continue
		grid::paused = false;
	}
}

// callback for gameover message
void gameoverCallback(int res) {
	stats::save();
	cleanup();
	delete grid::wv;
	draw::speed = 1;
	drawMainMenu();
	bgPonies = new backgroundPonies();
}

// build callback
void menuCallbackBuild(int res) {
	currentMenu = NULL;
	// default towers
	if(res == 1) new tower(xM, yM, RIFLE);
	else if(res == 2) new tower(xM, yM, RAILGUN);
	else if(res == 3) new tower(xM, yM, FLAMETHROWER);
	else if(res == 4) new tower(xM, yM, LASER);
	else if(res == 5) new tower(xM, yM, ARTILLERY);
	else if(res == 6) new tower(xM, yM, SHIELD);
	// support towers
	else if(res == 10) new supTower(xM, yM, FENCE);
	else if(res == 11) new supTower(xM, yM, DRONE);
	else if(res == 12) new supTower(xM, yM, RADAR);
}

// upgrade callback
void menuCallbackUpgrade(int res) {
	currentMenu = NULL;
	// strategy
	if(res == 2) grid::map[xM][yM].tw->setStrategy(CLOSEST);
	else if(res == 3) grid::map[xM][yM].tw->setStrategy(STRONGEST);
	else if(res == 4) grid::map[xM][yM].tw->setStrategy(FIRST);
	else if(res == 5) grid::map[xM][yM].tw->setStrategy(LAST);
	// upgrade stuff
	else if(res == 6) grid::map[xM][yM].tw->doUpgrade();
	else if(res == 7) grid::map[xM][yM].tw->doRepair();
	else if(res == 8) grid::map[xM][yM].tw->doRecover();
}

// upgrade callback (support towers)
void menuCallbackSupUpgrade(int res) {
	currentMenu = NULL;
	if(res == 1) grid::map[xM][yM].ts->doUpgrade();
	else if(res == 2) grid::map[xM][yM].ts->doRecover();
}

// functions to check if player can afford item…
bool aff(int type, int upgrade = BASE) {
	if(grid::money >= tower::getPrice(type, upgrade))
		return false;
	return true;
}

bool affUpg(int x, int y) {
	if(grid::map[x][y].type == NONE) {
		class tower *t = grid::map[x][y].tw;
		if(grid::money >= tower::getPrice(t->getType(),t->getUpgrade()+1))
			return false;
		return true;
	}
	if(grid::map[x][y].type == WAY) {
		class supTower *t = grid::map[x][y].ts;
		if(grid::money >= supTower::getPrice(t->getType(),t->getUpgrade()+1))
			return false;
		return true;
	}
	return true;
}

bool affRep(int x, int y) {
	if(grid::money >= grid::map[x][y].tw->getRepairPrice())
		return false;
	return true;
}

bool affSup(int type) {
	if(grid::money >= supTower::getPrice(type, BASE))
		return false;
	return true;
}

// mouse callback for running game
void mouseClickCallback(int button, int state, int xc, int yc) {
	// only left click
	if(button != 0 || state != 1) return;

	// calculate mouse position
	xM = grid::pfx(draw::relX(xc));
	yM = grid::pfy(draw::relY(yc));
	if(xM == -1 || yM == -1) return;

	// default build menu
	if(grid::map[xM][yM].tw == NULL && grid::map[xM][yM].type == NONE) {
		vec *menuPos = grid::fp(xM, yM);
		char textPrice[256];
		const char *text[] = {textPrice, NULL};
		class menu *m = new menu("Build",  menuPos, 0.35, &menuCallbackBuild);
		currentMenu = m;

		snprintf(textPrice, 256, "%d$", (int)tower::getPrice(RIFLE, BASE));
		m->addEntry("Rifle", (char**)text, tex::TOWER_RIFLE_PREVIEW, 1, aff(RIFLE));

		if(stats::has(RAILGUN)) {
			snprintf(textPrice, 256, "%d$", (int)tower::getPrice(RAILGUN, BASE));
			m->addEntry("Railgun", (char**)text, tex::TOWER_RAILGUN_PREVIEW, 2, aff(RAILGUN));
		}

		if(stats::has(FLAMETHROWER)) {
			snprintf(textPrice, 256, "%d$", (int)tower::getPrice(FLAMETHROWER, BASE));
			m->addEntry("Flamethrower", (char**)text, tex::TOWER_FLAMETHROWER_PREVIEW, 3, aff(FLAMETHROWER));
		}

		if(stats::has(LASER)) {
			snprintf(textPrice, 256, "%d$", (int)tower::getPrice(LASER, BASE));
			m->addEntry("Laser", (char**)text, tex::TOWER_LASER_PREVIEW, 4, aff(LASER));
		}

		if(stats::has(ARTILLERY)) {
			snprintf(textPrice, 256, "%d$", (int)tower::getPrice(ARTILLERY, BASE));
			m->addEntry("Artillery", (char**)text, tex::TOWER_ARTILLERY_PREVIEW, 5, aff(ARTILLERY));
		}

		if(stats::has(SHIELD)) {
			snprintf(textPrice, 256, "%d$", (int)tower::getPrice(SHIELD, BASE));
			m->addEntry("Shield", (char**)text, tex::TOWER_SHIELD_PREVIEW, 6, aff(SHIELD));
		}
		m->showMenu();
	}

	// support tower build menu
	else if(grid::map[xM][yM].type == WAY && grid::map[xM][yM].ts == NULL) {
		vec *menuPos = grid::fp(xM, yM);
		char textPrice[256];
		const char *text[] = {textPrice, NULL};
		class menu *m = new menu("Build",  menuPos, 0.35, &menuCallbackBuild);
		currentMenu = m;

		if(stats::has(FENCE)) {
			snprintf(textPrice, 256, "%d$", (int)supTower::getPrice(FENCE, BASE));
			m->addEntry("Electric Fence", (char**)text, tex::TOWER_FENCE_PREVIEW, 10, affSup(FENCE));
		}

		if(stats::has(DRONE)) {
			snprintf(textPrice, 256, "%d$", (int)supTower::getPrice(DRONE, BASE));
			m->addEntry("Attack Drone", (char**)text, tex::TOWER_DRONE_ATTACK_PREVIEW, 11, affSup(DRONE));
		}
		snprintf(textPrice, 256, "%d$", (int)supTower::getPrice(RADAR, BASE));
		m->addEntry("Recon Drone", (char**)text, tex::TOWER_DRONE_PREVIEW, 12, affSup(RADAR));
		m->showMenu();
	}

	// upgrade and settings menu
	else if(grid::map[xM][yM].type == NONE && grid::map[xM][yM].tw != NULL ) {
		vec *menuPos = grid::fp(xM, yM);
		const char *textStrategy[] = {"Targeting mode", NULL};

		char textPrice[256];
		const char *text[] = {textPrice, NULL};
		class tower *t = grid::map[xM][yM].tw;

		class menu *m = new menu("Upgrade",  menuPos, 0.35, &menuCallbackUpgrade);
		currentMenu = m;
		// targeting
		if(!grid::map[xM][yM].tw->isShield()) {
			m->addEntry("Strategy", (char**)textStrategy, tex::MENU_STRATEGY, 1);
			int strat = grid::map[xM][yM].tw->getStrategy();
			if(strat == FIRST) m->addSubEntry("First", NULL, 0, 4, true);
			else m->addSubEntry("First", NULL, 0, 4);
			if(strat == LAST) m->addSubEntry("Last", NULL, 0, 5, true);
			else m->addSubEntry("Last", NULL, 0, 5);
			if(strat == CLOSEST) m->addSubEntry("Distance", NULL, 0, 2, true);
			else m->addSubEntry("Distance", NULL, 0, 2);
			if(strat == STRONGEST) m->addSubEntry("Strength", NULL, 0, 3, true);
			else m->addSubEntry("Strength", NULL, 0, 3);
		}
		// upgrade
		if(grid::map[xM][yM].tw->hasUpgrade()) {
			snprintf(textPrice, 256, "%d$", (int)tower::getPrice(t->getType(), t->getUpgrade()+1));
			m->addEntry("Upgrade", (char**)text, tex::MENU_UPGRADE, 6, affUpg(xM, yM));
		}
		// repair
		if(grid::map[xM][yM].tw->isDamaged()) {
			snprintf(textPrice, 256, "%d$", (int)t->getRepairPrice());
			m->addEntry("Repair", (char**)text, tex::MENU_REPAIR, 7, affRep(xM, yM));
		}
		snprintf(textPrice, 256, "%d$", (int)t->getRecoverMoney());
		m->addEntry("Sell", (char**)text, tex::MENU_RECOVER, 8);
		m->showMenu();
	}

	// upgrade and settings menu for support tower
	else if(grid::map[xM][yM].type == WAY && grid::map[xM][yM].ts != NULL ) {
		vec *menuPos = grid::fp(xM, yM);
		char textPrice[256];
		const char *text[] = {textPrice, NULL};
		class supTower *t = grid::map[xM][yM].ts;

		class menu *m = new menu("Upgrade",  menuPos, 0.35, &menuCallbackSupUpgrade);
		currentMenu = m;
		if(grid::map[xM][yM].ts->hasUpgrade()) {
			snprintf(textPrice, 256, "%d$", (int)supTower::getPrice(t->getType(), t->getUpgrade()+1));
			m->addEntry("Upgrade", (char**)text, tex::MENU_UPGRADE, 1, affUpg(xM, yM));
		}
		snprintf(textPrice, 256, "%d$", (int)t->getRecoverMoney());
		m->addEntry("Sell", (char**)text, tex::MENU_RECOVER, 2);
		m->showMenu();
	}
}

class menuEntry *ent;

// respond to keyboard events in running game
void keyboardCallback(unsigned char key, int x, int y) {
	// exit question
	const char *text[] = { "Save game and return to", "main menu?", NULL };
	if(key == 27) {
		new question("Quit", (char**)text, "Yes", "No", 0, &menuCallbackExit);
		grid::paused = true;
	}

	// change speed
	if(key == '1') draw::speed = 1;
	if(key == '2') draw::speed = 2;
	if(key == '3') draw::speed = 4;
	if(key == '4') draw::speed = 8;
	if(key == '5') draw::speed = 16;

	// toggle debug window
	if(key == 'd') {
		if(draw::debugMode) draw::debugMode = false;
		else draw::debugMode = true;
	}

	// toggle pause
	if(key == ' ') {
		if(grid::paused) grid::paused = false;
		else grid::paused = true;
	}
}

void drawMainMenu() {
	draw::setBackground(tex::BG_DESERT);

	// check and close old menu
	if(currentMenu != NULL) {
		currentMenu->closeMenu();
		currentMenu = NULL;
	}

	// create main menu
	class menu *m = new menu("PonyDefense",  new vec(-0.25,0.3), 0.5, &mainMenuCallback, 15);
	const char *empty[] = {"", NULL};
	m->addEntry("New Game", (char**)empty, 0, 1);
	const char *textMap1[] = {"Easy", "Multiplier: 1", NULL};
	const char *textMap2[] = {"Easy", "Multiplier: 2", NULL};
	const char *textMap3[] = {"Medium", "Multiplier: 3", NULL};
	const char *textMap4[] = {"Medium", "Multiplier: 3", NULL};
	const char *textMap5[] = {"Hard", "Multiplier: 4", NULL};
	if(stats::has(MAP1)) m->addSubEntry("Map I", (char**)textMap1, tex::MAP_1_PREVIEW, 10);
	if(stats::has(MAP2)) m->addSubEntry("Map II", (char**)textMap2, tex::MAP_2_PREVIEW, 11);
	if(stats::has(MAP3)) m->addSubEntry("Map III", (char**)textMap3, tex::MAP_3_PREVIEW, 12);
	if(stats::has(MAP4)) m->addSubEntry("Map IV", (char**)textMap4, tex::MAP_4_PREVIEW, 13);
	if(stats::has(MAP5)) m->addSubEntry("Map V", (char**)textMap5, tex::MAP_5_PREVIEW, 14);

	char *loadBuf = savegameInfo();
	const char *loadText[] = {loadBuf, NULL};
	if(loadBuf == NULL)
		m->addEntry("Load Game", (char**)empty, 0, 99, 1);
	else {
		m->addEntry("Load Game", (char**)loadText, 0, 99, 0);
		delete[] loadBuf;
	}
		

	// show highscores if available
	m->addEntry("Highscores", (char**)empty, 0, 21);
	struct stats::mapstats *hs = stats::maps;
	char textPoints[512], textWaves[512];
	const char *text[] = {textWaves, textPoints, NULL};
	if(hs[MAP1].points > 0) {
		snprintf(textWaves, 512, "Wave: %d", (int)hs[MAP1].waves);
		snprintf(textPoints, 512, "Score: %d", (int)hs[MAP1].points);
		m->addSubEntry("Map I", (char**)text, tex::MAP_1_PREVIEW, -1);
	}
	if(hs[MAP2].points > 0) {
		snprintf(textWaves, 512, "Wave: %d", (int)hs[MAP2].waves);
		snprintf(textPoints, 512, "Score: %d", (int)hs[MAP2].points);
		m->addSubEntry("Map II", (char**)text, tex::MAP_2_PREVIEW, -1);
	}
	if(hs[MAP3].points > 0) {
		snprintf(textWaves, 512, "Wave: %d", (int)hs[MAP3].waves);
		snprintf(textPoints, 512, "Score: %d", (int)hs[MAP3].points);
		m->addSubEntry("Map III", (char**)text, tex::MAP_3_PREVIEW, -1);
	}
	if(hs[MAP4].points > 0) {
		snprintf(textWaves, 512, "Wave: %d", (int)hs[MAP4].waves);
		snprintf(textPoints, 512, "Score: %d", (int)hs[MAP4].points);
		m->addSubEntry("Map IV", (char**)text, tex::MAP_4_PREVIEW, -1);
	}
	if(hs[MAP5].points > 0) {
		snprintf(textWaves, 512, "Wave: %d", (int)hs[MAP5].waves);
		snprintf(textPoints, 512, "Score: %d", (int)hs[MAP5].points);
		m->addSubEntry("Map V", (char**)text, tex::MAP_5_PREVIEW, -1);
	}

	m->addEntry("Exit", (char**)empty, 0, 22);
	m->showMenu();
}

void mainMenuCallback(int res) {
	if(res == 10) {
		delete bgPonies;
		grid::init(MAP1);
		grid::wv = new wave();
		draw::setBackground(tex::BG_DESERT);
	}
	else if(res == 11) {
		delete bgPonies;
		grid::init(MAP2);
		grid::wv = new wave();
		draw::setBackground(tex::BG_DESERT);
	}
	else if(res == 12) {
		delete bgPonies;
		grid::init(MAP3);
		grid::wv = new wave();
		draw::setBackground(tex::BG_DESERT);
	}
	else if(res == 13) {
		delete bgPonies;
		grid::init(MAP4);
		grid::wv = new wave();
		draw::setBackground(tex::BG_SNOW);
	}
	else if(res == 14) {
		delete bgPonies;
		grid::init(MAP5);
		grid::wv = new wave();
		draw::setBackground(tex::BG_SNOW);
	}
	else if(res == 22) exit(0);
	else if(res == 99) loadGame();
	else drawMainMenu();
	stats::t = 0;
}

void tutorialCallback(int a) {
	drawMainMenu();
	bgPonies->enableStats();
};

int main(int argc, char **argv) {
	// initialize draw class and set callbacks
	srand(time(NULL));
	draw::init(&argc, argv);
	draw::setMouseMotionCallback(&mouseCallback);
	draw::setKeyboardCallback(&keyboardCallback);
	draw::setMouseCallback(&mouseClickCallback);

	// load global stats
	if(stats::load()) {
		// open tutorial if there is no savegame
		draw::setBackground(tex::BG_DESERT);
		bgPonies = new backgroundPonies(false);
		const char *text[] = {	"",
					"Like in any other tower",
					"defense game you have to",
					"place towers to stop your",
					"opponent on a fixed path.",
					"",
					"left click: build/upgrade",
					"space:      play/pause",
					"1...5:      control speed",
					"",
					"With each rank a new",
					"tower or map is unlocked."
					"",
					NULL };
		new question("Tutorial", (char**)text, "Ok", NULL, 0, &tutorialCallback);
	}
	else {
		// initialize main menu
		drawMainMenu();
		bgPonies = new backgroundPonies();
	}

	glutMainLoop();
	return 0;
}
