// tower.cpp
// tower classes 


class tower {
	private:
		TOWERTYPE type;
		UPGRTYPE upgrade;
		int x, y; // field position
		double rotation = 0, health = 1, range, explRange; // for flamethr./artillery
		int cooldown = 0, cooldownMax, strategy = FIRST;
		int texIDBase, texID;
		int texIDFire[30];
		double damage, damageType;
		int rotSteps, rotDir, rotDelay; // for inactivity movement


		// set tower properties depending on type/upgrade
		void setTower() {
			// prevent upgrade flicker
			for(int i = 0; i < 30; i++) texIDFire[i] = 0;
			cooldown = 0;

			if(type == RIFLE) {
				if(upgrade == BASE) {
					cooldownMax = 16;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_RIFLE_1;
					texIDFire[0] = tex::TOWER_RIFLE_1_FIRE_1;
					texIDFire[1] = tex::TOWER_RIFLE_1_FIRE_1;
					texIDFire[2] = tex::TOWER_RIFLE_1_FIRE_1;
					texIDFire[3] = tex::TOWER_RIFLE_1_FIRE_2;
					texIDFire[4] = tex::TOWER_RIFLE_1_FIRE_2;
					texIDFire[5] = tex::TOWER_RIFLE_1_FIRE_2;
					texIDFire[6] = tex::TOWER_RIFLE_1_FIRE_3;
					texIDFire[7] = tex::TOWER_RIFLE_1_FIRE_3;
					texIDFire[8] = tex::TOWER_RIFLE_1_FIRE_3;
					range = 2;
					damage = 8;
					damageType = DMGGROUND;
					return;
				}
				if(upgrade == UPGRADE1) {
					cooldownMax = 10;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_RIFLE_2;
					texIDFire[0] = tex::TOWER_RIFLE_2_FIRE_1;
					texIDFire[1] = tex::TOWER_RIFLE_2_FIRE_1;
					texIDFire[2] = tex::TOWER_RIFLE_2_FIRE_1;
					texIDFire[3] = tex::TOWER_RIFLE_2_FIRE_2;
					texIDFire[4] = tex::TOWER_RIFLE_2_FIRE_2;
					texIDFire[5] = tex::TOWER_RIFLE_2_FIRE_2;
					texIDFire[6] = tex::TOWER_RIFLE_2_FIRE_3;
					texIDFire[7] = tex::TOWER_RIFLE_2_FIRE_3;
					texIDFire[8] = tex::TOWER_RIFLE_2_FIRE_3;
					range = 2.25;
					damage = 10;
					damageType = DMGGROUND;
					return;
				}
				if(upgrade == UPGRADE2) {
					cooldownMax = 8;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_RIFLE_3;
					texIDFire[0] = tex::TOWER_RIFLE_3_FIRE_1;
					texIDFire[1] = tex::TOWER_RIFLE_3_FIRE_1;
					texIDFire[2] = tex::TOWER_RIFLE_3_FIRE_2;
					texIDFire[3] = tex::TOWER_RIFLE_3_FIRE_2;
					texIDFire[4] = tex::TOWER_RIFLE_3_FIRE_3;
					range = 2.5;
					damage = 14;
					damageType = DMGGROUND;
					return;
				}
			}
			if(type == RAILGUN) {
				if(upgrade == BASE) {
					cooldownMax = 40;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_RAILGUN_1;
					texIDFire[0] = texIDFire[1] = tex::TOWER_RAILGUN_1_FIRE_1;
					texIDFire[2] = texIDFire[3] = tex::TOWER_RAILGUN_1_FIRE_2;
					texIDFire[4] = texIDFire[5] = tex::TOWER_RAILGUN_1_FIRE_3;
					texIDFire[6] = texIDFire[7] = tex::TOWER_RAILGUN_1_FIRE_4;
					texIDFire[8] = texIDFire[9] = tex::TOWER_RAILGUN_1_FIRE_5;
					texIDFire[10] = texIDFire[11] = tex::TOWER_RAILGUN_1_FIRE_6;
					texIDFire[12] = texIDFire[13] = tex::TOWER_RAILGUN_1_FIRE_7;
					texIDFire[14] = texIDFire[15] = tex::TOWER_RAILGUN_1_FIRE_8;
					texIDFire[16] = texIDFire[17] = tex::TOWER_RAILGUN_1_FIRE_9;
					texIDFire[18] = texIDFire[19] = tex::TOWER_RAILGUN_1_FIRE_10;
					texIDFire[20] = texIDFire[21] = tex::TOWER_RAILGUN_1_FIRE_11;
					texIDFire[22] = texIDFire[23] = tex::TOWER_RAILGUN_1_FIRE_12;
					texIDFire[24] = texIDFire[25] = tex::TOWER_RAILGUN_1_FIRE_13;
					texIDFire[26] = texIDFire[27] = tex::TOWER_RAILGUN_1_FIRE_14;
					range = 3.1;
					damage = 40;
					damageType = DMGAIR;
					return;
				}
				if(upgrade == UPGRADE1) {
					cooldownMax = 40;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_RAILGUN_2;
					texIDFire[0] = texIDFire[1] = tex::TOWER_RAILGUN_2_FIRE_1;
					texIDFire[2] = texIDFire[3] = tex::TOWER_RAILGUN_2_FIRE_2;
					texIDFire[4] = texIDFire[5] = tex::TOWER_RAILGUN_2_FIRE_3;
					texIDFire[6] = texIDFire[7] = tex::TOWER_RAILGUN_2_FIRE_4;
					texIDFire[8] = texIDFire[9] = tex::TOWER_RAILGUN_2_FIRE_5;
					texIDFire[10] = texIDFire[11] = tex::TOWER_RAILGUN_2_FIRE_6;
					texIDFire[12] = texIDFire[13] = tex::TOWER_RAILGUN_2_FIRE_7;
					texIDFire[14] = texIDFire[15] = tex::TOWER_RAILGUN_2_FIRE_8;
					texIDFire[16] = texIDFire[17] = tex::TOWER_RAILGUN_2_FIRE_9;
					texIDFire[18] = texIDFire[19] = tex::TOWER_RAILGUN_2_FIRE_10;
					texIDFire[20] = texIDFire[21] = tex::TOWER_RAILGUN_2_FIRE_11;
					texIDFire[22] = texIDFire[23] = tex::TOWER_RAILGUN_2_FIRE_12;
					texIDFire[24] = texIDFire[25] = tex::TOWER_RAILGUN_2_FIRE_13;
					texIDFire[26] = texIDFire[27] = tex::TOWER_RAILGUN_2_FIRE_14;
					texIDFire[28] = texIDFire[29] = tex::TOWER_RAILGUN_2_FIRE_15;
					range = 3.1;
					damage = 90;
					damageType = DMGAIR;
					return;
				}
				if(upgrade == UPGRADE2) {
					cooldownMax = 35;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_RAILGUN_3;
					texIDFire[0] = texIDFire[1] = tex::TOWER_RAILGUN_3_FIRE_1;
					texIDFire[2] = texIDFire[3] = tex::TOWER_RAILGUN_3_FIRE_2;
					texIDFire[4] = texIDFire[5] = tex::TOWER_RAILGUN_3_FIRE_3;
					texIDFire[6] = texIDFire[7] = tex::TOWER_RAILGUN_3_FIRE_4;
					texIDFire[8] = texIDFire[9] = tex::TOWER_RAILGUN_3_FIRE_5;
					texIDFire[10] = texIDFire[11] = tex::TOWER_RAILGUN_3_FIRE_6;
					texIDFire[12] = texIDFire[13] = tex::TOWER_RAILGUN_3_FIRE_7;
					texIDFire[14] = texIDFire[15] = tex::TOWER_RAILGUN_3_FIRE_8;
					texIDFire[16] = texIDFire[17] = tex::TOWER_RAILGUN_3_FIRE_9;
					texIDFire[18] = texIDFire[19] = tex::TOWER_RAILGUN_3_FIRE_10;
					texIDFire[20] = texIDFire[21] = tex::TOWER_RAILGUN_3_FIRE_11;
					texIDFire[22] = texIDFire[23] = tex::TOWER_RAILGUN_3_FIRE_12;
					texIDFire[24] = texIDFire[25] = tex::TOWER_RAILGUN_3_FIRE_13;
					texIDFire[26] = texIDFire[27] = tex::TOWER_RAILGUN_3_FIRE_14;
					texIDFire[28] = texIDFire[29] = tex::TOWER_RAILGUN_3_FIRE_15;
					range = 3.2;
					damage = 138;
					damageType = DMGAIR;
					return;
				}
			}
			if(type == FLAMETHROWER) {
				if(upgrade == BASE) {
					cooldownMax = 5;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_FLAMETHROWER_1;
					texIDFire[0] = tex::TOWER_FLAMETHROWER_1;
					range = 2.5;
					explRange = 0.5;
					damage = 3;
					damageType = DMGGROUND;
					return;
				}
				if(upgrade == UPGRADE1) {
					cooldownMax = 5;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_FLAMETHROWER_2;
					texIDFire[0] = tex::TOWER_FLAMETHROWER_2;
					range = 2.5;
					explRange = 0.5;
					damage = 8;
					damageType = DMGGROUND;
					return;
				}
				if(upgrade == UPGRADE2) {
					cooldownMax = 5;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_FLAMETHROWER_3;
					texIDFire[0] = tex::TOWER_FLAMETHROWER_3;
					range = 2.5;
					explRange = 0.65;
					damage = 10;
					damageType = DMGGROUND;
					return;
				}
			}
			if(type == LASER) {
				if(upgrade == BASE) {
					cooldownMax = 1;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_LASER_1;
					texIDFire[0] = tex::TOWER_LASER_1;
					range = 3;
					damage = 1.25;
					damageType = DMGAIR;
					return;
				}
				if(upgrade == UPGRADE1) {
					cooldownMax = 1;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_LASER_2;
					texIDFire[0] = tex::TOWER_LASER_2;
					range = 3;
					damage = 3.4;
					damageType = DMGAIR;
					return;
				}
				if(upgrade == UPGRADE2) {
					cooldownMax = 1;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_LASER_3;
					texIDFire[0] = tex::TOWER_LASER_3;
					range = 3;
					damage = 6.2;
					damageType = DMGAIR;
					return;
				}
			}
			if(type == ARTILLERY) {
				if(upgrade == BASE) {
					cooldownMax = 60;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_ARTILLERY_1;
					texIDFire[0] = texIDFire[1] = tex::TOWER_ARTILLERY_1_FIRE_1;
					texIDFire[2] = texIDFire[3] = tex::TOWER_ARTILLERY_1_FIRE_2;
					texIDFire[4] = texIDFire[5] = tex::TOWER_ARTILLERY_1_FIRE_3;
					texIDFire[6] = texIDFire[7] = tex::TOWER_ARTILLERY_1_FIRE_4;
					texIDFire[8] = texIDFire[9] = tex::TOWER_ARTILLERY_1_FIRE_5;
					texIDFire[10] = texIDFire[11] = tex::TOWER_ARTILLERY_1_FIRE_6;
					texIDFire[12] = texIDFire[13] = tex::TOWER_ARTILLERY_1_FIRE_7;
					texIDFire[14] = texIDFire[15] = tex::TOWER_ARTILLERY_1_FIRE_8;
					texIDFire[16] = texIDFire[17] = tex::TOWER_ARTILLERY_1_FIRE_9;
					texIDFire[18] = texIDFire[19] = tex::TOWER_ARTILLERY_1_FIRE_10;
					texIDFire[20] = texIDFire[21] = tex::TOWER_ARTILLERY_1_FIRE_11;
					texIDFire[22] = texIDFire[23] = tex::TOWER_ARTILLERY_1_FIRE_12;
					texIDFire[24] = texIDFire[25] = tex::TOWER_ARTILLERY_1_FIRE_13;
					texIDFire[26] = texIDFire[27] = tex::TOWER_ARTILLERY_1_FIRE_14;
					texIDFire[28] = texIDFire[29] = tex::TOWER_ARTILLERY_1_FIRE_15;
					range = 4.5;
					damage = 30;
					explRange = 0.65;
					damageType = DMGGROUND;
					return;
				}
				if(upgrade == UPGRADE1) {
					cooldownMax = 50;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_ARTILLERY_2;
					texIDFire[0] = texIDFire[1] = tex::TOWER_ARTILLERY_2_FIRE_1;
					texIDFire[2] = texIDFire[3] = tex::TOWER_ARTILLERY_2_FIRE_2;
					texIDFire[4] = texIDFire[5] = tex::TOWER_ARTILLERY_2_FIRE_3;
					texIDFire[6] = texIDFire[7] = tex::TOWER_ARTILLERY_2_FIRE_4;
					texIDFire[8] = texIDFire[9] = tex::TOWER_ARTILLERY_2_FIRE_5;
					texIDFire[10] = texIDFire[11] = tex::TOWER_ARTILLERY_2_FIRE_6;
					texIDFire[12] = texIDFire[13] = tex::TOWER_ARTILLERY_2_FIRE_7;
					texIDFire[14] = texIDFire[15] = tex::TOWER_ARTILLERY_2_FIRE_8;
					texIDFire[16] = texIDFire[17] = tex::TOWER_ARTILLERY_2_FIRE_9;
					texIDFire[18] = texIDFire[19] = tex::TOWER_ARTILLERY_2_FIRE_10;
					texIDFire[20] = texIDFire[21] = tex::TOWER_ARTILLERY_2_FIRE_11;
					texIDFire[22] = texIDFire[23] = tex::TOWER_ARTILLERY_2_FIRE_12;
					texIDFire[24] = texIDFire[25] = tex::TOWER_ARTILLERY_2_FIRE_13;
					texIDFire[26] = texIDFire[27] = tex::TOWER_ARTILLERY_2_FIRE_14;
					texIDFire[28] = texIDFire[29] = tex::TOWER_ARTILLERY_2_FIRE_15;
					range = 4.5;
					damage = 60;
					explRange = 0.75;
					damageType = DMGGROUND;
					return;
				}
				if(upgrade == UPGRADE2) {
					cooldownMax = 50;
					texIDBase = tex::TOWER_BASE;
					texID = tex::TOWER_ARTILLERY_3;
					texIDFire[0] = texIDFire[1] = tex::TOWER_ARTILLERY_3_FIRE_1;
					texIDFire[2] = texIDFire[3] = tex::TOWER_ARTILLERY_3_FIRE_2;
					texIDFire[4] = texIDFire[5] = tex::TOWER_ARTILLERY_3_FIRE_3;
					texIDFire[6] = texIDFire[7] = tex::TOWER_ARTILLERY_3_FIRE_4;
					texIDFire[8] = texIDFire[9] = tex::TOWER_ARTILLERY_3_FIRE_5;
					texIDFire[10] = texIDFire[11] = tex::TOWER_ARTILLERY_3_FIRE_6;
					texIDFire[12] = texIDFire[13] = tex::TOWER_ARTILLERY_3_FIRE_7;
					texIDFire[14] = texIDFire[15] = tex::TOWER_ARTILLERY_3_FIRE_8;
					texIDFire[16] = texIDFire[17] = tex::TOWER_ARTILLERY_3_FIRE_9;
					texIDFire[18] = texIDFire[19] = tex::TOWER_ARTILLERY_3_FIRE_10;
					texIDFire[20] = texIDFire[21] = tex::TOWER_ARTILLERY_3_FIRE_11;
					texIDFire[22] = texIDFire[23] = tex::TOWER_ARTILLERY_3_FIRE_12;
					texIDFire[24] = texIDFire[25] = tex::TOWER_ARTILLERY_3_FIRE_13;
					texIDFire[26] = texIDFire[27] = tex::TOWER_ARTILLERY_3_FIRE_14;
					texIDFire[28] = texIDFire[29] = tex::TOWER_ARTILLERY_3_FIRE_15;
					range = 4.5;
					damage = 90;
					explRange = 0.85;
					damageType = DMGGROUND;
					return;
				}
			}
			if(type == SHIELD) {
				texIDBase = tex::TOWER_BASE;
				texID = tex::TOWER_SHIELD;
				if(upgrade == BASE) range = 3.25;
				if(upgrade == UPGRADE1) range = 5.5;
			}
			else {
				fprintf(stderr, "Error: unknown tower or upgrade (%d, %d).\n", type, upgrade);
				exit(1);
			}
		}

