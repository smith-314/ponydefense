// draw.cpp
// OpenGL/GLUT wrappers


#define TARGET_FPS 60

// sqrt approximation
double sqrt(double n) {
	double curr = (n + 1) / 2;
	double prev;

	const double roundoff = curr * 0.001;
	do {
		prev = curr;
		curr = (curr + n/curr)/2;
	} while(prev-curr > roundoff);

	return curr;
}

// arccos approximation
//double acos(double x) {
//	return (-0.69813170079773212*x*x - 0.87266462599716477)*x + 1.5707963267948966;
//}

// generate random integer
int randi(int min, int max) {
	return min + (rand() % (max-min +1));
}

// generate random double
double randd(double min, double max) {
	return min + (rand() / (RAND_MAX/(max-min)));
}

// class for color and transparency
class color {
	public:
		double r,g,b,t;
		color(double _r = 0, double _g = 0, double _b = 0, double _t = 1) {
			r = _r; g = _g; b = _b; t = _t;
		}
		color *clone() {
			return new color(r, g, b, t);
		}
};

// simple 2D vector class for relative positions
class vec {
	public:
		double x,y;
		vec(double _x, double _y) {
			x = _x; y = _y;
		}
		vec() {
			x = y = 0;
		}
		vec *clone() {
			return new vec(x,y);
		}
		double abs() {
			return sqrt(x*x+y*y);
		}
		static double getDist(vec *v1, vec *v2) {
			return (v1->x-v2->x)*(v1->x-v2->x)+(v1->y-v2->y)*(v1->y-v2->y);
		}
		static double getAngle(vec *v1, vec *v2) {
			return acos((v1->x*v2->x+v1->y*v2->y)/(v1->abs()*v2->abs()));
		}
};

// callback-structure for linked list of the render-functions
struct _callback {
	void *id; // random value to identify the callback
	unsigned int priority; // order of execution
	std::function<void()> ptr; // (member compatible) pointer to the callback
	struct _callback *next;
};


// dummy functions for the input-hook
// => prevents "bad_function_call"
void _dummyMouseMotion(int a, int b) {};
void _dummyMouse(int a, int b, int c, int d) {};
void _dummyKeyboard(unsigned char a, int b, int c) {};

// OpenGL/GLUT-wrappers
class draw {
	private:
		static FTGL::FTGLfont *ftFont;
		static int fontSize;
		static struct _callback *cbList; // render callback list
		// input callbacks
		static std::function<void(int,int)> mouseMotionCallback;
		static std::function<void(int,int,int,int)> mouseCallback;
		static std::function<void(unsigned char,int,int)> keyboardCallback;

		static void loadFont() {
			// possible font paths (at least on Arch/Debian)
			const char *fontPaths[] = {
				"/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
				"/usr/share/fonts/TTF/LiberationMono-Bold.ttf",
				"/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
				"/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
				"DejaVuSans-Bold.ttf",
				"LiberationMono-Bold.ttf", NULL};

			// try to load fonts
			for(int i = 0;; i++) {
				ftFont = FTGL::ftglCreatePixmapFont(fontPaths[i]);
				if(ftFont != NULL) break;
				if(fontPaths[i+1] == NULL) {
					fprintf(stderr, "Error: no suitable font found.\n");
					exit(1);
				}
			}
			ftglSetFontFaceSize(ftFont, 15, 0);
		}

	public:
		// callback priorities
		enum {HIGHEST, HIGH, DEFAULT, LOW, LOWEST};
		// window and scene size
		static int wx, wy; // window
		static int sx, sy; // scene
		static double relx, rely; // relative size
		static double ar; // aspect ratio
		// background texture
		static int bgTex;
		// render speed 
		static unsigned int speed;
		static bool enableDraw;

		// debug mode: track callbacks and performance
		static bool debugMode;
		static long timeRender;
		static long timePreRender;
		static int callbackCounter;
		static long frameTime;
		static int frame;
		static int fps;

