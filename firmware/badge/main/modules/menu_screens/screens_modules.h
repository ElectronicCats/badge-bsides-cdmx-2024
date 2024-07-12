#pragma once

#include <stddef.h>

/**
 * @brief Scrolling text flag
 *
 * Used to identify if the text is scrollable
 *
 * Add this flag to the text to make it scrollable
 */
#define VERTICAL_SCROLL_TEXT "vertical_scroll_text"

/**
 * @brief Configuration menu items
 *
 * Used to identify the configuration menu items
 *
 * Add this flag to the menu item to make it a configuration item
 */
#define CONFIGURATION_MENU_ITEMS "configuration"

#define QUESTION_MENU_ITEMS "question"

/**
 * @brief Enum of menus
 *
 * Used to navigate through the different menus
 *
 * Modify this menu also requires to modify the `menu_list`, `next_menu_table`,
 * `prev_menu_table` and `menu_items` arrays
 */
typedef enum {
  MENU_MAIN = 0,
  MENU_APPLICATIONS,
  MENU_ABOUT,
  /* Applications */
  MENU_WIFI_APPS,
  MENU_BLUETOOTH_APPS,
  MENU_ZIGBEE_APPS,
  MENU_THREAD_APPS,
  MENU_GAMES,
  MENU_BADGE_FINDER,
  /* WiFi applications */
  MENU_WIFI_ANALIZER,
  MENU_WIFI_DEAUTH,
  MENU_WIFI_DOS,
  /* WiFi analizer items */
  MENU_WIFI_ANALIZER_RUN,
  MENU_WIFI_ANALIZER_SETTINGS,
  /* WiFi analizer start items */
  MENU_WIFI_ANALIZER_ASK_SUMMARY,
  MENU_WIFI_ANALIZER_SUMMARY,
  /* WiFi analizer settings */
  MENU_WIFI_ANALIZER_CHANNEL,
  MENU_WIFI_ANALIZER_DESTINATION,
  /* Bluetooth applications */
  MENU_BLUETOOTH_TRAKERS_SCAN,
  MENU_BLUETOOTH_SPAM,
  MENU_BLUETOOTH_CTF,
  /* Zigbee applications */
  MENU_ZIGBEE_SPOOFING,
  MENU_ZIGBEE_SWITCH,
  MENU_ZIGBEE_LIGHT,
  MENU_ZIGBEE_SNIFFER,
  /* Thread applications */
  MENU_THREAD_BROADCAST,
  /* Games module */
  MENU_GAMES_PLAY,
  MENU_GAMES_HELP,
  /* Games HELP */
  MENU_GAMES_MAIN_HELP,
  MENU_RAUL_GAME_HELP,
  MENU_ROPE_GAME_HELP,
  MENU_KEVIN_GAME_HELP,
  /* Badge finder */
  MENU_BADGE_FINDER_SCAN,
  MENU_BADGE_FINDER_HELP,
  /* About items */
  MENU_ABOUT_VERSION,
  MENU_ABOUT_LICENSE,
  MENU_ABOUT_CREDITS,
  MENU_ABOUT_LEGAL,
  /* About submenus */
  /* Menu count */
  MENU_COUNT,
} screen_module_menu_t;

/**
 * @brief List of menus
 *
 * Used to get the menu name from the enum value
 * following the order of the `screen_module_menu_t` enum
 *
 * Usage: menu_list[screen_module_menu_t]
 */
const char* menu_list[] = {
    "MENU_MAIN",
    "MENU_APPLICATIONS",
    "MENU_ABOUT",
    /* Applications */
    "MENU_WIFI_APPS",
    "MENU_BLUETOOTH_APPS",
    "MENU_ZIGBEE_APPS",
    "MENU_THREAD_APPS",
    "MENU_GAMES",
    "MENU_BADGE_FINDER",
    /* WiFi applications */
    "MENU_WIFI_ANALIZER",
    "MENU_WIFI_DEAUTH",
    "MENU_WIFI_DOS",
    /* WiFi analizer items */
    "MENU_WIFI_ANALIZER_RUN",
    "MENU_WIFI_ANALIZER_SETTINGS",
    /* WiFi analizer start items */
    "MENU_WIFI_ANALIZER_ASK_SUMMARY",
    "MENU_WIFI_ANALIZER_SUMMARY",
    /* WiFi analizer settings */
    "MENU_WIFI_ANALIZER_CHANNEL",
    "MENU_WIFI_ANALIZER_DESTINATION",
    /* Bluetooth applications */
    "MENU_BLUETOOTH_TRAKERS_SCAN",
    "MENU_BLUETOOTH_SPAM",
    "MENU_BLUETOOTH_CTF",
    /* Zigbee applications */
    "MENU_ZIGBEE_SPOOFING",
    "MENU_ZIGBEE_SWITCH",
    "MENU_ZIGBEE_LIGHT",
    "MENU_ZIGBEE_SNIFFER",
    /* Thread applications */
    "MENU_THREAD_BROADCAST",
    /* Games module */
    "MENU_GAMES_PLAY",
    "MENU_GAMES_HELP",
    /* Games HELP */
    "MENU_GAMES_MAIN_HELP",
    "MENU_RAUL_GAME_HELP",
    "MENU_ROPE_GAME_HELP",
    "MENU_KEVIN_GAME_HELP",
    /* Badge finder */
    "MENU_BADGE_FINDER_SCAN",
    "MENU_BADGE_FINDER_HELP",
    /* About items */
    "MENU_ABOUT_VERSION",
    "MENU_ABOUT_LICENSE",
    "MENU_ABOUT_CREDITS",
    "MENU_ABOUT_LEGAL",
};

