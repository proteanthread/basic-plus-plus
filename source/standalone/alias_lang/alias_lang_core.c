/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: alias_lang_core.c
 * Subsystem: Natural Language Translation Alias Pack
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Provides translation alias pack registers and mappings for remapping keywords.
 *
 * 2. WHAT TO EXPECT:
 *    Maps localized aliases to core KeywordIds in constant time.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Aliases, language mappings, dictionary keys.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Core keyword index IDs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If localized keywords fail to parse, verify dialect compatibility flags.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE ALIAS LANGUAGE CORE
 * File: alias_lang_core.c
 * ===================================================================== */

#include <string.h>
#include "alias_lang_core.h"

// --- Spanish (ES) ---
static const AliasLangPackEntry lang_es[] = {
    { KW_PRINT,    "IMPRIMIR" },
    { KW_INPUT,    "ENTRADA" },
    { KW_IF,       "SI" },
    { KW_THEN,     "ENTONCES" },
    { KW_ELSE,     "SINO" },
    { KW_FOR,      "PARA" },
    { KW_NEXT,     "SIGUIENTE" },
    { KW_TO,       "HASTA" },
    { KW_STEP,     "PASO" },
    { KW_GOTO,     "IRAA" },
    { KW_GOSUB,    "LLAMAR" },
    { KW_RETURN,   "VOLVER" },
    { KW_END,      "FIN" },
    { KW_WHILE,    "MIENTRAS" },
    { KW_DO,       "HACER" },
    { KW_LET,      "PONER" },
    { KW_DIM,      "DIMENSION" },
    { KW_REM,      "NOTA" },
    { KW_DATA,     "DATOS" },
    { KW_READ,     "LEER" },
    { KW_CLS,      "LIMPIAR" },
    { KW_LOCATE,   "POSICION" },
    { KW_COLOR,    "COLOR" },
    { KW_BEEP,     "SONIDO" },
    { KW_SLEEP,    "ESPERAR" },
    { KW_RUN,      "CORRER" },
    { KW_LIST,     "LISTAR" },
    { KW_SAVE,     "GUARDAR" },
    { KW_LOAD,     "CARGAR" },
    { KW_NEW,      "NUEVO" },
    { KW_STOP,     "PARAR" },
    { KW_CONT,     "CONTINUAR" },
    { KW_AND,      "Y" },
    { KW_OR,       "O" },
    { KW_NOT,      "NO" },
    { KW_COUNT, NULL }
};

// --- Portuguese (PT) ---
static const AliasLangPackEntry lang_pt[] = {
    { KW_PRINT,    "IMPRIMIR" },
    { KW_INPUT,    "ENTRADA" },
    { KW_IF,       "SE" },
    { KW_THEN,     "ENTAO" },
    { KW_ELSE,     "SENAO" },
    { KW_FOR,      "PARA" },
    { KW_NEXT,     "PROXIMO" },
    { KW_TO,       "ATE" },
    { KW_STEP,     "PASSO" },
    { KW_GOTO,     "VAPARA" },
    { KW_GOSUB,    "CHAMAR" },
    { KW_RETURN,   "VOLTAR" },
    { KW_END,      "FIM" },
    { KW_WHILE,    "ENQUANTO" },
    { KW_DO,       "FAZER" },
    { KW_LET,      "DEFINIR" },
    { KW_DIM,      "DIMENSAO" },
    { KW_REM,      "NOTA" },
    { KW_DATA,     "DADOS" },
    { KW_READ,     "LER" },
    { KW_CLS,      "LIMPAR" },
    { KW_LOCATE,   "POSICAO" },
    { KW_COLOR,    "COR" },
    { KW_BEEP,     "APITO" },
    { KW_SLEEP,    "ESPERAR" },
    { KW_RUN,      "EXECUTAR" },
    { KW_LIST,     "LISTAR" },
    { KW_SAVE,     "SALVAR" },
    { KW_LOAD,     "CARREGAR" },
    { KW_NEW,      "NOVO" },
    { KW_STOP,     "PARAR" },
    { KW_CONT,     "CONTINUAR" },
    { KW_AND,      "E" },
    { KW_OR,       "OU" },
    { KW_NOT,      "NAO" },
    { KW_COUNT, NULL }
};

