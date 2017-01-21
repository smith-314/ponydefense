// menu.cpp
// draw messages, questions or menus


// basic class to draw all kind of messages
// inherited by message, question and menu class
class menuBase {
	private:
		// input hook backup
		std::function<void(int,int)> mouseMotionBackup;
		std::function<void(int,int,int,int)> mouseBackup;
		std::function<void(unsigned char,int,int)> keyboardBackup;

		// returns number of string till NULL
		int getLines(char **text) {
			if(text == NULL) return 0;
			char **ptr = text;
			int lines = 0;
			while(ptr[lines] != NULL) lines++;
			return lines;
		}

	public:
		// copies string to heap
		char *copyStr(const char *str) {
			if(str == NULL) return NULL;
			char *ptr = new char[strlen(str)+1];
			strcpy(ptr, str);
			return ptr;
		}

		// copies string-array to heap
		char **copyStrArray(char **str) {
			if(str == NULL) return NULL;
			int lines = getLines(str);
			char **ptr = new char*[lines+1];
			for(int i = 0; i < lines; i++)
				ptr[i] = copyStr(str[i]);
			ptr[lines] = NULL;
			return ptr;
		}

		// deletes string-array from heap
		void deleteStrArray(char **str) {
			int lines = getLines(str);
			for(int i = 0; i < lines; i++) delete[] str[i];
			delete[] str;
		}

		// returns (y-)size of a message element
		double getElementLength(char **text) {
			return getLines(text)*0.04 + 0.07;
		}

		// returns relative length of a text
		double getTextLength(const char *text) {
			return (1.0/60)*strlen(text);
		}

		// takeover of the input hook
		void setInputHook(std::function<void(int,int)> mouseMotion, std::function<void(int,int,int,int)> mouse,
				std::function<void(unsigned char,int,int)> keyboard) {
			// backup old input hook
			mouseMotionBackup = draw::getMouseMotionCallback();
			mouseBackup = draw::getMouseCallback();
			keyboardBackup = draw::getKeyboardCallback();

			// set new input hook
			draw::setMouseMotionCallback(mouseMotion);
			draw::setMouseCallback(mouse);
			draw::setKeyboardCallback(keyboard);
		}

		// revert backup
		void unsetInputHook() {
			draw::setMouseMotionCallback(mouseMotionBackup);
			draw::setMouseCallback(mouseBackup);
			draw::setKeyboardCallback(keyboardBackup);
		}

		// simply draw a frame with title, text and texture
		void drawMessage(const char *title, char **text, int texID, vec *pos, double sizex,
				double t = 1, bool disabled = false, bool selected = false) {
			int lines = getLines(text);
			double sizey = lines*0.04 + 0.07;

			// set coordinates
			vec *vFrame1 = new vec(pos->x, pos->y);
			vec *vFrame2 = new vec(pos->x + sizex, pos->y - sizey);
			vec *vTitle = new vec(pos->x + 0.02, pos->y - 0.045);
			vec *vTex;
			if(texID != 0) {
				vTitle->x+= sizey*(1/draw::ar)-0.01;
				vTex = new vec(pos->x+0.5*sizey*(1/draw::ar), pos->y - sizey/2);
			}

			// set colors
			color *cFrame = new color(0,0,0,0.5*t);
			color *cTitle = new color(1,1,1,1.0*t);
			color *cText = new color(1,1,1,0.7*t);

			// render everything
			draw::rect(vFrame1->clone(), vFrame2->clone(), cFrame);

			if(selected) draw::rect(vFrame1->clone(), vFrame2->clone(), new color(1,1,1,0.1*t));
			draw::print(vTitle, title, cTitle);
			if(texID != 0) draw::tex(vTex, sizey-0.025, texID);
			for(int i = 0; i < getLines(text); i++) {
				vec *vText = new vec(pos->x + 0.02, pos->y - 0.085 -i*0.04);
				if(texID != 0) vText->x+=sizey*(1/draw::ar)-0.01;
				draw::print(vText, text[i], cText->clone());
			}
			if(disabled) draw::rect(vFrame1->clone(), vFrame2->clone(), new color(0,0,0,0.2*t));
			delete vFrame1; delete vFrame2; delete cText;
		}

