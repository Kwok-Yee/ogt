#include "Game.h"
#include "InputHandler.h"
#include <iostream>
#include "PacketStream.h"

using namespace std;

Game *Game::instance = 0;
SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(1);

Game::Game()
{
	portNumber = 6789;
	host = "localhost";
	timeLimit = 10000;
}

bool Game::init(const char *title, int xPosition, int yPosition, int height, int width, bool fullScreen)
{
	int flags = SDL_WINDOW_SHOWN;
	this->height = height;
	this->width = width;
	if (fullScreen) {
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	SDLNet_Init();

	window = SDL_CreateWindow(title, xPosition, yPosition, height, width, flags);

	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!renderer) {
		return false;
	}

	// Create the socket set with enough space to store our desired number of connections (i.e. sockets)
	if (socketSet == NULL)
	{
		cout << "Failed to allocate the socket set: " << SDLNet_GetError() << endl;
		exit(-1); // Quit!
	}
	else
	{
		cout << "Successfully allocated socket set." << endl;
	}

	// Try to resolve the host. If successful, this places the connection details in the serverIP object
	int hostResolved = SDLNet_ResolveHost(&ip, host, portNumber);

	if (hostResolved == -1)
	{
		cout << "Failed to resolve the server hostname: " << SDLNet_GetError() << "\nContinuing...\n";
	}
	else // If we successfully resolved the host then output the details
	{
		// Get our IP address in proper dot-quad format by breaking up the 32-bit unsigned host address and splitting it into an array of four 8-bit unsigned numbers...
		Uint8 * dotQuad = (Uint8*)&ip.host;

		//... and then outputting them cast to integers. Then read the last 16 bits of the serverIP object to get the port number
		cout << "Successfully resolved host to IP: " << (unsigned short)dotQuad[0] << "." << (unsigned short)dotQuad[1] << "." << (unsigned short)dotQuad[2] << "." << (unsigned short)dotQuad[3];
		cout << " port " << SDLNet_Read16(&ip.port) << endl << endl;
	}

	// Try to resolve the IP of the server, just for kicks
	if ((server = SDLNet_ResolveIP(&ip)) == NULL)
	{
		cout << "Failed to resolve the server IP address: " << SDLNet_GetError() << endl;
	}
	else
	{
		cout << "Successfully resolved IP to host: " << server << endl;
	}

	// Flag to keep track of when to disconnect and finish up. We initially set it so that we CANNOT connect, and only change this to false when we got an "OK" response from the server
	bool shutdownClient = true;

	// Try to open a connection to the server and quit out if we can't connect
	client = SDLNet_TCP_Open(&ip);
	if (!client)
	{
		cout << "Failed to open socket to server: " << SDLNet_GetError() << "\n";
		exit(-1);
	}
	else // If we successfully opened a connection then check for the server response to our connection
	{
		SDLNet_TCP_AddSocket(socketSet, client);
	}
	// Wait for up to five seconds for a response from the server
	// Note: If we don't check the socket set and WAIT for the response, we'll be checking before the server can respond, and it'll look as if the server sent us nothing back
	int activeSockets = SDLNet_CheckSockets(socketSet, 5000);

	cout << "There are " << activeSockets << " socket(s) with data on them at the moment." << endl;

	if (SDLNet_SocketReady(client) != 0)
	{
		memset(tmp, 0, sizeof(tmp));
		SDLNet_TCP_Recv(client, tmp, maxMessageLength);
		int num = tmp[0] - '0';
		int j = 1;
		while (tmp[j] >= '0' && tmp[j] <= '9')
		{
			num *= 10;
			num += tmp[j] - '0';
			j++;
			cout << num << endl;
		}
		if (num == 0) {
			int tmpvar;
			sscanf_s(tmp, "0 %d", &tmpvar);
			Dot* dot;
			if (host == "localhost") {
				you = new Dot(true, tmpvar);
				other = new Dot(false, tmpvar + 1);
				other->SetPosition(200, 200);
			}
			else {
				you = new Dot(false, tmpvar);
				you->SetPosition(200, 200);
				other = new Dot(true, tmpvar - 1);
			}
			players.push_back(you);
			players.push_back(other);
			you->Init(renderer);
			other->Init(renderer);
		}
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	running = true;
	return true;
}

void Game::render()
{
	SDL_RenderClear(renderer);
	for (size_t i = 0; i < players.size(); i++)
	{
		players[i]->render(renderer);
	}

	SDL_RenderPresent(renderer);
}

void Game::handleEvents()
{
	SDL_Event e;
	if (SDL_PollEvent(&e) != 0) {
		you->handleEvent(e);
	}
}

void Game::update()
{
	SDLNet_CheckSockets(socketSet, 0);
	if (SDLNet_SocketReady(client) != 0)
	{
		memset(tmp, 0, sizeof(tmp));
		SDLNet_TCP_Recv(client, tmp, maxMessageLength);
		int num = tmp[0] - '0';
		int j = 1;
		while (tmp[j] >= '0' && tmp[j] <= '9')
		{
			num *= 10;
			num += tmp[j] - '0';
			j++;
		}
		if (num == 2)
		{
			cout << "YOU WIN" << endl;
		}
		else if (num == 3) {
			cout << "Chaser got the player!" << endl;
		}
		else {
			packetStream.fromCharArray(tmp);
			int x, y;
			packetStream.readInt(x);
			packetStream.readInt(y);
			int tempId;
			packetStream.readInt(tempId);
			int caught;
			packetStream.readInt(caught);
			other->SetIsCaught(caught);
			other->SetPosition(x, y);
		}
	}
	you->move(width, height);
	if (host == "localhost") {
		time = SDL_GetTicks();
		if (time > timeLimit)
		{
			sprintf_s(tmp, "2 \n");
			you->SetIsCaught(1);
			SDLNet_TCP_Send(client, tmp, maxMessageLength);
			return;
		}
		if (you->CheckCollision(other->GetCenterX(), other->GetCenterY())) {
			other->SetIsCaught(1);
			sprintf_s(tmp, "3 \n");
			SDLNet_TCP_Send(client, tmp, maxMessageLength);
			return;
		}
	}

	memset(tmp, 0, sizeof(tmp));
	packetStream.writeInt(you->GetCenterX());
	packetStream.writeInt(you->GetCenterY());
	packetStream.writeInt(you->GetId());
	packetStream.writeInt(you->GetIsCaught());
	packetStream.toCharArray(tmp);
	SDLNet_TCP_Send(client, tmp, maxMessageLength);
}

void Game::clean()
{
	running = false;

	SDLNet_TCP_Close(client);

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDLNet_Quit();
	SDL_Quit();
}