// --- French (FR) ---
static const AliasLangPackEntry lang_fr[] = {
    { KW_PRINT,    "AFFICHER" },
    { KW_INPUT,    "SAISIR" },
    { KW_IF,       "SI" },
    { KW_THEN,     "ALORS" },
    { KW_ELSE,     "SINON" },
    { KW_FOR,      "POUR" },
    { KW_NEXT,     "SUIVANT" },
    { KW_TO,       "JUSQUA" },
    { KW_STEP,     "PAS" },
    { KW_GOTO,     "ALLERA" },
    { KW_GOSUB,    "APPELER" },
    { KW_RETURN,   "RETOUR" },
    { KW_END,      "FIN" },
    { KW_WHILE,    "TANTQUE" },
    { KW_DO,       "FAIRE" },
    { KW_LET,      "METTRE" },
    { KW_DIM,      "DIMENSION" },
    { KW_REM,      "REMARQUE" },
    { KW_DATA,     "DONNEES" },
    { KW_READ,     "LIRE" },
    { KW_CLS,      "EFFACER" },
    { KW_LOCATE,   "PLACER" },
    { KW_COLOR,    "COULEUR" },
    { KW_BEEP,     "BIP" },
    { KW_SLEEP,    "ATTENDRE" },
    { KW_RUN,      "EXECUTER" },
    { KW_LIST,     "LISTER" },
    { KW_SAVE,     "SAUVER" },
    { KW_LOAD,     "CHARGER" },
    { KW_NEW,      "NOUVEAU" },
    { KW_STOP,     "ARRETER" },
    { KW_CONT,     "CONTINUER" },
    { KW_AND,      "ET" },
    { KW_OR,       "OU" },
    { KW_NOT,      "NON" },
    { KW_COUNT, NULL }
};

// --- German (DE) ---
static const AliasLangPackEntry lang_de[] = {
    { KW_PRINT,    "DRUCKE" },
    { KW_INPUT,    "EINGABE" },
    { KW_IF,       "WENN" },
    { KW_THEN,     "DANN" },
    { KW_ELSE,     "SONST" },
    { KW_FOR,      "FUER" },
    { KW_NEXT,     "NAECHST" },
    { KW_TO,       "BIS" },
    { KW_STEP,     "SCHRITT" },
    { KW_GOTO,     "GEHEZU" },
    { KW_GOSUB,    "RUFAUF" },
    { KW_RETURN,   "ZURUECK" },
    { KW_END,      "ENDE" },
    { KW_WHILE,    "SOLANGE" },
    { KW_DO,       "MACHE" },
    { KW_LET,      "SETZE" },
    { KW_DIM,      "FELD" },
    { KW_REM,      "BEMERKUNG" },
    { KW_DATA,     "DATEN" },
    { KW_READ,     "LESEN" },
    { KW_CLS,      "LOESCHE" },
    { KW_LOCATE,   "PLATZ" },
    { KW_COLOR,    "FARBE" },
    { KW_BEEP,     "PIEP" },
    { KW_SLEEP,    "WARTEN" },
    { KW_RUN,      "STARTE" },
    { KW_LIST,     "AUFLISTEN" },
    { KW_SAVE,     "SPEICHERN" },
    { KW_LOAD,     "LADEN" },
    { KW_NEW,      "NEU" },
    { KW_STOP,     "STOPP" },
    { KW_CONT,     "WEITER" },
    { KW_AND,      "UND" },
    { KW_OR,       "ODER" },
    { KW_NOT,      "NICHT" },
    { KW_COUNT, NULL }
};