		// return tower position
		vec *getPos() {
			return grid::fp(x, y);
		}

		// return angle between tower and pony
		double getAngle(class regPony *target) {
			vec *pos = getPos();
			vec *ang = new vec(target->pos->x-pos->x, target->pos->y-pos->y);
			vec *vgl = new vec(0,1);
			double rotation = vec::getAngle(ang, vgl);
			if(target->pos->x - pos->x <= 0) rotation = -rotation;
			delete pos; delete ang; delete vgl;
			return rotation;
		}

		// do damage to multiple ponies
		void areaDamage(vec *v, double dist, double damage, int damageType) {
			if(ponyList == NULL) return;
			dist*=grid::sty;
			dist=dist*dist;
			class regPony *ptr = ponyList;
			do {
				double tmp = vec::getDist(ptr->pos, v);
				if(tmp <= dist)
					ptr->py->addDamage(damage, damageType);
				ptr = ptr->next;
			} while(ptr != NULL);
		}

		// draw tower health
		void drawBars() {
			const double barSizeX = 0.12;
			const double barSizeY = 0.02;
			vec *v = getPos();
			if(health < 1) {
				vec *h1 = new vec(v->x-grid::stx*barSizeX, v->y+grid::sty*0.4);
				vec *h2 = new vec(v->x+grid::stx*barSizeX, v->y+grid::sty*(0.4+barSizeY));
				vec *h3 = new vec(v->x+grid::stx*barSizeX*(2*health-1), v->y+grid::sty*(0.4+barSizeY));
				draw::rect(h1->clone(), h2, new color(1,0,0));
				draw::rect(h1, h3, new color(0,1,0));
			}
			delete v;
		}

