#include"PixENG.h"
#include<random>
#include<ctime>

class test : public PixENG
{
	void OnStart() override
	{
		//Init Name
		sAppName = "PixENG.exe v(0.0.2)";

		Sprite *bBaller = new Sprite(loadTexture("BballSprite2.png"));
		bBaller->pos.y = 0;
		bBaller->pos.x = 0;
		bBaller->pos.w = 32 * rPixW;
		bBaller->pos.h = 32 * rPixH;
		fTextureArray.push_back(bBaller);
	}
	bool OnUpdate(float dT) override
	{
		//Draw BackGround

		return true;
	}
};

int main()
{
	srand(time(nullptr));
	test eng;
	if (eng.Init(1280, 960))
		eng.Start(10, 10);
	return 0;
}