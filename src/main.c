#include <genesis.h>
#include <resources.h>

#define LEVEL_NUM 3

#define SPAWN_TILE 4
#define EXIT_TILE 5
#define COIN_TILE 6

#define SFX_COIN 64
#define SFX_UNLOCK 65

#define TILESIZE 8
#define MAP_WIDTH 8
#define MAP_HEIGHT 8

#define SOLID_TILE 1

#define ANIM_DOWN 0
#define ANIM_UP 1
#define ANIM_SIDE 2

#define MAX_COINS 3

typedef enum { up, down, left, right, none } moveDirection;

typedef u8 map[MAP_HEIGHT][MAP_WIDTH];

typedef struct {
  u8 x;
  u8 y;
} Point;

typedef struct {
  Point pos;
  Point tilePos;
  int w;
  int h;
  int health;
  bool moving;
  moveDirection dir;
  Sprite *sprite;
  char name[6];
} Entity;

typedef struct {
  Point pos;
  u8 w;
  u8 h;
  Sprite *sprite;
  u8 health;
} Coin;

map level1 = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 6, 0},
              {0, 4, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 1, 1, 0, 0, 0},
              {0, 6, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0},
              {0, 0, 0, 6, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 5}};
map level2 = {{0, 0, 0, 6, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
              {0, 4, 0, 1, 1, 1, 0, 0}, {0, 0, 0, 6, 0, 1, 0, 0},
              {0, 0, 0, 1, 1, 1, 6, 0}, {0, 0, 0, 1, 0, 0, 0, 0},
              {0, 0, 0, 1, 1, 1, 5, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
map level3 = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 1, 1, 1, 1, 0, 0},
              {0, 0, 0, 0, 6, 1, 0, 0}, {0, 0, 0, 1, 1, 1, 6, 0},
              {0, 0, 0, 0, 0, 1, 0, 0}, {0, 6, 1, 1, 1, 1, 0, 0},
              {0, 4, 0, 0, 0, 0, 5, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};

u8 *currentLevel;
static u8 currentLevelIndex = 0;
map *levelList[3];

Entity player = {{0, 0}, {0, 0}, 8, 8, 0, FALSE, none, NULL, "PLAYER"};
Coin coins[MAX_COINS];

int getTileAt(u8 X, u8 Y) { return *(currentLevel + (Y * MAP_HEIGHT + X)); }

// HUD
u8 coinsCollected = 0;
char hud_string[10] = "";

Point exitLocation = {0, 0};
bool exitUnlocked = FALSE;

void updateScoreDisplay() {
  sprintf(hud_string, "SCORE: %d", coinsCollected);
  VDP_clearText(8, 0, 10);
  VDP_drawText(hud_string, 8, 0);
}

void clearLevel() {
  VDP_clearPlane(BG_B, TRUE);
  SPR_reset();
  // The tutorial uses this one but it does not work.
  // Probably this happened due to SGDK version differences.
  // VDP_clearSprites(); 
  // SPR_clear();
  // VDP_resetSprites();
  // VDP_releaseAllSprites();
  coinsCollected = 0;
  updateScoreDisplay();
  exitUnlocked = FALSE;
}

void loadLevel() {
  u8 x = 0;
  u8 y = 0;
  u8 t = 0;
  u8 i = 0;
  u8 total = MAP_HEIGHT * MAP_WIDTH;
  u8 coinNum = 0;
  Coin *c = coins;

  clearLevel();
  currentLevel = (u8 *)levelList[currentLevelIndex];

  for (i = 0; i < total; i++) {
    t = *(currentLevel + i);
    if (t == SPAWN_TILE) {
      VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), x, y);

      player.tilePos.x = x;
      player.tilePos.y = y;
      player.pos.x = player.tilePos.x * TILESIZE;
      player.pos.y = player.tilePos.y * TILESIZE;

      player.sprite = SPR_addSprite(&spr_player, player.pos.x, player.pos.y,
                                    TILE_ATTR(PAL2, 0, FALSE, FALSE));
    } else {
      VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, t + 1), x,
                       y);
    }

    if (t == COIN_TILE) {
      if (coinNum < MAX_COINS) {
        VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), x, y);

        c = &coins[coinNum];
        c->pos.x = x * TILESIZE;
        c->pos.y = y * TILESIZE;
        c->w = 8;
        c->h = 8;
        c->health = 1;
        c->sprite = SPR_addSprite(&coin, c->pos.x, c->pos.y,
                                  TILE_ATTR(PAL2, 0, FALSE, FALSE));
        coinNum++;
      }
    } else if (t == EXIT_TILE) {
      exitLocation.x = x;
      exitLocation.y = y;
      VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), x, y);
    }

    x++;
    if (x >= MAP_WIDTH) {
      y++;
      x = 0;
    }
  }
}

