#include "lib/leetlib.h"
#include <math.h>
#include <vector>

typedef void* sprite;

struct enemy
{
  int x, y;
  int xo, yo;
  sprite sprite;

  enemy()
  {
    x = y = xo = yo = 0;
    sprite = LoadSprite("gfx/Little Invader.png");
  }
};

struct player
{
  int x, y;
  sprite sprite;

  player() : x(400), y(550)
  {
    sprite = LoadSprite("gfx/Big Invader.png");
  }
};


struct bullet
{
  float x, y, a;
  sprite sprite;

  bullet(int x, int y) : x(x), y(y), a(0)
  {
    sprite = LoadSprite("gfx/bullet.png");
  }
};

struct GameState
{
  enemy enemies[50];
  std::vector<bullet> bullets;
  player player;
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

void update()
{
  ++gameState.time;

  for (int n = 0; n < 50; ++n)
  {
    gameState.enemies[n].xo = 0, gameState.enemies[n].yo = 0;
    int n1 = gameState.time + n * n + n * n*n;
    int n2 = gameState.time + n + n * n + n * n*n * 3;

    if (((n1 >> 6) & 0x7) == 0x7)
      gameState.enemies[n].xo += (1 - cos((n1 & 0x7f) / 64.0f*2.f*3.141592))*(20 + ((n*n) % 9));

    if (((n1 >> 6) & 0x7) == 0x7)
      gameState.enemies[n].yo += (sin((n1 & 0x7f) / 64.0f*2.f*3.141592))*(20 + ((n*n) % 9));

    if (((n2 >> 8) & 0xf) == 0xf)
      gameState.enemies[n].yo += (1 - cos((n2 & 0xff) / 256.0f*2.f*3.141592))*(150 + ((n*n) % 9));

  }

  if (IsKeyDown(VK_LEFT))
    gameState.player.x -= 7;

  if (IsKeyDown(VK_RIGHT))
    gameState.player.x += 7;

  static int b = 0;
  static int count = 0;

  if (count)
    --count;

  if (!IsKeyDown(VK_SPACE))
    count = 0;

  for (auto& bullet : gameState.bullets)
  {
    bullet.y -= 4;
    bullet.a += 0.1f;
  }

  if (IsKeyDown(VK_SPACE) && count == 0)
  {
    gameState.bullets.emplace_back(gameState.player.x, gameState.player.y);

    b = (b + 1) % 10;
    count = 15;
  }
}

void draw()
{
  for (int n = 0; n < 50; ++n)
  {
    DrawSprite(gameState.enemies->sprite, gameState.enemies[n].x + gameState.enemies[n].xo, gameState.enemies[n].y + gameState.enemies[n].yo,
      (10 + ((n) % 17)), (10 + ((n) % 17)), 0, white);
  }

  DrawSprite(gameState.player.sprite, gameState.player.x, gameState.player.y, 50, 50, PI + sin(gameState.time*0.1)*0.1, white);

  for (auto& bullet : gameState.bullets)
  {
    DrawSprite(bullet.sprite, bullet.x, bullet.y, 10, 10, bullet.a, white);
  }

  for (int n = 0; n < strlen("space invaders"); ++n)
  {
    if (n != 5)
      DrawSprite(gameState.Text[n], n * 40 + 150, 30, 20, 20, sin(gameState.time*0.1)*n*0.01);
  }

  Flip();
}

void setup()
{
  void* Text[] =
  {
    LoadSprite("gfx/slet.png"),
    LoadSprite("gfx/plet.png"),
    LoadSprite("gfx/alet.png"),
    LoadSprite("gfx/clet.png"),
    LoadSprite("gfx/elet.png"),
    0,
    LoadSprite("gfx/ilet.png"),
    LoadSprite("gfx/nlet.png"),
    LoadSprite("gfx/vlet.png"),
    LoadSprite("gfx/alet.png"),
    LoadSprite("gfx/dlet.png"),
    LoadSprite("gfx/elet.png"),
    LoadSprite("gfx/rlet.png"),
    LoadSprite("gfx/slet.png")
  };

  for (int i = 0; i < strlen("space invaders"); ++i)
  {
    gameState.Text[i] = Text[i];
  }

  gameState.player = player();

  for (int n = 0; n < 50; ++n)
  {
    gameState.enemies[n] = enemy();
    gameState.enemies[n].x = (n % 10) * 60 + 120;
    gameState.enemies[n].y = (n / 10) * 60 + 70;
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