		// partial rendering to make a build/destroy effect
		void drawBuild(char **text, vec *pos, double sizex, double build) {
			int lines = getLines(text);
			double sizey = lines*0.04 + 0.07;
			vec *vFrame1 = new vec(pos->x, pos->y);
			vec *vFrame2 = new vec(pos->x + sizex, pos->y - sizey*build);
			draw::rect(vFrame1, vFrame2, new color(0,0,0,0.5));
		}
};

// class to draw a simple message
class message: menuBase {
	private:
		char *title, **text;
		unsigned int ticks; // remaining render ticks
		unsigned int maxTicks; // initial render ticks value
		int texID;
		double t = 0; // transparency
		vec *pos;

	public:
		// position
		enum {TOP, MIDDLE, BOTTOM};

		// save message and set callback
		message(const char *_title, char **_text, int _tex, int _pos, unsigned int _ticks) {
			title = copyStr(_title);
			text = copyStrArray(_text);
			ticks = maxTicks = _ticks;
			texID = _tex;
			pos = new vec(-0.225,0.095);
			if(_pos == BOTTOM) pos->y-=0.8;
			if(_pos == TOP) pos->y+=0.8;

			draw::addRenderCallback(std::bind(&message::draw, this), (void*)pos, draw::LOW);
		}

		// delete message and callback
		~message() {
			draw::delRenderCallback((void*)pos);
			delete[] title;
			if(text != NULL) deleteStrArray(text);
			delete pos;
		}

		// render callback
		void draw() { 
			if(!draw::enableDraw) return;
			// fade in/out after timeout
			if(ticks == 0) {
				delete this;
				return;
			}
			ticks--;
			if(ticks > maxTicks-15)
				t+=1.0/15;
			else if (ticks < 15)
				t-=1.0/15;

			// render
			drawMessage(title, text, texID, pos, 0.45, t);
		}
};

// class to draw a question
class question: menuBase {
	private:
		char *title, **text, *ans1, *ans2;
		color *cselected, *cbackground;
		vec *pos, *vAns11, *vAns12, *vAns21, *vAns22, *vAns1, *vAns2;
		int texID; // texture
		double t = 0; // transparency
		unsigned int ticks = 0; // ticks left
		int sel = 0; // selection (from mouse position)
		std::function<void(int)> resultCallback;

	public:
		// save message and set callback
		question(const char *_title, char **_text, const char *_ans1, const char *_ans2,
				int tex, std::function<void(int)> resCall) {
			title = copyStr(_title);
			ans1 = copyStr(_ans1);
			ans2 = copyStr(_ans2);
			text = copyStrArray(_text);
			resultCallback = resCall;
			texID = tex;

			// set colors
			cselected = new color(0.2,0.2,0.2,0.5);
			cbackground = new color(0,0,0,0.4);

			// set coordinates
			pos = new vec(-0.225,0.11);
			double y = pos->y - getElementLength(text);
			vAns11 = new vec(-0.225, y);
			vAns12 = new vec(0, y - 0.06);
			vAns21 = new vec(0, y);
			vAns22 = new vec(0.225, y - 0.06);
			vAns1 = new vec(-(0.225+getTextLength(ans1))/2.0, y - 0.04);

			// answer position depending on number of answers
			if(ans2 != NULL) vAns2 = new vec((0.225-getTextLength(ans2))/2.0, y - 0.04);
			else vAns1->x = -getTextLength(ans1)/2.0;

			// set callbacks
			draw::addRenderCallback(std::bind(&question::draw, this), (void*)title, draw::LOWEST);
			setInputHook(std::bind(&question::mouseMotion, this, std::placeholders::_1, std::placeholders::_2),
					std::bind(&question::mouse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
						std::placeholders::_4), std::bind(&question::keyboard, this, std::placeholders::_1,
							std::placeholders::_2, std::placeholders::_3));
		}

