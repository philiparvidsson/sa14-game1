#ifndef game_h_
#define game_h_

/*------------------------------------------------
 * INCLUDES
 *----------------------------------------------*/

#include "base/common.h"

typedef struct gameT gameT;

#include "engine/component.h"
#include "engine/entity.h"
#include "engine/subsystem.h"

#include "input/keyboard.h"
#include "input/mouse.h"

struct gameT {
    arrayT* entities;
    arrayT* subsystems;
};


/*------------------------------------------------
 * FUNCTIONS
 *----------------------------------------------*/

void initGame(const string* title, int screen_width, int screen_height);
void exitGame(void);

void gameMain(void);

void addSubsystemToGame(gameSubsystemT* subsystem);
//void addEntityToGame(gameEntityT* entity);

#endif // game_h_
