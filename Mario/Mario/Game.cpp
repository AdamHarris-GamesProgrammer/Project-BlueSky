#include "Game.h"

#include "Constants.h"
#include "SpriteComponent.h"
#include "Actor.h"
#include "BGSpriteComponent.h"
#include "AnimSpriteComponent.h"
#include "SpriteComponent.h"
#include "Mario.h"

bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO != 0)) {
		printf("Failed to initialize SDL! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	int imgFlags = IMG_INIT_PNG;
	if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
		printf("Failed to initialize IMG! IMG_Error: %s\n", IMG_GetError());
		return false;
	}

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) != 0) {
		printf("Failed to initialize Mixer! Mixer_Error: %s\n", Mix_GetError());
		return false;
	}

	if (TTF_Init() != 0) {
		printf("Failed to initialize TTF! TTF_Error: %s\n", TTF_GetError());
		return false;
	}

	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, NULL, &mWindow, &mRenderer) != 0) {
		printf("Failed to create window/renderer! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	LoadContent();

	mTicksCount = SDL_GetTicks();

	return true;
}

void Game::LoadContent()
{
	//Background clouds
	Actor* temp = new Actor(this);
	temp->SetPosition(Vector2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));

	BGSpriteComponent* bg = new BGSpriteComponent(temp);
	bg->SetScreenSize(Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
	std::vector<SDL_Texture*> bgtexs = {
		GetTexture("Assets/bgScrolling.png"),
		GetTexture("Assets/bgScrolling.png")
	};

	bg->SetBGTexture(bgtexs);
	bg->SetScrollSpeed(-25.0f);

	//foreground textures
	Actor* tempFG = new Actor(this);
	tempFG->SetPosition(Vector2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));

	BGSpriteComponent* fg = new BGSpriteComponent(tempFG);
	fg->SetScreenSize(Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
	std::vector<SDL_Texture*> fgtexs = {
		GetTexture("Assets/BGForeground1.png"),
		GetTexture("Assets/BGForeground2.png"),
		GetTexture("Assets/BGForeground3.png")
	};

	fg->SetBGTexture(fgtexs);
	fg->SetScrollSpeed(-10.0f);

	//Mario
	player = new Mario(this);
	player->SetPosition(Vector2(100.0f, 100.0f));
}


void Game::UnloadContent()
{
	//deletes Actors
	while (!mActors.empty()) {
		delete mActors.back();
	}

	//Destroys textures
	for (auto i : mTextures) {
		SDL_DestroyTexture(i.second);
	}
	mTextures.clear();
}

void Game::RunLoop()
{
	while (!bShouldQuit)
	{
		PollInput();
		Update();
		Render();
	}
}


void Game::AddActor(Actor* actor)
{
	//if in the process of updating actors then the new actor needs to be added to pending
	if (mUpdatingActors) {
		mPendingActors.emplace_back(actor);
	}
	else {
		mActors.emplace_back(actor);
	}
}

void Game::RemoveActor(Actor* actor)
{
	//Is the actor in the pending actors vector
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end()) {
		//swaps the actor to the end of the vector and then erases it
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	//Is the actor in the actors vector
	iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end()) {
		//swaps the actor to the end of the vector and then erases it
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}
}

void Game::AddSprite(class SpriteComponent* sprite)
{
	int myDrawOrder = sprite->GetDrawOrder();
	auto iter = mSprites.begin();
	for(; iter != mSprites.end(); ++iter) {
		if (myDrawOrder < (*iter)->GetDrawOrder()) {
			break;
		}
	}

	mSprites.insert(iter, sprite);
}

void Game::RemoveSprite(class SpriteComponent* sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	mSprites.erase(iter);
}

SDL_Texture* Game::GetTexture(const std::string& fileName, bool useColorKey)
{
	SDL_Texture* tex = nullptr;

	//is the texture in the mTextures map
	auto iter = mTextures.find(fileName);
	//if the texture has been found
	if (iter != mTextures.end()) {
		tex = iter->second;
	}
	else {
		//loads the image onto a SDL_Surface
		SDL_Surface* surf = IMG_Load(fileName.c_str());
		if (!surf) {
			printf("Failed to load texture file %s! IMG_Error: %s\n", fileName.c_str(), IMG_GetError());
			return nullptr;
		}

		//Creates a texture from surface
		if (useColorKey) {
			SDL_SetColorKey(surf, useColorKey, SDL_MapRGB(surf->format, 0xFF, 0xFF, 0xFF));
		}
		else {
			SDL_SetColorKey(surf, true, SDL_MapRGBA(surf->format, 0x00, 0x00, 0x00, 0x00));
		}

		tex = SDL_CreateTextureFromSurface(mRenderer, surf);
		SDL_FreeSurface(surf);

		if (!tex) {
			printf("Failed to make texture out of surface %s! IMG_Error: %s\n", fileName.c_str(), IMG_GetError());
			return nullptr;
		}

		mTextures.emplace(fileName.c_str(), tex);
	}
	return tex;
}

void Game::PollInput()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			bShouldQuit = true;
		}
		else if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				bShouldQuit = true;
				break;
			}
		}

		
	}

	const Uint8* state = SDL_GetKeyboardState(NULL);
	player->HandleEvents(state);
}

void Game::Update()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)); //delays the game 16ms, creating (roughly) 60fps gameplay

	//calculates delta time
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	if (deltaTime > 0.05f) {
		deltaTime = 0.05f;
	}
	mTicksCount = SDL_GetTicks();

	//update all actors
	mUpdatingActors = true;
	for (auto actor : mActors) {
		actor->Update(deltaTime);
	}
	mUpdatingActors = false;

	//moves pending actors into mActors
	for (auto pending : mPendingActors) {
		mActors.emplace_back(pending);
	}
	mPendingActors.clear();

	//Adds any dead actors to a temporary vector to be removed
	std::vector<Actor*> deadActors;
	for (auto actor : mActors) {
		if (actor->GetState() == Actor::EDead) {
			deadActors.emplace_back(actor);
		}
	}

	//deletes any dead actors from deadActor, this also deletes them from mActors
	for (auto actor : deadActors) {
		delete actor;
	}
}

void Game::Render()
{
	SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(mRenderer);

	for (auto sprite : mSprites) {

		if (sprite->GetOwner() == player) {
			player->Draw();
		}
		else {
			sprite->Draw(mRenderer);
		}
	}

	SDL_RenderPresent(mRenderer);
}


void Game::Shutdown()
{
	UnloadContent();
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);

	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

