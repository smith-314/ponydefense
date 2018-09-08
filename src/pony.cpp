// pony.cpp
// pony class and related effects


// fade out a single texture
class fadeTex {
	private:
		int texID, ticks, ticksMax;
		double size; // texture size
		vec *pos; // texture position

	public:
		fadeTex(vec *_pos, int _texID, double _size, int _ticks) {
			texID = _texID; ticks = ticksMax = _ticks; size = _size; pos = _pos;
			draw::addRenderCallback(std::bind(&fadeTex::draw, this), (void*)&ticks, draw::HIGH);
		}

		~fadeTex() {
			draw::delRenderCallback((void*)&ticks);
			delete pos;
		}

		void draw() { 
			draw::tex(pos->clone(), size, texID, 0, 1.0*ticks/ticksMax);
			ticks--;
			if(ticks <= 0) delete this;
		}
};

// linked list of ponies to register for search by towers
class regPony *ponyList = NULL;
class regPony {
	public:
		int type;
		vec *pos;
		double prog; // walked fields
		bool ignore = false;
		class pony *py;
		class regPony *next = NULL;

		// register new pony
		regPony(int _type, vec *_pos, double _prog, class pony *_py) {
			type = _type; pos = _pos; prog = _prog; py = _py;
			// insert in linked list
			if(ponyList == NULL) ponyList = this;
			else {
				class regPony *reg = ponyList;
				while(reg->next != NULL) reg = reg->next;
				reg->next = this;
			}
		}

		// delete pony from list
		~regPony() {
			delete pos;
			if(ponyList == this) ponyList = this->next;
			else {
				class regPony *reg = ponyList;
				while(reg->next != this) reg = reg->next;
				reg->next = this->next;
			}
		}

		// refresh position and progress
		void refresh(vec *_pos, double _prog) {
			delete pos; pos = _pos;
			prog = _prog;
		}

		// search for next pony
		static class regPony *getClosest(vec *v, double dist) {
			if(ponyList == NULL) {
				delete v;
				return NULL;
			}
			double distBest;
			dist*=grid::sty;
			dist=dist*dist;
			class regPony *best = NULL, *ptr = ponyList;
			do {
				double tmp = vec::getDist(ptr->pos, v);
				if(tmp < dist && (best == NULL || tmp < distBest) && !ptr->ignore) {
					best = ptr; distBest = tmp;
				}
				ptr = ptr->next;
			} while(ptr != NULL);
			delete v;
			return best;
		}

		// search for first pony
		static class regPony *getFirst(vec *v, double dist) {
			if(ponyList == NULL) {
				delete v;
				return NULL;
			}
			dist*=grid::sty;
			dist=dist*dist;
			class regPony *best = NULL, *ptr = ponyList;
			do {
				double tmp = vec::getDist(ptr->pos, v);
				if(tmp <= dist && (best == NULL || ptr->prog >= best->prog) && !ptr->ignore)
					best = ptr;
				ptr = ptr->next;
			} while(ptr != NULL);
			delete v;
			return best;
		}

		// search for last pony
		static class regPony *getLast(vec *v, double dist) {
			if(ponyList == NULL) {
				delete v;
				return NULL;
			}
			dist*=grid::sty;
			dist=dist*dist;
			class regPony *best = NULL, *ptr = ponyList;
			do {
				double tmp = vec::getDist(ptr->pos, v);
				if(tmp <= dist && (best == NULL || ptr->prog <= best->prog) && !ptr->ignore)
					best = ptr;
				ptr = ptr->next;
			} while(ptr != NULL);
			delete v;
			return best;
		}

		// search for strongest pony
		static class regPony *getStrongest(vec *v, double dist) {
			if(ponyList == NULL) {
				delete v;
				return NULL;
			}
			int typeBest;
			dist*=grid::sty;
			dist=dist*dist;
			class regPony *best = NULL, *ptr = ponyList;
			do {
				double tmp = vec::getDist(ptr->pos, v);
				if(tmp <= dist && (best == NULL || typeBest <= ptr->type) && !ptr->ignore) {
					best = ptr; typeBest = ptr->type;
				}
				ptr = ptr->next;
			} while(ptr != NULL);
			delete v;
			return best;
		}
};

// pathfinding engine inherited by pony class
class pathfinding {
	private:
		int possibleMoves[4]; // possible directions

		// add direction to array
		void addDirectionToArray(int dir) {
			if(possibleMoves[0] == NONE) possibleMoves[0] = dir;
			else if(possibleMoves[1] == NONE) possibleMoves[1] = dir;
			else if(possibleMoves[2] == NONE) possibleMoves[2] = dir;
			else possibleMoves[3] = dir;
		}

		// check array and decide
		int evaluateArray() {
			if(possibleMoves[0] == NONE) return NONE;
			if(possibleMoves[1] == NONE) return possibleMoves[0];
			if(possibleMoves[2] == NONE) return possibleMoves[randi(0,1)];
			if(possibleMoves[3] == NONE) return possibleMoves[randi(0,2)];
			return possibleMoves[randi(0,3)];
		}

	public:
		// direction definitions
		enum {NONE, UP, DOWN, LEFT, RIGHT};

