#include"PixENG.h"
#include<random>
#include<ctime>

class bBall : public PixENG
{
	std::vector<PhysEntity*> bEntities;

	void OnStart() override
	{
		//Init Name
		sAppName = "PixENG.exe v(0.0.2)";
		debug = true;
		if (!InitAssests())
			printf("Could not init all Assests!!!");
		for (int i = 0; i < bEntities.size(); i++)
			dRect.push_back(bEntities[i]->pBounds());
	}
	bool OnUpdate(float dT) override
	{
		////Phys Related
		//Move
		for (int i = 0; i < bEntities.size(); i++)
		{
			if (bEntities[i]->Gravity())
				bEntities[i]->AddVelocity(0.0f, bEntities[i]->GravityValue());
			bEntities[i]->ApplyVelocity();
		}

		//Check for Collisions
		if (bEntities.size() > 1)
		{
			for (int i = 0; i < bEntities.size(); i++)
			{
				for (int n = i + 1; n < bEntities.size(); n++)
					if (PhysicsAsset::CollisionCheck(*bEntities[i]->pBounds(), *bEntities[n]->pBounds()))
					{
						bEntities[i]->HandleCollision(*bEntities[n]->pBounds());
					}
			}
		}
		return true;
	}
	bool InitAssests()
	{
		bool success = true;

		PhysEntity* bBaller, * bCourt;
		Sprite* asset;
		
		//Background
		asset = new Sprite(loadTexture("Backdrop_1.png"));
		asset->pos.w *= rPixW;
		asset->pos.h *= rPixH;
		asset->pos.x = -0.5 * (asset->pos.w - screenW);
		asset->pos.y = -(asset->pos.h - screenH);
		tList.insert(asset->texture, &asset->pos, 0);

		//Bballer
		asset = new Sprite(loadTexture("BballSprite2.png"));
		asset->pos.w *= rPixW;
		asset->pos.h *= rPixH;
		asset->nLayer = 1;
		bBaller = new PhysEntity(asset);
		bBaller->setGravity(0.025f);
		bBaller->AddVelocity(0.0f, 5.0f);
		tList.insert(asset->texture, &asset->pos, &asset->nLayer);
		bEntities.push_back(bBaller);

		//Court
		asset = new Sprite(loadTexture("Bball_Court.png"));
		asset->pos.w *= rPixW;
		asset->pos.h *= rPixH;
		bCourt = new PhysEntity(asset, SDL_Rect{ 0,screenH - 10,asset->pos.w,10 * (int)rPixH });
		bCourt->Move(-0.5 * (asset->pos.w - screenW), -(asset->pos.h - screenH));
		bCourt->setGravity(0.0f);
		tList.insert(asset->texture, &asset->pos, 0);
		bEntities.push_back(bCourt);

		
		
		//bBaller #2
		/*
		asset = new Sprite(loadTexture("BballSprite2.png"));
		asset->pos.w *= rPixW;
		asset->pos.h *= rPixH;
		asset->pos.x += asset->pos.w * 2;
		fTextureArray.push_back(asset);
		*/


		return success;
	}
};

int main()
{
	srand(time(nullptr));
	bBall eng;
	if (eng.Init(1280, 960))
		eng.Start(5, 5);
	return 0;
}