/**
 * @brief List of menus
 *
 * Used to get the next menu to display when the user selects an option
 * following the order of the `screen_module_menu_t` enum
 *
 * Usage: next_menu_table[screen_module_menu_t][selected_item]
 */
const int next_menu_table[][8] = {
    // MENU_MAIN
    {MENU_APPLICATIONS, MENU_ABOUT},
    // MENU_APPLICATIONS
    {MENU_WIFI_APPS, MENU_BLUETOOTH_APPS, MENU_ZIGBEE_APPS, MENU_THREAD_APPS,
     MENU_GAMES, MENU_BADGE_FINDER},
    // MENU_ABOUT
    {MENU_ABOUT_VERSION, MENU_ABOUT_LICENSE, MENU_ABOUT_CREDITS,
     MENU_ABOUT_LEGAL},
    // MENU_WIFI_APPS
    {MENU_WIFI_ANALIZER, MENU_WIFI_DEAUTH, MENU_WIFI_DOS},
    // MENU_BLUETOOTH_APPS
    {MENU_BLUETOOTH_TRAKERS_SCAN, MENU_BLUETOOTH_SPAM, MENU_BLUETOOTH_CTF},
    // MENU_ZIGBEE_APPS
    {MENU_ZIGBEE_SPOOFING, MENU_ZIGBEE_SNIFFER},
    // MENU_THREAD_APPS
    {MENU_THREAD_BROADCAST},
    // MENU_GAMES
    {MENU_GAMES_PLAY, MENU_GAMES_HELP},
    // MENU_BADGE_FINDER
    {MENU_BADGE_FINDER_SCAN, MENU_BADGE_FINDER_HELP},
    // MENU_WIFI_ANALIZER
    {MENU_WIFI_ANALIZER_RUN, MENU_WIFI_ANALIZER_SETTINGS},
    // MENU_WIFI_DEAUTH
    {MENU_WIFI_DEAUTH},
    // MENU_WIFI_DOS
    {MENU_WIFI_DOS},
    // MENU_WIFI_ANALIZER_RUN
    {MENU_WIFI_ANALIZER_RUN},
    // MENU_WIFI_ANALIZER_SETTINGS
    {MENU_WIFI_ANALIZER_CHANNEL, MENU_WIFI_ANALIZER_DESTINATION},
    // MENU_WIFI_ANALIZER_ASK_SUMMARY [0] -> Yes, [1] -> No
    {MENU_WIFI_ANALIZER_SUMMARY, MENU_WIFI_ANALIZER},
    // MENU_WIFI_ANALIZER_SUMMARY
    {MENU_WIFI_ANALIZER_SUMMARY},
    // MENU_WIFI_ANALIZER_CHANNEL
    {MENU_WIFI_ANALIZER_CHANNEL},
    // MENU_WIFI_ANALIZER_DESTINATION
    {MENU_WIFI_ANALIZER_DESTINATION},
    // MENU_BLUETOOTH_TRAKERS_SCAN
    {MENU_BLUETOOTH_TRAKERS_SCAN},
    // MENU_BLUETOOTH_SPAM
    {MENU_BLUETOOTH_SPAM},
    // MENU_BLUETOOTH_CTF
    {MENU_BLUETOOTH_CTF},
    // MENU_ZIGBEE_SPOOFING
    {MENU_ZIGBEE_SWITCH, MENU_ZIGBEE_LIGHT},
    // MENU_ZIGBEE_SWITCH
    {MENU_ZIGBEE_SWITCH},
    // MENU_ZIGBEE_LIGHT
    {MENU_ZIGBEE_LIGHT},
    // MENU_ZIGBEE_SNIFFER
    {MENU_ZIGBEE_SNIFFER},
    // MENU_THREAD_BROADCAST
    {MENU_THREAD_BROADCAST},
    // MENU_GAMES_PLAY
    {MENU_GAMES_PLAY},
    // MENU_GAMES_HELP
    {MENU_GAMES_MAIN_HELP, MENU_RAUL_GAME_HELP, MENU_ROPE_GAME_HELP,
     MENU_KEVIN_GAME_HELP},
    // MENU_GAMES_MAIN_HELP
    {MENU_GAMES_MAIN_HELP},
    // MENU_RAUL_GAME_HELP
    {MENU_RAUL_GAME_HELP},
    // MENU_ROPE_GAME_HELP
    {MENU_ROPE_GAME_HELP},
    // MENU_KEVIN_GAME_HELP
    {MENU_KEVIN_GAME_HELP},
    // MENU_BADGE_FINDER_SCAN
    {MENU_BADGE_FINDER_SCAN},
    // MENU_BADGE_FINDER_HELP
    {MENU_BADGE_FINDER_HELP},
    // MENU_ABOUT_VERSION
    {MENU_ABOUT_VERSION},
    // MENU_ABOUT_LICENSE
    {MENU_ABOUT_LICENSE},
    // MENU_ABOUT_CREDITS
    {MENU_ABOUT_CREDITS},
    // MENU_ABOUT_LEGAL
    {MENU_ABOUT_LEGAL},
};

