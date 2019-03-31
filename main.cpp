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
typedef void* sound;

struct enemy
{
	float x, y;
	float width, height;
	float xo, yo;
	float x2, y2;
	sprite sprite;
	sound killed;

	enemy()
	{
		x = y = xo = yo = width = height = 0;
		sprite = LoadSprite("gfx/Little Invader.png");
		killed = LoadSnd("sounds/invaderkilled.wav");
	}
};

struct player
{
	float x, y, x2, y2, width, height;
	sprite sprite;
	sound shoot;
	sound moved;

	player() : x(400), y(550)
	{
		width = height = 50;
		x2 = x + width;
		y2 = y + height;
		sprite = LoadSprite("gfx/Big Invader.png");
		shoot = LoadSnd("sounds/shoot.wav");
	}
};


struct bullet
{
	float x, y, x2, y2, a, width, height;
	sprite sprite;

	bullet(int x, int y) : x(x), y(y), a(0)
	{
		width = height = 10;
		x2 = x + width;
		y2 = y + height;
		sprite = LoadSprite("gfx/bullet.png");
	}
};

struct GameState
{
	std::vector<enemy> enemies;
	std::vector<bullet> bullets;
	player player;
	int reload{};
	void* Text[14];
	sound enemyMoved;
	sound gameOver;

	int time;

	GameState() : Text{}
	{
		time = reload = 0;
	}
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
			gameState.enemies[n].y += 1;
			PlaySnd(gameState.enemyMoved);
		}
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
	{
		gameState.player.x -= 7;
		gameState.player.x2 = gameState.player.x + gameState.player.width;
	}

	if (IsKeyDown(VK_RIGHT))
	{
		gameState.player.x += 7;
		gameState.player.x2 = gameState.player.x + gameState.player.width;
	}
}

void shoot()
{
	if (gameState.reload > 0)
		--gameState.reload;

	if (IsKeyDown(VK_SPACE) && gameState.reload == 0)
	{
		gameState.bullets.emplace_back(gameState.player.x, gameState.player.y);
		PlaySnd(gameState.player.shoot);
		gameState.reload = 15;
	}
}

bool valueInRange(float value, float min, float max)
{
	return (value >= min) && (value <= max);
}

bool overlap(enemy e, bullet b)
{
	bool xOverlap = valueInRange(e.x + e.xo, b.x, b.x2) ||
		valueInRange(b.x, e.x + e.xo, e.x2 + e.xo);

	bool yOverlap = valueInRange(e.y + e.yo, b.y, b.y2) ||
		valueInRange(b.y, e.y + e.yo, e.y2 + e.yo);

	return xOverlap && yOverlap;
}

bool overlap(player p, enemy e)
{
	bool xOverlap = valueInRange(p.x, e.x + e.xo, e.x2 + e.xo) ||
		valueInRange(e.x + e.xo, p.x, p.x2);

	bool yOverlap = valueInRange(p.y, e.y + e.yo, e.y2 + e.yo) ||
		valueInRange(e.y + e.yo, p.y, p.y2);

	return xOverlap && yOverlap;
}


bool checkCollisions()
{
	for (int i = 0; i < gameState.enemies.size(); ++i)
	{
		if (overlap(gameState.player, gameState.enemies[i]))
		{
			int id = MessageBox(NULL, "Game Over", "Space Invaders", MB_OK);
			if (id == 1)
				return false;
		}

		for (int j = 0; j < gameState.bullets.size(); ++j)
		{
			if (overlap(gameState.enemies[i], gameState.bullets[j]))
			{
				PlaySnd(gameState.enemies[i].killed);
				removeFrom(gameState.enemies, i);
				removeFrom(gameState.bullets, j);
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
		return false;

	return checkCollisions();
}

/**
 * Draws all sprites in to the game window.
 */
void draw()
{
	for (auto& enemy : gameState.enemies)
	{
		DrawSprite(enemy.sprite, enemy.x + enemy.xo, enemy.y + enemy.yo,
			enemy.width, enemy.height, 0, white);
	}

	DrawSprite(gameState.player.sprite,
		gameState.player.x,
		gameState.player.y,
		gameState.player.width,
		gameState.player.height,
		PI + sin(gameState.time*0.1)*0.1,
		white);

	for (auto& bullet : gameState.bullets)
	{
		DrawSprite(bullet.sprite, bullet.x, bullet.y, bullet.width, bullet.height, bullet.a, white);
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
	gameState.enemyMoved = LoadSnd("sounds/fastinvader1.wav");
	PlayMusic("sounds/music.mp3");

	for (int n = 0; n < 50; ++n)
	{
		enemy e;
		e.x = (n % 10) * 60 + 120;
		e.y = (n / 10) * 60 + 70;
		e.width = 10 + n % 17;
		e.height = 10 + n % 17;
		e.x2 = e.x + e.width;
		e.y2 = e.y + e.height;
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
