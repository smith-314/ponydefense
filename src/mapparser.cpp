// class for reading mapfiles
// used by main and grid to create maps and menu entries
class mapparser {
	private:
		// check for a tag or comment
		static int checkline(char c) {
			if(c=='#') return 0;
			else if(c=='[') return 1;
			else if(c==']') return 3;
			else return 2;
		}
		// interpret fields in map
		static int getfield(char c) {
			if(c=='-') return 0;
			else if(c=='b') return 1;
			else if(c=='s') return 2;
			else if(c=='w') return 3;
			else return 4;
		}
		// create directory if necessary (used for maps directory)
		static void createDir(const char *dir) {
			struct stat st = {0};
			if (stat(dir, &st) == -1 && mkdir(dir, 0700) == -1) {
				fprintf(stderr, "Error: can't create directory:\n%s\n", dir);
				perror(NULL);
				exit(1);
			}
		}

		//get map path
		static char* getPath(int mapnumber) {
			static char *path;
			path = new char[512];
			const char *home = getenv("HOME");
			if(home != NULL) snprintf(path, 512, "%s/.config", home);
			else strncpy(path, ".config", 512);
			strncat(path, "/ponydefense", 512);
			strncat(path, "/maps/", 512);

			char fileName[20];
			sprintf(fileName,"custom_map_%d",mapnumber);
			strncat(path,fileName, 512);
			return path;
		}

		// check if map exists
		// should be called first so path creation is here
		static bool customMapExists( int mapnumber ) {
			static char *path;
			path = new char[512];
			const char *home = getenv("HOME");
			if(home != NULL) snprintf(path, 512, "%s/.config", home);
			else strncpy(path, ".config", 512);
			createDir(path);
			strncat(path, "/ponydefense", 512);
			createDir(path);
			strncat(path, "/maps/", 512);
			createDir(path);

			char fileName[20];
			sprintf(fileName,"custom_map_%d",mapnumber);
			strncat(path,fileName, 512);
			std::ifstream infile(path);
			if(infile.good()) return true;
			else return false;
		}

	public:
		// generate the map template
		// maybe better to download it with git an move it while installing
		static void generateMapTemplate(){
		}

		//check if map syntax is valid
		static bool customMapValid(int mapnumber) {
			static char *path;
			path = getPath(mapnumber);

			int rowcounter = 0;
			bool  maptrigger=false, errortrigger=false;

			if(!customMapExists(mapnumber)) errortrigger = true;// check if map exists

			int mapsize = customMapSize(mapnumber);
			std::string line;

			std::ifstream mapfile (path);
			if (mapfile.is_open()) {
			while(getline(mapfile,line)) {
				if(line.length()) {
						if (checkline(line.at(0)) == 1 && checkline(line.at(line.length()-1)) == 3) {
							if(line == "[begin map]") maptrigger=true;
							else if (line == "[end map]") maptrigger=false;
						}
						else if (checkline(line.at(0)) == 2 && maptrigger) {
							if(line.length() != mapsize) errortrigger = true;// check if line matches mapsize
							else if(rowcounter == mapsize) errortrigger = true;// trigger when more rows than size are given
							rowcounter++;
						}
					}
				}
				mapfile.close();
			}
			if(rowcounter != mapsize) errortrigger = true;//check if rows match mapsize

			delete[] path;
			return !errortrigger;
		}

		// get map background
		static int customMapBackground( int mapnumber ) {
			
			static char *path;
			path = getPath(mapnumber);// get path of map

			bool bgtrigger = false;

			std::string line;
			std::string mapbg = "";

			std::ifstream mapfile (path);
			if (mapfile.is_open() && customMapExists(mapnumber)) {
				while(getline(mapfile,line)) {
					if(line.length()) {
						if (checkline(line.at(0)) == 1 && checkline(line.at(line.length()-1)) == 3) {
							if (line == "[begin bg]") bgtrigger=true;
							else if (line == "[end bg]") bgtrigger=false;
						}
						else if (checkline(line.at(0)) == 2 && bgtrigger) mapbg = line;
					}
				}
			}

			delete[] path;
			if(mapbg.length()) {
				if(mapbg == "desert") return 1;
				else if(mapbg == "snow") return 2;
				else return 0;
			}
			else return 0;
		}
			
		// TODO: add creator name
		// get name of map
		static std::string customMapName( int mapnumber ) {
			
			static char *path;
			path = getPath(mapnumber);// get path of map

			bool nametrigger = false;

			std::string line;
			std::string mapname;

			std::ifstream mapfile (path);
			if (mapfile.is_open() && customMapExists(mapnumber)) {
				while(getline(mapfile,line)) {
					if(line.length()) {
						if (checkline(line.at(0)) == 1 && checkline(line.at(line.length()-1)) == 3) {
							if (line == "[begin name]") nametrigger=true;
							else if (line == "[end name]") nametrigger=false;
						}
						else if (checkline(line.at(0)) == 2 && nametrigger) mapname = line;
					}
				}
			}
			else mapname = "";

			delete[] path;
			return mapname;
		}

		// get size of map
		static int customMapSize( int mapnumber ) {
			
			static char *path;// get path of map
			path = getPath(mapnumber);

			int mapsize = 0;
			bool sizetrigger = false;

			std::string line;

			std::ifstream mapfile (path);
			if (mapfile.is_open()) {
				while(getline(mapfile,line)) {
					if(line.length()) {
						if (checkline(line.at(0)) == 1 && checkline(line.at(line.length()-1)) == 3) {
							if (line == "[begin size]") sizetrigger=true;
							else if (line == "[end size]") sizetrigger=false;
						}
						else if (checkline(line.at(0)) == 2 && sizetrigger) mapsize = stoi(line);
					}
				}
			}
			
			delete[] path;
			return mapsize;
		}

		// get content of map
		static int* customMapGet(int mapnumber, int maparr[]) {
			
			static char *path;
			path = getPath(mapnumber);// get path for map

			std::string line;// string for line
			int rowcounter = 0;// iterator for map rows
			bool  maptrigger=false;// trigger for maptag

			int mapsize = customMapSize(mapnumber);// get size of map
			for(int i=0; i<mapsize*mapsize; i++) maparr[i]=0;// set array to 0

			std::ifstream mapfile (path);
			if (mapfile.is_open() && customMapValid(mapnumber)) {
				while(getline(mapfile,line)) {
					if(line.length()) {
						if (checkline(line.at(0)) == 1 && checkline(line.at(line.length()-1)) == 3) {
							if(line == "[begin map]") maptrigger=true;
							else if (line == "[end map]") maptrigger=false;
						}
						else if (checkline(line.at(0)) == 2 && maptrigger) {
							for(int j=0; j<mapsize; j++) maparr[j+mapsize*rowcounter] = getfield(line.at(j));
							rowcounter++;
						}
					}
				}
				mapfile.close();
			}

			delete[] path;
			return maparr;
		}

};
