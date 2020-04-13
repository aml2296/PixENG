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
#include <math.h>
#include "SINet.h"

//For Flag Comparisons
template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

class Pixel
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
	operator Uint32() const{ return c; };
	Pixel operator=(const Pixel& o)
	{
		r = o.r;
		g = o.g;
		b = o.b;
		a = o.a;
		return *this;
	};
};
enum class SpriteFlags
{
	NoFlag = 0 << 0,
	hasMask = 1 << 0
};
class Sprite
{
private:
	SDL_Texture* texture;
public:
	SpriteFlags flags = SpriteFlags::NoFlag;
	SDL_Rect pos;
	int nLayer = 0;

	bool SetTexture(SDL_Texture* t)
	{
		texture = t;
		SDL_QueryTexture(t, nullptr, nullptr, &pos.w, &pos.h);
		return t;
	}
	SDL_Texture* GetTexture() { return texture; }
	Sprite(SDL_Texture* t, int layer, int x, int y);
	Sprite operator=(const Sprite& o)
	{
		flags = o.flags;
		texture = o.texture;
		pos = o.pos;
		nLayer = o.nLayer;
		return *this;
	}
	~Sprite()
	{
		if (texture)
			SDL_DestroyTexture(texture);
	}
};
Sprite::Sprite(SDL_Texture* t = nullptr, int layer = 0, int x = 10, int y = 10)
{
	texture = t;
	nLayer = layer;
	pos.x = x;
	pos.y = y;
	SDL_QueryTexture(t, nullptr, nullptr, &pos.w, &pos.h);
}

class ATexture
{
private:
	SDL_Point dimensions;
	SDL_Texture* drawTexture;
	SDL_Rect pos;
	Pixel penColor;
	void* pixPtr;
	int pitch;

	int DirectLine(float slope, int direction, SDL_Point originalPoint, SDL_Point currentPoint);
public:
	const double PI = 3.145927;