	public:
		// initialize tower
		tower(int _x, int _y, TOWERTYPE _type) {
			if(grid::map[_x][_y].tw != NULL) {
				fprintf(stderr, "Error: there already is a tower (%d,%d).\n", _x, _y);
				exit(1);
			}
			if(grid::map[_x][_y].type != NONE) {
				fprintf(stderr, "Error: field is blocked (%d,%d).\n", _x, _y);
				exit(1);
			}
			grid::map[_x][_y].tw = this;
			type = _type; x = _x; y = _y;
			upgrade = BASE;
			setTower();
			grid::money -= getPrice(type, upgrade);

			// initialize random movement
			rotDelay = randi(60, 100);
			rotSteps = randi(40, 120);
			rotDir = randi(0,1);

			// set callback
			if(type == SHIELD)
				draw::addRenderCallback(std::bind(&tower::draw, this), (void*)&type, draw::LOW);
			else
				draw::addRenderCallback(std::bind(&tower::draw, this), (void*)&type, draw::DEFAULT);
		}

		~tower() {
			draw::delRenderCallback((void*)&type);
			grid::map[x][y].tw = NULL;
		}

		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: tower()\n");

			// special case: shield
			// => only draw texture
			if(type == SHIELD) {
				draw::tex(getPos(), grid::sty, texID);
				draw::tex(getPos(), grid::sty*range, tex::SHIELD, rotation);
				return;
			}
			// draw tower base
			draw::tex(getPos(), grid::sty, texIDBase);
			if(!grid::paused) {
				// search target
				class regPony *target;
				if(strategy == CLOSEST) target = regPony::getClosest(getPos(), range);
				else if(strategy == FIRST) target = regPony::getFirst(getPos(), range);
				else if(strategy == LAST) target = regPony::getLast(getPos(), range);
				else target = regPony::getStrongest(getPos(), range);

				// if there is a pony in range
				if(target != NULL) {
					// set angle
					rotation = getAngle(target);
					// shoot if possible
					if(cooldown <= 0) {
						cooldown = cooldownMax;
						new tracer(type, upgrade, getPos(), target->pos->clone());
						if(type == ARTILLERY) {
							new explosion(target->pos->clone(), 2*explRange*grid::sty, 0.05);
							areaDamage(target->pos, explRange, damage, damageType);
						}
						else if(type == FLAMETHROWER) {
							new flames(getPos(), target->pos->clone());
							areaDamage(target->pos, explRange, damage, damageType);
						}
						else {
							if(type == RIFLE || type == RAILGUN) new blood(target, rotation, type);
							target->py->addDamage(damage, damageType);
						}
					}
					rotDelay = 200;
				}

				// inactivity movement
				else {
					if(rotDelay > 0) {
						rotDelay--;
					}
					else {
						if(rotSteps > 0) {
							if(rotDir) rotation+=0.01;
							else rotation-=0.01;
							rotSteps--;
						}
						else {
							rotDelay = randi(60, 250);
							rotSteps = randi(60, 150);
							rotDir = randi(0,1);
						}
					}
				}
			}

