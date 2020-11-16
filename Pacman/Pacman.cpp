#include "Pacman.h"
#include <sstream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv)
{
	_pacman = new Player();
	_pacman->direction = 0;
	_pacman->frame = 0;
	_pacman->currentFrameTime = 0;
	_pacman->speedMultiplier = 1.0f;
	cherryChangedPosition = false;
	_cherry = new PickUp();
	_cherry->frameCount = 0;
	_cherry->currentFrameTime = 0;
	srand(time(NULL));
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		_munchies[i] = new PickUp();
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frameTime = rand() % 500 + 50;
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
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		delete _munchies[i]->sourceRect;
		delete _munchies[i]->texture;
		delete _munchies[i]->rect;
		delete _munchies[i];
	}
	delete _munchies;
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
	_pacman->position = new Vector2(350.0f, 350.0f);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	Texture2D* munchieTexture = new Texture2D();
	munchieTexture->Load("Textures/munchie.png", false);
	// Load munchies[i]
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		_munchies[i]->texture = munchieTexture;
		_munchies[i]->rect = new Rect(rand() % Graphics::GetViewportWidth(), rand() % Graphics::GetViewportHeight(), 12, 12);
		_munchies[i]->sourceRect = new Rect(0.0f, 0.0f, 12, 12);

	}

	//Load Cherry
	_cherry->texture = new Texture2D();
	_cherry->texture->Load("Textures/Cherry.png", true);
	_cherry->rect = new Rect(200.0f, 450.0f, 32, 32);
	_cherry->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
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
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "Pacman X: " << _pacman->position->X << " Y: " << _pacman->position->Y;

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
	SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect);
	UpdateMunchie(elapsedTime);
	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);
	if (_paused) {
		std::stringstream menuStream;
		menuStream << "PAUSED!";
		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
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
			_munchies[i]->currentFrameTime += elapsedTime;
			if (_munchies[i]->currentFrameTime > _munchies[i]->frameTime) {
				_munchies[i]->frameCount++;
				if (_munchies[i]->frameCount >= 2) {
					_munchies[i]->frameCount = 0;
				}
				_munchies[i]->currentFrameTime = 0;
			}
			_munchies[i]->sourceRect->Y = _munchies[i]->sourceRect->Height;
			_munchies[i]->sourceRect->X = _munchies[i]->sourceRect->Width * _munchies[i]->frameCount;
		}
	}
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		SpriteBatch::Draw(_munchies[i]->texture, _munchies[i]->rect, _munchies[i]->sourceRect);
	}
}