	bool lockTexture();
	bool unlockTexture();
	void SetTexture(SDL_Texture* t, SDL_Renderer* r, int w, int h);
	void* GetPixels()
	{
		return pixPtr;
	};
	int& GetPitch()
	{
		return pitch;
	};
	int GetWidth();
	int GetHeight();
	SDL_Texture* GetTexture()
	{
		return drawTexture;
	};
	void SetColor(Pixel p)
	{
		penColor = p;
	};
	Pixel GetColor()
	{ 
		return penColor;
	};
	bool DrawRect(SDL_Rect r, int thickness);
	bool DrawCircle(SDL_Point center, int radius, int thickness);
	bool DrawLine(SDL_Point A, SDL_Point B);
	SDL_Rect& getRect()
	{
		return pos;
	}
	ATexture()
	{
		dimensions = { -1,-1 };
		penColor = { 255,255,255,255 };
		pixPtr = nullptr;
		drawTexture = nullptr;
		pitch = 0;
	}
};
bool ATexture::lockTexture()
{
	if (pixPtr != nullptr)
	{
		printf("Can't modify alread locked Texture!\n");
		return false;
	}
	else
	{
		if (SDL_LockTexture(drawTexture, NULL, &pixPtr, &pitch) != 0)
		{
			printf("Could not lock texture!! SDL_Error: %s\n", SDL_GetError());
			return false;
		}
		return true;
	}
}
bool ATexture::unlockTexture()
{
	if (pixPtr == nullptr)
	{
		printf("Texture is not locked!\n");
		return false;
	}
	else
	{
		SDL_UnlockTexture(drawTexture);
		pixPtr = nullptr;
		pitch = 0;
	}
}
void ATexture::SetTexture(SDL_Texture* t, SDL_Renderer* r, int w, int h)
{
	if (t == nullptr)
	{
		drawTexture = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w, h);
		SDL_SetTextureBlendMode(drawTexture, SDL_BLENDMODE_BLEND);
	}
	else
		drawTexture = t;
	SDL_QueryTexture(drawTexture, nullptr, nullptr, &dimensions.x, &dimensions.y);
	pos = SDL_Rect{ 0, 0, dimensions.x, dimensions.y };
	if (lockTexture())
	{
		void* newTVoid;
		Uint32* newPixels;
		Uint32* pixels = (Uint32*)GetPixels();
		int newTPitch;
		int count = (GetPitch() / 4) * GetHeight();
		SDL_LockTexture(t, NULL, &newTVoid, &newTPitch);
		if (count != (newTPitch / 4) * h)
			return;
		else
		{
			newPixels = (Uint32*)newTVoid;
			for (int i = 0; i < count; i++)
				*(pixels + i) = (Uint32)newPixels[i];
		}
		SDL_UnlockTexture(t);
		unlockTexture();
	}
}
int ATexture::GetWidth()
{
	if (drawTexture == nullptr)
		return -1;
	else
		return dimensions.x;
}
int ATexture::GetHeight()
{
	if (drawTexture == nullptr)
		return -1;
	else
		return dimensions.y;
}
bool ATexture::DrawRect(SDL_Rect r, int thickness)
{
	if (!lockTexture())
		return false;
	else
	{
		Uint32* pixels = (Uint32*)GetPixels();
		Uint32 pFormat = SDL_PIXELFORMAT_RGBA8888;
		SDL_PixelFormat* mappingFormat = SDL_AllocFormat(pFormat);

		for (int by = (r.y > 0) ? r.y : 0; by < r.y + r.h; by++)
			for (int bx = (r.x > 0) ? r.x : 0; bx < r.x + r.w; bx++)
			{
				if (bx < r.x + thickness || bx >= r.x + r.w - thickness
					|| by < r.y + thickness || by >= r.y + r.h - thickness)
					pixels[bx + by * GetWidth()] = SDL_MapRGBA(mappingFormat, penColor.r, penColor.g, penColor.b, penColor.a);
			}
		unlockTexture();
		SDL_FreeFormat(mappingFormat);
		return true;
	}
}
bool ATexture::DrawCircle(SDL_Point center, int radius, int thickness)
{
	if (!lockTexture())
		return false;
	else
	{
		Uint32* pixels = (Uint32*)GetPixels();
		Uint32 pFormat = SDL_PIXELFORMAT_RGBA8888;
		SDL_PixelFormat* mappingFormat = SDL_AllocFormat(pFormat);

		std::vector<SDL_Point> CircleApoints;
		std::vector<SDL_Point> CircleBpoints;
		SDL_Point container = { radius + 1, radius + 1 };
		double CircleAx = radius + 1;
		double CircleAy = radius + 1;
		double CircleBx = radius + 1;
		double CircleBy = radius + 1;
		SDL_Point newPoint;
		int theta = 0;

		for (int d = 0; d < 360; d += 1)
		{
			if (d < 90)

			{
				CircleAx = radius * cos((double)d * PI / 180);
				CircleAy = radius * sin((double)d * PI / 180);
				CircleBx = (radius - thickness) * cos((double)d * PI / 180);
				CircleBy = (radius - thickness) * sin((double)d * PI / 180);
			}
			else if (d < 180)
			{
				theta = d - 90;
				CircleAx = -radius * cos((double)theta * PI / 180);
				CircleAy = radius * sin((double)theta * PI / 180);
				CircleBx = -(radius - thickness) * cos((double)theta * PI / 180);
				CircleBy = (radius - thickness) * sin((double)theta * PI / 180);

			}
			else if (d < 270)
			{
				theta = d - 180;
				CircleAx = -radius * cos((double)theta * PI / 180);
				CircleAy = -radius * sin((double)theta * PI / 180);
				CircleAx = -(radius - thickness) * cos((double)theta * PI / 180);
				CircleAy = -(radius - thickness) * sin((double)theta * PI / 180);
			}
			else
			{
				theta = d - 270;
				CircleAx = radius * cos((double)theta * PI / 180);
				CircleAy = -radius * sin((double)theta * PI / 180);
				CircleBx = (radius - thickness) * cos((double)theta * PI / 180);
				CircleBy = -(radius - thickness) * sin((double)theta * PI / 180);
			}
			newPoint = SDL_Point{ (int)CircleAx,(int)CircleAy };
			if (newPoint.x + center.x < GetWidth() && newPoint.x + center.x > 0
				&& newPoint.y + center.y < GetHeight() && newPoint.y + center.y > 0
				&& (newPoint.x != container.x || newPoint.y != container.y))
			{
				CircleApoints.push_back(newPoint);
				container = newPoint;
			}
		}

		for (int i = 0; i < CircleApoints.size(); i++)
			pixels[center.x + CircleApoints[i].x + (center.y + CircleApoints[i].y) * GetWidth()] = SDL_MapRGBA(mappingFormat, penColor.r, penColor.g, penColor.b, penColor.a);
		unlockTexture();
		SDL_FreeFormat(mappingFormat);
		return true;
	}
}
bool ATexture::DrawLine(SDL_Point A, SDL_Point B)
{
	if (!lockTexture())
		return false;
	else
	{
		Uint32* pixels = (Uint32*)GetPixels();
		SDL_PixelFormat* mappingFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
		Uint32 pen = SDL_MapRGBA(mappingFormat, penColor.r, penColor.g, penColor.b, penColor.a);
		int num = B.y - A.y;
		int den = B.x - A.x;
		float slope;

		SDL_Point drawPoint = A;
		float posY = 1;

		pixels[(A.y * GetWidth()) + A.x] = pen;
		pixels[(B.y * GetWidth()) + B.x] = pen;

		if (num == 0)
		{
			if (den > 0)
			{
				for (int i = 0; i < den; i++)
					pixels[(A.y * GetWidth()) + A.x + i] = pen;
			}
			else if (den < 0)
			{
				for (int i = 0; i > den; i--)
					pixels[(A.y * GetWidth()) + A.x + i] = pen;
			}
		}
		else if (den == 0)
		{
			if (num > 0)
			{
				for (int i = 0; i < num; i++)
					pixels[((A.y + i) * GetWidth()) + A.x] = pen;
			}
			else if (num < 0)
			{
				for (int i = 0; i > num; i--)
					pixels[((A.y + i) * GetWidth()) + A.x] = pen;
			}
		}
		else
		{
			float direction[8];// 0 = Right, 1 = UpRight, 2 = Up, 3 = UpLeft, 4 = Left, 5 = BottLeft, 6 = Bottom, 7 = BottRight
			int directInt = -1;

			if (num > 0)
				if (den > 0)
					directInt = 7;
				else
					directInt = 5;
			else
				if (den > 0)
					directInt = 1;
				else
					directInt = 3;

			slope = ((float)num / (float)den);
			switch (directInt)
			{
			case 7:
				while (drawPoint.x < B.x && drawPoint.y < B.y)
				{
					directInt = DirectLine(slope, directInt, A, drawPoint);
					switch (directInt)
					{
					case 0:
						drawPoint.x++;
						break;
					case 1:
						drawPoint.x++;
						drawPoint.y--;
						break;
					case 2:
						drawPoint.y--;
						break;
					case 3:
						drawPoint.y--;
						drawPoint.x--;
						break;
					case 4:
						drawPoint.x--;
						break;
					case 5:
						drawPoint.x--;
						drawPoint.y++;
						break;
					case 6:
						drawPoint.y++;
						break;
					case 7:
						drawPoint.x++;
						drawPoint.y++;
						break;
					default:
						break;
					}

					pixels[(drawPoint.y * GetWidth()) + drawPoint.x] = pen;
				}
				break;
			case 3:
				while (drawPoint.x > B.x&& drawPoint.y > B.y)
				{
					switch (directInt)
					{
					case 0:
						drawPoint.x++;
						break;
					case 1:
						drawPoint.x++;
						drawPoint.y--;
						break;
					case 2:
						drawPoint.y--;
						break;
					case 3:
						drawPoint.y--;
						drawPoint.x--;
						break;
					case 4:
						drawPoint.x--;
						break;
					case 5:
						drawPoint.x--;
						drawPoint.y++;
						break;
					case 6:
						drawPoint.y++;
						break;
					case 7:
						drawPoint.x++;
						drawPoint.y++;
						break;
					default:
						break;
					}
					directInt = DirectLine(slope, directInt, A, drawPoint);
					pixels[(drawPoint.y * GetWidth()) + drawPoint.x] = pen;
				}
				break;
			case 1:
				while (drawPoint.x < B.x && drawPoint.y > B.y)
				{
					switch (directInt)
					{
					case 0:
						drawPoint.x++;
						break;
					case 1:
						drawPoint.x++;
						drawPoint.y--;
						break;
					case 2:
						drawPoint.y--;
						break;
					case 3:
						drawPoint.y--;
						drawPoint.x--;
						break;
					case 4:
						drawPoint.x--;
						break;
					case 5:
						drawPoint.x--;
						drawPoint.y++;
						break;
					case 6:
						drawPoint.y++;
						break;
					case 7:
						drawPoint.x++;
						drawPoint.y++;
						break;
					default:
						break;
					}
					directInt = DirectLine(slope, directInt, A, drawPoint);
					pixels[(drawPoint.y * GetWidth()) + drawPoint.x] = pen;
				}
				break;
			case 5:
				while (drawPoint.x > B.x&& drawPoint.y < B.y)
				{
					switch (directInt)
					{
					case 0:
						drawPoint.x++;
						break;
					case 1:
						drawPoint.x++;
						drawPoint.y--;
						break;
					case 2:
						drawPoint.y--;
						break;
					case 3:
						drawPoint.y--;
						drawPoint.x--;
						break;
					case 4:
						drawPoint.x--;
						break;
					case 5:
						drawPoint.x--;
						drawPoint.y++;
						break;
					case 6:
						drawPoint.y++;
						break;
					case 7:
						drawPoint.x++;
						drawPoint.y++;
						break;
					default:
						break;
					}
					directInt = DirectLine(slope, directInt, A, drawPoint);
					pixels[(drawPoint.y * GetWidth()) + drawPoint.x] = pen;
				}
				break;
			default:
				break;
			}
		}
		unlockTexture();
	}
}
int ATexture::DirectLine(float slope, int direction, SDL_Point originalPoint, SDL_Point currentPoint)
{
	if (direction < 0 || direction > 7)
		return -1;
	else
	{
		SDL_Point newPoint;
		float lowest_dS = -999999;
		int newDirection[3] = { direction - 1,direction,direction + 1 };
		int returnInt = -1;
		if (newDirection[0] == -1)
			newDirection[0] = 7;
		if (newDirection[2] == 8)
			newDirection[2] = 0;
		for (int i = 0; i < 3; i++)
		{
			float dS = 0.0f;
			newPoint = currentPoint;
			switch (newDirection[i])
			{
			case 0:
				newPoint.x++;
				break;
			case 1:
				newPoint.x++;
				newPoint.y--;
				break;
			case 2:
				newPoint.y--;
				break;
			case 3:
				newPoint.y--;
				newPoint.x--;
				break;
			case 4:
				newPoint.x--;
				break;
			case 5:
				newPoint.x--;
				newPoint.y++;
				break;
			case 6:
				newPoint.y++;
				break;
			case 7:
				newPoint.x++;
				newPoint.y++;
				break;
			default:
				break;
			}
			dS = ((float)(newPoint.y - originalPoint.y) / (float)(newPoint.x - originalPoint.x));
			if (abs(slope - dS) < abs(slope - lowest_dS))
			{
				returnInt = newDirection[i];
				lowest_dS = dS;
			}
		}
		return returnInt;
	}
}

