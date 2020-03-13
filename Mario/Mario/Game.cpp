#include "Game.h"

#include "ComponentsList.h"
#include "Constants.h"

#include "Actor.h"

#include "Mario.h"
#include "Coin.h"
#include "Tile.h"
#include "LevelGoal.h"
#include "Koopa.h"
#include "PowBlock.h"
#include "ScoreManager.h"
#include "ButtonComponent.h"
#include "NextLevelButton.h"


bool Game::Initialize()
{
	mEngine = new BlueSky(this);
	if (mEngine->Initialize())
	{
		LoadContent();
		mEngine->RunLoop();

		return true;
	}
	else
	{
		return false;
	}
}

void Game::LoadContent()
{

	//Background clouds
	bgActor = new Actor(this);
	bgActor->SetPosition(Vector2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));

	BGSpriteComponent* bg = new BGSpriteComponent(bgActor);
	bg->SetScreenSize(Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
	std::vector<SDL_Texture*> bgtexs = {
		mEngine->GetTexture("Assets/bgScrolling.png"),
		mEngine->GetTexture("Assets/bgScrolling.png")
	};

	bg->SetBGTexture(bgtexs);
	bg->SetScrollSpeed(-25.0f);

	mBGMusic = new Music();
	mBGMusic->SetVolume(20);
	mBGMusic->Load("Assets/Audio/BgMusic.mp3");


	//foreground textures
	fgActor = new Actor(this);
	fgActor->SetPosition(Vector2(SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 32.0f));

	BGSpriteComponent* fg = new BGSpriteComponent(fgActor);
	fg->SetScreenSize(Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
	std::vector<SDL_Texture*> fgtexs = {
		mEngine->GetTexture("Assets/BGForeground1.png"),
		mEngine->GetTexture("Assets/BGForeground2.png"),
		mEngine->GetTexture("Assets/BGForeground3.png")
	};

	fg->SetBGTexture(fgtexs);
	fg->SetScrollSpeed(-10.0f);

	//Mario
	mPlayer = new Mario(this);

	mCurrentLevel = 0;
	mScoreManager = new ScoreManager(this);
	mHighScore = mScoreManager->GetHighscore();
	std::cout << "Highscore: " << mHighScore << std::endl;
	mScoreText = new Actor(this);
	mScoreText->SetPosition(Vector2(SCREEN_WIDTH / 2, 32));
	mScoreTsc = new TextSpriteComponent(mScoreText);
	mScoreTsc->SetText("Score: 0");
	mScoreTsc->SetTextSize(72);

	mLevel1 = new GameScreen(this, "Assets/Maps/MarioMainMenu.csv");
	mLevel2 = new GameScreen(this, "Assets/Maps/Mario01.csv");
	mLevel3 = new GameScreen(this, "Assets/Maps/Mario02.csv");
	mLevel4 = new GameScreen(this, "Assets/Maps/Mario02.csv");
	mLevel5 = new GameScreen(this, "Assets/Maps/Mario02.csv");


	mLevels[0] = mLevel1;
	mLevels[1] = mLevel2;
	mLevels[2] = mLevel3;
	mLevels[3] = mLevel4;
	mLevels[4] = mLevel5;

	mLevels[mCurrentLevel]->LoadLevel();

	mNextLevelScreen = new NextLevelScreen(this);
	mMainMenu = new MainMenuScreen(this);
	mGameOverScreen = new GameOverScreen(this);
}

void Game::IncrementScore(int amount) {
	mScore += amount;
	std::string text = "Score: " + std::to_string(mScore);
	mScoreTsc->SetText(text);
}


void Game::SetPlayerSpawnPoint(Vector2 position) {
	mPlayer->SetPosition(position);
	mPlayer->ChangePlayerTile(position);
}

void Game::NextLevel()
{
	mNextLevelScreen->SetActive(false);

	gameOver = true;

	mScore = 0;
	mScoreTsc->SetText("Score: " + mScore);
	mCurrentLevel++; 

	if (mCurrentLevel > 3) {
		mCurrentLevel = 0;
	}
	mLevels[mCurrentLevel]->EmptyMap();
	mHighScore = mScoreManager->GetHighscore();
	mLevels[mCurrentLevel]->LoadLevel();
	gameOver = false;
}

void Game::PlayFirstLevel()
{
	mMainMenu->SetActive(false);

	gameOver = true;

	mScore = 0;
	mScoreTsc->SetText("Score: " + mScore);

	mLevels[mCurrentLevel]->EmptyMap();
	mCurrentLevel = 1;
	mHighScore = mScoreManager->GetHighscore();
	mLevels[mCurrentLevel]->LoadLevel();
	gameOver = false;
	bPaused = false;
}

void Game::ReturnToMainMenu()
{
	mNextLevelScreen->SetActive(false);
	mGameOverScreen->SetActive(false);


	bPaused = true;

	mMainMenu->SetActive(true);

	mPlayer->SetDead(false);
	gameOver = true;

	mScore = 0;
	mScoreTsc->SetText("Score: " + mScore);

	mLevels[mCurrentLevel]->EmptyMap();
	mHighScore = mScoreManager->GetHighscore();
	mCurrentLevel = 0;
	mLevels[mCurrentLevel]->LoadLevel();
}

void Game::RetryLevel()
{
	mGameOverScreen->SetActive(false);

	mScore = 0;
	mScoreTsc->SetText("Score: " + mScore);

	testScreen->EmptyMap();
	mHighScore = mScoreManager->GetHighscore();
	mLevels[mCurrentLevel]->LoadLevel();
	mPlayer->SetDead(false);
}

void Game::OnPlayerDeath()
{
	mGameOverScreen->SetActive(true);
}

void Game::PollInput()
{
	SDL_Event event;
	SDL_PollEvent(&event);
	mNextLevelScreen->Update(0.16f, event);
	mMainMenu->Update(0.16f, event);
	mGameOverScreen->Update(0.16f, event);

	mEngine->PollInput();

	const Uint8* state = SDL_GetKeyboardState(NULL);
	mPlayer->HandleEvents(state);

	//if (state[SDL_SCANCODE_1]) {
	//	gameOver = true;
	//	mScore = 0;
	//	mScoreTsc->SetText("Score: " + mScore);
	//	mCurrentLevel = 0;
	//	testScreen->EmptyMap();
	//	map->LoadMap(mLevels[mCurrentLevel]);
	//	gameOver = false;
	//	
	//}
	//else if (state[SDL_SCANCODE_2]) {
	//	gameOver = true;
	//	mScore = 0;
	//	mScoreTsc->SetText("Score: " + mScore);
	//	EmptyMap();
	//	mCurrentLevel = 1;
	//	map->LoadMap(mLevels[mCurrentLevel]);
	//	gameOver = false;
	//}
	//else if (state[SDL_SCANCODE_3]) {
	//	gameOver = true;
	//	mScore = 0;
	//	mScoreTsc->SetText("Score: " + mScore);
	//	EmptyMap();
	//	mCurrentLevel = 2;
	//	map->LoadMap(mLevels[mCurrentLevel]);
	//	gameOver = false;
	//}
	//else if (state[SDL_SCANCODE_4]) {
	//	gameOver = true;
	//	mScore = 0;
	//	mScoreTsc->SetText("Score: " + mScore);
	//	EmptyMap();
	//	mCurrentLevel = 3;
	//	map->LoadMap(mLevels[mCurrentLevel]);
	//	gameOver = false;
	//}
}

void Game::Update()
{
	mCamera.x = mPlayer->GetPosition().x - SCREEN_WIDTH / 2;

	if (!mBGMusic->IsPlaying()) {
		mBGMusic->Play();
	}

	if (mCamera.x < 0)
		mCamera.x = 0;
	if (mCamera.x > mCamera.w)
		mCamera.x = mCamera.w;
	if (mCamera.y < 0)
		mCamera.y = 0;
	if (mCamera.y > mCamera.h)
		mCamera.y = mCamera.h;

	mEngine->Update();
}

void Game::Render()
{
	mEngine->Render();
}

void Game::LoadNextLevelMenu()
{
	if (mScore > mHighScore) {
		mScoreManager->SetHighscore(mScore);
		mHighScore = mScore;
	}

	mNextLevelScreen->SetActive(true);
}

void Game::Shutdown()
{
	mEngine->Shutdown();
}