#include "Pacman.h"
#include <sstream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv)
{
	grid = new Grid();
	_pacman = new Player();
	_pacman->direction = 0;
	_pacman->frame = 0;
	_pacman->currentFrameTime = 0;
	_pacman->speedMultiplier = 1.0f;
	_pacman->dead = false;
	cherryChangedPosition = false;
	_cherry = new PickUp();
	_cherry->frameCount = 0;
	_cherry->currentFrameTime = 0;
	srand(time(NULL));
	//initialise munchies
	//initialise ghosts
	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i] = new Enemy();
		_ghosts[i]->directionX = 0;
		_ghosts[i]->directionY = 0;

	}
	_frameCount = 0;
	_started = false;
	_paused = false;
	_pKeyDown = false;
	//Initialise important Game aspects
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

Pacman::~Pacman()
{
	delete _cherry->texture;
	delete _cherry->sourceRect;
	delete _cherry->rect;
	for (int i = 0; i < grid->walls.size(); i++) {
		delete grid->walls.at(i)->position;
		delete grid->walls.at(i)->texture;
		delete grid->walls.at(i);
	}
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		delete grid->_munchies[i]->sourceRect;
		delete grid->_munchies[i]->texture;
		delete grid->_munchies[i]->rect;
		delete grid->_munchies[i];
	}
	delete grid->_munchies;
	for (int i = 0; i < GHOSTCOUNT; i++) {
		delete _ghosts[i]->sourceRect;
		delete _ghosts[i]->texture;
		delete _ghosts[i]->position;
		delete _ghosts[i]->gridPos;
		delete _ghosts[i];
	}
	delete _ghosts;

	delete _pacman->texture;
	delete _pacman->sourceRect;
	delete _pacman->position;
	delete _pacman;

}