//List of Texture's, organizes by Layer upon insertion then by time of insertion if on the same layer
struct TextureListNode
{
	bool SelfDelete = false;
	SDL_Texture* texture = nullptr;
	SDL_Rect* bounds = nullptr;
	SDL_Rect* stretch = nullptr;
	int layer = 0;

	void Destruct()
	{
		if (texture)
			SDL_DestroyTexture(texture);
		if (bounds)
		{
			delete bounds;
			bounds = nullptr;
		}
		if (stretch)
		{
			delete stretch;
			stretch = nullptr;
		}
		layer = 0;
	}
};
class TextureList
{
	public:
		std::vector<TextureListNode> list;
		void insert(bool selfDestruct, SDL_Texture* t, SDL_Rect *r, SDL_Rect *s, int l)
		{
			std::vector<TextureListNode>::iterator it = list.begin();
			while(it != list.end())
			{
				if (it->layer > l)
					break;
				it++;
			}
			list.insert(it, TextureListNode{ selfDestruct, t,r, s,l });
		};
		void insert(SDL_Texture* t, SDL_Rect &r, int l)
		{
			SDL_Rect* ptr = new SDL_Rect(r);
			insert(true,t, ptr, nullptr, l);
		}
		void insert(SDL_Texture* t, SDL_Rect* r, int l)
		{
			insert(false, t, r, nullptr, l);
		}
		void insert(SDL_Texture* t, SDL_Rect* r, SDL_Rect* s, int l)
		{
			insert(false, t, r, s, l);
		}