		// redraw timer callback to ensure fps limit
		static void redrawTimer(int param) {
			glutPostRedisplay();
			glutTimerFunc(1000/TARGET_FPS, redrawTimer, 0);
		}

		// initialize OpenGL/GLUT
		static void init(int *argc, char **argv) {
			glutInit(argc, argv);
			glutInitWindowPosition(250, 150);
			glutInitWindowSize(wx, wy);
			glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
			glEnable(GL_MULTISAMPLE);

			// create window
			glutCreateWindow("PonyDefense");

			// set (dummy) input-hooks
			setMouseCallback(&_dummyMouse);
			setMouseMotionCallback(&_dummyMouseMotion);
			setKeyboardCallback(&_dummyKeyboard);

			// set GLUT callbacks
			glutDisplayFunc(draw::render);
			glutReshapeFunc(draw::resize);
			glutMouseFunc(draw::mouse);
			glutPassiveMotionFunc(draw::mouseMotion);
			glutKeyboardFunc(draw::keyboard);
			glutTimerFunc(1000/TARGET_FPS, redrawTimer, 0);

			// load fonts and textures
			loadFont();
			tex::init();

			// init window related values
			resize(wx,wy);
		}

		// convert pixel to relative position
		static double relX(int x) {
			return -relx + relx*(2.0*x/wx);
		}

		static double relY(int y) {
			return rely - rely*(2.0*y/wy);
		}

