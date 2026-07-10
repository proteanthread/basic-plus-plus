/* BASIC++ Help Database - Program Management & Editing */
#include "help.h"

const HelpEntry help_db_progmgmt[] = {
    { "RUN", "Execute the stored program", "RUN [line]", HCAT_PROGMGMT },
    { "LIST", "List stored program lines", "LIST [start]-[end]", HCAT_PROGMGMT },
    { "LLIST", "Print stored program lines to LPT", "LLIST [start]-[end]", HCAT_PROGMGMT },
    { "SAVE", "Save program to disk", "SAVE \"file.bas\"", HCAT_PROGMGMT },
    { "LOAD", "Load program from disk", "LOAD \"file.bas\"", HCAT_PROGMGMT },
    { "NEW", "Clear program and variables", "NEW", HCAT_PROGMGMT },
    { "MERGE", "Merge program lines from disk", "MERGE \"file.bas\"", HCAT_PROGMGMT },
    { "CHAIN", "Load and execute program, passing vars", "CHAIN \"file.bas\"", HCAT_PROGMGMT },
    { "BSAVE", "Save binary memory image", "BSAVE \"file.bin\", start, length", HCAT_PROGMGMT },
    { "BLOAD", "Load binary memory image", "BLOAD \"file.bin\" [, offset]", HCAT_PROGMGMT },
    { "BRUN", "Load and execute binary program", "BRUN \"file.bin\"", HCAT_PROGMGMT },
    { "CSAVE", "Save program to cassette", "CSAVE \"PROG\"", HCAT_PROGMGMT },
    { "CLOAD", "Load program from cassette", "CLOAD \"PROG\"", HCAT_PROGMGMT },
    { "CRUN", "Load and run from cassette", "CRUN", HCAT_PROGMGMT },
    { "UNLOAD", "Unload module or program", "UNLOAD", HCAT_PROGMGMT },
    { "UNSAVE", "Delete saved program (alias for KILL)", "UNSAVE \"file.bas\"", HCAT_PROGMGMT },
    { "AUTO", "Automatic line numbering", "AUTO [start] [, step]", HCAT_EDIT },
    { "DELETE", "Delete program lines", "DELETE [start]-[end]", HCAT_EDIT },
    { "RENUM", "Renumber program lines", "RENUM [newstart] [, [oldstart] [, step]]", HCAT_EDIT },
    { "EDIT", "Edit a program line", "EDIT line", HCAT_EDIT },
    { NULL, NULL, NULL, 0 }
};