		~TextureList()
		{
			for(int i = 0; i < list.size();i++)
				if (list[i].SelfDelete)
				{
					list[i].Destruct();
				}
		}
};

//List of Rect's and SDL_Colors to Draw for Debug Purposes
struct DebugRectNode
{
	DebugRectNode* next;
	SDL_Color* pen;
	SDL_Rect* rect;

	DebugRectNode(SDL_Rect* r, SDL_Color* c);
};
DebugRectNode::DebugRectNode(SDL_Rect* r = nullptr, SDL_Color* c = nullptr)
{
	next = nullptr;
	pen = c;
	rect = r;
}
class DebugRectList
{
private:
	DebugRectNode* head = nullptr;
	DebugRectNode* tail = nullptr;
	int count = 0;

	DebugRectNode* Find(SDL_Rect* r);
public:
	void Insert(SDL_Rect* r, SDL_Color* p);
	bool Remove(SDL_Rect* r);
	int Size() { return count; };

	DebugRectNode *operator[](int num) 
	{
		if (num < 0 || num > count)
			return nullptr;

		DebugRectNode* temp = head;
		for (int i = 0; i < num; i++)
			temp = temp->next;
		return temp;
	}
};
DebugRectNode* DebugRectList::Find(SDL_Rect* r)
{
	DebugRectNode* temp = head;

	while (temp->next)
	{
		if (temp->next->rect)
			return temp;
		temp = temp->next;
	}
	return nullptr;
}
void DebugRectList::Insert(SDL_Rect* r, SDL_Color* p)
{
	if (tail)
	{
		tail->next = new DebugRectNode(r, p);
		tail = tail->next;
	}
	else
		head = tail = new DebugRectNode(r, p);
	count++;
}
bool DebugRectList::Remove(SDL_Rect* r)
{
	DebugRectNode *temp = head;
	if (temp)
	{
		if (head->rect == r)
		{
			if (temp->next)
				head = temp->next;
			else
				head = nullptr;
			delete temp;
			return true;
		}
		else
		{
			if (tail->rect == r)
			{
				while (temp->next != tail)
					temp = temp->next;
				temp->next = nullptr;
				delete tail;
				return true;
			}
			else
			{
				temp = Find(r);
				if (temp != nullptr)
				{
					temp->next = temp->next->next;
					delete temp->next;
					return true;
				}
			}
		}
	}
	return false;
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

	ATexture background;
	TextureList tList;
	DebugRectList debugRect;
	std::vector<SDL_Point*> dPoint;

	SDL_Window* gWind = nullptr;
	SDL_DisplayMode dMode;
	SDL_Renderer* gRend = nullptr;
	SDL_Event gEvent;
	Uint32 winID = 0;
	
	const static uint32_t aMask = 0x000000ff;
	const static uint32_t bMask = 0x0000ff00;
	const static uint32_t gMask = 0x00ff0000;
	const static uint32_t rMask = 0xff000000;

	SIMaster layerMaster;


	PixENG();
	~PixENG();
	virtual void OnStart();
	virtual bool OnUpdate(float dT);
	virtual void OnExit() {};
	bool Init(int screenW, int screenH);
	void Start(int pixW, int pixH);
	int GameLoop();

	static SDL_Texture* CreateTextureFromPixels(Pixel* pixels, SDL_Renderer* r, int w, int h);
	static SDL_Texture* CreateTransparentTexture(SDL_Renderer* r, int w, int h);
	SDL_Texture* CreateTextureFromPixels(Pixel *pixels, int w, int h);
	void SetDrawColor(Pixel c)
	{
		background.SetColor(c);
	};
	Sprite DrawRect(SDL_Rect pos, unsigned int borderT);
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
	SDL_GL_SetSwapInterval(0);
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

		layerMaster.CollisionCheck();

		////RENDER
		//Clear Screen
		SDL_SetRenderDrawColor(gRend, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRend);

		//Draw Textures
		for (int i = 0; i < tList.list.size(); i++)
		{
			SDL_Rect* stretch = tList.list[i].stretch != nullptr ? tList.list[i].stretch : tList.list[i].bounds;
			if (SDL_RenderCopy(gRend, tList.list[i].texture, nullptr,stretch) < 0)
				printf("Unable to render texture %i! SDL Error: %s\n", i, SDL_GetError());
		}
		//Debug Draw
		if (debug)
		{
			SDL_Color currentPen;
			SDL_SetRenderDrawColor(gRend, 0, 255, 0, 255);
			for (int i = 0; i < debugRect.Size(); i++)
			{
				currentPen = *debugRect[i]->pen;
				SDL_SetRenderDrawColor(gRend, currentPen.r, currentPen.g, currentPen.b, currentPen.a);
				SDL_RenderDrawRect(gRend, debugRect[i]->rect);
			}
			SDL_SetRenderDrawColor(gRend, 255, 0, 0, 255);
			for (int i = 0; i < dPoint.size(); i++)
				SDL_RenderDrawPoint(gRend, dPoint[i]->x, dPoint[i]->y);
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
		SDL_Delay(50);
	}
	return 0;
}


