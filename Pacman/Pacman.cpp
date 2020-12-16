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
	srand(time(NULL));
	//initialise munchies
	//initialise ghosts
	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i] = new Enemy();
		_ghosts[i]->direction = 0;

	}
	_frameCount = 0;
	_started = false;
	_paused = false;
	_pKeyDown = false;
	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

Pacman::~Pacman()
{
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
	delete _pacman->gridPos;
	delete _pacman;
	delete _startBG;
	delete _startRect;
	delete gameOverBG;
	delete _gameOverRect;
	delete winBG;
	delete winRect;

}

void Pacman::LoadContent()
{
	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->position = new Vector2(32 * 15, 32 * 18);
	_pacman->gridPos = new Vector2(32 * 15, 32 * 18);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	Texture2D* munchieTexture = new Texture2D();
	//load ghosts
	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i]->texture = new Texture2D();
		_ghosts[i]->texture->Load("Textures/GhostBlue.png", false);
		_ghosts[i]->position = new Vector2((32 * 14) + (32 * i), (32 * 11));
		_ghosts[i]->gridPos = new Vector2(0, 0);
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
	}

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
	gameOverBG = new Texture2D();
	gameOverBG->Load("Textures/gameover.png", false);
	_gameOverRect = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	winBG = new Texture2D();
	winBG->Load("Textures/win.png", false);
	winRect = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
}

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	//Get the mouse state
	Input::MouseState* mouseState = Input::Mouse::GetState();


	CheckStarted(keyboardState, Input::Keys::SPACE);
	if (win || gameOver) {
		if (keyboardState->IsKeyDown(Input::Keys::SPACE)) {
			RestartLevel();
		}
	}
	if (_started && !gameOver && !win) {
		CheckPaused(keyboardState, Input::Keys::P);
		if (!_paused) {
			CheckMunchieCollisions();
			fScore -= ((float)elapsedTime / 500);
			if (fScore < 0.0f) {
				fScore = 0.0f;
			}
			score = (int)fScore;
			UpdatePacman(elapsedTime);
			Input(elapsedTime, keyboardState, mouseState);
			CheckViewportColl();
			CheckWallCollisions();
			for (int i = 0; i < GHOSTCOUNT; i++) {
				UpdateGhost(_ghosts[i], elapsedTime, *grid);
			}
			CheckGhostCollisions();
			if (collectedMunchies >= MUNCHIECOUNT) {
				win = true;
			}
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
	if (_started && !gameOver && !win) {
		for (int i = 0; i < grid->walls.size(); i++) {
			SpriteBatch::Draw(grid->walls.at(i)->texture, grid->walls.at(i)->position);
			std::stringstream livesStream;
			livesStream << "LIVES: " + to_string(_pacman->lives);
			std::stringstream scoreStream;
			scoreStream << "SCORE: " + to_string(score);
			SpriteBatch::DrawString(livesStream.str().c_str(), &Vector2(100, 755), Color::Green);
			SpriteBatch::DrawString(scoreStream.str().c_str(), &Vector2(900, 755), Color::Green);
		}
	}
	std::stringstream restartStream;
	std::stringstream finalScoreStream;
	finalScoreStream << "Your final score was: " + to_string(score);
	restartStream << "Press space to restart.";
	if (gameOver) {
		SpriteBatch::Draw(gameOverBG, _gameOverRect, nullptr);
		if (_frameCount < 30) {
			SpriteBatch::DrawString(restartStream.str().c_str(), _startStringPosition, Color::White);
		}
		SpriteBatch::DrawString(finalScoreStream.str().c_str(), &Vector2(432, 755), Color::White);
	}
	if (win) {
		SpriteBatch::Draw(winBG, winRect, nullptr);
		if (_frameCount < 30) {
			SpriteBatch::DrawString(restartStream.str().c_str(), _startStringPosition, Color::White);
		}
		SpriteBatch::DrawString(finalScoreStream.str().c_str(), &Vector2(432, 755), Color::White);
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
#pragma region KeyboardInput
	if (keyboardState->IsKeyUp(Input::Keys::D) && keyboardState->IsKeyUp(Input::Keys::A) && keyboardState->IsKeyUp(Input::Keys::W) && keyboardState->IsKeyUp(Input::Keys::S)) {
		inputPressed = false;
	}
	if (keyboardState->IsKeyDown(Input::Keys::D) && inputPressed == false) {
		inputPressed = true;
		movingXPositive = true;
		movingXNegative = false;
		movingYPositive = false;
		movingYNegative = false;
		_pacman->direction = 0;

	}
	else if (keyboardState->IsKeyDown(Input::Keys::A) && inputPressed == false) {
		inputPressed = true;
		movingXPositive = false;
		movingXNegative = true;
		movingYPositive = false;
		movingYNegative = false;
		_pacman->direction = 2;
	}

	else if (keyboardState->IsKeyDown(Input::Keys::W) && inputPressed == false) {
		inputPressed = true;
		movingXPositive = false;
		movingXNegative = false;
		movingYPositive = true;
		movingYNegative = false;
		_pacman->direction = 3;
	}

	else if (keyboardState->IsKeyDown(Input::Keys::S) && inputPressed == false) {
		inputPressed = true;
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
void Pacman::findGridPosition(const Vector2& position, Vector2* gridPos) {
	float gridPosUnroundedX = position.X / 32;
	float gridPosUnroundedY = position.Y / 32;
	int gridPosX = (int)(round(gridPosUnroundedX));
	int gridPosY = (int)(round(gridPosUnroundedY));
	gridPos->X = gridPosX;
	gridPos->Y = gridPosY;
}
bool Pacman::CheckCollision(const Vector2& position1, const Rect& sourceRect1, const Vector2& position2, const Rect& sourceRect2)
{
	int bottom1 = position1.Y + sourceRect1.Height;
	int bottom2 = position2.Y + sourceRect2.Height;
	int left1 = position1.X;
	int left2 = position2.X;
	int right1 = position1.X + sourceRect1.Width;
	int right2 = position2.X + sourceRect2.Width;
	int top1 = position1.Y;
	int top2 = position2.Y;
	if (bottom1 > top2 && top1<bottom2 && right1 >left2 && left1 < right2) {
		return true;
	}
	else {
		return false;
	}
}
void Pacman::CheckMunchieCollisions()
{
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		if (CheckCollision(*_pacman->position, *_pacman->sourceRect, Vector2(grid->_munchies[i]->rect->X, grid->_munchies[i]->rect->Y), *grid->_munchies[i]->sourceRect)) {
			fScore += 10.0f;
			collectedMunchies++;
			grid->_munchies[i]->rect->X = 2000.0f;
		}
	}
}
void Pacman::UpdatePacman(int elapsedTime) {
	float pacmanFinalSpeed = _pacman->speed * _pacman->speedMultiplier;
	if (_pacman->lives > 0 && _pacman->dead == true) {
		_pacman->position->X = 32 * 15;
		_pacman->position->Y = 32 * 18;
		_pacman->dead = false;

	}
	if (_pacman->lives < 0) {
		_pacman->lives = 0;
		gameOver = true;
	}
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
	for (int i = 0; i < GHOSTCOUNT; i++) {
		if (CheckCollision(*_pacman->position, *_pacman->sourceRect, *_ghosts[i]->position, *_ghosts[i]->sourceRect)) {
			_pacman->lives--;
			_pacman->dead = true;
			i = GHOSTCOUNT;
		}
	}
}
void Pacman::UpdateGhost(Enemy* ghost, int elapsedTime, const Grid& grid) {
	Vector2 oldGridPos = *ghost->gridPos;
	if (ghost->direction == 0) {
		ghost->position->X += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 1) {
		ghost->position->X -= ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 2) {
		ghost->position->Y -= ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 3) {
		ghost->position->Y += ghost->speed * elapsedTime;
	}
	findGridPosition(*ghost->position, ghost->gridPos);
	Vector2 newGridPos = *ghost->gridPos;

	if (oldGridPos.X <= newGridPos.X - 0.1f || oldGridPos.X >= newGridPos.X + 0.1f || oldGridPos.Y <= newGridPos.Y - 0.1f || oldGridPos.Y >= newGridPos.Y + 0.1f) {
		int originalDir = ghost->direction;
		if (ghost->direction == 0) {
			bool right;
			bool top;
			bool bottom;
			bool edge = false;
			if ((int)round(ghost->gridPos->X) == 31) {
				edge = true;
				right = false;
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
			}
			if ((int)round(ghost->gridPos->Y) == 0) {
				edge = true;
				top = false;
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
			}
			if ((int)round(ghost->gridPos->Y) == 23) {
				edge = true;
				bottom = false;
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
			}
			if (!edge) {
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
			}
			if (!right && !top && !bottom) {
				ghost->direction = 1;
			}
			else {
				bool possibleOutcomes[3] = { right,top,bottom };
				vector<int> outcomePositions;
				int chosenPosition;
				for (int i = 0; i < 3; i++) {
					if (possibleOutcomes[i] == true) {
						outcomePositions.push_back(i);
					}
				}
				int random = rand() % outcomePositions.size();
				chosenPosition = outcomePositions.at(random);
				if (chosenPosition == 0) {
					ghost->direction = 0;
				}
				else if (chosenPosition == 1) {
					ghost->direction = 2;
				}
				else if (chosenPosition == 2) {
					ghost->direction = 3;
				}
			}
		}
		else if (ghost->direction == 1) {
			bool left;
			bool top;
			bool bottom;
			bool edge = false;
			if ((int)round(ghost->gridPos->X) == 0) {
				edge = true;
				left = false;
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
			}
			if ((int)round(ghost->gridPos->Y) == 0) {
				edge = true;
				top = false;
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
			}
			if ((int)round(ghost->gridPos->Y) == 23) {
				edge = true;
				bottom = false;
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
			}
			if (!edge) {
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
			}
			if (!left && !top && !bottom) {
				ghost->direction = 0;
			}
			else {
				bool possibleOutcomes[3] = { left,top,bottom };
				vector<int> outcomePositions;
				int chosenPosition;
				for (int i = 0; i < 3; i++) {
					if (possibleOutcomes[i] == true) {
						outcomePositions.push_back(i);
					}
				}
				int random = rand() % outcomePositions.size();
				chosenPosition = outcomePositions.at(random);
				if (chosenPosition == 0) {
					ghost->direction = 1;
				}
				else if (chosenPosition == 1) {
					ghost->direction = 2;
				}
				else if (chosenPosition == 2) {
					ghost->direction = 3;
				}
			}
		}
		else if (ghost->direction == 2) {
			bool left;
			bool top;
			bool right;
			bool edge = false;
			if ((int)round(ghost->gridPos->Y) == 0) {
				edge = true;
				top = false;
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
			}
			if ((int)round(ghost->gridPos->X) == 0) {
				edge = true;
				left = false;
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
			}
			if ((int)round(ghost->gridPos->X) == 31) {
				edge = true;
				right = false;
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
			}
			if (!edge) {
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				top = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) - 1];
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
			}
			if (!left && !top && !right) {
				ghost->direction = 3;
			}
			else {
				bool possibleOutcomes[3] = { left,top,right };
				vector<int> outcomePositions;
				int chosenPosition;
				for (int i = 0; i < 3; i++) {
					if (possibleOutcomes[i] == true) {
						outcomePositions.push_back(i);
					}
				}
				int random = rand() % outcomePositions.size();
				chosenPosition = outcomePositions.at(random);
				if (chosenPosition == 0) {
					ghost->direction = 1;
				}
				else if (chosenPosition == 1) {
					ghost->direction = 2;
				}
				else if (chosenPosition == 2) {
					ghost->direction = 0;
				}
			}

		}
		else if (ghost->direction == 3) {
			bool left;
			bool bottom;
			bool right;
			bool edge = false;
			if ((int)round(ghost->gridPos->Y) == 23) {
				edge = true;
				bottom = false;
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
			}
			if ((int)round(ghost->gridPos->X) == 0) {
				edge = true;
				left = false;
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
			}
			if ((int)round(ghost->gridPos->X) == 31) {
				edge = true;
				right = false;
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
			}
			if (!edge) {
				left = grid.empty[(int)round(ghost->gridPos->X) - 1][(int)round(ghost->gridPos->Y)];
				bottom = grid.empty[(int)round(ghost->gridPos->X)][(int)round(ghost->gridPos->Y) + 1];
				right = grid.empty[(int)round(ghost->gridPos->X) + 1][(int)round(ghost->gridPos->Y)];
			}
			if (!left && !bottom && !right) {
				ghost->direction = 2;
			}
			else {
				bool possibleOutcomes[3] = { left,bottom,right };
				vector<int> outcomePositions;
				int chosenPosition;
				for (int i = 0; i < 3; i++) {
					if (possibleOutcomes[i] == true) {
						outcomePositions.push_back(i);
					}
				}
				int random = rand() % outcomePositions.size();
				chosenPosition = outcomePositions.at(random);
				if (chosenPosition == 0) {
					ghost->direction = 1;
				}
				else if (chosenPosition == 1) {
					ghost->direction = 3;
				}
				else if (chosenPosition == 2) {
					ghost->direction = 0;
				}
			}
		}
		if (ghost->direction != originalDir) {
			SnapToGrid(ghost->position, ghost->gridPos);
		}
	}

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
			else  if (right1 > left2 && left1 < left2) {
				//update to the left
				_pacman->position->X = left2 - 32;
			}
			if (bottom1 > top2 && top1 < top2) {
				//update to the top
				_pacman->position->Y = top2 - 32;
			}
			else if (top1 < bottom2 && bottom1 > bottom2) {
				//update to the bottom
				_pacman->position->Y = bottom2;
			}
		}

	}
}
void Grid::GenerateMap(Grid* grid) {
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
	_munchies[m]->originalRect = *_munchies[m]->rect;
	_munchies[m]->sourceRect = new Rect(0.0f, 0.0f, 12, 12);
	grid.empty[i][k] = true;
	m++;

	//mirror image
	_munchies[m] = new PickUp();
	_munchies[m]->currentFrameTime = 0;
	_munchies[m]->frameTime = rand() % 500 + 50;
	_munchies[m]->texture = munchieTexture;
	_munchies[m]->rect = new Rect(11 + (32 * (31 - i)), 10 + (32 * k), 12, 12);
	_munchies[m]->originalRect = *_munchies[m]->rect;
	_munchies[m]->sourceRect = new Rect(0.0f, 0.0f, 12, 12);
	grid.empty[31 - i][k] = true;
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

void Pacman::SnapToGrid(Vector2* position, const Vector2* gridPos)
{
	Vector2 newPos;
	newPos.X = gridPos->X * 32;
	newPos.Y = gridPos->Y * 32;
	position->X = newPos.X;
	position->Y = newPos.Y;
}

void Pacman::RestartLevel()
{
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		grid->_munchies[i]->rect = &grid->_munchies[i]->originalRect;
	}
	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i]->position = new Vector2((32 * 14) + (32 * i), (32 * 11));
	}
	score = 0;
	win = false;
	gameOver = false;
	_started = true;
	collectedMunchies = 0;
	_pacman->dead = false;
	_pacman->lives = 3;
	movingXPositive = false;
	movingXNegative = false;
	movingYPositive = false;
	movingYNegative = false;
	_pacman->position = new Vector2(32 * 15, 32 * 18);
}
