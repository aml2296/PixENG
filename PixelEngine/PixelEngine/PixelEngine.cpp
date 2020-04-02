#include"PixENG.h"
#include<random>
#include<ctime>


class Players : public Sprite
{
public:
	int xAxis = 0;
	int yAxis = 0;

	
	void HandleInput()
	{
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		yAxis = xAxis = 0;
		if (currentKeyStates[SDL_SCANCODE_UP])
			yAxis += -1;
		if (currentKeyStates[SDL_SCANCODE_DOWN])
			yAxis += 1;
		if (currentKeyStates[SDL_SCANCODE_LEFT])
			xAxis += -1;
		if (currentKeyStates[SDL_SCANCODE_RIGHT])
			xAxis += 1;
	};

	Players(Sprite* s,float rPixW, float rPixH ) : Sprite(*s)
	{
		s->nLayer = 1;
	}
	~Players()
	{

	}
};

class Level : public Sprite
{
public:
	std::vector <Pixel*> bPixels;

	Level() {
	};
	~Level()
	{
		destroy();
	}
	bool Init();
	void destroy();
	bool UpdatePix();
};

bool Level::Init()
{
	bool success = true;
	for (int y = 0; y < pos.h; y++)
		for (int x = 0; x < pos.w; x++)
			bPixels.push_back(new Pixel{ 255,0,0 });
	return true;
}
void Level::destroy()
{
	while (bPixels.size() != 0)
	{
		Pixel* p = bPixels.front();
		if (p != nullptr)
		{
			bPixels.erase(bPixels.begin());
			delete p;
		}
	}
}


class bBallGame : public PixENG
{
	Level map;

	void OnStart() override
	{
		//Init Name
		sAppName = "PixENG.exe v(0.0.2)";
		debug = true;
		if (!InitAssests())
			printf("Could not init all Assests!!!");
	}
	bool OnUpdate(float dT) override
	{
		for (int y = 0; y < map.pos.h; y++)
			for (int x = 0; x < map.pos.w; x++)
			{
				map.bPixels[x + (y * map.pos.w)]->r = (map.bPixels[x + (y * map.pos.w)]->r + 5 % 255);
				map.bPixels[x + (y * map.pos.w)]->g = 0;
				map.bPixels[x + (y * map.pos.w)]->b = 0;
				map.bPixels[x + (y * map.pos.w)]->a = 255;
			}
		UpdateTexture(map.texture, map.bPixels);
		return true;
	}
	bool InitAssests()
	{
		bool success = true;

		map.pos = SDL_Rect{ 0,0,screenW,screenH };
		map.Init();
		map.texture = bBallGame::CreateTextureFromPixels(map.bPixels, screenW, screenH);
		tList.insert(map.texture, &map.pos, 0);

		return true;
	}
};

int main()
{
	bBallGame eng;
	if (eng.Init(1280, 960))
		eng.Start(5, 5);
	return 0;
}