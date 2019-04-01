#include "lib/leetlib.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <d3dx9core.h>
#include <map>
#include <algorithm>
#include <chrono>

/**
		main.cpp
		Purpose: Refactor source code of Space Invaders project, add collision detection, write score and make it better!

		@author Martin Studna
		@version 1.0 01/03/17
*/

enum mode { Play, Score };

typedef void* sprite;
typedef void* sound;
const DWORD white = 0xffffffff;

struct enemy
{
	float xcentre, ycentre;
	float width, height;
	float xo, yo;
	sprite sprite;
	sound killed;
	RECT rect;

	enemy()
	{
		xcentre = ycentre = xo = yo = width = height = 0;
		rect = { 0,0,0,0 };
		sprite = LoadSprite("gfx/Little Invader.png");
		killed = LoadSnd("sounds/invaderkilled.wav");
	}
};

struct player
{
	float xcentre, ycentre, width, height;
	float angle;
	int score;
	sprite sprite;
	sound shoot;
	sound moved;
	RECT rect;

	player() : xcentre(400), ycentre(550)
	{
		score = 0;
		angle = 0.f;
		sprite = shoot = moved = nullptr;
		rect = RECT();
		width = height = 50;
		sprite = LoadSprite("gfx/Big Invader.png");
		shoot = LoadSnd("sounds/shoot.wav");
	}
};

struct bullet
{
	float xcentre, ycentre, angle, width, height;
	sprite sprite;
	RECT rect;

	bullet(const int xcentre, const int ycentre) : xcentre(xcentre), ycentre(ycentre), angle(0)
	{
		width = height = 10;
		rect = RECT();
		sprite = LoadSprite("gfx/bullet.png");
	}
};

struct GameState
{
	std::vector<enemy> enemies;
	std::vector<bullet> bullets;
	std::string results;
	mode mode;
	bool read;
	player player;
	int reload;
	void* Text[14]{};
	sound enemyMoved;
	sound gameOver;
	RECT windowRect{};
	std::chrono::steady_clock::time_point start;

	int time;

	GameState() 
	{
		mode = Play;
		start = std::chrono::steady_clock::now();
		results = "";
		read = false;
		time = reload = 0;
		GetWindowRect(GetActiveWindow(), &windowRect);
		gameOver = LoadSnd("sounds/over.wav");
		reload = 0;
		enemyMoved = LoadSnd("sounds/fastinvader1.wav");
		PlayMusic("sounds/music.mp3");
	}
};

struct score
{
	int kills;
	int time;

	score(int kills, int time) : kills(kills), time(time) {}
};

GameState gameState;

RECT setCurrentRect(float xcentre, float ycentre, float width, float height, float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);

#define ROTATE_X(xx,yy) (xcentre+(xx)*c+(yy)*s)
#define ROTATE_Y(xx,yy) (ycentre+(yy)*c-(xx)*s)

	return RECT
	{
		LONG(ROTATE_X(-width, -height)),
		LONG(ROTATE_Y(-width, -height)),
		LONG(ROTATE_X(width, height)),
		LONG(ROTATE_Y(width, height)),
	};
}

template<class T>
void removeFrom(std::vector<T>& list, int i)
{
	if (!list.empty())
	{
		list[i] = list.back();
		list.pop_back();
	}
}

int myPow(int x, int p) {
	if (p == 0)
		return 1;
	if (p == 1)
		return x;
	return x * myPow(x, p - 1);
}

void updateEnemiesMovements()
{
	for (int n = 0; n < gameState.enemies.size(); ++n)
	{
		gameState.enemies[n].xo = 0, gameState.enemies[n].yo = 0;
		int n1 = gameState.time + myPow(n, 2) + myPow(n, 3);
		int n2 = gameState.time + n + myPow(n, 2) + myPow(n, 3) * 3;

		if (((n1 >> 6) & 7) == 7)
		{
			gameState.enemies[n].xo += (1 - cos((n1 & 127) / 64.0f*2.f*PI))*(20 + (myPow(n, 2) % 9));
			gameState.enemies[n].yo += (sin((n1 & 127) / 64.0f*2.f*PI))*(20 + (myPow(n, 2) % 9));
		}

		if (((n2 >> 8) & 15) == 15)
			gameState.enemies[n].yo += (1 - cos((n2 & 255) / 256.0f*2.f*PI))*(150 + (myPow(n, 2) % 9));

		if (gameState.time > 400 && gameState.time % 400 < 50)
		{
			gameState.enemies[n].ycentre += 1;
			PlaySnd(gameState.enemyMoved);
		}

		gameState.enemies[n].rect = setCurrentRect(
			gameState.enemies[n].xcentre + gameState.enemies[n].xo,
			gameState.enemies[n].ycentre + gameState.enemies[n].yo,
			gameState.enemies[n].width,
			gameState.enemies[n].height,
			0);
	}
}

void updateBulletMovements()
{
	for (auto& bullet : gameState.bullets)
	{
		bullet.ycentre -= 4;
		bullet.angle += 0.1f;

		bullet.rect = setCurrentRect(
			bullet.xcentre,
			bullet.ycentre,
			bullet.width,
			bullet.height,
			0);
	}
}

void updatePlayerMovements()
{
	if (IsKeyDown(VK_LEFT) && gameState.player.rect.left > 0)
	{
		gameState.player.xcentre -= 7;
	}

	if (IsKeyDown(VK_RIGHT) && gameState.player.rect.right < gameState.windowRect.right)
	{
		gameState.player.xcentre += 7;
	}

	gameState.player.angle = PI + sin(gameState.time*0.1)*0.1;

	gameState.player.rect = setCurrentRect(
		gameState.player.xcentre,
		gameState.player.ycentre,
		gameState.player.width,
		gameState.player.height,
		0);
}

