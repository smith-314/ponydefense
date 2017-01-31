// grid.cpp
// manage grid layout and running game


// important type definitions
enum TOWERTYPE {RIFLE, RAILGUN, FLAMETHROWER, LASER, ARTILLERY, SHIELD, LIGHTNING};
enum SUPTWTYPE {FENCE, DRONE, RADAR};
enum UPGRTYPE {BASE, UPGRADE1, UPGRADE2};
enum TARGETTYPE {CLOSEST, FIRST, LAST, STRONGEST};
enum PONYTYPE {RARITY, FLUTTERSHY, PINKIE_PIE, TWILIGHT_SPARKLE, TROJAN, RARITY_SHIELD, RAINBOW_DASH, APPLEJACK, TWILIGHT_SPARKLE_SHIELD};
enum DMGTYPE {DMGAIR, DMGGROUND};
enum CELLTYPE {NONE, WAY, SPAWN, BLOCKED};
enum MAP {MAP1, MAP2, MAP3, MAP4, MAP5, MAPCUSTOM};

// cell structure for grid array
struct _cell {
	CELLTYPE type;
	union { // pointer to tower or NULL
		class tower *tw; // if type = WAY
		class supTower *ts; // if type = NONE
	};
};

// definition in main.cpp
void gameoverCallback(int);
// mouse field position
int xF = -1, yF = -1;
// position buffer for menu selection
int xM = -1, yM = -1;

class grid {
	private:
		// initialize array
		//create custom map from file
		static void createMapCustom(int mapnumber) {
				//load map functions 
				if(mapparser::customMapExists(mapnumber) && mapparser::customMapValid(mapnumber)){//this should be unneccesary because menu link only exists if map exists
					size = mapparser::customMapSize(mapnumber);
					map = new _cell*[size];
					int mapfields[size*size];
					int *maparr = mapparser::customMapGet(mapnumber, mapfields);

					for(unsigned int x = 0; x < size; x++) {
						map[x] = new _cell[size];
						for(unsigned int y = 0; y < size; y++) {
							if(maparr[x+size*y] == 0) map[x][y].type = NONE;
							else if(maparr[x+size*y] == 1) map[x][y].type = BLOCKED;
							else if(maparr[x+size*y] == 2) map[x][y].type = SPAWN;
							else if(maparr[x+size*y] == 3) map[x][y].type = WAY;
							else map[x][y].type = NONE;
							map[x][y].tw = NULL;
						}
					}
				}
				else createMap(11);
			}

			static void createMap(int sz) {
				size = sz;
				map = new _cell*[size];
				for(unsigned int x = 0; x < size; x++) {
					map[x] = new _cell[size];
					for(unsigned int y = 0; y < size; y++) {
						map[x][y].type = NONE;
						map[x][y].tw = NULL;
					}
				}
			}

			// status frame: title
			static void drawTitle(double ypos, const char *title) {
				color *cFrame = new color(0,0,0,0.6);
				color *cTitle = new color(1,1,1,0.6);
				vec *v1 = new vec(-1.0, ypos+0.05);
				vec *v2 = new vec(-0.85, ypos);
				draw::rect(v1, v2->clone(), cFrame->clone());
				vec *v3 = new vec(margin-0.1, ypos);
				v1 = new vec(-0.85, ypos+0.05);
				draw::triangle(v1, v2, v3, cFrame);
				draw::print(new vec(-0.99, ypos+0.01), title, cTitle);
			}

			// status frame: background
			static void drawFrame(double ypos, double ysize) {
				color *cFrame = new color(0,0,0,0.5);
				vec *v1 = new vec(-1.0, ypos);
				vec *v2 = new vec(margin-0.1, ypos-ysize);
			draw::rect(v1, v2, cFrame);
		}

	public:
		// cell structure
		static _cell ** map;
		static MAP mapid;
		static unsigned int size;
		static double stx; // relative cellsize y
		static double sty; // relative cellsize x
		static double margin; // left x-border

		// stats of the running game 
		static bool paused, gameover;
		static double lives, score, money;
		static unsigned int wave;
		static class wave *wv;

		// delete cell array and callback
		static void cleanup() {
			for(unsigned int x = 0; x < size; x++) {
				delete[] map[x];
			}
			delete[] map;
			map = NULL;
			draw::delRenderCallback((void*)&grid::size);
		}

