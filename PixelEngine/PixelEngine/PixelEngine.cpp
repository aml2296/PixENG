#include"PixENG.h"
#include<random>
#include<ctime>


class Players : public PhysEntity
{
public:
	float xAxis = 0;
	float yAxis = 0;

	CollisionBox groundCheck;

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
		SetVelocity(xAxis, yAxis);
	};

	Players(Sprite* s,float rPixW, float rPixH ) : PhysEntity(s)
	{
		pBoundingBox.w = s->pos.w *= rPixW;
		pBoundingBox.h = s->pos.h *= rPixH;
		s->nLayer = 1;
		setGravity(0);

		groundCheck.pBoundingBox = SDL_Rect{ 0,pBoundingBox.h,pBoundingBox.w, 1 * (int)rPixH };
	}
	~Players()
	{

	}
};


class bBallGame : public PixENG
{
	std::vector<PhysEntity*> pEntitities;
	Players *p;


	void OnStart() override
	{
		//Init Name
		sAppName = "PixENG.exe v(0.0.2)";
		debug = true;
		if (!InitAssests())
			printf("Could not init all Assests!!!");
		for (int i = 0; i < pEntitities.size(); i++)
			dRect.push_back(pEntitities[i]->pBounds());
	}
	bool OnUpdate(float dT) override
	{
		////Handle Player Input
		//
		if (p)
			p->HandleInput();

		////Phys Related
		//Move
		for (int i = 0; i < pEntitities.size(); i++)
		{
			if (pEntitities[i]->Gravity())
				pEntitities[i]->AddVelocity(0.0f, pEntitities[i]->GravityValue());
			pEntitities[i]->ApplyVelocity();
		}

		//Check for Collisions
		if (pEntitities.size() > 1)
		{
			for (int i = 0; i < pEntitities.size(); i++)
			{
				for (int n = i + 1; n < pEntitities.size(); n++)
					if (PhysicsAsset::CollisionCheck(*pEntitities[i]->pBounds(), *pEntitities[n]->pBounds()))
					{
						pEntitities[i]->HandleCollision(*pEntitities[n]->pBounds());
					}
			}
		}
		return true;
	}
	bool InitAssests()
	{
		bool success = true;

		PhysEntity* bCourt;
		Sprite* asset;
		
		//Background
		asset = new Sprite(loadTexture("Backdrop_1.png"));
		asset->pos.w *= rPixW;
		asset->pos.h *= rPixH;
		tList.insert(asset->texture, &asset->pos, 0);

		//Bballer
		p = new Players(new Sprite(PixENG::loadTexture("BballSprite2.png")), rPixW, rPixH);
		tList.insert(p->getTexture(), p->pBounds(), 1);
		pEntitities.push_back(p);
		dRect.push_back(&p->groundCheck.pBoundingBox);


		//Court
		
		asset = new Sprite(loadTexture("Bball_Court.png", Pixel{ 255,0,255 }));
		asset->pos.w *= rPixW;
		asset->pos.h *= rPixH;
		bCourt = new PhysEntity(asset, SDL_Rect{ 0,screenH - 10,asset->pos.w,10 * (int)rPixH });
		bCourt->Move(-0.5 * (asset->pos.w - screenW), -(asset->pos.h - screenH));
		bCourt->setGravity(0.0f);
		tList.insert(asset->texture, &asset->pos, 0);
		pEntitities.push_back(bCourt);

		return success;
	}
};

int main()
{
	bBallGame eng;
	if (eng.Init(1280, 960))
		eng.Start(5, 5);
	return 0;
}