void shoot()
{
	if (gameState.reload > 0)
		--gameState.reload;

	if (IsKeyDown(VK_SPACE) && gameState.reload == 0)
	{
		gameState.bullets.emplace_back(gameState.player.xcentre, gameState.player.ycentre);
		PlaySnd(gameState.player.shoot);
		gameState.reload = 15;
	}
}

void writeScore()
{
	std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::steady_clock::now() - gameState.start);
	std::ofstream outfile("score.txt", std::ios_base::app | std::ios_base::out);
	outfile << "Killed aliens: " << gameState.player.score << " time: " << elapsed.count() << " s" << std::endl;
	outfile.close();
}

std::vector<score> scores;

struct comparator
{
	bool operator() (const score& struct1, const score& struct2) const
	{
		if (struct1.kills > struct2.kills)
			return true;

		if (struct1.kills == struct2.kills)
			return struct1.time < struct2.time;

		return false;
	}
};

std::string readScore()
{
	std::ifstream infile;
	infile.open("score.txt"); //open the input file
	std::string line;
	std::vector<std::string> tokens;

	while (std::getline(infile, line))
	{
		std::stringstream strStream(line);
		std::string token;

		for (int i = 0; i < 5; ++i)
		{
			if (strStream.good())
			{
				strStream >> token;
				tokens.push_back(token);
			}
		}

		if (tokens.size() == 5)
			scores.emplace_back(std::stoi(tokens[2]), std::stoi(tokens[4]));

		tokens.clear();
	}
	infile.close();

	std::sort(scores.begin(), scores.end(), comparator());

	std::string results;

	for (int i = 0; i < scores.size(); ++i)
	{
		if (i == 10)
			break;

		results += "Killed aliens: " + std::to_string(scores[i].kills) + " Time: " + std::to_string(scores[i].time) + " s\n";
	}

	std::ofstream outfile("score.txt");
	outfile << results;
	outfile.close();

	return results;
}


void checkCollisions()
{
	RECT inter;

	for (int i = 0; i < gameState.enemies.size(); ++i)
	{
		if (IntersectRect(&inter, &gameState.enemies[i].rect, &gameState.player.rect))
		{
			StopMusic();
			PlaySnd(gameState.gameOver);
			gameState.mode = mode::Score;
			writeScore();
			break;
		}

		for (int j = 0; j < gameState.bullets.size(); ++j)
		{
			if (IntersectRect(&inter, &gameState.enemies[i].rect, &gameState.bullets[j].rect))
			{
				PlaySnd(gameState.enemies[i].killed);
				removeFrom(gameState.enemies, i);
				removeFrom(gameState.bullets, j);
				gameState.player.score++;
				break;
			}
		}
	}
}

/**
 * Updates all important states of the game like position of player, enemies, bullets and etc.
 */
void update()
{
	++gameState.time;

	updateEnemiesMovements();
	updatePlayerMovements();
	updateBulletMovements();
	shoot();
	checkCollisions();

	if (gameState.enemies.empty())
	{
		writeScore();
		gameState.mode = mode::Score;
	}
}

/**
 * Draws all sprites in to the game window.
 */
void draw()
{
	for (auto& enemy : gameState.enemies)
	{
		DrawSprite(enemy.sprite, enemy.xcentre + enemy.xo, enemy.ycentre + enemy.yo,
			enemy.width, enemy.height, 0, white);
	}

	DrawSprite(gameState.player.sprite,
		gameState.player.xcentre,
		gameState.player.ycentre,
		gameState.player.width,
		gameState.player.height,
		gameState.player.angle,
		white);

	for (auto& bullet : gameState.bullets)
	{
		DrawSprite(bullet.sprite, bullet.xcentre, bullet.ycentre, bullet.width, bullet.height, bullet.angle, white);
	}

	for (int n = 0; n < strlen("space invaders"); ++n)
	{
		if (n != 5)
			DrawSprite(gameState.Text[n], n * 40 + 150, 30, 20, 20, sin(gameState.time*0.1)*n*0.01);
	}

	Flip();
}

void drawScore()
{
	if (!gameState.read)
	{
		gameState.results = readScore();
		gameState.read = true;
	}

	DrawSomeText(100, 100, 40, white, false, "Score:");
	DrawSomeText(100, 150, 30, white, false, gameState.results.c_str());
	DrawSomeText(100, 480, 30, white, false, "Press Esc to close window.");
	Flip();
}


/**
 * Initializes all important resources to start the game.
 */
void setup()
{
	std::string title = "space invaders";
	gameState = GameState();
	gameState.player = player();


	// Loads symbols for Space Invaders title
	for (int n = 0; n < title.length(); ++n)
	{
		auto path = std::string("gfx/") + title[n] + "let.png";
		const char* p = path.c_str();

		gameState.Text[n] = n != 5 ? LoadSprite(p) : 0;
	}

	for (int n = 0; n < 50; ++n)
	{
		enemy e;
		e.xcentre = (n % 10) * 60 + 120;
		e.ycentre = (n / 10) * 60 + 70;
		e.width = 10 + n % 17;
		e.height = 10 + n % 17;
		gameState.enemies.push_back(e);
	}
}

void Game()
{
	setup();

	while (!WantQuit() && !IsKeyDown(VK_ESCAPE))
	{
		switch (gameState.mode)
		{
			case Play:
			{
				update();
				draw();
				break;
			}
			case Score:
				drawScore();
				break;
		}
	}
}