/**
 * @brief List of menus
 *
 * Used to get the previous menu to display when the user returns to the
 * previous menu in `menu_screens_exit_submenu`. Add the previous menu
 * following the order of the `screen_module_menu_t` enum
 *
 * Usage: prev_menu_table[screen_module_menu_t]
 */
const int prev_menu_table[] = {
    MENU_MAIN,                       // MENU_MAIN
    MENU_MAIN,                       // MENU_APPLICATIONS
    MENU_MAIN,                       // MENU_ABOUT
    MENU_APPLICATIONS,               // MENU_WIFI_APPS
    MENU_APPLICATIONS,               // MENU_BLUETOOTH_APPS
    MENU_APPLICATIONS,               // MENU_ZIGBEE_APPS
    MENU_APPLICATIONS,               // MENU_THREAD_APPS
    MENU_APPLICATIONS,               // MENU_GAMES
    MENU_APPLICATIONS,               // MENU_BADGE_FINDER
    MENU_WIFI_APPS,                  // MENU_WIFI_ANALIZER
    MENU_WIFI_APPS,                  // MENU_WIFI_DEAUTH
    MENU_WIFI_APPS,                  // MENU_WIFI_DOS
    MENU_WIFI_ANALIZER_ASK_SUMMARY,  // MENU_WIFI_ANALIZER_RUN
    MENU_WIFI_ANALIZER,              // MENU_WIFI_ANALIZER_SETTINGS
    MENU_WIFI_ANALIZER_RUN,          // MENU_WIFI_ANALIZER_ASK_SUMMARY
    MENU_WIFI_ANALIZER,              // MENU_WIFI_ANALIZER_SUMMARY
    MENU_WIFI_ANALIZER_SETTINGS,     // MENU_WIFI_ANALIZER_CHANNEL
    MENU_WIFI_ANALIZER_SETTINGS,     // MENU_WIFI_ANALIZER_DESTINATION
    MENU_BLUETOOTH_APPS,             // MENU_BLUETOOTH_TRAKERS_SCAN
    MENU_BLUETOOTH_APPS,             // MENU_BLUETOOTH_SPAM
    MENU_BLUETOOTH_APPS,             // MENU_BLUETOOTH_CTF
    MENU_ZIGBEE_APPS,                // MENU_ZIGBEE_SPOOFING
    MENU_ZIGBEE_SPOOFING,            // MENU_ZIGBEE_SWITCH
    MENU_ZIGBEE_SPOOFING,            // MENU_ZIGBEE_LIGHT
    MENU_ZIGBEE_APPS,                // MENU_ZIGBEE_SNIFFER
    MENU_THREAD_APPS,                // MENU_THREAD_BROADCAST
    MENU_GAMES,                      // MENU_GAMES_PLAY
    MENU_GAMES,                      // MENU_GAMES_HELP
    MENU_GAMES_HELP,                 // MENU_GAMES_MAIN_HELP
    MENU_GAMES_HELP,                 // MENU_RAUL_GAME_HELP
    MENU_GAMES_HELP,                 // MENU_ROPE_GAME_HELP
    MENU_GAMES_HELP,                 // MENU_KEVIN_GAMES_HELP
    MENU_BADGE_FINDER,               // MENU_BADGE_FINDER_SCAN
    MENU_BADGE_FINDER,               // MENU_BADGE_FINDER_HELP
    MENU_ABOUT,                      // MENU_ABOUT_VERSION
    MENU_ABOUT,                      // MENU_ABOUT_LICENSE
    MENU_ABOUT,                      // MENU_ABOUT_CREDITS
    MENU_ABOUT,                      // MENU_ABOUT_LEGAL
};