			// draw tower
			int fireID = -cooldown+cooldownMax; // iterate through fire array
			if(fireID < 30 && texIDFire[fireID] != 0) {
				draw::tex(getPos(), grid::sty, texIDFire[fireID], rotation);
			}
			else draw::tex(getPos(), grid::sty, texID, rotation);
			drawBars();
			if(!grid::paused) {
				cooldown--;
			}

			// show range if selected
			if(x == xF && y == yF) {
				draw::circle(getPos(), range*grid::sty, new color(1,0.5,0,0.4));
			}

			if(health <= 0) delete this;
		}

		double getHealth() {
			return health;
		}

		void setHealth(double h) {
			health = h;
		}

		int getStrategy() {
			return strategy;
		}

		int getType() {
			return type;
		}

		int getUpgrade() {
			return upgrade;
		}

		void setStrategy(int str) {
			strategy = str;
		}

		void doUpgrade() {
			if(upgrade == BASE) upgrade = UPGRADE1;
			else if(upgrade == UPGRADE1) upgrade = UPGRADE2;
			setTower();
			grid::money -= getPrice(type, upgrade);
			health = 1;
		}

		bool hasUpgrade() {
			if(type == SHIELD && upgrade == UPGRADE1)
				return false;
			if(upgrade == UPGRADE2) return false;
			return true;
		}

		void doRecover() {
			grid::map[x][y].tw = NULL;
			grid::money += getRecoverMoney();
			delete this;
		}

		double getRecoverMoney() {
			double price = getPrice(type, BASE);
			if(upgrade != BASE) price += getPrice(type, UPGRADE1);
			if(upgrade == UPGRADE2) price += getPrice(type, UPGRADE2);
			return price*0.7;
		}

		bool isShield() {
			if(type == SHIELD) return true;
			return false;
		}

		bool isDamaged() {
			if(health < 1) return true;
			else return false;
		}

		double getRepairPrice() {
			double price = getPrice(type, BASE);
			if(upgrade != BASE) price += getPrice(type, UPGRADE1);
			if(upgrade == UPGRADE2) price += getPrice(type, UPGRADE2);
			return (1-health)*price*0.6;
		}

		void doRepair() {
			grid::money -= getRepairPrice();
			health = 1;
		}

		// add damage from airstrike
		void doDamage(double dmg) {
			// search for shield
			for(int i = -2; i < 3; i++) {
				for(int z = -2; z < 3; z++) {
					int fx = x + i;
					int fy = y + z;
					if(fx >= 0 && fx < (int)grid::size && fy >= 0 && fy < (int)grid::size) {
						if(grid::map[fx][fy].type == NONE && grid::map[fx][fy].tw != NULL && grid::map[fx][fy].tw->isShield()) {
							if((i == -2 || i == 2) && (z == -2 || z == 2)) continue;
							if(grid::map[fx][fy].tw->getUpgrade() == UPGRADE1) return;
							if(i > -2 && i < 2 && z > -2 && z < 2) return;
						}
					}
				}
			}
			health -= dmg;
		}

		static double getPrice(int type, int upgrade) {
			if(type == RIFLE) {
				if(upgrade == BASE) return 250;
				else if(upgrade == UPGRADE1) return 300;
				else if(upgrade == UPGRADE2) return 500;
			}
			else if(type == RAILGUN) {
				if(upgrade == BASE) return 750;
				else if(upgrade == UPGRADE1) return 900;
				else if(upgrade == UPGRADE2) return 1400;
			}
			else if(type == FLAMETHROWER) {
				if(upgrade == BASE) return 550;
				else if(upgrade == UPGRADE1) return 800;
				else if(upgrade == UPGRADE2) return 1200;
			}
			else if(type == LASER) {
				if(upgrade == BASE) return 900;
				else if(upgrade == UPGRADE1) return 1600;
				else if(upgrade == UPGRADE2) return 2200;
			}
			else if(type == ARTILLERY) {
				if(upgrade == BASE) return 800;
				else if(upgrade == UPGRADE1) return 1400;
				else if(upgrade == UPGRADE2) return 2000;
			}
			else if(type == SHIELD) {
				if(upgrade == BASE) return 2250;
				else if(upgrade == UPGRADE1) return 3500;
			}
			else {
				fprintf(stderr, "Error: unknown tower or upgrade (%d,%d).\n", type, upgrade);
				exit(1);
			}
			return 0;
		}
};

