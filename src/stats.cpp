// stats.cpp
// manage rank and global stats


// definition in main.cpp
void mainMenuCallback(int);
void saveGame(FILE *fp);

class stats {
	private:
		static double points; // global score
		static unsigned int rank;
		static class menuBase base;

		// create directory if necessary
		static void createDir(const char *dir) {
			struct stat st = {0};
			if (stat(dir, &st) == -1 && mkdir(dir, 0700) == -1) {
				fprintf(stderr, "Error: can't create directory:\n%s\n", dir);
				perror(NULL);
				exit(1);
			}
		}

	public:
		// structure to save highscores
		struct mapstats {
			unsigned int points = 0, waves = 0;
		};

		static char *path; // path to savegame
		static struct mapstats maps[5];
		static double t;

		// load stats/highscore
		static bool load() {
			// generate path
			path = new char[512];
			const char *home = getenv("HOME");
			if(home != NULL) snprintf(path, 512, "%s/.config", home);
			else strncpy(path, ".config", 512);
			createDir(path);
			strncat(path, "/ponydefense", 512);
			createDir(path);
			strncat(path, "/savegame", 512);

			// try to read savegame
			FILE *fp = fopen(path, "rb");
			if(fp != NULL) {
				fseek(fp, 0, SEEK_END);
				unsigned int size = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				if(size >= sizeof(points)+sizeof(maps)) {
					if(fread((void*)&points,sizeof(points),1,fp) != 1) {
						fprintf(stderr, "Error: can't read file:\n%s\n", path);
						perror(NULL);
						exit(1);
					}
					if(fread((void*)maps,sizeof(maps),1,fp) != 1) {
						fprintf(stderr, "Error: can't read file:\n%s\n", path);
						perror(NULL);
						exit(1);
					}
				}
				fclose(fp);
			}
			else return true;

			// refresh rank
			rank = getRank();
			return false;
		}

		// save stats/highscore
		static void save() {
			if(path == NULL) return;
			FILE *fp = fopen(path, "wb");
			if(fp == NULL) {
				fprintf(stderr, "Error: can't write file:\n%s\n", path);
				perror(NULL);
			}
			else {
				fwrite((void*)&points, sizeof(points), 1, fp);
				fwrite((void*)&maps, sizeof(maps), 1, fp);
				// save running game
				if(!grid::gameover) saveGame(fp);
				fclose(fp);
			}
		}

		// calculate rank
		static unsigned int getRank() {
			for(int i = 1; i < 10; i++) {
				if(points < i*i*i*500) return i-1;
			}
			return 9;
		}

		/* rank    reward 
		   1       RAILGUN
		   2       Map 2
		   3       FLAMETHROWER
		   4       FENCE, DRONE
		   5       Map 3
		   6       SHIELD
		   7       LASER
		   8       Map 4
		   9       ARTILLERY */

		// functions to check if rank unlocked item...
		static bool has(TOWERTYPE type) {
			if(type == RAILGUN && rank >= 1) return true;
			if(type == FLAMETHROWER && rank >= 3) return true;
			if(type == SHIELD && rank >= 6) return true;
			if(type == LASER && rank >= 7) return true;
			if(type == ARTILLERY && rank >= 9) return true;
			return false;
		}

		static bool has(SUPTWTYPE type) {
			if(type == FENCE || type == DRONE) {
				if(rank >= 4) return true;
			}
			return false;
		}

		static bool has(MAP mapid) {
			if(mapid == MAP1) return true;
			if(mapid == MAP6) return true;
			if((mapid == MAP2 || mapid == MAP3) && rank >= 2) return true;
			if(mapid == MAP4 && rank >= 5) return true;
			if(mapid == MAP5 && rank >= 8) return true;
			return false;
		}

