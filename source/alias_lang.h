/*
 * ---
 * BASIC++ Interpreter - alias_lang.h
 * ---
 *
 * Language pack API for keyword aliasing.
 * Provides pre-built alias sets for multiple natural
 * languages (Spanish, Portuguese, French, German,
 * Italian, Japanese romaji).
 *
 * ---
 */

#ifndef BASICPP_ALIAS_LANG_H
#define BASICPP_ALIAS_LANG_H

/*
 * alias_lang_load - Load a language pack by code.
 *
 * Valid codes: "ES", "PT", "FR", "DE", "IT", "JA"
 * Returns the number of aliases loaded, or -1 on error.
 */
int alias_lang_load(const char *code);

/*
 * alias_lang_clear - Remove all language-pack aliases.
 */
void alias_lang_clear(void);

/*
 * alias_lang_list - List available language packs.
 */
void alias_lang_list(void);

#endif /* BASICPP_ALIAS_LANG_H */