// support tower on "WAY" cell
class supTower {
	private:
		int type, upgrade, x, y;
		double rotation = 0, health = 1, range;
		int cooldown = 0, cooldownMax, texID;

		// check if cell is WAY or SPAWN
		bool checkCell(int _x, int _y) {
			if(_x < 0 || _y < 0 || _x > grid::size-1 || _y > grid::size-1)
				return false;
			if(grid::map[_x][_y].type == WAY) return true;
			if(grid::map[_x][_y].type == SPAWN) return true;
			return false;
		}

		// check if surrounding fits (FENCE orientation)
		bool checkSurr(bool up, bool down, bool left, bool right) {
			if(up != checkCell(x, y+1)) return false;
			if(down != checkCell(x, y-1)) return false;
			if(left != checkCell(x-1, y)) return false;
			if(right != checkCell(x+1, y)) return false;
			return true;
		}

		void setTower() {
			if(type == FENCE) {
				cooldownMax = 3;

				// fix fence orientation
				if(checkSurr(1,1,1,1)) texID = tex::TOWER_FENCE_JUNCTION; // full junction
				else if(checkSurr(0,0,1,1)) texID = tex::TOWER_FENCE; // horizontal
				else if(checkSurr(1,1,0,0)) { // vertical
					texID = tex::TOWER_FENCE;
					rotation = M_PI/2;
				}

				// four types of corners
				else if(checkSurr(1,0,1,0)) {
					texID = tex::TOWER_FENCE_CORN;
					rotation = -M_PI/2;
				}
				else if(checkSurr(1,0,0,1)) {
					texID = tex::TOWER_FENCE_CORN;
					rotation = M_PI;
				}
				else if(checkSurr(0,1,0,1)) {
					texID = tex::TOWER_FENCE_CORN;
					rotation = M_PI/2;
				}
				else if(checkSurr(0,1,1,0)) texID = tex::TOWER_FENCE_CORN;

				// three branches
				else if(checkSurr(1,0,1,1)) {
					texID = tex::TOWER_FENCE_HALF;
					rotation = M_PI;
				}
				else if(checkSurr(0,1,1,1)) {
					texID = tex::TOWER_FENCE_HALF;
					rotation = 0;
				}
				else if(checkSurr(1,1,1,0)) {
					texID = tex::TOWER_FENCE_HALF;
					rotation = -M_PI/2;
				}
				else if(checkSurr(1,1,0,1)) {
					texID = tex::TOWER_FENCE_HALF;
					rotation = M_PI/2;
				}
				
				// horizontal and vertical end
				else if(checkSurr(1,0,0,0) || checkSurr(0,1,0,0)) {
					texID = tex::TOWER_FENCE;
					rotation = M_PI/2;
				}
				else if(checkSurr(0,0,1,0) || checkSurr(0,0,0,1)) {
					texID = tex::TOWER_FENCE;
				}

				// should not happen
				else texID = tex::TOWER_FENCE_JUNCTION;
			}
			else if(type == DRONE) {
				texID = tex::TOWER_DRONE_ATTACK;
				cooldownMax = 3;
			}
			else {
				cooldownMax = 300;
				texID = tex::TOWER_DRONE;
			}
		}