void unlockExit() {
  exitUnlocked = TRUE;
  VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, 3),
                   exitLocation.x, exitLocation.y);
  XGM_startPlayPCM(SFX_UNLOCK, 1, SOUND_PCM_CH2);
}

void movePlayer(moveDirection Direction) {
  // Move the player
  if (player.moving == FALSE) {
    switch (Direction) {
    case up:
      if (player.tilePos.y > 0 &&
          getTileAt(player.tilePos.x, player.tilePos.y - 1) != SOLID_TILE) {
        player.tilePos.y -= 1;
        player.moving = TRUE;
        player.dir = Direction;
        SPR_setAnim(player.sprite, ANIM_UP);
      }
      break;
    case down:
      if (player.tilePos.y < MAP_HEIGHT - 1 &&
          getTileAt(player.tilePos.x, player.tilePos.y + 1) != SOLID_TILE) {
        player.tilePos.y += 1;
        player.moving = TRUE;
        player.dir = Direction;
        SPR_setAnim(player.sprite, ANIM_DOWN);
      }
      break;
    case left:
      if (player.tilePos.x > 0 &&
          getTileAt(player.tilePos.x - 1, player.tilePos.y) != SOLID_TILE) {
        player.tilePos.x -= 1;
        player.moving = TRUE;
        player.dir = Direction;
        SPR_setAnim(player.sprite, ANIM_SIDE);
        SPR_setHFlip(player.sprite, TRUE);
      }
      break;
    case right:
      if (player.tilePos.x < MAP_WIDTH - 1 &&
          getTileAt(player.tilePos.x + 1, player.tilePos.y) != SOLID_TILE) {
        player.tilePos.x += 1;
        player.moving = TRUE;
        player.dir = Direction;
        SPR_setAnim(player.sprite, ANIM_SIDE);
        SPR_setHFlip(player.sprite, FALSE);
      }
      break;
    default:
      break;
    }
  }
}

void myJoyHandler(u16 joy, u16 changed, u16 state) {
  if (joy == JOY_1) {
    if (state & BUTTON_UP) {
      movePlayer(up);
    } else if (state & BUTTON_DOWN) {
      movePlayer(down);
    } else if (state & BUTTON_LEFT) {
      movePlayer(left);
    } else if (state & BUTTON_RIGHT) {
      movePlayer(right);
    }
  }
}

int main() {
  SYS_disableInts();
  JOY_init();
  JOY_setEventHandler(&myJoyHandler);
  SPR_init();
  XGM_setPCM(SFX_COIN, sfx_coin, sizeof(sfx_coin));
  XGM_setPCM(SFX_UNLOCK, sfx_unlock, sizeof(sfx_unlock));

  VDP_loadTileSet(floortiles.tileset, 1, DMA);
  VDP_setPalette(PAL1, floortiles.palette->data);
  VDP_setPalette(PAL2, spr_player.palette->data);

  levelList[0] = &level1;
  levelList[1] = &level2;
  levelList[2] = &level3;

  loadLevel();

  Coin *coinToCheck;

  updateScoreDisplay();

  SYS_enableInts();
  while (1) {
    SYS_disableInts();

    if (player.moving == TRUE) {
      switch (player.dir) {
      case up:
        player.pos.y -= 1;
        break;
      case down:
        player.pos.y += 1;
        break;
      case left:
        player.pos.x -= 1;
        break;
      case right:
        player.pos.x += 1;
        break;
      default:
        break;
      }
    }

    if (player.pos.x % TILESIZE == 0 && player.pos.y % TILESIZE == 0) {
      player.moving = FALSE;
      // Landed on the exit?
      if (exitUnlocked == TRUE && player.tilePos.x == exitLocation.x &&
          player.tilePos.y == exitLocation.y) {
        currentLevelIndex++;
        if (currentLevelIndex >= LEVEL_NUM) {
          currentLevelIndex = 0;
        }
        loadLevel();
      }
    }
    SPR_setPosition(player.sprite, player.pos.x, player.pos.y);

    u8 i = 0;
    // Check for coins
    for (i = 0; i < MAX_COINS; i++) {
      coinToCheck = &coins[i];
      if ((player.pos.x < coinToCheck->pos.x + coinToCheck->w &&
           player.pos.x + player.w > coinToCheck->pos.x &&
           player.pos.y < coinToCheck->pos.y + coinToCheck->h &&
           player.pos.y + player.h > coinToCheck->pos.y) == TRUE) {
        if (coinToCheck->health > 0) {
          coinToCheck->health = 0;
          SPR_setVisibility(coinToCheck->sprite, HIDDEN);
          coinsCollected++;
          XGM_startPlayPCM(SFX_COIN, 1, SOUND_PCM_CH2);
          updateScoreDisplay();
          if (coinsCollected == MAX_COINS) {
            unlockExit();
          }
        }
      }
    }

    SPR_update();
    SYS_enableInts();
    SYS_doVBlankProcess();
  }
  return (0);
}
