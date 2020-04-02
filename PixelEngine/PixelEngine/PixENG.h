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

//For Flag Comparisons
template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

enum class SpriteFlags
{
	NoFlag = 0 << 0,
	hasMask = 1 << 0
};

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
	SpriteFlags flags = SpriteFlags::NoFlag;
	SDL_Texture* texture;
	SDL_Rect pos;
	int nLayer = 0;

	Sprite(SDL_Texture* t, int layer, int x, int y);
	~Sprite()
	{
		if (texture)
			SDL_DestroyTexture(texture);
	}
};
Sprite::Sprite(SDL_Texture* t = nullptr, int layer = 0, int x = 0, int y = 0)
{
	texture = t;
	nLayer = layer;
	pos.x = x;
	pos.y = y;
	SDL_QueryTexture(t, nullptr, nullptr, &pos.w, &pos.h);
}

class CollisionBox
{
public:
	SDL_Rect pBoundingBox;
	CollisionBox()
	{
		pBoundingBox = SDL_Rect{ 0,0,0,0 };
	};
	CollisionBox(SDL_Rect r)
	{
		pBoundingBox = r;
	}
	static bool CollisionCheck(SDL_Rect a, SDL_Rect b);
	virtual void onCollision(CollisionBox other) {};
	virtual void onColliding(CollisionBox other) {};
	virtual void onExitCollision(CollisionBox other) {};
};
class PhysicsAsset : public CollisionBox
{
public:
	float velocityX = 0.0f;
	float velocityY = 0.0f;
	bool gravity = true;
	float gravityAmp = -9.8f;

	//Returns true if the two Rects are touching or colliding
};
class PhysEntity : public PhysicsAsset
{
private:
	int posX = 0;
	int posY = 0;
	Sprite* sprite;
public:
	PhysEntity(Sprite* spt) { sprite = spt; pBoundingBox = sprite->pos; };
	PhysEntity(Sprite* spt, SDL_Rect bBounds){sprite = spt;	pBoundingBox = bBounds;};
	//Moves to Point x,y
	void Move(int x, int y) { Translate(x - posX, y - posY); };
	//Translates PhysEntity and updates Children Componenets
	void Translate(int x, int y);
	bool Gravity() { return gravity; };
	void setGravity(float g) { gravityAmp = g; };
	float GravityValue() { return gravityAmp; };
	void AddVelocity(float x, float y) { velocityX += x;velocityY += y; };
	void SetVelocity(float x, float y) { velocityX = x; velocityY = y; };
	void ApplyVelocity() { Translate(velocityX, velocityY); };
	SDL_Texture* getTexture() { return sprite->texture; };
	SDL_Rect *pBounds() { return &pBoundingBox; };
	void HandleCollision(SDL_Rect b);
};

//List of Texture's, organizes by Layer upon insertion then by time of insertion if on the same layer
struct TextureListNode
{
	SDL_Texture* texture = nullptr;
	SDL_Rect* bounds = nullptr;
	int layer = 0;
};
class TextureList
{
	public:
		std::vector<TextureListNode> list;
		void insert(SDL_Texture* t, SDL_Rect *r, int l)
		{
			std::vector<TextureListNode>::iterator it = list.begin();
			while(it != list.end())
			{
				if (it->layer > l)
					break;
				it++;
			}
			list.insert(it, TextureListNode{ t,r,l });
		};
};


