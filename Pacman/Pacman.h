#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif
#define MUNCHIECOUNT 362
#define GHOSTCOUNT 4

// Just need to include main header file
#include "S2D/S2D.h"
#include <time.h>
#include <vector>
// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
struct Player {
	bool dead;
	Vector2* position;
	Rect* sourceRect;
	Texture2D* texture;
	const float speed = 0.15f;
	float speedMultiplier;
	int direction;
	int frame;
	int currentFrameTime;
	const int frameTime = 250;

};
struct PickUp {
	Rect* rect;
	Rect* sourceRect;
	int frameTime = 500;
	int currentFrameTime;
	int frameCount = 0;
	Texture2D* texture;
};
struct Enemy {
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	const float speed = 0.15f;
};
struct Wall {
	Vector2* position;
	Texture2D* texture;
};
class Grid {
	const int horizontalBlocks = 1024 / 32;
	const int verticalBlocks = 768 / 32;
	const Vector2* origin = new Vector2(0, 0);
public:
	vector <Wall*> walls;
	PickUp* _munchies[MUNCHIECOUNT];
	void GenerateMap();
	void CreateWall(int i, int k, Texture2D* wallText);
	void CreateMunchie(int i, int k, Texture2D* munchieTexture, int& m);
};
class Pacman : public Game
{
private:
	Grid* grid;
	Player* _pacman;
	PickUp* _cherry;
	Enemy* _ghosts[GHOSTCOUNT];
	//start string data
	int _frameCount;
	Vector2* _startStringPosition;

	// Position for String
	Vector2* _stringPosition;
	//const data

	//moving bools
	bool movingXPositive;
	bool movingYPositive;
	bool movingXNegative;
	bool movingYNegative;
	bool cherryChangedPosition;
	// Data for Menu
	Texture2D* _menuBackground;
	Rect* _menuRectangle;
	Vector2* _menuStringPosition;
	bool _paused;
	bool _pKeyDown;
	//Data for Start Screen
	Texture2D* _startBG;
	Rect* _startRect;
	bool _started;

	//Data for pacman animation

	//pacman methods

	//input
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	//pause
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckStarted(Input::KeyboardState* state, Input::Keys startKey);
	void CheckViewportColl();
	//Update methods
	void UpdatePacman(int elapsedTime);
	void UpdateMunchie(int elpsedTime);
	//ghosts methods
	void UpdateGhost(Enemy* ghost, int elapsedTime);
	void CheckGhostCollisions();
	void CheckWallCollisions();
public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);

};