		// delete message and callbacks
		~question() {
			draw::delRenderCallback((void*)title);
			if(text != NULL) deleteStrArray(text);
			delete[] title; delete[] ans1;
			if(ans2 != NULL) {
				delete[] ans2;
				delete vAns2;
			}
			delete cselected; delete cbackground; delete pos;
			delete vAns11; delete vAns12; delete vAns21;
			delete vAns22; delete vAns1;
		}

		// render callback
		void draw() { 
			if(!draw::enableDraw) return;
			// apply fade-in/out
			if (ticks < 15) {
				t+=1.0/15;
			}
			ticks++;

			// draw question
			draw::rect(new vec(draw::relx, draw::rely), new vec(-draw::relx, -draw::rely), new color(0,0,0,0.5));
			drawMessage(title, text, texID, pos, 0.45, t);

			// draw answer
			if(ans2 == NULL) {
				if(sel != 0) draw::rect(vAns11->clone(), vAns22->clone(), cselected->clone());
				else draw::rect(vAns11->clone(), vAns22->clone(), new color(0,0,0,0.5*t));
				draw::print(vAns1->clone(), ans1, new color(1,1,1,0.7*t));
			}
			else {
				if(sel == 1) draw::rect(vAns11->clone(), vAns12->clone(), cselected->clone());
				else draw::rect(vAns11->clone(), vAns12->clone(), new color(0,0,0,0.5*t));
				if(sel == 2) draw::rect(vAns21->clone(), vAns22->clone(), cselected->clone());
				else draw::rect(vAns21->clone(), vAns22->clone(), new color(0,0,0,0.5*t));
				draw::print(vAns1->clone(), ans1, new color(1,1,1,0.7*t));
				draw::print(vAns2->clone(), ans2, new color(1,1,1,0.7*t));
			}
		}

		// calculate selection from mouse position
		void mouseMotion(int x, int y) {
			double rx, ry;
			rx = draw::relX(x);
			ry = draw::relY(y);
			if(rx > vAns11->x && rx < vAns12->x && ry < vAns11->y && ry > vAns12->y)
				sel = 1;
			else if(rx > vAns21->x && rx < vAns22->x && ry < vAns21->y && ry > vAns22->y)
				sel = 2;
			else sel = 0;
		}

		// response to mouse click
		void mouse(int button, int state, int xc, int yc) {
			mouseMotion(xc, yc); // refresh position
			if(button == 0 && state == 1 && sel) { // left-click
				// send result
				unsetInputHook(); // called first so callbacks can create menus
				if(ans2 == NULL) resultCallback(1);
				else resultCallback(sel);
				delete this;
			}
		}

		// response to keyboard
		void keyboard(unsigned char key, int x, int y) {
			if(key == 27) { // ESC key
				unsetInputHook();
				resultCallback(-1);
				delete this;
			}
			if(key == 13) { // enter key
				unsetInputHook();
				resultCallback(1);
				delete this;
			}
		}
};

// class to draw a menu entry
class menuEntry: menuBase {
	private:
		char *title, **text;
		int texID, ticks = 0, fading;
		bool disabled, hidden = false, sel = false;
		bool subSel = false, shutdown = false;
		double xsize;
		class menuEntry *parent;
		vec *pos1, *pos2;

	public:
		class menuEntry *sub = NULL, *next = NULL; // submenu and entry chain
		int result; // value to identify decision

		// copy message
		menuEntry(const char *_title, char **_text, int _texID, int _result, bool _disabled = false,
				double _xsize = 0.4, class menuEntry *_parent = NULL, int _fading = 5) {
			title = copyStr(_title);
			text = copyStrArray(_text);
			texID = _texID;
			xsize = _xsize;
			disabled = _disabled;
			result = _result;
			parent = _parent;
			fading = _fading;
		}

		// delete message and callback
		~menuEntry() {
			draw::delRenderCallback((void*)title);
			delete[] title;
			if(text != NULL) deleteStrArray(text);
			delete pos1; delete pos2;
		}

