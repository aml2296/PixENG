#include"PixENG.h"
#include<random>
#include<ctime>

class Players
{
public:
	SDL_Rect position;
	Sprite sprite;
	int xAxis = 0;
	int yAxis = 0;

	float speedX = 0;
	float speedY = 0;
	const float startSpeed = 1;
	const float acceleration = 1.0025f;

	const float maxSpeed = 2.0f;
	const float minSpeed = 0.125f;

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
	bool HandlePlayerState()
	{
		adjustSpeed(acceleration, speedX, xAxis);
		adjustSpeed(acceleration, speedY, yAxis);
		printf("X: %f || Y: %f\n", speedX, speedY);
		Move({ (int)speedX, (int)speedY });
		return true;
	}
	void adjustSpeed(float accel, float &speed, int &axis)
	{
		if (speed != 0)
		{
			//If Axis direction is not equal to Speed Direction then Deccelerate
			if ((speed * axis) <= 0)
				speed += (1 / (3 * accel) * (speed > 0) ? -1 : 1);
			else
				speed += accel * (speed > 0) ? 1 : -1;

			if (speed > maxSpeed)
				speed = maxSpeed;
			else if (speed < -1 * maxSpeed)
				speed = -1 * maxSpeed;
			else if ((speed > 0 && speed < minSpeed) || (speed < 0 && speed > -minSpeed))
				speed = 0;
		}
		else
		{
			if (axis > 0)
				speed = startSpeed;
			else if (axis < 0)
				speed = -1 * startSpeed;
		}
	}
	void Move(SDL_Point p)
	{
		position.x += p.x;
		position.y += p.y;

		sprite.pos.x += p.x;
		sprite.pos.y += p.y;
	};
	void MoveTo(SDL_Point p)
	{
		position.x = sprite.pos.x = p.x;
		position.y = sprite.pos.y = p.y;
	}
	Players()
	{
	};
	~Players()
	{

	};
};

class Grid : public ATexture
{
private:
	const static int gridMin = 4;
	int gridMax = gridMin;
	int thickness = 1;
	int gridSize = gridMin;
public:
	Grid(SDL_Renderer *r,int width, int height);
	bool Build();
	Grid() {};
	bool SetThickness(int t);
	int GetThickness() { return thickness; };
	bool SetGridSize(int s);
	int GetGridSize() { return gridSize; };
};
Grid::Grid(SDL_Renderer *r, int width, int height)
{
	Pixel* pArray = new Pixel[width * height];
	for (int i = 0; i < width * height; i++)
		pArray[i] = Pixel{ 0,0,0,0 };
	SetTexture(PixENG::CreateTransparentTexture(r, width, height), r, width, height);
	delete[] pArray;
}
bool Grid::SetThickness(int t)
{
	if (t > 1)
	{
		if (t <= gridMax)
		{
			thickness = t;
			return true;
		}
		else
		{
			thickness = gridMax;
			return false;
		}
	}
	else
	{
		thickness = 1;
		return false;
	}
}
bool Grid::SetGridSize(int s)
{
	if (s >= 4)
	{
		if (s > GetWidth() || s > GetHeight())
		{
			gridSize = (GetWidth() <= GetHeight()) ? GetWidth() : GetHeight();
			return false;
		}
		else
		{
			gridSize = s;
			return true;
		}
	}
	else
	{
		gridSize = gridMin;
		return false;
	}
}
bool Grid::Build()
{
	if (GetHeight() >= GetWidth())
		gridMax = 0.5 * GetHeight();
	else
		gridMax = 0.5 * GetWidth();

	if (thickness < 1)
		thickness = 1;
	else if (thickness > gridMax)
		thickness = gridMax;

	if (gridSize < 0)
		gridSize = gridMin;
	
	if (lockTexture())
	{

		Uint32* pixels = (Uint32*)GetPixels();
		SDL_PixelFormat* mappingFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
		Pixel color = GetColor();
		Uint32 pen = SDL_MapRGBA(mappingFormat, color.r, color.g, color.b, color.a);
		SDL_SetTextureBlendMode(GetTexture(), SDL_BLENDMODE_BLEND);

		for (int y = 0; y < GetHeight(); y += gridSize)
			for (int x = 0; x < GetWidth(); x += gridSize)
				for (int pY = 0; pY < gridSize; pY++)
					for (int pX = 0; pX < gridSize; pX++)
						if (pY>=gridSize - thickness|| pX >= gridSize - thickness)
							pixels[(x + pX) + ((pY + y) * GetWidth())] = pen;
		unlockTexture();
	}
	else
		return false;
	return true;
}

