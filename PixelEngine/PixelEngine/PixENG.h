#pragma once
#define SDL_MAIN_HANDLED
#include <fstream>
#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_thread.h>
#include <chrono>


struct Pixel
{
public:
	SDL_Rect rect;
	union
	{
		uint32_t c = 0xFF000000;
		struct
		{
			uint8_t r, g, b, a;
		};
	};
	Pixel();
	Pixel(SDL_Rect rec)
	{
		rect = rec;
	}
	Pixel(SDL_Rect rec,uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
	{
		rect = rec;
		r = red;
		g = green;
		b = blue;
		a = alpha;
	};
};
struct Texture
{
	SDL_Texture* texture;
	unsigned int nLayer = 0;

	Texture(SDL_Texture &t, unsigned int n)
	{

	};
};

class PixENG
{
public:
	std::string sAppName;
	int screenW = 0;
	int screenH = 0;
	int pixelW = 0;
	int pixelH = 0;
	float rPixW = 0;
	float rPixH = 0;
	float dT = 0;
	float dTtimer = 1.0f;
	int FrameCount = 0;
	bool running = false;
	std::vector<Pixel> pixelArray;

	SDL_Window* gWind = nullptr;
	SDL_Renderer* gRend = nullptr;
	SDL_Event gEvent;
	Uint32 winID = 0;
	

	PixENG();
	~PixENG();
	virtual void OnStart();
	virtual bool OnUpdate(float dT);
	bool Init(int screenW, int screenH);
	void Start(int pixW, int pixH);
	int GameLoop();

	//SDL_Texture* CreateTextureFromPixels(std::vector<Pixel>, unsigned int);
};


PixENG::PixENG()
{
	
} 
PixENG::~PixENG()
{
	SDL_DestroyWindow(gWind);
}
bool PixENG::Init(int scrW, int scrH)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::ofstream error("error.txt", std::ios::trunc);
		error << "Video Initalize Error::" << SDL_GetError() << std::endl;
		error.close();
		SDL_ClearError();
		return false;
	}
	else
	{
		gWind = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scrW, scrH, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		if (gWind == nullptr)
		{
			std::ofstream error("error.txt", std::ios::trunc);
			error << "Window Creation Error::" << SDL_GetError() << std::endl;
			error.close();
			SDL_ClearError();
			return false;
		}
		else
		{
			gRend = SDL_CreateRenderer(gWind, -1, SDL_RENDERER_ACCELERATED);
			if (gRend == nullptr)
			{
				std::ofstream error("error.txt", std::ios::trunc);
				error << "Renderer Creation Error: " << SDL_GetError() << std::endl;
				error.close();
				SDL_ClearError();
				return false;
			}
			else
			{
				int imgFlags = IMG_INIT_PNG;
				winID = SDL_GetWindowID(gWind);
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					std::ofstream error("error.txt", std::ios::trunc);
					error.open("error.txt", std::ios::trunc);
					error << "SDL_Image could not initalize:" << IMG_GetError() << std::endl;
					error.close();
					return false;
				}
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					return false;
				}
			}
		}
	}
	screenW = scrW;
	screenH = scrH;
	return true;
}
void PixENG::OnStart()
{

}
bool PixENG::OnUpdate(float dT)
{
	(void)dT; return false;
};
void PixENG::Start(int pixW, int pixH)
{
	pixelW = screenW / pixW;
	pixelH = screenH / pixH;

	rPixW = pixW;
	rPixH = pixH;

	for (int y = 0; y < pixelH; y++)
		for (int x = 0; x < pixelW; x++)
			pixelArray.push_back(Pixel(SDL_Rect{ (int)(x * rPixW),(int)(y * rPixH),(int)rPixW,(int)rPixH }, 0, 0, 0));

	running = true;

	GameLoop();
}
int PixENG::GameLoop()
{
	OnStart();
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();



	while (running)
	{
		//Time
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> passedTime = tp2 - tp1;
		tp1 = tp2;
		dT = passedTime.count();

		////USER INPUT EVENTS
		//Handle Input
		while(SDL_PollEvent(&gEvent))
		{
			switch (gEvent.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			default:
				break;
			}
		}

		////FRAME UPDATES 
		//Handle Frame Updates
		if (!OnUpdate(dT))
			running = false;

		////RENDER
		//Clear Screen
		SDL_SetRenderDrawColor(gRend, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRend);

		//Draw Textures
		for (int x = 0; x < pixelW; x++)
			for (int y = 0; y < pixelH; y++)
			{
				SDL_SetRenderDrawColor(gRend, pixelArray[x + (y * pixelW)].r, pixelArray[x + (y * pixelW)].g, pixelArray[x + (y * pixelW)].b, pixelArray[x + (y * pixelW)].a);
				SDL_RenderFillRect(gRend, &pixelArray[x + (y * pixelW)].rect);
			}
		SDL_RenderPresent(gRend);

		////Update Title Bar
		dTtimer += dT;
		if (dTtimer >= 1.0f)
		{
			dTtimer -= 1.0f;
			std::string title = sAppName + " -FPS: " + std::to_string(FrameCount).c_str();
			SDL_SetWindowTitle(gWind, title.c_str());
			FrameCount = 0;
		}
		FrameCount++;
	}
	return 0;
}

/*SDL_Texture* PixENG::CreateTextureFromPixels(std::vector <Pixel> pixArray, unsigned int nLayer)
{
	SDL_Texture returntexture = SDL_CreateTexture;
}*/