// --- Italian (IT) ---
static const AliasLangPackEntry lang_it[] = {
    { KW_PRINT,    "STAMPA" },
    { KW_INPUT,    "INGRESSO" },
    { KW_IF,       "SE" },
    { KW_THEN,     "ALLORA" },
    { KW_ELSE,     "ALTRIMENTI" },
    { KW_FOR,      "PER" },
    { KW_NEXT,     "PROSSIMO" },
    { KW_TO,       "FINO" },
    { KW_STEP,     "PASSO" },
    { KW_GOTO,     "VAIA" },
    { KW_GOSUB,    "CHIAMA" },
    { KW_RETURN,   "RITORNA" },
    { KW_END,      "FINE" },
    { KW_WHILE,    "MENTRE" },
    { KW_DO,       "FARE" },
    { KW_LET,      "ASSEGNA" },
    { KW_DIM,      "DIMENSIONE" },
    { KW_REM,      "NOTA" },
    { KW_DATA,     "DATI" },
    { KW_READ,     "LEGGERE" },
    { KW_CLS,      "PULISCI" },
    { KW_LOCATE,   "POSIZIONA" },
    { KW_COLOR,    "COLORE" },
    { KW_BEEP,     "SUONO" },
    { KW_SLEEP,    "ATTENDI" },
    { KW_RUN,      "ESEGUI" },
    { KW_LIST,     "ELENCA" },
    { KW_SAVE,     "SALVA" },
    { KW_LOAD,     "CARICA" },
    { KW_NEW,      "NUOVO" },
    { KW_STOP,     "FERMA" },
    { KW_CONT,     "CONTINUA" },
    { KW_AND,      "E" },
    { KW_OR,       "O" },
    { KW_NOT,      "NON" },
    { KW_COUNT, NULL }
};

// --- Japanese Romaji (JA) ---
static const AliasLangPackEntry lang_ja[] = {
    { KW_PRINT,    "HYOUJI" },
    { KW_INPUT,    "NYUURYOKU" },
    { KW_IF,       "MOSHI" },
    { KW_THEN,     "NARABA" },
    { KW_ELSE,     "SORENARA" },
    { KW_FOR,      "KURIKAESHI" },
    { KW_NEXT,     "TSUGI" },
    { KW_TO,       "MADE" },
    { KW_STEP,     "KIZAMI" },
    { KW_GOTO,     "IDOU" },
    { KW_GOSUB,    "YOBIDASHI" },
    { KW_RETURN,   "MODORU" },
    { KW_END,      "OWARI" },
    { KW_WHILE,    "AIDA" },
    { KW_DO,       "SURU" },
    { KW_LET,      "DAINYU" },
    { KW_DIM,      "HAIRETSU" },
    { KW_REM,      "CHUUSHAKU" },
    { KW_DATA,     "DEETA" },
    { KW_READ,     "YOMU" },
    { KW_CLS,      "KURIA" },
    { KW_LOCATE,   "ICHI" },
    { KW_COLOR,    "IRO" },
    { KW_BEEP,     "ONSEI" },
    { KW_SLEEP,    "MATSU" },
    { KW_RUN,      "JIKKOU" },
    { KW_LIST,     "ICHIRAN" },
    { KW_SAVE,     "HOZON" },
    { KW_LOAD,     "YOMIKOMI" },
    { KW_NEW,      "ATARASHII" },
    { KW_STOP,     "TEISHI" },
    { KW_CONT,     "TSUZUKI" },
    { KW_AND,      "KATSU" },
    { KW_OR,       "MATAWA" },
    { KW_NOT,      "HITEI" },
    { KW_COUNT, NULL }
};

// --- Pack Table ---
static const AliasLangPack lang_packs[] = {
    { "ES", "Spanish (Espanol)",     lang_es },
    { "PT", "Portuguese (Portugues)", lang_pt },
    { "FR", "French (Francais)",     lang_fr },
    { "DE", "German (Deutsch)",      lang_de },
    { "IT", "Italian (Italiano)",    lang_it },
    { "JA", "Japanese (Romaji)",     lang_ja },
    { NULL, NULL, NULL }
};

static int str_eq_ci(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
        if (ca != cb) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

const AliasLangPack *alias_lang_core_find(const char *code)
{
    int i;
    if (!code) return NULL;
    for (i = 0; lang_packs[i].code != NULL; i++) {
        if (str_eq_ci(code, lang_packs[i].code)) {
            return &lang_packs[i];
        }
    }
    return NULL;
}

const AliasLangPack *alias_lang_core_get_all(int *out_count)
{
    int count = 0;
    while (lang_packs[count].code != NULL) {
        count++;
    }
    if (out_count) {
        *out_count = count;
    }
    return lang_packs;
}