		vec *getPos() {
			return grid::fp(x, y);
		}

		// lightning effect (fence)
		void genLightning(vec *pos) {
			vec *cell = grid::fp(x, y);
			vec *start = new vec();
			start->x = cell->x + randd(-grid::stx*0.45, grid::stx*0.45);
			if(rand()%2) start->y = cell->y + 0.45*grid::sty;
			else start->y = cell->y - 0.45*grid::sty;
			vec *mid = new vec(cell->x + randd(-grid::stx*0.3, grid::stx*0.3), cell->y + randd(-grid::sty*0.3, grid::sty*0.3));
			new tracer(LIGHTNING, BASE, start, mid->clone());
			new tracer(LIGHTNING, BASE, mid, pos->clone());
			delete cell;
		}

		// lightning effect (drone)
		void genLightning(vec *start, vec *pos) {
			vec *cell = grid::fp(x, y);
			vec *mid = new vec(cell->x + randd(-grid::stx*0.3, grid::stx*0.3), cell->y + randd(-grid::sty*0.3, grid::sty*0.3));
			new tracer(LIGHTNING, BASE, start->clone(), mid->clone());
			new tracer(LIGHTNING, BASE, mid, pos->clone());
			delete cell;
		}

		bool isPegasus(int type) {
			if(type == RARITY || type == PINKIE_PIE ||
					type == RARITY_SHIELD || type == APPLEJACK || type == TROJAN) return false;
			return true;
		}