		// calculate position and set callback
		void init(vec *_pos) {
			pos1 = _pos->clone();
			pos2 = new vec(pos1->x + xsize, pos1->y - getElementLength(text));
			draw::addRenderCallback(std::bind(&menuEntry::draw, this), (void*)title, draw::LOW);
		}

		// renderr callback
		void draw() { 
			if(!draw::enableDraw) return;
			// delete after close animation
			if(ticks == -1 && shutdown) {
				delete this;
				return;
			}

			// if hidden - render nothing
			if(hidden) {
				if(shutdown) ticks++;
				return;
			}

			// apply fade in/out
			if(ticks >= 0 && ticks < fading) {
				drawBuild(text, pos1, xsize, (1.0/fading)*ticks);
				ticks++;
				return;
			}
			else if(ticks < 0) {
				if(ticks == -1) hidden = true;
				drawBuild(text, pos1, xsize, (-1.0/fading)*ticks);
				ticks++;
				return;
			}

			// show entry 
			drawMessage(title, text, texID, pos1, xsize, 1.0, disabled, sel | subSel);
		}

		// calculate selection from mouse position
		bool isSelected(double rx, double ry) {
			if(hidden) return 0;
			if(rx >= pos1->x && rx <= pos2->x && ry <= pos1->y && ry >= pos2->y)
				return 1;
			return 0;
		}

		// animate hiding (submenu)
		void hide() {
			ticks = -fading;
		}

		// hide immediately (submenu init)
		void hideNow() {
			hidden = true;
		}

		// animate fade in
		void show() {
			hidden = false;
			ticks = 0;
		}

		// hide and shutdown
		void kill() {
			shutdown = true;
			ticks = -fading;
			killAll(sub);
		}

		// delete all entries
		void killAll(class menuEntry *ptr) {
			class menuEntry *ent = ptr;
			while(ent != NULL) {
				if(ent->sub != NULL) killAll(ent->sub);
				ent->kill();
				ent = ent->next;
			}
		}


		// if selected highlight entry
		void selected() {
			// inform parent entry
			if(parent != NULL) parent->subSelected();

			if(disabled) return;
			sel = true;

			// show submenu (if available)
			if(sub != NULL) {
				class menuEntry *it = sub;
				do {
					it->show();
					it = it->next;
				} while(it != NULL);
			}
		}

		void unselected(class menuEntry *newParent) {
			sel = false;

			// hide submenu
			if(sub != NULL && !subSel) {
				class menuEntry *it = sub;
				do {
					it->hide();
					it = it->next;
				} while(it != NULL);
			}

			// inform parent entry
			if(parent != NULL && parent != newParent) {
				parent->subUnselected();
			}
		}

		// if subentry is selected
		void subSelected() {
			subSel = true;
		}

		// if subentry isn't selected anymore
		void subUnselected() {
			subSel = false;

			// hide submenu
			if(sub != NULL) {
				class menuEntry *it = sub;
				do {
					it->hide();
					it = it->next;
				} while(it != NULL);
			}
		}

		// return (y-)size of the entry
		double getSize() {
			return getElementLength(text);
		}

		// return (y-)position of the entry
		double getYPos() {
			return pos1->y;
		}

		// return disabled state
		bool isDisabled() {
			if(disabled) return true;
			return false;
		}

		// return parent entry
		class menuEntry *getParent() {
			return parent;
		}
};

// class to draw a complete menu
class menu: menuBase {
	private:
		char *title;
		class menuEntry *entries = NULL;
		class menuEntry *sel = NULL, *lastSel = NULL; // last selected entry 
		vec *pos;
		double sizex, frameX = 0.04, frameY = 0.01;
		int ticks = 0, fading;
		std::function<void(int)> resultCallback;

		// return (x-)size of the menu
		double getMenuSizeX() {
			class menuEntry *ent = entries;
			while(ent != NULL) {
				if(ent->sub != NULL) { // only one submenu allowed
					return 2*sizex;
				}
				ent = ent->next;
			}
			return sizex;
		}