		// GLUT callback: render function
		static void render() {
			glClearColor(1.0, 1.0, 1.0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			drawBackground();

			long begTime, midTime;
			int counter = 0;
			if(debugMode) begTime = glutGet(GLUT_ELAPSED_TIME);

			// apply speed with multiple calls
			enableDraw = false;
			for(unsigned int sp = 0; sp < speed -1; sp++) {
				for(unsigned int i = HIGHEST; i <= LOWEST; i++) { // apply priorities
					struct _callback *cbptr = cbList;
					while(cbptr != NULL) {
						if(cbptr->priority == i) cbptr->ptr(); // execute callback
						cbptr = cbptr->next;
					}
				}
			}
			enableDraw = true;

			if(debugMode) midTime = glutGet(GLUT_ELAPSED_TIME);

			// render frame
			for(unsigned int i = HIGHEST; i <= LOWEST; i++) {
				struct _callback *cbptr = cbList;
				while(cbptr != NULL) {
					if(cbptr->priority == i) {
						counter++;
						cbptr->ptr();
					}
					cbptr = cbptr->next;
				}
			}

			if(debugMode) {
				// get render time
				long time = glutGet(GLUT_ELAPSED_TIME);
				timeRender = time - begTime;
				timePreRender = midTime - begTime;
				callbackCounter = counter;
				// calculate fps
				frame++;
				if(time - frameTime >= 500) {
					fps = frame*2;
					frame = 0;
					frameTime = time;
				}
			}

			glLoadIdentity();
			glClearColor(1.0, 1.0, 1.0, 0);
			glutSwapBuffers();
		}

		// GLUT callback: resize window
		static void resize(int x, int y) {
			// if window is really small - ignore
			if(x < 100 || y < 100) return;
			//glutReshapeWindow(x, y);
			glViewport(0, 0, x, y);

			// fit scene to window
			double rx, ry; // relative size of the scene
			if(1.0*x/y > ar) {
				ry = 1.0;
				rx = x/(ar*ry*y);
			}
			else {
				rx = 1.0;
				ry = (y*ar)/(rx*x);
			}

			// add border
			rx+=0.07;ry+=0.05;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(-rx,rx,-ry,ry);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			// save window settings
			wx = x; wy = y;
			sx = (int)(1.0*x/rx);
			sy = (int)(1.0*y/ry);
			relx = rx; rely = ry;

			// update font size
			fontSize = fixFontSize();
		}

		// GLUT callback: mouse click events
		static void mouse(int button, int state, int xc, int yc) {
			mouseCallback(button,state,xc,yc);
		}

		// GLUT callback: mouse move events
		static void mouseMotion(int xc, int yc) {
			mouseMotionCallback(xc,yc);
		}

		// GLUT callback: keyboard events
		static void keyboard(unsigned char key, int x, int y) {
			keyboardCallback(key,x,y);
		}

		// set background texture
		static void setBackground(int tex) {
			bgTex = tex;
		}

		// print string
		static void print(vec *v, const char *str, color *col = new color()) {
			if(!enableDraw) {
				delete v; delete col;
				return;
			}
			ftglSetFontFaceSize(ftFont, fontSize, 0);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glRasterPos2d(v->x, v->y);
			FTGL::ftglRenderFont(ftFont, str, FTGL::RENDER_ALL);
			delete v; delete col;
		}

		// draw simple line
		static void line(vec *v1, vec *v2, color *col = new color(), double width = 1.0) {
			if(!enableDraw) {
				delete v1; delete v2; delete col;
				return;
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glLineWidth(width);
			glBegin(GL_LINES);
			glVertex2d(v1->x, v1->y);
			glVertex2d(v2->x, v2->y);
			glEnd();
			glDisable(GL_BLEND);
			delete v1; delete v2; delete col;
		}

		// draw square
		static void square(vec *v, double size, color *col = new color()) {
			if(!enableDraw) {
				delete v; delete col;
				return;
			}
			double diff = size/2.0;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glBegin(GL_POLYGON);
			glVertex2d(v->x-diff, v->y+diff);
			glVertex2d(v->x+diff, v->y+diff);
			glVertex2d(v->x+diff, v->y-diff);
			glVertex2d(v->x-diff, v->y-diff);
			glEnd();
			glDisable(GL_BLEND);
			delete v; delete col;
		}

		// draw rectangle
		static void rect(vec *v1, vec *v2, color *col = new color()) {
			if(!enableDraw) {
				delete v1; delete v2; delete col;
				return;
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glBegin(GL_POLYGON);
			glVertex2d(v1->x, v1->y);
			glVertex2d(v2->x, v1->y);
			glVertex2d(v2->x, v2->y);
			glVertex2d(v1->x, v2->y);
			glEnd();
			glDisable(GL_BLEND);
			delete v1; delete v2; delete col;
		}

		// draw triangle
		static void triangle(vec *v1, vec *v2, vec *v3, color *col = new color()) {
			if(!enableDraw) {
				delete v1; delete v2; delete v3; delete col;
				return;
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glBegin(GL_POLYGON);
			glVertex2d(v1->x, v1->y);
			glVertex2d(v2->x, v2->y);
			glVertex2d(v3->x, v3->y);
			glEnd();
			glDisable(GL_BLEND);
			delete v1; delete v2; delete v3; delete col;
		}

		// draw edge of a circle
		static void circle(vec *v, double size, color *col = new color()) {
			if(!enableDraw) {
				delete v; delete col;
				return;
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glBegin(GL_LINE_LOOP);
			for(double phi = 0; phi < 2*M_PI; phi+=0.1) {
				glVertex2d(v->x + size*cos(phi)*(1/ar), v->y + size*sin(phi));
			}
			glEnd();
			glDisable(GL_BLEND);
			delete v; delete col;
		}

		// draw circle
		static void fullCircle(vec *v, double size, color *col = new color()) {
			if(!enableDraw) {
				delete v; delete col;
				return;
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glBegin(GL_POLYGON);
			for(double phi = 0; phi < 2*M_PI; phi+=0.1) {
				glVertex2d(v->x + size*cos(phi)*(1/ar), v->y + size*sin(phi));
			}
			glEnd();
			glDisable(GL_BLEND);
			delete v; delete col;
		}

		// draw blurred circle
		static void diffFullCircle(vec *v, double size, color *col = new color()) {
			if(!enableDraw) {
				delete v; delete col;
				return;
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4d(col->r, col->g, col->b, col->t);
			glBegin(GL_POLYGON);
			for(double phi = 0; phi < 2*M_PI; phi+=0.3) {
				glVertex2d(v->x + size*cos(phi)*(1/ar), v->y + size*sin(phi));
				const double var = 0.1;
				v->x += var*size*(rand()%2)-var*0.5*size;
				v->y += var*size*(rand()%2)-var*0.5*size;
			}
			glEnd();
			glDisable(GL_BLEND);
			delete v; delete col;
		}

		// draw texture (with optional rotation and transparency)
		static void tex(vec *v, double size, int id, double rot = 0, double t = 1, bool mirr = false) {
			if(!enableDraw) {
				delete v;
				return;
			}
			long double diff = size/2.0;
			glColor4d(1.0,1.0,1.0,t);
			glLoadIdentity();
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, id);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// rotate
			glTranslated(v->x, v->y, 0);
			glRotated(-rot*57.295779513082, 0, 0, 1.0); // rad to deg
			glTranslated(-v->x, -v->y, 0);
			glBegin(GL_QUADS);
			if(rot == 0) {
				if(mirr) {
					glTexCoord2d(1, 1); glVertex2d(v->x-diff*(1/ar), v->y+diff);
					glTexCoord2d(0, 1); glVertex2d(v->x+diff*(1/ar), v->y+diff);
					glTexCoord2d(0, 0); glVertex2d(v->x+diff*(1/ar), v->y-diff);
					glTexCoord2d(1, 0); glVertex2d(v->x-diff*(1/ar), v->y-diff);
				}
				else {
					glTexCoord2d(0, 1); glVertex2d(v->x-diff*(1/ar), v->y+diff);
					glTexCoord2d(1, 1); glVertex2d(v->x+diff*(1/ar), v->y+diff);
					glTexCoord2d(1, 0); glVertex2d(v->x+diff*(1/ar), v->y-diff);
					glTexCoord2d(0, 0); glVertex2d(v->x-diff*(1/ar), v->y-diff);
				}
			}
			else {
				// when rotated, the texture is stretched due to the aspect ratio
				// TODO: find better solution to fit to aspect ratio (?)

				// linear scale
				double xcorr, ycorr;
				if(rot < 0) rot = -rot;
				while(rot > M_PI) rot-=M_PI;
				if(rot > 0 && rot <= M_PI/2) {
					xcorr = 1.0 + (1-rot*(2/M_PI))*(1.0/ar - 1.0);
					ycorr = 1.0 + (rot*(2/M_PI))*(1.0/ar - 1.0);
				}
				else {
					xcorr = 1.0 + (1-(M_PI -rot)*(2/M_PI))*(1.0/ar - 1.0);
					ycorr = 1.0 + ((M_PI-rot)*(2/M_PI))*(1.0/ar - 1.0);
				}

				if(mirr) {
					glTexCoord2d(1, 1); glVertex2d(v->x-diff*xcorr, v->y+diff*ycorr);
					glTexCoord2d(0, 1); glVertex2d(v->x+diff*xcorr, v->y+diff*ycorr);
					glTexCoord2d(0, 0); glVertex2d(v->x+diff*xcorr, v->y-diff*ycorr);
					glTexCoord2d(1, 0); glVertex2d(v->x-diff*xcorr, v->y-diff*ycorr);
				}
				else {
					glTexCoord2d(0, 1); glVertex2d(v->x-diff*xcorr, v->y+diff*ycorr);
					glTexCoord2d(1, 1); glVertex2d(v->x+diff*xcorr, v->y+diff*ycorr);
					glTexCoord2d(1, 0); glVertex2d(v->x+diff*xcorr, v->y-diff*ycorr);
					glTexCoord2d(0, 0); glVertex2d(v->x-diff*xcorr, v->y-diff*ycorr);
				}
			}
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
			glLoadIdentity();
			delete v;
		}

		// add render callback entry
		static void addRenderCallback(std::function<void()> ptr, void *id, unsigned int priority = DEFAULT) {
			struct _callback *cbptr = cbList;
			if(cbList == NULL) cbList = cbptr = new _callback;
			else {
				while(cbptr->next != NULL) cbptr = cbptr->next;
				cbptr->next = new _callback;
				cbptr = cbptr->next;
			}
			cbptr->ptr = ptr;
			cbptr->id = id;
			cbptr->priority = priority;
			cbptr->next = NULL;
		}

		// delete render callback entry
		static void delRenderCallback(void *id) {
			struct _callback *cbptr = cbList;
			if(cbList->id == id) {
				cbList = cbList->next;
				delete cbptr;
				return;
			}
			while(cbptr->next->id != id && cbptr->next != NULL) cbptr = cbptr->next;
			if(cbptr->next == NULL) return;
			struct _callback *cbToDel = cbptr->next;
			cbptr->next = cbptr->next->next;
			delete cbToDel;
		}

		// wrappers to set/get callbacks...
		static void setMouseMotionCallback(std::function<void(int,int)> func) {
			mouseMotionCallback = func;
		}

		static void setMouseCallback(std::function<void(int,int,int,int)> func) {
			mouseCallback = func;
		}

		static void setKeyboardCallback(std::function<void(unsigned char,int,int)> func) {
			keyboardCallback = func;
		}

		static std::function<void(int,int)> getMouseMotionCallback() {
			return mouseMotionCallback;
		}

		static std::function<void(int,int,int,int)> getMouseCallback() {
			return mouseCallback;
		}

		static std::function<void(unsigned char,int,int)> getKeyboardCallback() {
			return keyboardCallback;
		}

	private:
		// draw background texture
		static void drawBackground() {
			if(bgTex != 0) {
				// in the case of a periodic arrangement, GLUT generates some kind of ugly "lines"
				// (bad) solution: stretch the background
				// TODO: maybe solve via GL_REPEAT?

				glColor4d(1.0,1.0,1.0,1.0);
				glLoadIdentity();
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, bgTex);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_QUADS);
				glTexCoord2d(0, 1); glVertex2d(relx, rely);
				glTexCoord2d(1, 1); glVertex2d(relx, -rely);
				glTexCoord2d(1, 0); glVertex2d(-relx, -rely);
				glTexCoord2d(0, 0); glVertex2d(-relx, rely);
				glEnd();
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_BLEND);
				glLoadIdentity();

				// old periodic arrangement:
				/*
				   double stepx = bgSize*(1/ar)*2*relx/wx; // relative Schrittweite
				   double stepy = bgSize*2*rely/wy;

				//double fix =  bgSize/wx; // Korrekturfaktor
				vec v(-relx+stepx/2,rely-stepy/2);
				do {
				do {
				tex(v.clone(), stepy, bgTex);
				v.x+=stepx;
				} while(v.x <= 2*relx);
				v.x = -relx+stepx/2;
				v.y-=stepy;
				} while(v.y >= -2*rely);
				 */
			}
		}

		// fit font size to window size
		static int fixFontSize() {
			if(sx > (int)(sy*ar)) {
				return (int)(0.014*sx);
			}
			else {
				return (int)(0.014*sy*ar);
			}
		}
};


// static definitions (draw class)
FTGL::FTGLfont *draw::ftFont = NULL;
int draw::fontSize = 0;
struct _callback *draw::cbList = NULL;
int draw::wx = 800;
int draw::wy = 576;
int draw::sx, draw::sy;
unsigned int draw::speed = 1;
double draw::relx, draw::rely;
int draw::bgTex = 0;
double draw::ar = 2.5/1.8;
bool draw::enableDraw = 1;
std::function<void(int,int)> draw::mouseMotionCallback;
std::function<void(int,int,int,int)> draw::mouseCallback;
std::function<void(unsigned char,int,int)> draw::keyboardCallback;

bool draw::debugMode = false;
long draw::timeRender = 0;
long draw::timePreRender = 0;
int draw::callbackCounter = 0;
long draw::frameTime = 0;
int draw::fps = 0, draw::frame = 0;