bool CollisionBox::CollisionCheck(SDL_Rect a, SDL_Rect b)
{
	if ((a.x <= b.x + b.w && a.x >= b.x) || (a.w + a.x <= b.w + b.x && a.w + a.x >= b.x))
		if ((a.y <= b.y + b.h && a.y >= b.y) || (a.h + a.x <= b.h + b.y && a.h + a.y >= b.y))
			return true;
	return false;
}
void PhysEntity::Translate(int x, int y)
{
	posX += x;
	posY += y;

	pBoundingBox.x += x;
	pBoundingBox.y += y;

	sprite->pos.x += x;
	sprite->pos.y += y;
}
void PhysEntity::HandleCollision(SDL_Rect b)
{
	if (velocityX > 0)
	{
		if (velocityY > 0)
		{
			if (velocityX > velocityY)
			{
				velocityY = 0;
				Translate(0, -(pBoundingBox.y + pBoundingBox.h - b.y));
			}
			else if (velocityY > velocityX)
			{
				velocityX = 0;
				Translate(-(pBoundingBox.x + pBoundingBox.w - b.x), 0);
			}
			else
			{
				velocityX = 0;
				velocityY = 0;
				Translate(-(pBoundingBox.x + pBoundingBox.w - b.x), -(pBoundingBox.y + pBoundingBox.h - b.y));
			}
		}
		else if (velocityY < 0)
		{
			if (velocityX > -1 * velocityY)
			{
				velocityY = 0;
				Translate(0, -(b.y + b.h - pBoundingBox.y));
			}
			else if (-1 * velocityY > velocityX)
			{
				velocityX = 0;
				Translate(-(pBoundingBox.x + pBoundingBox.w - b.x), 0);
			}
			else
			{
				velocityX = 0;
				velocityY = 0;
				Translate(-(pBoundingBox.x + pBoundingBox.w - b.x), -(b.y + b.h - pBoundingBox.y));
			}
		}
		else
		{
			velocityX = 0;
			Translate(-(pBoundingBox.x + pBoundingBox.w - b.x), 0);
		}
	}
	else if (velocityX < 0)
	{
		if (velocityY > 0)
		{
			if (-1*velocityX > velocityY)
			{
				velocityY = 0;
				Translate(0, -(pBoundingBox.y + pBoundingBox.h - b.y));
			}
			else if (velocityY > -1*velocityX)
			{
				velocityX = 0;
				Translate((b.x + b.w - pBoundingBox.x), 0);
			}
			else
			{
				velocityX = 0;
				velocityY = 0;
				Translate((b.x + b.w - pBoundingBox.x), -(pBoundingBox.y + pBoundingBox.h - b.y));
			}
		}
		else if (velocityY < 0)
		{
			if (-1 * velocityX > -1*velocityY)
			{
				velocityY = 0;
				Translate(0, (b.y + b.h - pBoundingBox.y));
			}
			else if (-1*velocityY > -1 * velocityX)
			{
				velocityX = 0;
				Translate((b.x + b.w - pBoundingBox.x), 0);
			}
			else
			{
				velocityX = 0;
				velocityY = 0;
				Translate((b.x + b.w - pBoundingBox.x), (b.y + b.h - pBoundingBox.y));
			}
		}
		else
		{
			velocityX = 0;
			Translate((b.x + b.w - pBoundingBox.x), 0);
		}
	}
	else
	{
		if (velocityY > 0)
		{
			velocityY = 0;
			Translate(0, -(pBoundingBox.y + pBoundingBox.h - b.y));
		}
		else if (velocityY < 0)
		{
			velocityY = 0;
			Translate(0, b.y + b.h - pBoundingBox.y);
		}
	}
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
	bool debug = false;

	TextureList tList;
	std::vector<SDL_Rect*> dRect;
	std::vector<Pixel> pixelArray;

	SDL_Window* gWind = nullptr;
	SDL_DisplayMode dMode;
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
	SDL_Texture* loadTexture(std::string location, Pixel mask);
	SDL_Texture* CreateTextureFromPixels(std::vector<Pixel*> &pixels, int w, int h);
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
		while (SDL_PollEvent(&gEvent))
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
		for (int i = 0; i < tList.list.size(); i++)
			if (SDL_RenderCopy(gRend, tList.list[i].texture, NULL, tList.list[i].bounds) < 0)
				printf("Unable to render texture %i! SDL Error: %s\n", i, SDL_GetError());

		//Debug Draw
		if (debug)
		{
			SDL_SetRenderDrawColor(gRend, 0, 255, 0, 255);
			for (int i = 0; i < dRect.size(); i++)
				SDL_RenderDrawRect(gRend, dRect[i]);
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
SDL_Texture* PixENG::loadTexture(std::string path, Pixel mask)
{
	SDL_Surface* loadSurface = IMG_Load(path.c_str());
	SDL_Texture* newT = nullptr;
	if (!loadSurface)
	{
		printf("Could not load Surface! %s", SDL_GetError());
	}
	else
	{
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(loadSurface, SDL_PIXELFORMAT_RGBA8888, 0);
		if (!formattedSurface)
		{
			printf("Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Create blank streamable texture
			newT = SDL_CreateTexture(gRend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
			SDL_SetTextureBlendMode(newT, SDL_BLENDMODE_BLEND);
			if (!newT)
			{
				printf("Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				
				Uint32 pFormat = SDL_PIXELFORMAT_RGBA8888;
				SDL_PixelFormat* mappingFormat = SDL_AllocFormat(pFormat);
				Uint32 colorKey = SDL_MapRGB(mappingFormat, (int)mask.r, (int)mask.g, (int)mask.b);
				Uint32 transparent = SDL_MapRGBA(mappingFormat, 0xFF, 0xFF, 0xFF, 0x00);
				void* pixelVoid = nullptr;
				int pitch = 0;

				SDL_LockTexture(newT, NULL, &pixelVoid, &pitch);
				memcpy(pixelVoid, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h);

				Uint32* pixels = (Uint32*)pixelVoid;
				for (int i = 0; i < (formattedSurface->pitch / 4) * formattedSurface->h; i++)
					if (pixels[i] == colorKey)
						pixels[i] = transparent;
				SDL_UnlockTexture(newT);
				SDL_FreeFormat(mappingFormat);
			}
		}
		SDL_FreeSurface(formattedSurface);
	}
	SDL_FreeSurface(loadSurface);
	SDL_ClearError();
	return newT;
}
//Does not work atm
SDL_Texture *PixENG::CreateTextureFromPixels(std::vector<Pixel*> &p, int w, int h)
{
	SDL_Surface* loadSurface = SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	SDL_Texture* newT = nullptr;
	if (!loadSurface)
	{
		printf("Could not load Surface! %s", SDL_GetError());
	}
	else
	{
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(loadSurface, SDL_PIXELFORMAT_RGBA8888, 0);
		if (!formattedSurface)
		{
			printf("Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Create blank streamable texture
			newT = SDL_CreateTexture(gRend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
			SDL_SetTextureBlendMode(newT, SDL_BLENDMODE_BLEND);
			if (!newT)
			{
				printf("Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
			}
			else
			{

				Uint32 pFormat = SDL_PIXELFORMAT_RGBA8888;
				SDL_PixelFormat* mappingFormat = SDL_AllocFormat(pFormat);
				void* pixelVoid = nullptr;
				int pitch = 0;

				SDL_LockTexture(newT, NULL, &pixelVoid, &pitch);
				memcpy(pixelVoid, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h);

				Uint32* pixels = (Uint32*)pixelVoid;
				for (int i = 0; i < (formattedSurface->pitch / 4) * formattedSurface->h; i++)
					pixels[i] = SDL_MapRGBA(mappingFormat, p[i]->r, p[i]->g, p[i]->b, p[i]->a);
				SDL_UnlockTexture(newT);
				SDL_FreeFormat(mappingFormat);
			}
		}
		SDL_FreeSurface(formattedSurface);
	}
	SDL_FreeSurface(loadSurface);
	SDL_ClearError();
 	return newT;
}
//Updates Texture
void UpdateTexture(SDL_Texture *texture,std::vector<Pixel*> pixels)
{
	Uint32* lockedPixels;
	int pitch;
	SDL_LockTexture(texture,NULL, reinterpret_cast< void** >( &lockedPixels),&pitch);
	std::copy( pixels.begin(), pixels.end(), lockedPixels );
	SDL_UnlockTexture(texture);
}