		// return (y-)size of the menu
		double getMenuSizeY() {
			double yMain = 0, ySub = 0;
			class menuEntry *ent = entries;
			while(ent != NULL) {
				if(ent->sub != NULL) {
					double ycmp = yMain;
					class menuEntry *sub = ent->sub;
					while(sub != NULL) {
						ycmp+=sub->getSize();
						sub = sub->next;
					}
					if(ySub < ycmp) ySub = ycmp;
				}
				yMain+=ent->getSize();
				ent = ent->next;
			}
			if(ySub > yMain) return ySub;
			else return yMain;
		}

		// return (y-)size of the parent menu
		double getMainMenuSizeY() {
			double y = 0;
			class menuEntry *ent = entries;
			while(ent != NULL) {
				y+=ent->getSize();
				ent = ent->next;
			}
			return y;
		}

		// return last entry in list
		class menuEntry *getLastEntry(class menuEntry *ptr) {
			if(ptr == NULL) return NULL;
			class menuEntry *ent = ptr;
			while(ent->next != NULL) ent = ent->next;
			return ent;
		}

		// search for selected entry
		class menuEntry *_searchEntriesSel(class menuEntry *ptr, double rx, double ry) {
			class menuEntry *ent = ptr;
			do {
				if(ent->isSelected(rx, ry)) return ent;
				if(ent->sub != NULL) {
					class menuEntry *tmp = _searchEntriesSel(ent->sub, rx, ry);
					if(tmp != NULL) return tmp;
				}
				ent = ent->next;
			} while(ent != NULL);
			return NULL;
		}

		// return selected entry
		class menuEntry *getSelEntry(int x, int y) {
			if(entries == NULL) return NULL;
			double rx = draw::relX(x);
			double ry = draw::relY(y);
			return _searchEntriesSel(entries, rx, ry);
		}

		// delete all entries
		void killAll(class menuEntry *ptr) {
			class menuEntry *ent = ptr;
			while(ent != NULL) {
				if(ent->sub != NULL) killAll(ent->sub);
				ent->kill();
				ent = ent->next;
			}
		}

	public:
		// copy message and set callback
		menu(const char *_title, vec *_pos, double _sizex, std::function<void(int)> result, int _fading = 5) {
			title = copyStr(_title);
			pos = _pos;
			sizex = _sizex;
			resultCallback = result;
			fading = _fading;
			draw::addRenderCallback(std::bind(&menu::draw, this), (void*)title, draw::LOWEST);
			setInputHook(std::bind(&menu::mouseMotion, this, std::placeholders::_1, std::placeholders::_2),
					std::bind(&menu::mouse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
						std::placeholders::_4), std::bind(&menu::keyboard, this, std::placeholders::_1,
							std::placeholders::_2, std::placeholders::_3));
		}

		// delete message and callback
		~menu() {
			killAll(entries);
			draw::delRenderCallback((void*)title);
			delete[] title;
			delete pos;
		}

		// add (parent) menu entry
		void addEntry(const char *_title, char **_text, int _texID, int _result, bool _disabled = false) {
			class menuEntry *last = getLastEntry(entries);
			if(last == NULL) {
				entries = new menuEntry(_title, _text, _texID, _result, _disabled, sizex, NULL, fading);
			}
			else {
				last->next = new menuEntry(_title, _text, _texID, _result, _disabled, sizex, NULL, fading);
			}
		}

		// add submenu entry to last parent entry
		void addSubEntry(const char *_title, char **_text, int _texID, int _result, bool _disabled = false) {
			class menuEntry *root = getLastEntry(entries);
			if(root != NULL) {
				if(root->sub == NULL) {
					root->sub = new menuEntry(_title, _text, _texID, _result, _disabled, sizex, root, fading);
					root->sub->hideNow();
				}
				else {
					class menuEntry *last = getLastEntry(root->sub);
					if(last == NULL) return;
					last->next = new menuEntry(_title, _text, _texID, _result, _disabled, sizex, root, fading);
					last->next->hideNow();
				}
			}
		}