/**
 * @brief History of selected items in each menu
 *
 * Used to keep track of the selected item in each menu
 *
 * Usage: selected_item_history[screen_module_menu_t]
 */
int selected_item_history[MENU_COUNT] = {0};

char* main_items[] = {
    "Applications",
    "About",
    NULL,
};

char* applications_items[] = {
    "WiFi", "Bluetooth", "Zigbee", "Thread", "Juegos", "Encontrar", NULL,
};

char* about_items[] = {
    "Version", "License", "Credits", "Legal", NULL,
};

char* version_text[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "",
    "",
    "",
    " BSIDES v1.0.0",
    "     BETA",
    NULL,
};

char* license_text[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "",
    "",
    "",
    "  GNU GPL 3.0",
    NULL,
};

char* credits_text[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "Insignia de",
    "BSides",
    "",
    "Agradecimientos",
    "a todos los",
    "colaboradores",
    "y la comunidad",
    "",
    "Esta insignia",
    "es patrocinada",
    "por",
    "- HSBC",
    "- Electronic",
    "  Cats",
    "",
    "Hardware",
    "desarrollado por",
    "- Edgar",
    "  Capuchino",
    "- Lizeth",
    "  Gallegos",
    "- Andres",
    "  Sabas",
    "@Sabasacustico",
    "",
    "Firmware",
    "desarrollado por",
    "- Francisco",
    "  @deimoshall",
    "- Kevin Leon",
    "  @kevlem97",
    "- Raul Vargas",
    "  @RegioDelta",
    "- Roberto",
    "  Arellano",
    NULL,
};

char* legal_text[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "The user",
    "assumes all",
    "responsibility",
    "for the use of",
    "this badge and",
    "agrees to use",
    "it legally and",
    "ethically,",
    "avoiding any",
    "activities that",
    "may cause harm,",
    "interference,",
    "or unauthorized",
    "access to",
    "systems or data.",
    NULL,
};

char* wifi_items[] = {
    "Analizer",
    "Deauth",
    NULL,
    NULL,
};

const char* wifi_analizer_items[] = {
    "Start",
    // "Settings",
    NULL,
};

char* wifi_analizer_summary[120] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "Summary",
    NULL,
};

char* wifi_analizer_settings_items[] = {
    "Channel",
    "Destination",
    NULL,
};

char* wifi_analizer_summary_question[] = {
    QUESTION_MENU_ITEMS, "Yes", "No", "Show summary?", NULL,
};

char* wifi_analizer_channel_items[] = {
    CONFIGURATION_MENU_ITEMS,
    "[ ] 1",
    "[ ] 2",
    "[ ] 3",
    "[ ] 4",
    "[ ] 5",
    "[ ] 6",
    "[ ] 7",
    "[ ] 8",
    "[ ] 9",
    "[ ] 10",
    "[ ] 11",
    "[ ] 12",
    "[ ] 13",
    "[ ] 14",
    NULL,
};

char* wifi_analizer_destination_items[] = {
    CONFIGURATION_MENU_ITEMS,
    "[ ] SD Card",
    "[ ] Internal",
    NULL,
};

char* bluetooth_items[] = {
    "Trakers scan",
    "Spam",
    "CTF",
    NULL,
};

char* zigbee_items[] = {
    "Spoofing",
    "Sniffer",
    NULL,
};

char* zigbee_spoofing_items[] = {
    "Switch",
    NULL,
};

char* thread_items[] = {
    NULL,
};

char* games_items[] = {
    "Sala",
    "Ayuda",
    NULL,
};
char* games_help_items[] = {
    "Sala?", "Vencidas?", "Cuerda?", "Peras?", NULL,
};