void Pacman::LoadContent()
{
	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->position = new Vector2(32 * 15, 32 * 18);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	Texture2D* munchieTexture = new Texture2D();
	//load ghosts
	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i]->texture = new Texture2D();
		_ghosts[i]->texture->Load("Textures/GhostBlue.png", false);
		_ghosts[i]->position = new Vector2((32*14)+(32*i), (32*11));
		_ghosts[i]->gridPos = new Vector2(0, 0);
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
	}
	//Load Cherry
	_cherry->texture = new Texture2D();
	_cherry->texture->Load("Textures/Cherry.png", true);
	_cherry->rect = new Rect(200.0f, 450.0f, 32, 32);
	_cherry->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	//Load Grid
	grid->GenerateMap(grid);
	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);
	//Load Pause menu
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/Transparency.png", false);
	_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);
	//Load Start Screen
	_startBG = new Texture2D();
	_startBG->Load("Textures/PacmanStartScreen.png", false);
	_startRect = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_startStringPosition = new Vector2((Graphics::GetViewportWidth() / 2.0f) - 80.0f, (Graphics::GetViewportHeight() / 1.25f));
}

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	//Get the mouse state
	Input::MouseState* mouseState = Input::Mouse::GetState();


	CheckStarted(keyboardState, Input::Keys::SPACE);
	if (_started) {
		CheckPaused(keyboardState, Input::Keys::P);
		if (!_paused) {
			Input(elapsedTime, keyboardState, mouseState);
			UpdatePacman(elapsedTime);
			CheckViewportColl();
			CheckWallCollisions();
			for (int i = 0; i < GHOSTCOUNT; i++) {
				UpdateGhost(_ghosts[i], elapsedTime, *grid);
			}
			CheckGhostCollisions();
		}

	}
	else {
		_frameCount++;
		if (_frameCount >= 60)
			_frameCount = 0;

	}
}
void Pacman::Draw(int elapsedTime)
{

	SpriteBatch::BeginDraw(); // Starts Drawing


	//draws cherry
	if (!_paused) {
		_cherry->currentFrameTime += elapsedTime;
		if (_cherry->currentFrameTime > _cherry->frameTime) {
			_cherry->frameCount++;
			if (_cherry->frameCount >= 2) {
				_cherry->frameCount = 0;
			}
			_cherry->currentFrameTime = 0;
		}
		_cherry->sourceRect->Y = _cherry->sourceRect->Height;
		_cherry->sourceRect->X = _cherry->sourceRect->Width * _cherry->frameCount;
	}
	SpriteBatch::Draw(_cherry->texture, _cherry->rect, _cherry->sourceRect);
	// Draws Pacman
	_pacman->currentFrameTime += elapsedTime;
	if (!_paused) {
		if (_pacman->currentFrameTime > _pacman->frameTime) {
			_pacman->frame++;
			if (_pacman->frame >= 2) {
				_pacman->frame = 0;
			}
			_pacman->currentFrameTime = 0;
		}
		_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacman->direction;
		_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame;
	}
	if (!_pacman->dead) {
		SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect);
	}
	for (int i = 0; i < GHOSTCOUNT; i++) {
		SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect);
	}
	UpdateMunchie(elapsedTime);
	if (_paused) {
		std::stringstream menuStream;
		menuStream << "PAUSED!";
		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	}
	if (_started) {
		for (int i = 0; i < grid->walls.size(); i++) {
			SpriteBatch::Draw(grid->walls.at(i)->texture, grid->walls.at(i)->position);
		}
	}
	if (!_started) {
		std::stringstream startStream;
		startStream << "Press space to INSERT COIN";
		SpriteBatch::Draw(_startBG, _startRect, nullptr);
		if (_frameCount < 30) {
			SpriteBatch::DrawString(startStream.str().c_str(), _startStringPosition, Color::White);
		}
	}
	SpriteBatch::EndDraw(); // Ends Drawing
}
void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState) {
#pragma region MouseInput
	if (mouseState->LeftButton == Input::ButtonState::PRESSED) {
		_cherry->rect->X = mouseState->X;
		_cherry->rect->Y = mouseState->Y;
	}
#pragma endregion
#pragma region KeyboardInput
	if (keyboardState->IsKeyDown(Input::Keys::D)) {
		movingXPositive = true;
		movingXNegative = false;
		movingYPositive = false;
		movingYNegative = false;
		_pacman->direction = 0;

	}
	if (keyboardState->IsKeyDown(Input::Keys::A)) {
		movingXPositive = false;
		movingXNegative = true;
		movingYPositive = false;
		movingYNegative = false;
		_pacman->direction = 2;
	}

	if (keyboardState->IsKeyDown(Input::Keys::W)) {
		movingXPositive = false;
		movingXNegative = false;
		movingYPositive = true;
		movingYNegative = false;
		_pacman->direction = 3;
	}

	if (keyboardState->IsKeyDown(Input::Keys::S)) {
		movingXPositive = false;
		movingXNegative = false;
		movingYPositive = false;
		movingYNegative = true;
		_pacman->direction = 1;

	}
#pragma endregion
#pragma region Sprint
	if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT)) {
		_pacman->speedMultiplier = 2.0f;
	}
	else {
		_pacman->speedMultiplier = 1.0f;
	}
#pragma endregion
#pragma region RandomizeCherryPos
	if (keyboardState->IsKeyDown(Input::Keys::R) && cherryChangedPosition == false) {
		_cherry->rect->X = rand() % Graphics::GetViewportWidth();
		_cherry->rect->Y = rand() % Graphics::GetViewportHeight();
		cherryChangedPosition = true;
	}
	else if (keyboardState->IsKeyUp(Input::Keys::R)) {
		cherryChangedPosition = false;
	}
