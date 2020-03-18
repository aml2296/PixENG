#include"PixENG.h"
#include<random>
#include<ctime>

class test : public PixENG
{
	std::vector<Pixel> playerModel; //7 x 16


	void OnStart() override
	{
		sAppName = "PixENG.exe v(0.0.1)";
		BuildPlayerModel(0,0);
	}
	bool OnUpdate(float dT) override
	{
		//Draw BackGround
		for (int x = 0; x < pixelW; x++)
			for (int y = 0; y < pixelH; y++)
			{
				if (y <= pixelH / 10 * 7)
				{
					pixelArray[x + (pixelW * y)].r = 111;
					pixelArray[x + (pixelW * y)].g = 220;
					pixelArray[x + (pixelW * y)].b = 237;
				}
				else
				{
					pixelArray[x + (pixelW * y)].r = 237;
					pixelArray[x + (pixelW * y)].g = 193;
					pixelArray[x + (pixelW * y)].b = 126;
				}
			}
		//Draw Player
		for (int x = 0; x < 7 ; x++)
			for (int y = 0; y < 2; y++)
				pixelArray[x + (pixelW * y)] = playerModel[x + (y * 7)];



		return true;
	}
	void BuildPlayerModel(int x, int y)
	{
		playerModel.push_back(Pixel(SDL_Rect{ x,y,(int)rPixW,(int)rPixH }, 0, 0, 0, 0));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW, y,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 2,y,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 3, y,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 4,y,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 5,y,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 6,y,(int)rPixW,(int)rPixH }, 0, 0, 0, 0));

		playerModel.push_back(Pixel(SDL_Rect{ x ,(int)rPixH ,y + (int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW,y + (int)rPixH ,(int)rPixW,(int)rPixH }, 0, 0, 0, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 2,y + (int)rPixH,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 3,y + (int)rPixH,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 4,y + (int)rPixH,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 5,y + (int)rPixH,(int)rPixW,(int)rPixH }, 0, 0, 0, 255));
		playerModel.push_back(Pixel(SDL_Rect{ x + (int)rPixW * 6,y + (int)rPixH,(int)rPixW,(int)rPixH }, 255, 255, 255, 255));
	}
};

int main()
{
	srand(time(nullptr));
	test eng;
	if (eng.Init(800, 600))
		eng.Start(5, 5);
	return 0;
}