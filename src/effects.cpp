// effects.cpp
// classes for visual effects


// draw weapon tracer
class tracer {
	private:
		TOWERTYPE type;
		UPGRTYPE upgrade;
		double t = 1, speed, size;
		vec *p1, *p2;

	public:
		tracer(TOWERTYPE _type, UPGRTYPE _upgrade, vec *_p1, vec *_p2) {
			draw::addRenderCallback(std::bind(&tracer::draw, this), (void*)&type, draw::HIGH);
			type = _type; p1 = _p1; p2 = _p2; upgrade = _upgrade;
			if(type == RIFLE) {
				size = 1.0; speed = 0.5;
				if(upgrade == UPGRADE1) size = 1.5;
				if(upgrade == UPGRADE2) size = 2.0;
			}
			else if(type == RAILGUN) {
				size = 2.5; speed = 0.25;
				if(upgrade == UPGRADE1) size = 4;
				if(upgrade == UPGRADE2) size = 6;
			}
			else if(type == LASER) {
				size = 2.5; speed = 1;
			}
			else if(type == LIGHTNING) {
				size = 2; speed = 0.1;
			}
			else {
				delete this;
			}
		}
		~tracer() {
			draw::delRenderCallback((void*)&type);
			delete p1; delete p2;
		}
		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: tracer()\n");

			if(type == RIFLE) draw::line(p1->clone(), p2->clone(), new color(0.5,0.5,0.5,t), size);
			if(type == RAILGUN) {
				draw::line(p1->clone(), p2->clone(), new color(1,0.5,0,0.25*t), size*2);
				draw::line(p1->clone(), p2->clone(), new color(0.3,0.3,0.3,t), size);
			}
			if(type == LASER) {
				if(upgrade == BASE) draw::line(p1->clone(), p2->clone(), new color(1,0,0,t), size);
				if(upgrade == UPGRADE1) draw::line(p1->clone(), p2->clone(), new color(0,1,0,t), size);
				if(upgrade == UPGRADE2) draw::line(p1->clone(), p2->clone(), new color(0,0,1,t), size);
			}
			if(type == LIGHTNING) {
				draw::line(p1->clone(), p2->clone(), new color(0,0.4,1.0,0.5*t), size);
			}
			t-=speed;
			if(t <= 0) delete this;
		}
};

// draw simple explosion
class explosion {
	private:
		vec *pos;
		double size, speed;
		double t = 1, t2 = 1, t3 = 1;

	public:
		explosion(vec *_pos, double _size, double _speed) {
			pos = _pos; size = _size; speed = _speed;
			draw::addRenderCallback(std::bind(&explosion::draw, this), (void*)&size);
		}

		~explosion() {
			draw::delRenderCallback((void*)&size);
			delete pos;
		}

		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: explosion()\n");

			draw::tex(pos->clone(), 2*(size - t*size), tex::FLAMES, 0, t);
			t-=speed;
			if(t <= 0) delete this;
		}
};

// draw flamethrower effect
class flames {
	private:
		vec *pos1, *pos2, *diff;
		double t = 0;

	public:
		flames(vec *_pos1, vec *_pos2) {
			pos1 = _pos1; pos2 = _pos2;
			diff = new vec(pos2->x - pos1->x, pos2->y - pos1->y);
			draw::addRenderCallback(std::bind(&flames::draw, this), (void*)&t);
		}

		~flames() {
			draw::delRenderCallback((void*)&t);
			delete pos1; delete pos2; delete diff;
		}

		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: flames()\n");

			t+=0.04;
			if(draw::speed != 1) {
				t+=0.04*draw::speed*draw::speed; // prevent performance overkill
			}
			if(t >= 1) delete this;
			else {
				vec *v = new vec(pos1->x + (diff->x)*t, pos1->y + (diff->y)*t);
				new fadeTex(v, tex::FLAMES, t*t*grid::sty, 5);
			}
		}
};

// draw radar effect
class radsignal {
	private:
		vec *pos;
		double size, t = 0;
		color *c;
		const double step = 0.04;

	public:
		radsignal(vec *_pos, double _size, color *_c = new color()) {
			pos = _pos; size = _size; c = _c;
			draw::addRenderCallback(std::bind(&radsignal::draw, this), (void*)&t);
			c->t = 0.5;
		}

		~radsignal() {
			draw::delRenderCallback((void*)&t);
			delete pos; delete c;
		}

		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: radsignal()\n");

			draw::fullCircle(pos->clone(), 0.6*t*size, c->clone());
			t+=step;
			c->t-=step/2;
			if(t > 1) delete this;
		}
};

// draw blood
class blood {
	private:
		double t = 1, rot;
		vec *pos;
		int shield;

	public:
		blood(class regPony *py, double _rot, TOWERTYPE) {
			draw::addRenderCallback(std::bind(&blood::draw, this), (void*)&t, draw::LOW);
			pos = py->pos->clone(); rot = _rot;
			shield = py->py->hasShield();
		}
		~blood() {
			draw::delRenderCallback((void*)&t);
			delete pos;
		}

		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: blood()\n");

			if(shield) draw::tex(pos->clone(), 0.75*grid::sty, tex::BLOOD_SHIELD, rot);
			else {
				draw::tex(pos->clone(), 0.75*grid::sty, tex::BLOOD_DEFAULT, rot);
			}
			t-=0.2;
			if(t <= 0) delete this;
		}
};

// background ponies in main menu
class backgroundPonies {
	private:
		vec *p[14];
		bool stats;

		void setRand(vec *v) {
			if(rand()%2) {
				v->x = -draw::relx;
				v->y = randd(-draw::rely,draw::rely);
			}
			else {
				v->y = -draw::rely;
				v->x = randd(-draw::relx,draw::relx);
			}
		}

	public:
		backgroundPonies(bool _stats = true) {
			draw::addRenderCallback(std::bind(&backgroundPonies::draw, this), (void*)&p, draw::HIGH);
			stats = _stats;
			for(int i = 0; i < 14; i++) {
				p[i] = new vec();
				setRand(p[i]);
			}
		}
		~backgroundPonies() {
			draw::delRenderCallback((void*)&p);
			for(int i = 0; i < 14; i++) delete p[i];
		}

		void draw() { 
			if(draw::debugMode) fprintf(stderr, "Callback: backgroundPonies()\n");

			for(int i = 0; i < 14; i++) {
				p[i]->x+=0.007;
				p[i]->y+=0.007;
				if(p[i]->x > draw::relx || p[i]->y  > draw::rely)
					setRand(p[i]);
			}
			for(int i = 0; i < 2; i++) {
				draw::tex(p[0+i*7]->clone(), 0.2, tex::PONY_RARITY);
				draw::tex(p[1+i*7]->clone(), 0.2, tex::PONY_APPLEJACK);
				draw::tex(p[2+i*7]->clone(), 0.2, tex::PONY_FLUTTERSHY);
				draw::tex(p[3+i*7]->clone(), 0.2, tex::PONY_PINKIE_PIE);
				draw::tex(p[4+i*7]->clone(), 0.2, tex::PONY_RAINBOW_DASH);
				draw::tex(p[5+i*7]->clone(), 0.2, tex::PONY_TWILIGHT_SPARKLE);
				draw::tex(p[6+i*7]->clone(), 0.2, tex::PONY_TROJAN);
			}

			// draw rank window
			if(stats) stats::showRank();
		}

		void enableStats() {
			stats = true;
		}
};