		// initialize grid
		static void init(MAP _mapid) {
			// reset stats 
			if(map != NULL) cleanup();
			mapid = _mapid;
			paused = true;
			gameover = false;
			lives = 10;
			money = 500;
			score = wave = 0;

			// load map
			if(mapid == MAP1) {
				createMap(11);
				map[1][1].type = SPAWN;
				for(int x = 2; x < 10; x++) map[x][1].type = WAY;
				for(int y = 1; y < 10; y++) map[9][y].type = WAY;
				for(int x = 1; x < 10; x++) map[x][9].type = WAY;
				for(int y = 3; y < 10; y++) map[1][y].type = WAY;
				for(int x = 1; x < 8; x++) map[x][3].type = WAY;
				for(int y = 3; y < 8; y++) map[7][y].type = WAY;
				for(int x = 3; x < 8; x++) map[x][7].type = WAY;
				for(int y = 5; y < 8; y++) map[3][y].type = WAY;
				for(int x = 3; x < 6; x++) map[x][5].type = WAY;
			}
			else if(mapid == MAP2) {
				createMap(11);
				map[9][2].type = SPAWN;
				map[7][2].type = WAY; map[8][2].type = WAY;
				for(int x = 5; x < 8; x++) map[x][1].type = WAY;
				for(int y = 1; y < 4; y++) map[4][y].type = WAY;
				for(int y = 3; y < 6; y++) map[5][y].type = WAY;
				for(int x = 4; x < 8; x++) map[x][5].type = WAY;
				map[4][6].type = WAY; map[1][9].type = WAY;
				for(int y = 6; y < 9; y++) map[3][y].type = WAY;
				map[2][8].type = WAY; map[2][9].type = WAY;
				for(int y = 5; y < 8; y++) map[7][y].type = WAY;
				for(int x = 7; x < 10; x++) map[x][7].type = WAY;
				for(int y = 7; y < 10; y++) map[9][y].type = WAY;
			}
			else if(mapid == MAP3) {
				createMap(11);
				map[5][5].type = SPAWN;
				map[0][5].type = WAY;
				map[10][5].type = WAY;
				for(int y = 1; y < 9; y++) {
					map[1][y].type = WAY;
					map[4][y].type = WAY;
					map[6][y].type = WAY;
					map[9][y].type = WAY;
				}
				for(int x = 1; x < 5; x++) {
					map[x][1].type = WAY;
					map[x][9].type = WAY;
				}
				for(int x = 6; x < 10; x++) {
					map[x][1].type = WAY;
					map[x][9].type = WAY;
				}
			}
			else if(mapid == MAP4) {
				createMap(9);
				for(int x = 1; x < 8; x++) map[x][2].type = WAY;
				for(int x = 1; x < 8; x++) map[x][6].type = WAY;
				for(int y = 2; y < 7; y++) map[1][y].type = WAY;
				for(int y = 2; y < 7; y++) map[7][y].type = WAY;
				map[4][2].type = SPAWN;
				for(int y = 3; y < 6; y++) map[2][y].type = BLOCKED;
				for(int y = 3; y < 6; y++) map[6][y].type = BLOCKED;
				for(int x = 2; x < 7; x++) map[x][4].type = BLOCKED;
			}
			else if(mapid == MAP5) {
				createMap(11);
				for(int x = 0; x < 11; x++) map[x][2].type = WAY;
				for(int y = 2; y < 11; y++) map[5][y].type = WAY;
				map[5][2].type = SPAWN;
			}
			else {
				fprintf(stderr, "Error: unknown map (%d).\n", mapid);
				exit(1);
			}

			// set relative cellsize
			sty = 2.0/size;
			margin = (-2.0 + draw::ar);
			stx = (1.0-margin)/size;

			// set callback
			draw::addRenderCallback(&grid::drawGrid,(void*)&grid::size,draw::HIGHEST);
		}

		//init grid for custom map
		static void init_custom(MAP _mapid, unsigned int _mapnumber) {
			// reset stats 
			if(map != NULL) cleanup();
			mapid = _mapid;
			paused = true;
			gameover = false;
			lives = 10;
			money = 500;
			score = wave = 0;

			unsigned int mapnumber = _mapnumber; // number of loaded file

			// load map
			if(mapid == MAPCUSTOM) {
				createMapCustom(mapnumber);
			}
			else {
				fprintf(stderr, "Error: unknown map (%d).\n", mapid);
				exit(1);
			}

			// set relative cellsize
			sty = 2.0/size;
			margin = (-2.0 + draw::ar);
			stx = (1.0-margin)/size;

			// set callback
			draw::addRenderCallback(&grid::drawGrid,(void*)&grid::size,draw::HIGHEST);
		}

