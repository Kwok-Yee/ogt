#pragma once
#include "LTexture.h"
#include "PacketStream.h"
#include "SDL_net.h"

class Dot
{
public:
	//The dimensions of the dot
	static const int DOT_WIDTH = 20;
	static const int DOT_HEIGHT = 20;

	//Maximum axis velocity of the dot
	static const int DOT_VEL = 10;

	Dot() {};
	//Initializes the variables
	Dot(bool host, int id);
	~Dot();

	void Init(SDL_Renderer *gRenderer);
	//Takes key presses and adjusts the dot's velocity
	void handleEvent(SDL_Event& e);

	//Moves the dot
	void move(int SCREEN_HEIGHT, int SCREEN_WIDTH);

	//Shows the dot on the screen
	void render(SDL_Renderer *gRenderer);

	std::string GetPosAsString();

	bool CheckCollision(int centerX, int centerY);

	bool GetLocal() { return isLocalplayer; };

	int GetCenterX();
	int GetCenterY();

	void SetPosition(int x, int y);
	int GetId() { return id; }
	void SetIsCaught(int state) { caught = state; }
	int GetIsCaught() { return caught; }

private:
	bool isLocalplayer;
	bool isChaser;
	int id;
	int caught;

	//The X and Y offsets of the dot
	int mPosX, mPosY;

	int width, height;

	int mCenterX, mCenterY;
	//The velocity of the dot
	int mVelX, mVelY;

	LTexture gDotTexture;
	PacketStream packetStream;
};