		// add points to global and running score
		static void addPoints(MAP mapid, unsigned int pts) {
			double multi = 1;
			if(mapid == MAP2) multi = 2;
			else if(mapid == MAP3) multi = 3;
			else if(mapid == MAP4) multi = 3;
			else if(mapid == MAP5) multi = 4;
			else if(mapid == MAP6) multi = 0;//mod
			if(grid::wave < 37) {
				points+=pts*multi;
				grid::score+=pts*multi;
			}
			else {
				double downscale = exp(-0.66*(grid::wave-37));
				points+=pts*multi*downscale;
				grid::score+=pts*multi*downscale;
			}

			// if better - save highscore
			if(grid::wave > maps[mapid].waves && mapid != MAP6) { //mod it werkz
				maps[mapid].waves = grid::wave;
				maps[mapid].points = grid::score;
			}

			// draw promotion message
			unsigned int newRank = getRank();
			if(newRank != rank) {
				rank = newRank;
				if(rank == 1) {
					const char *text[] = {"Rank: Private", "Reward: Railgun", NULL};
					new message("Promotion", (char**)text, tex::RANK_1, message::MIDDLE, 380);
				}
				else if(rank == 2) {
					const char *text[] = {"Rank: Specialist", "Reward: Map", NULL};
					new message("Promotion", (char**)text, tex::RANK_2, message::MIDDLE, 380);
				}
				else if(rank == 3) {
					const char *text[] = {"Rank: Corporal", "Reward: Flamethrower", NULL};
					new message("Promotion", (char**)text, tex::RANK_3, message::MIDDLE, 380);
				}
				else if(rank == 4) {
					const char *text[] = {"Rank: Sergeant", "Reward: Fence/Drone", NULL};
					new message("Promotion", (char**)text, tex::RANK_4, message::MIDDLE, 380);
				}
				else if(rank == 5) {
					const char *text[] = {"Rank: Lieutenant", "Reward: Map", NULL};
					new message("Promotion", (char**)text, tex::RANK_5, message::MIDDLE, 380);
				}
				else if(rank == 6) {
					const char *text[] = {"Rank: Major", "Reward: Shield", NULL};
					new message("Promotion", (char**)text, tex::RANK_6, message::MIDDLE, 380);
				}
				else if(rank == 7) {
					const char *text[] = {"Rank: Colonel", "Reward: Laser", NULL};
					new message("Promotion", (char**)text, tex::RANK_7, message::MIDDLE, 380);
				}
				else if(rank == 8) {
					const char *text[] = {"Rank: General", "Reward: Map", NULL};
					new message("Promotion", (char**)text, tex::RANK_8, message::MIDDLE, 380);
				}
				else if(rank == 9) {
					const char *text[] = {"Rank:Horn-von Hoegen", "Reward: Artillery", NULL};
					new message("Promotion", (char**)text, tex::RANK_9, message::MIDDLE, 380);
				}
			}
		}

		// show rank in main menu
		static void showRank() {
			t+=0.05; // fade in
			char textPoints[512];
			const char *textRank = NULL;
			char textNextRank[512];
			snprintf(textPoints, 512, "Score: %d", (int)points);
			int texID = 0;
			switch (rank) {
				case 0: textRank = "Derpy"; texID = tex::RANK_0; break;
				case 1: textRank = "Private"; texID = tex::RANK_1; break;
				case 2: textRank = "Specialist"; texID = tex::RANK_2; break;
				case 3: textRank = "Corporal"; texID = tex::RANK_3; break;
				case 4: textRank = "Sergeant"; texID = tex::RANK_4; break;
				case 5: textRank = "Lieutenant"; texID = tex::RANK_5; break;
				case 6: textRank = "Major"; texID = tex::RANK_6; break;
				case 7: textRank = "Colonel"; texID = tex::RANK_7; break;
				case 8: textRank = "General"; texID = tex::RANK_8; break;
				case 9: textRank = "Horn-von Hoegen"; texID = tex::RANK_9; break;
			}
			const char *text[] = {textRank, textPoints, textNextRank, NULL};
			if(rank == 9) textNextRank[0] = '\0';
			else snprintf(textNextRank, 512, "Next:  %d", (rank+1)*(rank+1)*(rank+1)*500-(int)points);
			vec *v = new vec(-0.25, -0.3);//changed from 0.2 to 0.3
			if(t >= 1) base.drawMessage("Rank", (char**)text, texID, v, 0.51);
			else base.drawBuild((char**)text, v, 0.5, t);
			delete v;

			// draw version 
			draw::print(new vec(0.915, -0.99), VERSION, new color(0,0,0,0.6));
		}
};

// static definitions (stats class)
double stats::points = 0, stats::t = 0;
unsigned int stats::rank = 0;
char *stats::path = NULL;
struct stats::mapstats stats::maps[5];
class menuBase stats::base;