//Loads Texture from file
SDL_Texture* loadTexture(std::string path, SDL_Renderer *r)
{
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		newTexture = SDL_CreateTextureFromSurface(r, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		SDL_FreeSurface(loadedSurface);
	}
	return newTexture;
}
SDL_Texture* loadTexture(std::string path, SDL_Renderer *r, Pixel maskColor)
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
			newT = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
			SDL_SetTextureBlendMode(newT, SDL_BLENDMODE_BLEND);
			if (!newT)
			{
				printf("Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				
				Uint32 pFormat = SDL_PIXELFORMAT_RGBA8888;
				SDL_PixelFormat* mappingFormat = SDL_AllocFormat(pFormat);
				Uint32 colorKey = SDL_MapRGB(mappingFormat, (int)maskColor.r, (int)maskColor.g, (int)maskColor.b);
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
SDL_Texture *PixENG::CreateTextureFromPixels(Pixel* p, int w, int h)
{
	SDL_Surface* loadSurface = SDL_CreateRGBSurface(0, w, h, 32, rMask, gMask, bMask, aMask);
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
					pixels[i] = SDL_MapRGBA(mappingFormat, p[i].r, p[i].g, p[i].b, p[i].a);
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
SDL_Texture* PixENG::CreateTextureFromPixels(Pixel* p, SDL_Renderer *r, int w, int h)
{
	SDL_Surface* loadSurface = SDL_CreateRGBSurface(0, w, h, 32, rMask, gMask, bMask, aMask);
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
			newT = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
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
					pixels[i] = SDL_MapRGBA(mappingFormat, p[i].r, p[i].g, p[i].b, p[i].a);
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
SDL_Texture* PixENG::CreateTransparentTexture(SDL_Renderer* r, int w, int h)
{
	SDL_Surface* loadSurface = SDL_CreateRGBSurface(0, w, h, 32, rMask, gMask, bMask, aMask);
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
			newT = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
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
				Uint32* pixels = nullptr;
				Uint32 transparent = SDL_MapRGBA(mappingFormat, 0xff, 0x00, 0xff, 0x00);;
				int pitch = 0;

				SDL_LockTexture(newT, NULL, &pixelVoid, &pitch);
				memcpy(pixelVoid, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h);

				pixels = (Uint32*)pixelVoid;
				for (int i = 0; i < w * h; i++)
					pixels[i] = transparent;
				SDL_UnlockTexture(newT);
				SDL_FreeSurface(formattedSurface);
				SDL_FreeFormat(mappingFormat);
			}
		}
	}
	SDL_ClearError();
	return newT;
}
void UpdateTexture(SDL_Texture *texture, Pixel* pArray, int count)
{
	Uint32* lockedPixels;
	int pitch;

	SDL_LockTexture(texture,NULL, reinterpret_cast< void** >( &lockedPixels),&pitch);
	for (int i = 0; i < count; i++)
		lockedPixels[i] = pArray[i].c;
	SDL_UnlockTexture(texture);
}