char* games_main_help[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "Abre el menu",
    "SALA, luego",
    "conecta badges",
    "de bsides como",
    "se indica para",
    "poder jugar.",
    "El juego",
    "dependera del",
    "numero de",
    "badges.",
    "",
    "El modo host",
    "muestra a",
    "los jugadores",
    "conectados",
    "y el juego",
    "seleccionado.",
    "Presiona el ",
    "boton (DERECHA)",
    "para comenzar",
    "el juego.",
    "",
    "El modo cliente",
    "muestra el ID",
    "de tu jugador",
    "y el juego",
    "seleccionado.",
    "Solo el host",
    "puede comenzar",
    "la partida.",
    "",
    "Una vez que",
    "comienza el",
    "juego, ya pueden",
    "separar los",
    "badges y jugar.",
    NULL,
};

char* raul_game_help[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "* HOW TO PLAY *",
    "--RAUL GAME---",
    "(Need 2 badges)",
    "...............",
    "...............",
    "...............",
    NULL,
};

char* rope_game_help[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "** COMO JUGAR **",
    "---- CUERDA ----",
    " (4 Jugadores) ",
    "",
    "Jugadores 1 & 2",
    "son EQUIPO 1, de",
    "color amarillo.",
    "Jugadores 3 & 4 ",
    "son EQUIPO 2, de",
    "color azul.",
    "",
    "Presiona boton",
    "(Derecha) una",
    "vez, luego",
    "(Arriba) una",
    "vez.",
    "Repite tan",
    "rapido como",
    "puedas para",
    "ser mas fuerte",
    "que tus",
    "oponentes.",
    "",
    "Si presionas",
    "doble o lento,",
    "tu fuerza",
    "disminuira.",
    NULL,
};

char* kevin_game_help[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "* Como jugar *",
    "--Juego Peras--",
    " (5 jugadores) ",
    "",
    "Presiona Derecha.",
    "Y luego Arriba.",
    "Repite lo mas",
    "rapido que",
    "puedas para",
    "golpear mas",
    "veces que tus "
    "oponentes.",
    NULL,
};
char* badge_link_item[] = {
    "Escanear",
    "Ayuda",
    NULL,
};
char* badge_link_help[] = {
    VERTICAL_SCROLL_TEXT,
    /***************/
    "Has ido a",
    "BSides?",
    "DragonJar?",
    "Ekoparty?",
    "BugCon?",
    "...",
    "",
    "Estamos unidos",
    "como LATAM",
    "Tus insignias",
    "tienen secretos",
    "buena suerte...",
    NULL,
};

char* empty_items[] = {
    NULL,
};

/**
 * @brief List of menu items
 *
 * Used to get the menu items from the menu enum value
 * following the order of the `screen_module_menu_t` enum
 *
 * Usage: menu_items[screen_module_menu_t]
 */
char** menu_items[] = {
    main_items,
    applications_items,
    about_items,
    /* Applications */
    wifi_items,
    bluetooth_items,
    zigbee_items,
    thread_items,
    games_items,      // GAMES
    badge_link_item,  // Badge Finder
    /* WiFi applications */
    wifi_analizer_items,              // WiFi Analizer
    empty_items,                      // WiFi Deauth
    empty_items,                      // WiFi DoS
    empty_items,                      // WiFi Analizer Start
    wifi_analizer_settings_items,     // WiFi Analizer Settings
    wifi_analizer_summary_question,   // MENU_WIFI_ANALIZER_ASK_SUMMARY
    wifi_analizer_summary,            // WiFi Analizer Summary
    wifi_analizer_channel_items,      // WiFi Analizer Channel
    wifi_analizer_destination_items,  // WiFi Analizer Destination
    /* Bluetooth applications */
    empty_items,  // Bluetooth Trakers scan
    empty_items,  // Bluetooth Spam
    empty_items,  // Bluetooth CTF
    /* Zigbee applications */
    zigbee_spoofing_items,
    empty_items,  // Zigbee Switch
    empty_items,  // Zigbee Light
    empty_items,  // Zigbee Sniffer
    /* Thread applications */
    empty_items,  // Thread CLI
    /* Games Module */
    empty_items,       // MENU_GAMES_PLAY
    games_help_items,  // MENU_GAMES_HELP
    /* Games HELP */
    games_main_help,  // MENU_GAMES_MAIN_HELP
    raul_game_help,   // MENU_RAUL_GAME_HELP
    rope_game_help,   // MENU_ROPE_GAME_HELP
    kevin_game_help,  // MENU_KEVIN_GAME_HELP
    /* Badge finder */
    empty_items,      // MENU_BADGE_FINDER_SCAN
    badge_link_help,  // MENU_BADGE_FINDER_HELP
    /* About */
    version_text,
    license_text,
    credits_text,
    legal_text,
};