		// calculate next direction
		int getDirection(int x, int y, int lastDir) {
			// reset array and check possibilities 
			possibleMoves[0] = possibleMoves[1] = possibleMoves[2] = possibleMoves[3] = NONE;
			if(y < ((int)grid::size-1) && grid::map[x][y+1].type == WAY && lastDir != DOWN)
				addDirectionToArray(UP);
			if(y > 0 && grid::map[x][y-1].type == WAY && lastDir != UP)
				addDirectionToArray(DOWN);
			if(x < ((int)grid::size-1) && grid::map[x+1][y].type == WAY && lastDir != LEFT)
				addDirectionToArray(RIGHT);
			if(x > 0 && grid::map[x-1][y].type == WAY && lastDir != RIGHT)
				addDirectionToArray(LEFT);

			return evaluateArray();
		}
};

class pony: pathfinding {
	private:
		int x, y; // field position
		double finePos; // field internal position (0...1)
		int type, texID, dir, lastDir = NONE;
		int fields = 0; // walked fields
		double health, maxhealth, shield, maxshield, speed, slowdown = 1;
		int trojanSpawn = RARITY, trojanNum = 1, trojanRec = 0;
		class regPony *reg;
		double value, rot;
		bool mirr;

		// fix rotation and mirroring
		void setRotation() {
			if(finePos >= 0.5) {
				if(dir == LEFT) {
					mirr = true;
					rot = 0;
				}
				else if(dir == RIGHT) {
					mirr = false;
					rot = 0;
				}
				else if(dir == DOWN) {
					mirr = false;
					rot = -1.5;
				}
				else if(dir == UP) {
					mirr = false;
					rot = 1.5;
				}
			}
			else {
				if(lastDir == LEFT) mirr = true;
				else mirr = false;
			}
		}

		// move pony one step
		void move() {
			if(grid::paused) return;
			finePos+=speed*slowdown;
			setRotation();

			if(finePos > 1) { // change cell
				if(dir == RIGHT) x+=1;
				else if(dir == LEFT) x-=1;
				else if(dir == UP) y+=1;
				else y-=1;
				finePos = 0;
				lastDir = dir;
				dir = getDirection(x, y, lastDir);
				fields++;
				slowdown = 1;
				if(type == TROJAN) reg->ignore = true;
				// if pony made it to the end
				if(dir == NONE) {
					if(type == TROJAN) grid::lives-=trojanNum;
					else grid::lives--;

					delete this;
					return;
				}
			}
			reg->refresh(getPos(), fields+finePos);
		}

		// calculate relative pony position
		vec *getPos() {
			vec *v = grid::fp(x, y);
			if(finePos < 0.5) { // first walk to the center of the field
				if(lastDir == UP) v->y+=grid::sty*(0.5-finePos);
				else if(lastDir == DOWN) v->y+=grid::sty*(finePos-0.5);
				else if(lastDir == LEFT) v->x+=grid::stx*(0.5-finePos);
				else v->x+=grid::stx*(finePos-0.5);
			}
			else {
				if(dir == UP) v->y-=grid::sty*(finePos-0.5);
				else if(dir == DOWN) v->y+=grid::sty*(finePos-0.5);
				else if(dir == LEFT) v->x-=grid::stx*(finePos-0.5);
				else v->x+=grid::stx*(finePos-0.5);
			}
			return v;
		}

		// draw life and shield bars
		void drawBars() {
			const double barSizeX = 0.12;
			const double barSizeY = 0.02;
			vec *v = getPos();
			if(health/maxhealth < 1 || (shield > 0 && shield/maxshield < 1)) {
				vec *h1 = new vec(v->x-grid::stx*barSizeX, v->y+grid::sty*0.4);
				vec *h2 = new vec(v->x+grid::stx*barSizeX, v->y+grid::sty*(0.4+barSizeY));
				vec *h3 = new vec(v->x+grid::stx*barSizeX*(2*(health/maxhealth)-1), v->y+grid::sty*(0.4+barSizeY));
				draw::rect(h1->clone(), h2, new color(1,0,0));
				draw::rect(h1, h3, new color(0,1,0));
			}

			if(shield > 0 && shield/maxshield < 1) {
				vec *h1 = new vec(v->x-grid::stx*barSizeX, v->y+grid::sty*0.4);
				vec *h2 = new vec(v->x+grid::stx*barSizeX*(2*shield/maxshield-1), v->y+grid::sty*(0.4+barSizeY));
				draw::rect(h1, h2, new color(0.2,0.2,1));
			}
			delete v;
		}

		// initialize pony properties
		void setPony(int _type, double _health, double _shield, double _speed) {
			type = _type;
			switch(type) {
				case RARITY: texID = tex::PONY_RARITY; break;
				case FLUTTERSHY: texID = tex::PONY_FLUTTERSHY; break;
				case PINKIE_PIE: texID = tex::PONY_PINKIE_PIE; break;
				case TWILIGHT_SPARKLE: texID = tex::PONY_TWILIGHT_SPARKLE; break;
				case RARITY_SHIELD: texID = tex::PONY_RARITY; break;
				case RAINBOW_DASH: texID = tex::PONY_RAINBOW_DASH; break;
				case APPLEJACK: texID = tex::PONY_APPLEJACK; break;
				case TWILIGHT_SPARKLE_SHIELD: texID = tex::PONY_TWILIGHT_SPARKLE; break;
				case TROJAN: texID = tex::PONY_TROJAN; break;
			}
			health = maxhealth = _health;
			shield = maxshield = _shield;
			speed = _speed;
		}

