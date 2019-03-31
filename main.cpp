#include "lib/leetlib.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <d3dx9core.h>

/**
		main.cpp
		Purpose: Refactor source code of Space Invaders project, add collision detection, write score and make it better!

		@author Martin Studna
		@version 1.0 01/03/17
*/

enum mode { Play, Score };


typedef void* sprite;
typedef void* sound;

mode _mode;

struct enemy
{
	float xcentre, ycentre;
	float width, height;
	float xo, yo;
	float x2, y2;
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
	float xcentre, ycentre, x2, y2, width, height;
	float angle;
	int score;
	sprite sprite;
	sound shoot;
	sound moved;
	RECT rect;

	player() : xcentre(400), ycentre(550)
	{
		score = 0;
		width = height = 50;
		x2 = xcentre + width;
		y2 = ycentre + height;
		sprite = LoadSprite("gfx/Big Invader.png");
		shoot = LoadSnd("sounds/shoot.wav");
	}
};

struct bullet
{
	float xcentre, ycentre, x2, y2, angle, width, height;
	sprite sprite;
	RECT rect;

	bullet(const int xcentre, const int ycentre) : xcentre(xcentre), ycentre(ycentre), angle(0)
	{
		width = height = 10;
		x2 = xcentre + width;
		y2 = ycentre + height;
		sprite = LoadSprite("gfx/bullet.png");
	}
};

struct GameState
{
	std::vector<enemy> enemies;
	std::vector<bullet> bullets;
	player player;
	int reload;
	void* Text[14];
	sound enemyMoved;
	sound gameOver;
	RECT windowRect;

	int time;

	GameState() : Text{}
	{
		time = reload = 0;
		GetWindowRect(GetActiveWindow(), &windowRect);
		gameOver = LoadSnd("sounds/over.wav");
		reload = 0;
		enemyMoved = LoadSnd("sounds/fastinvader1.wav");
		PlayMusic("sounds/music.mp3");
	}
};

const DWORD white = 0xffffffff;

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
		gameState.enemies[n].xo = gameState.enemies[n].yo = 0;
		int n1 = gameState.time + myPow(n, 2) + myPow(n, 3);
		int n2 = gameState.time + n + myPow(n, 2) + myPow(n, 3) * 3;

		if ((n1 / 128) % 8 == 7)
		{
			gameState.enemies[n].xo += (1 - cos(n1 % 128 / 64.0f*2.f*PI))*(20 + myPow(n, 2) % 9);
			gameState.enemies[n].xo += sin(n1 % 128 / 64.0f*2.f*PI)*(20 + myPow(n, 2) % 9);
		}

		if ((n2 / 256) % 16 == 15)
			gameState.enemies[n].yo += (1 - cos(n2 % 256 / 256.0f*2.f*PI))*(150 + myPow(n, 2) % 9);

		if (gameState.time > 600 && gameState.time % 600 < 50)
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

		int i = 0;
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
	std::ofstream outfile("score.txt", std::ios_base::app | std::ios_base::out);
	outfile << "Killed aliens: " << gameState.player.score << " time: " << gameState.time << std::endl;
	outfile.close();
}

std::string readScore()
{
	std::ifstream infile;
	infile.open("score.txt"); //open the input file

	std::stringstream strStream;
	strStream << infile.rdbuf(); //read the file
	std::string str = strStream.str();
	infile.close();

	return str;
}


bool checkCollisions()
{
	for (int i = 0; i < gameState.enemies.size(); ++i)
	{
		RECT inter;
		if (IntersectRect(&inter, &gameState.enemies[i].rect, &gameState.player.rect))
		{
			StopMusic();
			PlaySnd(gameState.gameOver);
			int id = MessageBox(nullptr, "Game Over", "Space Invaders", MB_OK);

			writeScore();

			MessageBox(nullptr, readScore().c_str(), "Score", MB_OK);

			return false;
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
	return true;
}

/**
 * Updates all important states of the game like position of player, enemies, bullets and etc.
 */
bool update()
{
	++gameState.time;

	updateEnemiesMovements();
	updatePlayerMovements();
	updateBulletMovements();
	shoot();

	if (gameState.enemies.empty())
	{
		MessageBox(nullptr, "You saved the humanity against the alien invasion!", "Space Invaders", MB_OK);

		writeScore();
		

		MessageBox(nullptr, readScore().c_str(), "Score", MB_OK);

		return false;
	}


	return checkCollisions();
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
		e.x2 = e.xcentre + e.width;
		e.y2 = e.ycentre + e.height;
		gameState.enemies.push_back(e);
	}
}

void Game()
{
	setup();

	while (!WantQuit() && !IsKeyDown(VK_ESCAPE))
	{
		if (!update())
			break;
		draw();
	}
}