		// render callback
		static void drawGrid() {
			if(draw::debugMode) fprintf(stderr, "Callback: drawGrid()\n");

			// draw grid lines
			for(unsigned int x = 1; x < size; x++) { // x-lines
				vec *v1 = new vec(margin+x*stx,1.0);
				vec *v2 = new vec(margin+x*stx,-1.0);
				draw::line(v1,v2, new color(0,0,0,0.5), 2.0);
			}
			for(unsigned int y = 1; y < size; y++) { // y-lines
				vec *v1 = new vec(margin,-1.0+y*sty);
				vec *v2 = new vec(1.0,-1.0+y*sty);
				draw::line(v1,v2, new color(0,0,0,0.5), 2.0);
			}

			// highlight fields
			for(unsigned int x = 0; x < size; x++) {
				for(unsigned int y = 0; y < size; y++) {
					// set coordinates
					vec *v1 = fp(x, y);
					v1->x-=0.5*stx; v1->y+=0.5*sty;
					vec *v2 = fp(x, y);
					v2->x+=0.5*stx; v2->y-=0.5*sty;

					// choose color depending on field type
					if((int)x == xF && (int)y == yF && map[x][y].type != BLOCKED && map[x][y].type != SPAWN)
						draw::rect(v1, v2, new color(0,0,0,0.3));
					else if(map[x][y].type == SPAWN)
						draw::rect(v1, v2, new color(0,1,0,0.15));
					else if(map[x][y].type == WAY)
						draw::rect(v1, v2, new color(0,0,0,0.15));
					else if(map[x][y].type == BLOCKED)
						draw::rect(v1, v2, new color(1,0,0,0.15));
					else {
						delete v1; delete v2;
					}
				}
			}

			// draw stats
			double pos = 0.3;
			color *c = new color(1,1,1,0.9);
			drawTitle(pos, "Money");
			drawFrame(pos, 0.06);
			char buffer[250];
			if(money < 1000) snprintf(buffer, 250, "%d$", (int)money);
			else if(money < 999000) snprintf(buffer, 250, "%0.1fk$", (money/1000)-0.05);
			else snprintf(buffer, 250, "%0.1fM$", (money/1000000)-0.05);
			draw::print(new vec(-0.9, pos-0.04), buffer,c->clone());

			pos = 0.15;
			drawTitle(pos, "Lives");
			drawFrame(pos, 0.06);
			snprintf(buffer, 250, "%d", (int)lives);
			draw::print(new vec(-0.9, pos-0.04), buffer,c->clone());

			pos = 0;
			drawTitle(pos, "Score");
			drawFrame(pos, 0.06);
			if(score < 1000) snprintf(buffer, 250, "%d", (int)score);
			else if(score < 999000) snprintf(buffer, 250, "%0.1fk", score/1000);
			else snprintf(buffer, 250, "%0.1fM", score/1000000);
			draw::print(new vec(-0.9, pos-0.04), buffer,c->clone());

			pos = -0.15;
			drawTitle(pos, "Wave");
			drawFrame(pos, 0.06);
			snprintf(buffer, 250, "%d", wave);
			draw::print(new vec(-0.9, pos-0.04), buffer, c->clone());

			if(paused) {
				pos = 0.70;
				drawTitle(pos, "Paused");
				drawFrame(pos, 0.06);
				draw::print(new vec(-0.99, pos-0.04), "Cont. with SPACE", c->clone());
			}

			if(draw::speed != 1) {
				pos = 0.85;
				drawTitle(pos, "Speed");
				drawFrame(pos, 0.06);
				snprintf(buffer, 250, "%dx", draw::speed);
				draw::print(new vec(-0.88, pos-0.04), buffer, c->clone());
			}
			// DEBUG
			if(draw::debugMode) {
				pos = -0.5;
				drawTitle(pos, "DEBUG");
				drawFrame(pos, 0.18);
				snprintf(buffer, 250, "FPS:    %d", draw::fps);
				draw::print(new vec(-0.99, pos-0.04), buffer, c->clone());
				snprintf(buffer, 250, "PrCalc: %ldms", draw::timePreRender);
				draw::print(new vec(-0.99, pos-0.08), buffer, c->clone());
				snprintf(buffer, 250, "Render: %ldms", draw::timeRender);
				draw::print(new vec(-0.99, pos-0.12), buffer, c->clone());
				snprintf(buffer, 250, "Callbs: %d", draw::callbackCounter);
				draw::print(new vec(-0.99, pos-0.16), buffer, c->clone());
			}

			delete c;

			// display gameover message
			if(!gameover && lives <= 0) {
				gameover = paused = true;
				char textPoints[512], textWaves[512];
				snprintf(textPoints, 512, "Score: %d", (int)score);
				snprintf(textWaves, 512,  "Wave:  %d", wave);
				const char *text[3] = {textWaves, textPoints, NULL};
				new question("GAME OVER", (char**)text, "Ok", NULL, tex::HAWHAW, &gameoverCallback);
			}
		}

		// convert relative position x to field position x
		static int pfx(double x) {
			if(x > 1.0 || x < margin) return -1;
			return (x-margin)/stx;
		}

		// convert relative position y to field position y
		static int pfy(double y) {
			if(y > 1.0 || y < -1.0) return -1;
			return (1.0-y)/sty;
		}

		// convert field position to relative position
		static vec *fp(int x, int y) {
			vec *v = new vec(-1.0,0);
			v->x = x*stx+margin + 0.5*stx;
			v->y = 1.0-y*sty -0.5*sty;
			return v;
		}

		// search and return spawn position
		static void setSpawnPos(int *px, int *py) {
			for(unsigned int x = 0; x < size; x++) {
				for(unsigned int y = 0; y < size; y++) {
					if(map[x][y].type == SPAWN) {
						*px = x; *py = y;
						return;
					}
				}
			}
			fprintf(stderr, "Error: map doesn't have a spawn (%d).\n", mapid);
			exit(1);
		}
};


// static definitions (grid class)
_cell **grid::map = NULL;
unsigned int grid::wave;
MAP grid::mapid;
bool grid::paused, grid::gameover;
unsigned int grid::size;
double grid::stx, grid::sty, grid::margin;
double grid::lives, grid::score, grid::money;
class wave *grid::wv = NULL;
