#pragma once

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 224

#define SPRITE_SCALE 1	// Set to 2 for 2x sprites, 4 for 4x, etc. Must be a power of 2

#define TILES_TO_PIXELS(x) ((int)((x) * 0x10))
#define PIXELS_TO_TILES(x) ((int)((x) / 0x10))
#define PIXELS_TO_UNITS(x) ((int)((x) * 0x200))
#define UNITS_TO_PIXELS(x) ((int)((x) / 0x200))
#define TILES_TO_UNITS(x) ((int)((x) * (0x200 * 0x10)))
#define UNITS_TO_TILES(x) ((int)((x) / (0x200 * 0x10)))

enum Collisions
{
	COLL_LEFT_WALL = 1,     // Touching a left wall
	COLL_CEILING = 2,       // Touching a ceiling
	COLL_RIGHT_WALL = 4,    // Touching a right wall
	COLL_GROUND = 8         // Touching the ground
	// To be continued
};

enum Direction
{
	DIR_LEFT = 0,
	DIR_UP = 1,
	DIR_RIGHT = 2,
	DIR_DOWN = 3,
	DIR_AUTO = 4
};
