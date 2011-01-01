#include <iostream>
#include <utility>
#include <queue>
#include <vector>
#include <cmath>
#include <SDL/SDL.h>

#define PI 3.14159265
#define ELEM_SIZE 20

using namespace std;

int key; //1 => Left, 2 => Right, 3 => Up, 4 => Down
int alive;

class Element;

vector< pair< pair<int, int>, Element* > > snake;

void putpixel(SDL_Surface *, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t);
void drawcircle(SDL_Surface *, uint16_t, uint16_t, double, int16_t, int16_t, uint8_t, uint8_t, uint8_t);
void drawrectangle(SDL_Surface *, uint16_t, uint16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t);
int move(void *);

class Element
{
	public:
		Element() : pos(0, 0), prev(-1, -1), screen(NULL) {}
		Element(SDL_Surface * scr, pair<int, int> p) : pos(p), prev(-1, -1), screen(scr) {}

		pair<int, int> getPos() const { return this->pos; }
		void setPos(pair<int, int> p) { pos = p; }
		void setScreen(SDL_Surface * s) { screen = s; }

		virtual Element& Move()
		{
			if(prev.first != -1)
				drawcircle(screen, prev.first, prev.second, (ELEM_SIZE / 2), 0, 0, 0x00, 0x00, 0x00);

			drawcircle(screen, pos.first, pos.second, (ELEM_SIZE / 2), 0, 0, 0xff, 0x00, 0x00);
			prev = pos;

			return *this;
		}

		Element& Move(pair<int, int> p)
		{
			setPos(p);
			return Move();
		}
	protected:
		pair<int, int> pos;
		pair<int, int> prev;
		SDL_Surface * screen;
};

class Head : public Element
{
	public:
		Head() : Element() {}
		Head(SDL_Surface * scr, pair<int, int> p) : Element(scr, p)  {}

		virtual Element& Move()
		{
			if(prev.first != -1)
				drawrectangle(screen, prev.first, prev.second, ELEM_SIZE, ELEM_SIZE, 0x00, 0x00, 0x00);

			drawrectangle(screen, pos.first, pos.second, ELEM_SIZE, ELEM_SIZE, 0xff, 0x00, 0x00);
			prev = pos;

			return *this;
		}
};

int main()
{
	SDL_Surface * screen = NULL;
	SDL_Event e;
	SDL_Thread * move_t = NULL;
	int pressed = 0;
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "Can't Initialize SDL: " << SDL_GetError() << endl;
		exit(1);
	}

	atexit(SDL_Quit);
 
	screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
	if(screen == NULL)
	{
		cout << "Can't use video mode 640x480: " << SDL_GetError() << endl;
		exit(1);
	}

	snake.push_back(pair< pair<int, int>, Element* >(pair<int, int>(-1, -1), new Head(screen, pair<int, int>(200, 50))));
	for(int i = 180; i > 0; i -= 20)
		snake.push_back(pair< pair<int, int>, Element* >(pair<int, int>(i + 20, 50), new Element(screen, pair<int, int>(i, 50))));

	key = 2;
	alive = 1;
	move_t = SDL_CreateThread(move, static_cast<void *>(screen));

	while((!pressed) || alive)
	{
        	while(SDL_PollEvent(&e))
        	{
            	switch(e.type)
            	{
                	case SDL_QUIT:
                    		pressed = 1;
                    		alive = 0;
                    	break;
 
                	case SDL_KEYDOWN:
                	{
                    		switch(e.key.keysym.sym)
                    		{
                        		case SDLK_UP:
                            			key = 3;
                            		break;

                        		case SDLK_DOWN:
                            			key = 4;
                            		break;

                        		case SDLK_LEFT:
                            			key = 1;
                            		break;

                        		case SDLK_RIGHT:
                            			key = 2;
                            		break;
                    		}

                		break;
                	} 
            	}
        	}
    		SDL_Delay(50);
    	}

	SDL_WaitThread(move_t, NULL);

	for(int i = 0; i < snake.size(); i++)
		delete(snake[i].second);

	return 0;
}