class BounceBox : public CollisionBox
{
public:
	float xVel;
	float yVel;
	void OnCollision(void *other) override
	{
		BounceBox* otherPtr = (BounceBox*)other;
		if (yVel <= 0)
			yVel *= -1;
		else if (otherPtr->yVel * yVel > 0)
			if (abs(yVel) > abs(otherPtr->yVel))
				yVel = yVel + (yVel - otherPtr->yVel);
			else
				yVel = yVel - (otherPtr->yVel - yVel);

		if (otherPtr->xVel * xVel < 0)
			xVel *= -1;
		else if (otherPtr->xVel * xVel > 0)
			if (abs(xVel) > abs(otherPtr->xVel))
				xVel = xVel + (xVel - otherPtr->xVel);
			else
				xVel = xVel - (otherPtr->xVel - xVel);
	}
};

class OutDoors : public PixENG
{
	int i = 0;
public:
	Players p1;
	SINet physNet;
	SDL_Color ColorA = { 255,0,0,255 };
	SDL_Color ColorB = { 0,255,255,255 };
	BounceBox boxA;
	BounceBox boxB;
	BounceBox boxC;
	SDL_Rect* newRect;
	SDL_Point* pts;

	void OnStart() override
	{
		srand(time(NULL));
		//Init Name
		sAppName = "PixENG.exe v(0.0.2)";
		debug = true;
		physNet.setRadius(100);
		if (!InitAssests())
			printf("Could not init all Assests!!!");
	}
	bool OnUpdate(float dT) override
	{
		p1.HandleInput();
		p1.HandlePlayerState();

		if (pts)
			delete[] pts;
		if (newRect)
			delete newRect;



		boxA.pBoundingBox.x += (int)boxA.xVel;
		boxA.pBoundingBox.y += (int)boxA.yVel;
		boxB.pBoundingBox.x += (int)boxB.xVel;
		boxB.pBoundingBox.x += (int)boxB.xVel;
		boxC.pBoundingBox.y += (int)boxC.yVel;
		boxC.pBoundingBox.y += (int)boxC.yVel;

		int length;

		pts = physNet.RadiusInTotalSystem(length);
		dPoint.clear();
		for (int i = 0; i < length; i++)
			dPoint.push_back(pts + i);

		return true;
	}
	bool InitAssests()
	{
		Pixel color = { (uint8_t)(rand() % 255),(uint8_t)(rand() % 255) ,(uint8_t)(rand() % 255) };

		background.SetTexture(CreateTextureFromPixels(&color, 1, 1), gRend, 1, 1);
		background.SetColor(Pixel{ 255,255,255 });
		tList.insert(background.GetTexture(), &background.getRect(), new SDL_Rect{ 0,0,screenW,screenH }, 0);

		p1.sprite = Sprite(nullptr, 2, 0, 0);
		p1.sprite.SetTexture(loadTexture("phil.png", gRend));
		p1.sprite.pos.w *= rPixW;
		p1.sprite.pos.h *= rPixH;
		tList.insert(p1.sprite.GetTexture(), &p1.position,&p1.sprite.pos, p1.sprite.nLayer);
	
		
		boxA.pBoundingBox = { 440,500,50,50 };
		boxA.xVel = 1;
		boxB.pBoundingBox = { 700,500,50,50 };
		boxB.xVel = -1;
		boxC.pBoundingBox = { 500,600,50,50 };
		boxC.yVel = -1;



		//NESW[0].pBoundingBox = { 0,0,screenW,(int)((1.0f / 20) * screenH) };
		//NESW[1].pBoundingBox = { screenW - (int)((1.0f / 20) * screenW),0,(int)((1.0f / 20) * screenW),screenH };
		//NESW[2].pBoundingBox = { 0,screenH - (int)((1.0f / 20) * screenH),screenW,(int)((1.0f / 20) * screenH) };
		//NESW[3].pBoundingBox = { 0,0,(int)((1.0f / 20) * screenW),screenH };

		//for (int i = 0; i < 4; i++)
		//	layerMaster.InsertCollisionBox(&NESW[i], 0);


		layerMaster.InsertCollisionBox(&boxA, 0);
		debugRect.Insert(&boxA.pBoundingBox, &ColorA);
		layerMaster.InsertCollisionBox(&boxB, 0);
		debugRect.Insert(&boxB.pBoundingBox, &ColorA);
		layerMaster.InsertCollisionBox(&boxC, 2);
		debugRect.Insert(&boxC.pBoundingBox, &ColorB);

		return true;
	}
	void OnExit()
	{

	}
};

int main()
{
	OutDoors eng;
	
	if (eng.Init(1280, 960))
		eng.Start(5, 5);
	return 0;
}