#include "lib/leetlib.h"
#include <vector>
#include <iostream>
#include <string>

/**
		main.cpp
		Purpose: Refactor source code of Space Invaders project, add collisions, write score and make it better!

		@author Martin Studna
		@version 1.0 01/03/17
*/


typedef void* sprite;

struct enemy
{
	float x, y;
	float xSize, ySize;
	float xo, yo;
	sprite sprite;

	enemy()
	{
		x = y = xo = yo = xSize = ySize = 0;
		sprite = LoadSprite("gfx/Little Invader.png");
	}
};

struct player
{
	float x, y, xSize, ySize;
	sprite sprite;

	player() : x(400), y(550)
	{
		xSize = ySize = 50;
		sprite = LoadSprite("gfx/Big Invader.png");
	}
};


struct bullet
{
	float x, y, a, xSize, ySize;
	sprite sprite;

	bullet(int x, int y) : x(x), y(y), a(0)
	{
		xSize = ySize = 10;
		sprite = LoadSprite("gfx/bullet.png");
	}
};

struct GameState
{
	enemy enemies[50];
	std::vector<bullet> bullets;
	player player;
	int reload;
	void* Text[14];

	int time;

	GameState() : Text{} { time = 0; }
};

const DWORD white = 0xffffffff;

GameState gameState;

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
	for (int n = 0; n < 50; ++n)
	{
		gameState.enemies[n].xo = gameState.enemies[n].yo = 0;
		int n1 = gameState.time + myPow(n, 2) + myPow(n, 3);
		int n2 = gameState.time + n + myPow(n, 2) + myPow(n, 3) * 3;

		if ((n1 / 128) % 8 == 7)
		{
			gameState.enemies[n].xo += (1 - cos(n1 % 128 / 64.0f*2.f*PI))*(20 + myPow(n, 2) % 9);
			gameState.enemies[n].yo += sin(n1 % 128 / 64.0f*2.f*PI)*(20 + myPow(n, 2) % 9);
		}

		if ((n2 / 256) % 16 == 15)
			gameState.enemies[n].yo += (1 - cos(n2 % 256 / 256.0f*2.f*PI))*(150 + myPow(n, 2) % 9);
	}
}

void updateBulletMovements()
{
	for (auto& bullet : gameState.bullets)
	{
		bullet.y -= 4;
		bullet.a += 0.1f;
	}
}

void updatePlayerMovements()
{
	if (IsKeyDown(VK_LEFT))
		gameState.player.x -= 7;

	if (IsKeyDown(VK_RIGHT))
		gameState.player.x += 7;
}

void shoot()
{
	if (gameState.reload > 0)
		--gameState.reload;

	if (IsKeyDown(VK_SPACE) && gameState.reload == 0)
	{
		gameState.bullets.emplace_back(gameState.player.x, gameState.player.y);
		gameState.reload = 15;
	}
}

void checkCollisions()
{
	for (auto& enemy : gameState.enemies)
	{
		for (auto& bullet : gameState.bullets)
		{

		}
	}
}

void overlap()
{

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
}

/**
 * Draws all sprites in to the game window.
 */
void draw()
{
	for (auto& enemy : gameState.enemies)
	{		
		DrawSprite(gameState.enemies->sprite, enemy.x + enemy.xo, enemy.y + enemy.yo,
		           enemy.xSize, enemy.ySize, 0, white);
	}

	DrawSprite(gameState.player.sprite,
		gameState.player.x,
		gameState.player.y,
		gameState.player.xSize,
		gameState.player.ySize,
		PI + sin(gameState.time*0.1)*0.1,
		white);

	for (auto& bullet : gameState.bullets)
	{
		DrawSprite(bullet.sprite, bullet.x, bullet.y, bullet.xSize, bullet.ySize, bullet.a, white);
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

	// Loads symbols for Space Invaders title
	for (int n = 0; n < title.length(); ++n)
	{
		auto path = std::string("gfx/") + title[n] + "let.png";
		const char* p = path.c_str();

		gameState.Text[n] = n != 5 ? LoadSprite(p) : 0;
	}

	gameState.player = player();
	gameState.reload = 0;

	for (int n = 0; n < 50; ++n)
	{
		gameState.enemies[n] = enemy();
		gameState.enemies[n].x = (n % 10) * 60 + 120;
		gameState.enemies[n].y = (n / 10) * 60 + 70;
		gameState.enemies[n].xSize = 10 + n % 17;
		gameState.enemies[n].ySize = 10 + n % 17;
	}
}


void Game()
{
	setup();

	while (!WantQuit() && !IsKeyDown(VK_ESCAPE))
	{
		update();
		draw();
	}
}
