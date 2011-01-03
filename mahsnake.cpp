#include <iostream>
#include <utility>
#include <vector>
#include <ctime>
#include <SDL/SDL.h>

#define ELEM_SIZE 20

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(X) std::cout << __FILE__ << ' ' << __LINE__ << ' ' << X << std::endl;
#else
#define DEBUG_PRINT(X) //
#endif

using namespace std;

short key; //1 => Left, 2 => Right, 3 => Up, 4 => Down
bool alive;
SDL_Surface * background = NULL;

class Element;

vector< pair< pair<int, int>, Element* > > snake;

void drawelement(SDL_Surface *, SDL_Surface *, uint16_t, uint16_t, uint16_t, uint16_t);
void cleanrectangle(SDL_Surface *, uint16_t, uint16_t, uint16_t, uint16_t);
int move(void *);

typedef struct ThreadS
{
	SDL_Surface * screen;
	SDL_Surface * elem;
	SDL_Surface * food;
}ThreadS;

class Element
{
	public:
		Element() : pos(0, 0), prev(-1, -1), screen(NULL), element(NULL) {}
		Element(SDL_Surface * scr, SDL_Surface * elem, pair<int, int> p) : pos(p), prev(-1, -1), screen(scr), element(elem) {}

		pair<int, int> getPos() const { return this->pos; }
		void setPos(pair<int, int> p) { pos = p; }
		void setScreen(SDL_Surface * s) { screen = s; }

		virtual Element& Move()
		{
			if(prev.first != -1)
				cleanrectangle(screen, prev.first, prev.second, ELEM_SIZE + 1, ELEM_SIZE + 1);

			drawelement(element, screen, pos.first, pos.second, ELEM_SIZE, ELEM_SIZE);
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
		SDL_Surface * element;
};

int main()
{
	SDL_Surface * screen = NULL;
	SDL_Surface * background_tmp = NULL;
	SDL_Surface * elem_tmp = NULL;
	SDL_Surface * elem = NULL;
	SDL_Surface * food_tmp = NULL;
	SDL_Event e;
	SDL_Thread * move_t = NULL;
	ThreadS ts;
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

	SDL_WM_SetCaption("Mahsnake", NULL);

	background_tmp = SDL_LoadBMP("bkg.bmp");
	if(background_tmp == NULL){
		cout << "Can't load background" << endl;
		exit(1);
	}

	background = SDL_DisplayFormat(background_tmp);
	SDL_FreeSurface(background_tmp);

	SDL_BlitSurface(background, NULL, screen, NULL);
	SDL_Flip(screen);

	elem_tmp = SDL_LoadBMP("snake.bmp");
	if(elem_tmp == NULL){
		cout << "Can't load element bitmap" << endl;
		exit(1);
	}

	elem = SDL_DisplayFormat(elem_tmp);
	SDL_FreeSurface(elem_tmp);

	snake.push_back(pair< pair<int, int>, Element* >(pair<int, int>(-1, -1), new Element(screen, elem, pair<int, int>(80, 50))));
	for(int i = 60; i > 0; i -= 20)
		snake.push_back(pair< pair<int, int>, Element* >(pair<int, int>(i + 20, 50), new Element(screen, elem, pair<int, int>(i, 50))));

	key = 2;
	alive = true;
	ts.screen = screen;

	food_tmp = SDL_LoadBMP("food.bmp");
	if(food_tmp == NULL){
		cout << "Can't load food bitmap" << endl;
		exit(1);
	}

	ts.food = SDL_DisplayFormat(food_tmp);
	ts.elem = elem;
	SDL_FreeSurface(food_tmp);

	move_t = SDL_CreateThread(move, static_cast<void *>(&ts));

	while((!pressed) && alive)
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
						if(key != 4)
                            				key = 3;
                            		break;

                        		case SDLK_DOWN:
						if(key != 3)
                            				key = 4;
                            		break;

                        		case SDLK_LEFT:
						if(key != 2)
                            				key = 1;
                            		break;

                        		case SDLK_RIGHT:
						if(key |= 1)
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
	SDL_FreeSurface(ts.food);
	SDL_FreeSurface(elem);
	SDL_FreeSurface(background);
	SDL_FreeSurface(screen);

	for(int i = 0; i < snake.size(); i++)
		delete(snake[i].second);

	return 0;
}

int move(void * p)
{
	ThreadS * ts = static_cast<ThreadS *>(p);
	Element food(ts->screen, ts->food, pair<int, int>(200, 200));
	SDL_Rect head_p, food_p;
	pair<int, int> rand_pos;

	food.Move();

	DEBUG_PRINT("Move thread starting");
	srand(time(NULL));

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

					SDL_Flip(ts->screen);

					snake[0].second->setPos(make_pair(coord.first - ELEM_SIZE, coord.second));
				}
				else
				{
					cout << "You lost" << endl;
					alive = false;
				}
			break;

