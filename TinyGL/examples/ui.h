// FILENAME: ui.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (gears.c, mech.c, nanox.c, spin.c, texobj.c, x11.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for ui within BASIC++.
//
// ---- Includes ----

void draw( void );
void idle( void );
GLenum key(int k, GLenum mask);
void reshape( int width, int height );
void init( void );
int ui_loop(int argc, char **argv, const char *name);
void tkSwapBuffers(void);

#define KEY_UP     0xe000
#define KEY_DOWN   0xe001
#define KEY_LEFT   0xe002
#define KEY_RIGHT  0xe003
#define KEY_ESCAPE 0xe004

