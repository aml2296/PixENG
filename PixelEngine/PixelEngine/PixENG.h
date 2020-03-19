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
	union
	{
		uint32_t c = 0xFF0000FF;
		struct
		{
			uint8_t r, g, b, a;
		};
	};
	Pixel()
	{
		
	};
	Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
	{
		r = red;
		g = green;
		b = blue;
		a = alpha;
	};
};
class Sprite
{
public:
	SDL_Texture* texture;
	SDL_Rect pos;
	unsigned int nLayer = 0;

	Sprite(SDL_Texture* t, unsigned int layer, SDL_Rect r);
	~Sprite()
	{
		if (texture)
			SDL_DestroyTexture(texture);
	}
};

Sprite::Sprite(SDL_Texture* t = nullptr, unsigned int layer = 0, SDL_Rect r = SDL_Rect{ 0,0,0,0 })
{
	texture = t;
	pos = r;
	nLayer = layer;
}



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
	std::vector<Sprite*> fTextureArray;
	std::vector<Pixel> pixelArray;

	SDL_Window* gWind = nullptr;
	SDL_Renderer* gRend = nullptr;
	SDL_Event gEvent;
	Uint32 winID = 0;
	
	const uint32_t aMask = 0x000000FF;
	const uint32_t bMask = 0x0000FF00;
	const uint32_t gMask = 0x00FF0000;
	const uint32_t rMask = 0xFF000000;


	PixENG();
	~PixENG();
	virtual void OnStart();
	virtual bool OnUpdate(float dT);
	bool Init(int screenW, int screenH);
	void Start(int pixW, int pixH);
	int GameLoop();

	SDL_Texture* loadTexture(std::string location);
	SDL_Texture* CreateTextureFromPixels(std::vector<Pixel> &pixels, unsigned int nLayer, int w, int h);
};


PixENG::PixENG()
{
	
} 
PixENG::~PixENG()
{
	SDL_DestroyRenderer(gRend);
	SDL_DestroyWindow(gWind);
	SDL_Quit();
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
		for (int i = 0; i < fTextureArray.size(); i++)
			if (SDL_RenderCopy(gRend, fTextureArray[i]->texture, NULL, &fTextureArray[i]->pos) < 0)
				printf("Unable to render texture %i! SDL Error: %s\n", i, SDL_GetError());
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


//Loads Texture from file
SDL_Texture* PixENG::loadTexture(std::string path)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRend, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}
//Does not work atm
SDL_Texture *PixENG::CreateTextureFromPixels(std::vector<Pixel> &p, unsigned int nLayer, int w, int h)
{
	SDL_Texture* sdlTexture = nullptr;
	uint32_t pixFormat = SDL_GetWindowPixelFormat(gWind);
	SDL_PixelFormat* pixFormatMap = SDL_AllocFormat(pixFormat);
	sdlTexture = SDL_CreateTexture(gRend, pixFormat, SDL_TEXTUREACCESS_STREAMING, w*rPixW, h*rPixH);

	if (sdlTexture == nullptr)
	{
		std::ofstream error("error.txt", std::ios::trunc);
		error << "Could not Create SDL_Texture:: " << SDL_GetError() << std::endl;
		error.close();
		SDL_ClearError();
	}
	else
	{
		SDL_Surface* surf = SDL_CreateRGBSurface(0, w * rPixW, h * rPixH, 32, rMask, gMask, bMask, aMask);
		SDL_LockSurface(surf);
		if (SDL_LockTexture(sdlTexture, NULL, &surf->pixels, &surf->pitch) < 0)
		{
			printf("SDL_LockTexture failed: %s\n", SDL_GetError());
			SDL_ClearError();
		}
		else
		{
			uint32_t* textPix = (uint32_t*)surf->pixels;
			for (int tX = 0; tX < w; tX++)
				for (int tY = 0; tY < h; tY++)
					memset(textPix, SDL_MapRGBA(pixFormatMap, p[tX + (tY * w)].r, p[tX + (tY * w)].g, (uint8_t)(w * h / 255) * tX + (tY * w), p[tX + (tY * w)].a), sizeof(uint32_t));
			SDL_UnlockTexture(sdlTexture);
			SDL_FreeFormat(pixFormatMap);
			SDL_UnlockSurface(surf);
		}
	}
	return sdlTexture;
}