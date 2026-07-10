#include "help.h"

const HelpEntry help_db_sound[] = {
  { "BEEP", "Emit an audible beep", "BEEP", HCAT_SOUND },
  { "SOUND", "Play a tone at frequency", "SOUND 440, 18", HCAT_SOUND },
  { NULL, NULL, NULL, 0 }
};