#pragma endregion

}
void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey) {
	if (state->IsKeyDown(pauseKey) && _pKeyDown == false) {
		_pKeyDown = true;
		_paused = !_paused;
	}
	if (state->IsKeyUp(pauseKey)) {
		_pKeyDown = false;
	}
}
void Pacman::CheckStarted(Input::KeyboardState* state, Input::Keys startKey) {
	if (!_started && state->IsKeyDown(startKey)) {
		_started = true;
	}
}
void Pacman::CheckViewportColl() {
	if (_pacman->position->X + _pacman->sourceRect->Width > 1024 + _pacman->sourceRect->Width) {
		_pacman->position->X = 0 - _pacman->sourceRect->Width;
	}
	if (_pacman->position->X < 0 - _pacman->sourceRect->Width) {
		_pacman->position->X = Graphics::GetViewportWidth();
	}
	if (_pacman->position->Y + _pacman->sourceRect->Height > 768 + _pacman->sourceRect->Height) {
		_pacman->position->Y = 0 - _pacman->sourceRect->Height;
	}
	if (_pacman->position->Y < 0 - _pacman->sourceRect->Height) {
		_pacman->position->Y = Graphics::GetViewportHeight();
	}

}
void Pacman::findGridPosition(const Vector2 &position, Vector2 &gridPos) {
	float gridPosUnroundedX = position.X / 32;
	float gridPosUnroundedY = position.Y / 24;
	int gridPosX = (int)(round(gridPosUnroundedX));
	int gridPosY = (int)(round(gridPosUnroundedY));
	gridPos.X = gridPosX;
	gridPos.Y = gridPosY;
}
void Pacman::UpdatePacman(int elapsedTime) {
	float pacmanFinalSpeed = _pacman->speed * _pacman->speedMultiplier;
	if (movingXPositive) {
		_pacman->position->X += pacmanFinalSpeed * elapsedTime; //Moves Pacman across X axis
	}
	else if (movingXNegative) {
		_pacman->position->X -= pacmanFinalSpeed * elapsedTime; //Moves Pacman across X axis
	}
	if (movingYPositive) {
		_pacman->position->Y -= pacmanFinalSpeed * elapsedTime;
	}
	else if (movingYNegative) {
		_pacman->position->Y += pacmanFinalSpeed * elapsedTime;
	}
}
void Pacman::UpdateMunchie(int elapsedTime) {
	if (!_paused) {
		for (int i = 0; i < MUNCHIECOUNT; i++) {
			grid->_munchies[i]->currentFrameTime += elapsedTime;
			if (grid->_munchies[i]->currentFrameTime > grid->_munchies[i]->frameTime) {
				grid->_munchies[i]->frameCount++;
				if (grid->_munchies[i]->frameCount >= 2) {
					grid->_munchies[i]->frameCount = 0;
				}
				grid->_munchies[i]->currentFrameTime = 0;
			}
			grid->_munchies[i]->sourceRect->Y = grid->_munchies[i]->sourceRect->Height;
			grid->_munchies[i]->sourceRect->X = grid->_munchies[i]->sourceRect->Width * grid->_munchies[i]->frameCount;
		}
	}
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		SpriteBatch::Draw(grid->_munchies[i]->texture, grid->_munchies[i]->rect, grid->_munchies[i]->sourceRect);
	}
}
void Pacman::CheckGhostCollisions() {
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;
	for (i = 0; i < GHOSTCOUNT; i++) {
		bottom2 = _ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height;
		left2 = _ghosts[i]->position->X;
		right2 = _ghosts[i]->position->X + _ghosts[i]->sourceRect->Width;
		top2 = _ghosts[i]->position->Y;
		if (bottom1 > top2 && top1<bottom2 && right1 >left2 && left1 < right2) {
			_pacman->dead = true;
			i = GHOSTCOUNT;
		}
	}
}
void Pacman::UpdateGhost(Enemy* ghost, int elapsedTime, const Grid& grid) {
	findGridPosition(*ghost->position, *ghost->gridPos);

}
void Pacman::CheckWallCollisions() {
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;
	for (i = 0; i < grid->walls.size(); i++) {
		bottom2 = grid->walls.at(i)->position->Y + 32;
		left2 = grid->walls.at(i)->position->X;
		right2 = grid->walls.at(i)->position->X + 32;
		top2 = grid->walls.at(i)->position->Y;
		if (bottom1 > top2 && top1<bottom2 && right1 >left2 && left1 < right2) { //collision is detected at walls[i]
			if (left1 < right2 && right1 > right2) {
				//update to the right
				_pacman->position->X = right2;
			}
			else  if (right1 > left2 && left1 <left2) {
				//update to the left
				_pacman->position->X = left2-32;
			}
			if (bottom1 > top2 && top1 < top2) {
				//update to the top
				_pacman->position->Y = top2-32;
			}
			else if (top1 < bottom2 && bottom1 > bottom2) {
				//update to the bottom
				_pacman->position->Y = bottom2;
			}
		}

	}	
}
void Grid::GenerateMap(Grid *grid) {
	Texture2D* wallText = new Texture2D();
	Texture2D* munchieTexture = new Texture2D();
	int m = 0;
	wallText->Load("Textures/wall.png", false);
	munchieTexture->Load("Textures/munchie.png", false);
	for (int k = 0; k < verticalBlocks; k++) {
		for (int i = 0; i < horizontalBlocks / 2; i++) {
			if (k == 0) {
				if (i < 14) {
					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 1) {
				if (i < 1 || i == 15) {
					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 2) {
				if (i == 0 || i == 2 || i == 4 || i == 5 || i == 7 || (i >= 9 && i <= 13) || i == 15) {
					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 3) {
				if (i == 0 || i == 2 || i == 4 || i == 5 || i == 7 || i == 15) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}

			}
			else if (k == 4) {
				if (i == 0 || i == 2 || i == 4 || i == 5 || i == 7 || (i >= 9 && i <= 15)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 5) {
				if (i == 0 || (i >= 2 && i <= 7) || i == 15) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 6) {
				if (i == 0 || (i >= 9 && i <= 13) || i == 15) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 7) {
				if (i == 0 || i == 2 || (i >= 4 && i <= 7) || (i >= 9 && i <= 13) || i == 15) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 8) {
				if (i == 0 || i == 2 || (i >= 4 && i <= 5) || (i >= 9 && i <= 13)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 9) {
				if (i == 0 || i == 2 || (i >= 4 && i <= 5) || i == 7) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 10) {
				if (i == 2 || (i >= 4 && i <= 5) || i == 7 || (i >= 11 && i <= 14)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 11) {
				if (i == 1 || i == 2 || (i >= 4 && i <= 5) || (i >= 7 && i <= 9) || i == 11) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 12) {
				if (i == 1 || i == 2 || (i >= 4 && i <= 5) || i == 7 || (i >= 11 && i <= 15)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 13) {
				if (i == 2 || (i >= 4 && i <= 5) || i == 7 || i == 9) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}

			}
			else if (k == 14) {
				if (i == 0 || i == 2 || (i >= 4 && i <= 5) || (i >= 9 && i <= 15)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 15) {
				if (i == 0 || i == 2 || (i >= 4 && i <= 5) || i == 7) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 16) {
				if (i == 0 || i == 2 || (i >= 4 && i <= 5) || i == 7 || (i >= 9 && i <= 11) || (i >= 13 && i <= 15)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 17) {
				if (i == 0 || (i >= 13 && i <= 15)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 18) {
				if (i == 0 || (i >= 2 && i <= 6) || (i >= 8 && i <= 11)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 19) {
				if (i == 0 || i == 2 || i == 4 || i == 6 || (i >= 8 && i <= 15)) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 20) {
				if (i == 0 || i == 2 || i == 4 || i == 6 || i == 15) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 21) {
				if (i == 0 || i == 2 || i == 4 || (i >= 6 && i <= 13) || i == 15) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 22) {
				if (i == 0 || i == 15) {

					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
			else if (k == 23) {
				if (i < 14) {
					CreateWall(i, k, wallText, *grid);
				}
				else {
					CreateMunchie(i, k, munchieTexture, m, *grid);
				}
			}
		}
	}
}
void Grid::CreateMunchie(int i, int k, Texture2D* munchieTexture, int& m, Grid& grid) {
	//left side
	_munchies[m] = new PickUp();
	_munchies[m]->currentFrameTime = 0;
	_munchies[m]->frameTime = rand() % 500 + 50;
	_munchies[m]->texture = munchieTexture;
	_munchies[m]->rect = new Rect(11 + (32 * i), 10 + (32 * k), 12, 12);
	_munchies[m]->sourceRect = new Rect(0.0f, 0.0f, 12, 12);
	grid.empty[i][k] = true;
	m++;
	
	//mirror image
	_munchies[m] = new PickUp();
	_munchies[m]->currentFrameTime = 0;
	_munchies[m]->frameTime = rand() % 500 + 50;
	_munchies[m]->texture = munchieTexture;
	_munchies[m]->rect = new Rect(11 + (32 * (31 - i)), 10 + (32 * k), 12, 12);
	_munchies[m]->sourceRect = new Rect(0.0f, 0.0f, 12, 12);
	grid.empty[31-i][k] = true;
	m++;
}
void Grid::CreateWall(int i, int k, Texture2D* wallText, Grid& grid) {
	//left side
	Wall* nextWall = new Wall();
	walls.push_back(nextWall);
	nextWall->position = new Vector2(32 * i, 32 * k);
	nextWall->texture = wallText, grid;
	grid.empty[i][k] = false;
	//mirror image
	Wall* nextWallMirror = new Wall();
	walls.push_back(nextWallMirror);
	nextWallMirror->position = new Vector2(32 * (31 - i), 32 * k);
	nextWallMirror->texture = wallText, grid;
	grid.empty[31 - i][k] = false;
}