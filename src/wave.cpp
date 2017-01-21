// wave.cpp
// spawn wave mechanics 


class wave {
	private:
		int cooldown, subwave, mainwave, ponynum;
		bool waitFinish;
		int sub1 = 0, sub2 = 0;
		int subTime, ponyTime; // time between ponies/waves

		// wrapper to init trojan pony
		void spawn(int type) {
			class pony *p = new pony(type);
			if(type == TROJAN) {
				if(mainwave < 9) {
					if(mainwave-1 == TROJAN)
						p->setTrojanSpawn(mainwave-2, randi(1,3));
					else
						p->setTrojanSpawn(mainwave-1, randi(1,3));
				}
				else {
					p->setTrojanSpawn(8, randi(1,5));
				}
			}
			ponynum++;
		}

		// spawn random pony (type depending on mainwave)
		void spawnRandom() {
			if(mainwave < 2) spawn(randi(0,mainwave));
			else spawn(randi(mainwave-2,mainwave));
		}

		void next() {
			ponynum = sub1 = sub2 = 0;
			waitFinish = true;
		}

		void finalSpawn(int rec) { // "final" recursive trojan ponies
			class pony *p = new pony(TROJAN);
			if(rec > 0) {
				p->setTrojanSpawn(TROJAN, 2, rec);
			}
			else {
				p->setTrojanSpawn(randi(RAINBOW_DASH,TWILIGHT_SPARKLE_SHIELD), randi(1,5));
			}
			ponynum++;
		}

	public:
		wave() {
			draw::addRenderCallback(std::bind(&wave::draw, this), (void*)&cooldown);
			cooldown = 120;
			ponynum = 0;
			waitFinish = false;
			grid::wave = 1;
		}

		~wave() {
			draw::delRenderCallback((void*)&cooldown);
		}

		void draw() {
			if(grid::paused) return;
			if(grid::gameover) {
				stats::save();
				grid::paused = true;
			}
			if(cooldown > 0) {
				cooldown--;
				return;
			}

			if(waitFinish) {
				if(ponyList == NULL) {
					waitFinish = false;
					cooldown = 240;
					grid::wave++;
					stats::save();

					// message about pony type
					if(grid::wave == 4) {
						const char *text[] = {"New opponent: ", "Fluttershy", NULL};
						new message("Warning", (char**)text, tex::PONY_FLUTTERSHY, message::BOTTOM, 240);
					}
					else if(grid::wave == 8) {
						const char *text[] = {"New opponent: ", "Pinkie Pie", NULL};
						new message("Warning", (char**)text, tex::PONY_PINKIE_PIE, message::BOTTOM, 240);
					}
					else if(grid::wave == 12) {
						const char *text[] = {"New opponent: ", "Twilight Sparkle", NULL};
						new message("Warning", (char**)text, tex::PONY_TWILIGHT_SPARKLE, message::BOTTOM, 240);
					}
					else if(grid::wave == 16) {
						const char *text[] = {"New opponent: ", "Trojan Pony", NULL};
						new message("Warning", (char**)text, tex::PONY_TROJAN, message::BOTTOM, 240);
					}
					else if(grid::wave == 20) {
						const char *text[] = {"New opponent: ", "Rarity (+Shield)", NULL};
						new message("Warning", (char**)text, tex::PONY_RARITY, message::BOTTOM, 240);
					}
					else if(grid::wave == 24) {
						const char *text[] = {"New opponent: ", "Rainbow Dash", NULL};
						new message("Warning", (char**)text, tex::PONY_RAINBOW_DASH, message::BOTTOM, 240);
					}
					else if(grid::wave == 28) {
						const char *text[] = {"New opponent: ", "Applejack", NULL};
						new message("Warning", (char**)text, tex::PONY_APPLEJACK, message::BOTTOM, 240);
					}
					else if(grid::wave == 32) {
						const char *text[] = {"New opponent: ", "Twilight (+Shield)", NULL};
						new message("Warning", (char**)text, tex::PONY_TWILIGHT_SPARKLE, message::BOTTOM, 240);
					}
					else if(grid::wave == 39) {
						const char *text[] = {"Next wave:", "Troj. Ponycalypse", NULL};
						new message("Warning", (char**)text, tex::PONY_TROJAN, message::BOTTOM, 240);
					}
					else return;
				}
				return;
			}

			// fix times
			subTime = 420;
			ponyTime = 123-3*grid::wave;
			if(ponyTime < 8) ponyTime = 8;

			// fix waves
			subwave = ((grid::wave-1)%4)+1;
			mainwave = ((grid::wave-1)/4);

			// spawn "final" recursive trojan ponies
			if(grid::wave > 39) {
				if(sub1 == 0) {
					sub1 = 3;
				}
				if(ponynum < sub1) {
					finalSpawn(grid::wave-40);
					cooldown = 75;
				}
				else {
					if(randi(1, 4) == 4) new airstrike();
					finalSpawn(grid::wave-40);
					next();
				}
				return;
				
			}

			// fill "wave gap" (36…40)
			if(mainwave > 8) {
				if(sub1 == 0) {
					sub1 = (int)((24.0/36.0)*grid::wave);
					sub2 = grid::wave;
				}
				if(ponynum < sub1) {
					spawn(randi(0,8));
					cooldown = ponyTime;
				}
				else if(ponynum < sub2) {
					spawn(randi(0,8));
					cooldown = ponyTime;
				}
				else {
					if(randi(1, 4) == 4) new airstrike();
					spawn(randi(0,8));
					next();
				}
				return;
			}

			// spawn main/subwaves
			if(subwave == 1) {
				if(ponynum < 1) {
					spawn(mainwave);
					cooldown = subTime;
				}
				else if(ponynum < 3) {
					spawn(mainwave);
					cooldown = ponyTime;
				}
				else if(ponynum < 4) {
					spawn(mainwave);
					next();
				}
			}
			else if(subwave == 2) {
				if(ponynum < 4) {
					spawnRandom();
					cooldown = ponyTime;
				}
				else if(ponynum < 5) {
					spawnRandom();
					cooldown = subTime;
				}
				else if(ponynum < 12) {
					spawnRandom();
					cooldown = ponyTime;
				}
				else if(ponynum < 13) {
					spawnRandom();
					next();
				}
			}
			else if(subwave == 3) {
				if(ponynum < 11) {
					spawnRandom();
					cooldown = ponyTime;
				}
				else if(ponynum < 12) {
					spawnRandom();
					cooldown = subTime;
					if(grid::wave > 14) {
						if(randi(1, 4) == 4) new airstrike();
					}
				}
				else if(ponynum < 29) {
					spawnRandom();
					cooldown = ponyTime;
				}
				else if(ponynum < 30) {
					spawnRandom();
					next();
				}
			}
			else if(subwave == 4) {
				if(ponynum < 23) {
					spawnRandom();
					cooldown = ponyTime;
				}
				else if(ponynum < 24) {
					spawnRandom();
					cooldown = subTime;
					if(grid::wave > 17) {
						if(randi(1, 2) == 2) new airstrike();
					}
				}
				else if(ponynum < 55) {
					spawnRandom();
					cooldown = ponyTime;
				}
				else if(ponynum < 56) {
					spawnRandom();
					next();
				}
			}
		}
};