	public:
		supTower(int _x, int _y, int _type) {
			if(grid::map[_x][_y].ts != NULL) {
				fprintf(stderr, "Error: there already is a tower (%d,%d).\n", _x, _y);
				exit(1);
			}
			if(grid::map[_x][_y].type != WAY) {
				fprintf(stderr, "Error: wrong field type for support Tower (%d,%d).\n", _x, _y);
				exit(1);
			}
			grid::map[_x][_y].ts = this;
			type = _type; x = _x; y = _y;
			upgrade = BASE;
			setTower();
			grid::money-=getPrice(type, upgrade);

			draw::addRenderCallback(std::bind(&supTower::draw, this), (void*)&type, draw::LOW);
		}

		~supTower() {
			draw::delRenderCallback((void*)&type);
		}

		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: suptower()\n");

			// iterate ponies and set slowdown
			class regPony *ptr = ponyList;
			while(ptr != NULL) {
				if(ptr->py->isInField(x,y)) {
					bool pegasus = isPegasus(ptr->py->getType()); // split ponytypes
					if(type == FENCE && !pegasus) { // if fence and ground pony
						ptr->py->setSlowdown(0.5);
						if(cooldown <= 0) {
							cooldown = cooldownMax;
							genLightning(ptr->pos);
						}
					}
					else if(type == DRONE && pegasus && !ptr->ignore) { // if drone and pegasus pony
						ptr->py->setSlowdown(0.5);
						if(cooldown <= 0) {
							cooldown = cooldownMax;
							vec *v = getPos();
							v->x += sin(rotation)*grid::stx*0.5;
							v->y += cos(rotation)*grid::sty*0.5;
							genLightning(v, ptr->pos);
							delete v;
						}
					}
					else if(type == RADAR && ptr->ignore) {
						ptr->ignore = false;
					}
				}
				// if radar got upgraded => check nearby fields
				if(type == RADAR && upgrade == UPGRADE1) {
					if(ptr->py->isInField(x,y) || ptr->py->isInField(x+1,y) || ptr->py->isInField(x-1,y) || ptr->py->isInField(x,y-1) || ptr->py->isInField(x,y+1)) {
						if(ptr->ignore) ptr->ignore = false;
					}
				}

				ptr = ptr->next;
			}

