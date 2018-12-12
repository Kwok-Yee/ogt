#include "Dot.h"

Dot::Dot(bool host, int id)
{
	
	isChaser = host;
	isLocalplayer = !host;
	this->id = id;

	//Initialize the offsets
	mPosX = 50;
	mPosY = 50;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;

	if (isChaser)
	{
		gDotTexture.setColor(244, 66, 66);
	}
	else
	{
		gDotTexture.setColor(66, 69, 244);
	}

	width = 10;
	height = 10;
}

Dot::~Dot()
{
	gDotTexture.free();
}

void Dot::SetPosition(int x, int y)
{
	mPosX = x;
	mPosY = y;

	mCenterX = mPosX + (width / 2);
	mCenterY = mPosY + (height / 2);
}

//"dot.bmp"
void Dot::Init(SDL_Renderer *gRenderer)
{
	if (isChaser)
	{
		if (!gDotTexture.loadFromFile("Data/reddot.png", gRenderer))
		{
			printf("Failed to load dot texture!\n");
		}
	}
	else {
		if (!gDotTexture.loadFromFile("Data/bluedot.png", gRenderer))
		{
			printf("Failed to load dot texture!\n");
		}
	}
}

void Dot::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_w: mVelY -= DOT_VEL; break;
		case SDLK_s: mVelY += DOT_VEL; break;
		case SDLK_a: mVelX -= DOT_VEL; break;
		case SDLK_d: mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_w: mVelY += DOT_VEL; break;
		case SDLK_s: mVelY -= DOT_VEL; break;
		case SDLK_a: mVelX += DOT_VEL; break;
		case SDLK_d: mVelX -= DOT_VEL; break;
		}
	}

}

void Dot::move(int SCREEN_HEIGHT, int SCREEN_WIDTH)
{
	//Move the dot left or right
	mPosX += mVelX;

	//If the dot went too far to the left or right
	if ((mPosX < 0) || (mPosX + DOT_WIDTH > SCREEN_WIDTH))
	{
		//Move back
		mPosX -= mVelX;
	}

	//Move the dot up or down
	mPosY += mVelY;

	//If the dot went too far up or down
	if ((mPosY < 0) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT))
	{
		//Move back
		mPosY -= mVelY;
	}

	mCenterX = mPosX + (width / 2);
	mCenterY = mPosY + (height / 2);

}

void Dot::render(SDL_Renderer *gRenderer)
{
	//Show the dot
	gDotTexture.render(mPosX, mPosY, gRenderer);
}

std::string Dot::GetPosAsString()
{
	return std::string("X: "+ std::to_string(mPosX) + ", " + "Y: " + std::to_string(mPosY));
}

int Dot::GetCenterX()
{
	return mCenterX;
}

int Dot::GetCenterY()
{
	return mCenterY;
}

bool Dot::CheckCollision(int centerX, int centerY)
{
	int distance = sqrt(((mCenterX - centerX) * (mCenterX - centerX)) + ((mCenterY - centerY) * (mCenterY - centerY)));
	
	if (distance <= width)
	{
		return true;
	}
	else
	{
		return false;
	}
}