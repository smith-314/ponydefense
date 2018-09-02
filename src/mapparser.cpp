// mapparser.cpp
// reading and validating mapfiles


class mapparser {
	private:
		// import map file to array
		static void importMap(const char *name) {
			// get next unused slot in array
			int i = 0;
			while(map_size[i] > 0 && i < 32) i++;
			if(i == 32) return;

			// generate path and open file
			char path[PATH_MAX];
			stats::genConfigPath(path);
			strncat(path, "/", PATH_MAX-1);
			strncat(path, name, PATH_MAX-1);
			FILE *fp = fopen(path, "r");
			if(fp == NULL) return;

			// skip empty lines or those with #
			char buffer[512];
			do {
				if(fgets(buffer, 512, fp) == NULL) {
					fclose(fp);
					return;
				}
			} while(buffer[0] == '#' || buffer[0] == '\n');

			// get initial map size
			map_size[i] = strlen(buffer)-1;
			map_buf[i] = new char[map_size[i]*map_size[i]];

			// parse map structure
			bool spawn = false;
			fseek(fp, -map_size[i]-1, SEEK_CUR);
			for(int sz = 0; sz < map_size[i]*map_size[i]; sz++) {
				int c = fgetc(fp);
				if(c == EOF) {
					fprintf(stderr, "%s: fields do not fit map size (%d of %d).\n", name, sz+1, map_size[i]*map_size[i]);
					map_size[i] = 0;
					delete[] map_buf[i];
					fclose(fp);
					return;
				}
				if(c == 'b' || c == 'w' || c == 's' || c == '-') {
					if(c == 's') spawn = true;
					map_buf[i][sz] = (char)c;
				}
				else sz--;
			}
			fclose(fp);
			if(!spawn) {
				fprintf(stderr, "%s: no spawn field found.\n", name);
				map_size[i] = 0;
				delete[] map_buf[i];
				return;
			}

			// save map name
			map_name[i] = new char[strlen(name)+1];
			strcpy(map_name[i], name);
		}
	
	public:
		// custom map array
		static int map_size[32];
		static char *map_buf[32];
		static char *map_name[32];

		// fetch maps in config directory
		static void fetchMaps() {
			// array cleanup
			for(int i = 0; i < 32; i++) {
				if(map_size[i] > 0) {
					delete[] map_buf[i];
					delete[] map_name[i];
					map_size[i] = 0;
				}
			}

			// iterate map_* files in ~/.config/ponydefense
			char path[PATH_MAX];
			stats::genConfigPath(path);
			DIR *pdir = opendir(path);
			if(pdir == NULL) {
				fprintf(stderr, "Error: can't open directory:\n%s\n", path);
				perror(NULL);
				return;
			}

			struct dirent *ent = readdir(pdir);
			while(ent != NULL) {
				if(ent->d_type == DT_REG && !strncmp(ent->d_name, "map_", 4))
					importMap(ent->d_name);
				ent = readdir(pdir);
			}
			closedir(pdir);
		}

		// write example map file
		static void writeExample() {
			char path[PATH_MAX];
			stats::genConfigPath(path);
			strncat(path, "/map_Example", PATH_MAX-1);
			
			FILE *fp = fopen(path, "w");
			if(fp == NULL) return;

			const char *example =
				"# Custom map example\n#\n"
				"# Every map_* file in this directory will be\n"
				"# read in. Height and width must be the same.\n#\n"
				"# field types:\n"
				"# - (free)\n"
				"# s (spawn)\n"
				"# w (way)\n"
				"# b (blocking)\n#\n\n"
				"--------\n"
				"-swwww--\n"
				"-----ww-\n"
				"-w---bw-\n"
				"-wbbbbw-\n"
				"-wwbbww-\n"
				"--wwww--\n"
				"--------\n";
			fwrite(example, strlen(example), 1, fp);
			fclose(fp);
		}
};


// static definitions (mapparser class)
int mapparser::map_size[32] = {0};
char *mapparser::map_buf[32];
char *mapparser::map_name[32];