			case 2:
				if(snake[0].second->getPos().first < (ts->screen->w - ELEM_SIZE))
				{
					pair<int, int> coord = snake[0].second->getPos();
					snake[0].second->Move();

					for(int i = 1; i < snake.size(); i++)
					{
						snake.at(i).second->Move();
						snake.at(i).second->setPos(snake.at(i).first);
						snake.at(i).first = ((i - 1) == 0) ? make_pair(coord.first + ELEM_SIZE, coord.second) : snake.at(i - 1).second->getPos();
					}

					SDL_Flip(ts->screen);

					snake[0].second->setPos(make_pair(coord.first + ELEM_SIZE, coord.second));
				}
				else
				{
					cout << "You lost" << endl;
					alive = false;
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

					SDL_Flip(ts->screen);

					snake[0].second->setPos(make_pair(coord.first, coord.second - ELEM_SIZE));
				}
				else
				{
					cout << "You lost" << endl;
					alive = false;
				}
			break;

			case 4:
				if(snake[0].second->getPos().second < (ts->screen->h - ELEM_SIZE))
				{
					pair<int, int> coord = snake[0].second->getPos();
					snake[0].second->Move();

					for(int i = 1; i < snake.size(); i++)
					{
						snake.at(i).second->Move();
						snake.at(i).second->setPos(snake.at(i).first);
						snake.at(i).first = ((i - 1) == 0) ? make_pair(coord.first, coord.second + ELEM_SIZE) : snake.at(i - 1).second->getPos();
					}

					SDL_Flip(ts->screen);

					snake[0].second->setPos(make_pair(coord.first, coord.second + ELEM_SIZE));
				}
				else
				{
					cout << "You lost" << endl;
					alive = false;
				}
			break;
		}

		head_p.x = snake[0].second->getPos().first - (ELEM_SIZE / 2);
		head_p.y = snake[0].second->getPos().second - (ELEM_SIZE / 2);
		head_p.w = ELEM_SIZE;
		head_p.h = ELEM_SIZE;

		food_p.x = food.getPos().first - (ELEM_SIZE / 2);
		food_p.y = food.getPos().second - (ELEM_SIZE / 2);
		food_p.w = ELEM_SIZE;
		food_p.h = ELEM_SIZE;

		if((head_p.x <= (food_p.x + food_p.w)) && 
			((head_p.y <= (food_p.y + food_p.h))) &&
			(((head_p.x + head_p.w) >= food_p.x)) &&
			(((head_p.y + head_p.h) >= food_p.y)))
		{
			while((rand_pos.first = (rand() % ts->screen->w)) != 0)
				if(((rand_pos.first % ELEM_SIZE) == 0) && (rand_pos.first != 0))
					break;

			while((rand_pos.second = (rand() % ts->screen->h)) != 0)
				if(((rand_pos.second % ELEM_SIZE) == 0) && (rand_pos.second != 0))
					break;

			DEBUG_PRINT(rand_pos.first << ", " << rand_pos.second);

			food.setPos(rand_pos);
			food.Move();

			snake.push_back(pair< pair<int, int>, Element* >((*(snake.rbegin())).second->getPos(), new Element(ts->screen, ts->elem, (*(snake.rbegin())).second->getPos())));
		}

		SDL_Delay(100);
		
	}
	alive = false;

	DEBUG_PRINT("Move thread ending");

	return 0;
}

void drawelement(SDL_Surface * in, SDL_Surface * out, uint16_t x, uint16_t y, uint16_t base, uint16_t height)
{
	SDL_Rect rect;

	rect.x = x - (base / 2);
    	rect.y = y - (height / 2);
    	rect.w = base;
    	rect.h = height;

	SDL_BlitSurface(in, NULL, out, &rect);
}

void cleanrectangle(SDL_Surface * screen, uint16_t x, uint16_t y, uint16_t base, uint16_t height)
{
	SDL_Rect rect;

	rect.x = x - (base / 2);
    	rect.y = y - (height / 2);
    	rect.w = base;
    	rect.h = height;

	SDL_BlitSurface(background, &rect, screen, &rect);
}