	public:
		// create and register pony
		pony(int _type, int _x = -1, int _y = -1, double _finePos = 0.5, int _dir = NONE, int _fields = 0) {
			// set field position
			if(_x == -1 || _y == -1) grid::setSpawnPos(&x, &y);
			else {
				x = _x; y = _y;
			}
			finePos = _finePos;

			// set direction
			if(_dir == NONE) dir = getDirection(x, y, NONE);
			else dir = _dir;
			setRotation();

			// set pony properties
			if(_type == RARITY)
				setPony(RARITY, 100, 0, 0.01);
			else if(_type == FLUTTERSHY)
				setPony(FLUTTERSHY, 100, 0, 0.02);
			else if(_type == PINKIE_PIE)
				setPony(PINKIE_PIE, 400, 0, 0.01);
			else if(_type == TWILIGHT_SPARKLE)
				setPony(TWILIGHT_SPARKLE, 400, 0, 0.02);
			else if(_type == RARITY_SHIELD)
				setPony(RARITY_SHIELD, 533, 1067, 0.01);
			else if(_type == RAINBOW_DASH)
				setPony(RAINBOW_DASH, 1280, 0, 0.025);
			else if(_type == APPLEJACK)
				setPony(APPLEJACK, 6400, 0, 0.01);
			else if(_type == TWILIGHT_SPARKLE_SHIELD)
				setPony(TWILIGHT_SPARKLE_SHIELD, 2133, 4267, 0.02);
			else if(_type == TROJAN) {
				if(grid::wave < 40) setPony(TROJAN, 400, 0, 0.01);
				else setPony(TROJAN, 2000*exp((grid::wave-40)*0.25), 0, 0.01); 
			}
			else {
				fprintf(stderr, "Error: unknown pony (%d).\n", _type);
				exit(1);
			}

			// register pony
			fields = _fields;
			reg = new regPony(type, getPos(), fields+finePos, this);
			if(type == TROJAN) reg->ignore = true;


			// set callback
			draw::addRenderCallback(std::bind(&pony::draw, this), (void*)&finePos);

			// set value ( = points & money)
			if(maxshield > 0) value = (maxhealth+maxshield)*speed;
			else value = maxhealth*speed;
		}

		// delete callback
		~pony() {
			draw::delRenderCallback((void*)&finePos);
			delete reg;
		}

		// render callback
		void draw() { 
			draw::tex(getPos(), 0.8*grid::sty, texID, rot, 1, mirr);
			if(shield > 0) draw::tex(getPos(), 0.8*grid::sty, tex::PONY_SHIELD);
			drawBars();
			move();
		}

		// manage damage
		void addDamage(double dmg, int dmgtype) {
			if(shield > 0) { // first damage shield
				if(dmgtype == DMGAIR) { // half damage if ground pony
					if(shield < dmg/2) {
						health-=(dmg-2*shield);
						shield = 0;
					}
					else shield-=dmg/2;
				}
				else shield-=dmg;
			}
			else {
				// half damage if pegasus pony
				if(type == DMGGROUND && (type == FLUTTERSHY || type == RAINBOW_DASH || type == TWILIGHT_SPARKLE || type == TWILIGHT_SPARKLE_SHIELD))
					health-=dmg/2;
				else health-=dmg;
			}

			// add points and money if pony died
			if(health <= 0) {
				if(type == TROJAN) {
					for(int i = 0; i < trojanNum; i++) {
						class pony *p = new pony(trojanSpawn, x, y, 0.5+(0.4/trojanNum)*i, dir, reg->prog);
						if(trojanRec > 0) {
							p->setTrojanSpawn(TROJAN, 2, trojanRec-1);
						}
						else p->setTrojanSpawn(randi(RAINBOW_DASH,TWILIGHT_SPARKLE_SHIELD), randi(1,5));
					}
				}
				else {
					new fadeTex(getPos(), tex::PONY_DEAD, 0.8*grid::sty, 120);
					stats::addPoints(grid::mapid, value);
					grid::money += value*13*exp(-0.039*grid::wave);
				}
				delete this;
			}
		}

		int hasShield() {
			if(shield <= 0) return 0;
			else return 1;
		}

		int getType() {
			return type;
		}

		// fence/drone slowdown 
		void setSlowdown(double sd) {
			slowdown = sd;
		}

		// check if pony is in specified field
		bool isInField(int _x, int _y) {
			if(_x == x && _y == y) return true;
			return false;
		}

		// spawn settings for trojan pony
		void setTrojanSpawn(int ponyID, int num, int rec = 0) {
			trojanSpawn = ponyID;
			trojanNum = num;
			trojanRec = rec;
		}
};
