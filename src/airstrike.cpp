// airstrike.cpp
// draw airstrike and damage towers


class airstrike {
	private:
		double t = 0, x, y, step;
		double tfade = 0;
		unsigned int oldspeed;

		std::function<void(int,int)> mouseMotionBackup;
		std::function<void(int,int,int,int)> mouseBackup;
		std::function<void(unsigned char,int,int)> keyboardBackup;

		void towerDamage(int fx, int fy, double dmg) {
			if(fx < 0 || fx >= (int)grid::size || fy < 0 || fy >= (int)grid::size) return;
			if(grid::map[fx][fy].type == NONE && grid::map[fx][fy].tw != NULL) {
				grid::map[fx][fy].tw->doDamage(dmg);
			}
		}

		// add damage to towers in range
		void areaDamage() {
			// get field position
			int fx = grid::pfx(x+step);
			int fy = grid::pfy(y+step);
			for(int i = -2; i < 3; i++) {
				for(int z = -2; z < 3; z++)
					towerDamage(fx+i, fy+z, 0.6);
			}
			for(int i = -1; i < 2; i++) {
				for(int z = -1; z < 2; z++)
					towerDamage(fx+i, fy+z, 0.4);
			}
		}

		// set input-hook
		void setInputHook(std::function<void(int,int)> mouseMotion, std::function<void(int,int,int,int)> mouse, std::function<void(unsigned char,int,int)> keyboard) {
			mouseMotionBackup = draw::getMouseMotionCallback();
			mouseBackup = draw::getMouseCallback();
			keyboardBackup = draw::getKeyboardCallback();

			draw::setMouseMotionCallback(mouseMotion);
			draw::setMouseCallback(mouse);
			draw::setKeyboardCallback(keyboard);
		}

		// restore input-hook
		void unsetInputHook() {
			draw::setMouseMotionCallback(mouseMotionBackup);
			draw::setMouseCallback(mouseBackup);
			draw::setKeyboardCallback(keyboardBackup);
		}

		// draw airstrike warning
		void drawWarning() {
			const double sizex = 0.10;
			const double sizey = 0.05;
			const double gap = 0.005;
			draw::rect(new vec(-sizex+0.1-gap,sizey+gap), new vec(sizex+0.1+gap,-sizey+0.02-gap), new color(0,0,0,0.8*fabs(tfade)));
			draw::rect(new vec(-sizex+0.1,sizey), new vec(sizex+0.1,-sizey+0.02), new color(0.7,0,0,0.7*fabs(tfade)));
			draw::print(new vec(-sizex + 0.12, 0), "Airstrike!", new color(0.8,0.9,0.9,fabs(tfade)));
			if(tfade >= 1) tfade = -1;
			tfade+=0.03;
		}

	public:
		airstrike() {
			draw::addRenderCallback(std::bind(&airstrike::draw, this), (void*)&t, draw::LOW);
			setInputHook(std::bind(&airstrike::mouseMotion, this, std::placeholders::_1, std::placeholders::_2),
					std::bind(&airstrike::mouse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
					std::bind(&airstrike::keyboard, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			// set initial position
			if(rand()%2) {
				y = 1.2;
				x = randd(-0.2,0.7);
				step = -randd(0.3, 0.5+x);
			}
			else {
				x = 1.2;
				y = randd(-0.2,0.7);
				step = -randd(0.6,0.8+y);
			}

			oldspeed = draw::speed;
			draw::speed = 1;
			grid::paused = true;
		}
		~airstrike() {
			draw::delRenderCallback((void*)&t);
			unsetInputHook();
			new explosion(new vec(x+step,y+step), 4*grid::sty, 0.03);
			areaDamage();
			grid::paused = false;
			draw::speed = oldspeed;
		}

		void draw() {
			if(draw::debugMode) fprintf(stderr, "Callback: airstrike()\n");
			vec *rainbow = new vec(x+step*t, y+step*t);
			vec *trail = new vec(x+step*(t-0.05), y+step*(t-0.05));
			draw::tex(rainbow, 8*grid::sty*(1-t), tex::PONY_RAINBOW_DASH_FLYING);
			new fadeTex(trail, tex::RAINBOW_DASH_TRAIL, 2*(1-t)*grid::sty, 100);
			t+=0.006;
			drawWarning();
			if(t > 1) delete this;
		}

		void mouseMotion(int x, int y) {}
		void mouse(int button, int state, int xc, int yc) {}
		void keyboard(unsigned char key, int x, int y) {}
};