			// draw textures
			if(type == RADAR) {
				draw::tex(getPos(), grid::sty, texID, rotation);
				if(cooldown <= 0) {
					cooldown = cooldownMax;
					if(upgrade == BASE) new radsignal(getPos(), grid::sty, new color(0,0.75,0));
					else new radsignal(getPos(), grid::sty*2, new color(0,0.75,0));
				}
			}
			else if(type == FENCE || type == DRONE) {
				draw::tex(getPos(), grid::sty, texID, rotation);
			}

			// apply rotation
			if(type == DRONE || type == RADAR) rotation-=0.01;
			cooldown--;
		}

		int getType() {
			return type;
		}

		int getUpgrade() {
			return upgrade;
		}

		void doUpgrade() {
			upgrade = UPGRADE1;
			grid::money -= getPrice(type, upgrade);
		}

		bool hasUpgrade() {
			if(type == RADAR && upgrade == BASE) return true;
			return false;
		}

		double getRecoverMoney() {
			double price = getPrice(type, BASE);
			if(upgrade == UPGRADE1) price += getPrice(type, UPGRADE1);
			return price*0.7;
		}

		double getHealth() {
			return health;
		}

		void setHealth(double h) {
			health = h;
		}

		void doRecover() {
			grid::map[x][y].ts = NULL;
			grid::money += getRecoverMoney();
			delete this;
		}

		static double getPrice(int type, int upgrade) {
			if(type == FENCE) {
				return 2000;
			}
			else if(type == DRONE) {
				return 2000;
			}
			else if(type == RADAR) {
				if(upgrade == BASE) return 800;
				else if(upgrade == UPGRADE1) return 2000;
			}
			else {
				fprintf(stderr, "Error: unknown tower or upgrade (%d,%d).\n", type, upgrade);
				exit(1);
			}
			return 0;
		}

};