		// calculate size of all entries and draw menu
		void showMenu() {
			// add space for title
			pos->x+=frameY;
			pos->y-=frameX;

			// move if menu doesn't fit into window
			if(pos->x + getMenuSizeX() > draw::relx) pos->x = draw::relx-getMenuSizeX();
			if(pos->y - getMenuSizeY() < -draw::rely) pos->y = -draw::rely+getMenuSizeY();

			// pass position to entries
			class menuEntry *ent = entries;
			vec *v = pos->clone();
			while(ent != NULL) {
				ent->init(v);
				if(ent->sub != NULL) {
					class menuEntry *sub = ent->sub;
					vec *vSub = new vec(v->x+sizex, v->y);
					while(sub != NULL) {
						sub->init(vSub);
						vSub->y-=sub->getSize();
						sub = sub->next;
					}
					delete vSub;
				}
				v->y-=ent->getSize();
				ent = ent->next;
			}
			delete v;
		}

		// render callback (title frame)
		void draw() { 
			if(!draw::enableDraw) return;
			color *cframe = new color(0,0,0,0.55);

			double sizex = 0.25; // relative size of the triangle
			// fade in/out
			if(ticks < fading) {
				if (ticks < fading/2) {
					draw::rect(new vec(pos->x-frameY, pos->y),
							new vec(pos->x+(0.19/(fading/2))*ticks, pos->y+frameX), cframe->clone());
				}
				else {
					draw::rect(new vec(pos->x-frameY, pos->y), new vec(pos->x+0.19, pos->y+frameX),
							cframe->clone());
					draw::triangle(new vec(pos->x+0.19, pos->y), new vec(pos->x+0.19, pos->y+frameX),
							new vec(pos->x+(sizex/fading)*ticks, pos->y), cframe->clone());
				}
				draw::rect(pos->clone(), new vec(pos->x-frameY, pos->y-(getMainMenuSizeY()/fading)*ticks),
						cframe->clone());
				draw::print(new vec(pos->x-0.005, pos->y+0.01), title, new color(1,1,1,(0.7/fading)*ticks));
				ticks++;
				delete cframe;
				return;
			}

			// render menu frame
			draw::rect(new vec(pos->x-frameY, pos->y), new vec(pos->x+0.19, pos->y+frameX), cframe->clone());
			draw::triangle(new vec(pos->x+0.19, pos->y), new vec(pos->x+0.19, pos->y+frameX),
					new vec(pos->x+sizex, pos->y), cframe->clone());
			draw::rect(pos->clone(), new vec(pos->x-frameY, pos->y-getMainMenuSizeY()), cframe->clone());
			draw::print(new vec(pos->x-0.005, pos->y+0.01), title, new color(1,1,1,0.7));
			delete cframe;
		}

		// get selected entry from mouse position
		void mouseMotion(int x, int y) {
			lastSel = sel;
			sel = getSelEntry(x, y);

			// if nothing selected - inform last entry
			if(sel == NULL) {
				if(lastSel != NULL) lastSel->unselected(NULL);
			}

			// inform entry about selection
			else {
				if(sel != lastSel) {
					sel->selected();
					if(lastSel != NULL) lastSel->unselected(sel->getParent());
				}
			}
		}

		// respond to mouse click
		void mouse(int button, int state, int xc, int yc) {
			mouseMotion(xc, yc); // refresh position
			// left click
			if(button == 0 && state == 1) {
				if(sel != NULL && sel->sub == NULL && !sel->isDisabled()) {
					unsetInputHook();
					resultCallback(sel->result);
					delete this;
				}
				else if(sel == NULL) {
					// nothing selected - close menu
					unsetInputHook();
					resultCallback(-1);
					delete this;
				}

			}
			// right click - close menu
			else if(button == 2 && state == 1) {
				unsetInputHook();
				resultCallback(-1);
				delete this;
			}
		}

		// respond to keyboard event
		void keyboard(unsigned char key, int x, int y) {
			if(key == 27) { // ESC key
				unsetInputHook();
				resultCallback(-1);
				delete this;
			}
		}
};