int move(void * p)
{
	SDL_Surface * screen = static_cast<SDL_Surface *>(p);

	cout << "Move thread starting" << endl;

	while(alive)
	{
		switch(key)
		{
			case 1:
				if(snake[0].second->getPos().first > ELEM_SIZE)
				{
					pair<int, int> coord = snake[0].second->getPos();
					snake[0].second->Move();

					for(int i = 1; i < snake.size(); i++)
					{
						snake.at(i).second->Move();
						snake.at(i).second->setPos(snake.at(i).first);
						snake.at(i).first = ((i - 1) == 0) ? make_pair(coord.first - ELEM_SIZE, coord.second) : snake.at(i - 1).second->getPos();
					}

					snake[0].second->setPos(make_pair(coord.first - ELEM_SIZE, coord.second));
				}
			break;

			case 2:
				if(snake[0].second->getPos().first < (screen->w - ELEM_SIZE))
				{
					pair<int, int> coord = snake[0].second->getPos();
					snake[0].second->Move();

					for(int i = 1; i < snake.size(); i++)
					{
						snake.at(i).second->Move();
						snake.at(i).second->setPos(snake.at(i).first);
						snake.at(i).first = ((i - 1) == 0) ? make_pair(coord.first + ELEM_SIZE, coord.second) : snake.at(i - 1).second->getPos();
					}

					snake[0].second->setPos(make_pair(coord.first + ELEM_SIZE, coord.second));
				}
			break;

			case 3:
				if(snake[0].second->getPos().second > ELEM_SIZE)
				{
					pair<int, int> coord = snake[0].second->getPos();
					snake[0].second->Move();

					for(int i = 1; i < snake.size(); i++)
					{
						snake.at(i).second->Move();
						snake.at(i).second->setPos(snake.at(i).first);
						snake.at(i).first = ((i - 1) == 0) ? make_pair(coord.first, coord.second - ELEM_SIZE) : snake.at(i - 1).second->getPos();
					}

					snake[0].second->setPos(make_pair(coord.first, coord.second - ELEM_SIZE));
				}
			break;

			case 4:
				if(snake[0].second->getPos().second < (screen->h - ELEM_SIZE))
				{
					pair<int, int> coord = snake[0].second->getPos();
					snake[0].second->Move();

					for(int i = 1; i < snake.size(); i++)
					{
						snake.at(i).second->Move();
						snake.at(i).second->setPos(snake.at(i).first);
						snake.at(i).first = ((i - 1) == 0) ? make_pair(coord.first, coord.second + ELEM_SIZE) : snake.at(i - 1).second->getPos();
					}

					snake[0].second->setPos(make_pair(coord.first, coord.second + ELEM_SIZE));
				}
			break;
		}

		SDL_Delay(75);
	}
	alive = 0;

	cout << "Move thread ending" << endl;

	return 0;
}

void putpixel(SDL_Surface * screen, uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
{
    uint16_t * ptr = NULL;
 
    if((x <= 0) || (y <= 0))
        return;
 
    if((x >= screen->w) || (y >= screen->h))
        return;
 
    ptr = (uint16_t *)screen->pixels + y*screen->pitch/2 + x;
    *ptr = SDL_MapRGB(screen->format, R, G, B);
}

void drawcircle(SDL_Surface * screen, uint16_t x, uint16_t y, double radius, int16_t a, int16_t b, uint8_t R, uint8_t G, uint8_t B)
{
    double i;
    SDL_Rect rect;
    int16_t xX, yY = 0;
 
    rect.x = x - radius - (b - a);
    rect.y = y - radius - (a - b);
    rect.w = (radius * 2) + ((b - a) * 2);
    rect.h = (radius * 2) + ((a - b) * 2);
 
    for(i = 0; i < 360; i+=0.1)
    {
        xX = x + ((radius + (b - a)) * cos(i * PI/180));
        yY = y + ((radius + (a - b)) * sin(i * PI/180));
 
        putpixel(screen, xX, yY, R, G, B);
    }
 
    SDL_UpdateRects(screen, 1, &rect);
}

void drawrectangle(SDL_Surface * screen, uint16_t x, uint16_t y, uint16_t base, uint16_t height, uint8_t R, uint8_t G, uint8_t B)
{
    int i;
    SDL_Rect rect;
 
    rect.x = x - (base / 2);
    rect.y = y - (height / 2);
    rect.w = base;
    rect.h = height;
 
    for(i = 0; i < base; i++)
        putpixel(screen, (rect.x + i), rect.y, R, G, B);
 
    for(i = 0; i < base; i++)
        putpixel(screen, (rect.x + i), (rect.y + height - 1), R, G, B);
 
    for(i = 0; i < height; i++)
        putpixel(screen, rect.x, (rect.y + i), R, G, B);
 
    for(i = 0; i < height; i++)
        putpixel(screen, (rect.x + base - 1), (rect.y + i), R, G, B);
 
 
    SDL_UpdateRects(screen, 1, &rect);
}
