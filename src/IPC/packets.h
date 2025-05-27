/*
** EPITECH PROJECT, 2025
** jetpack
** File description:
** packets.h
*/

#ifndef PACKETS_H
    #define PACKETS_H

    #include "network/map.h"
    #ifdef __cplusplus
        #include <cstdint>
        #include <cstddef>
    #else
        #include <stdint.h>
        #include <stddef.h>
    #endif

    #define PACKET_BUFFER_SIZE 1024

typedef float float32_t;
typedef double float64_t;

typedef enum packet_type_e {
    INVALID,
    ACKNOWLEDGE,
    HELLO,
    PLAYER_UPDATE,
    MAP_DESC,
    INPUT,
    PLAYER_STATS,
    GAME_ENDED,
    GAME_STARTED
} packet_type_t;

typedef enum player_input_e {
    UP,
    NONE
} player_input_t;

typedef struct packet_generic_s {
    packet_type_t type;
} packet_generic_t;

typedef struct packet_player_update_s {
    packet_type_t type;
    float32_t x;
    float32_t y;
    int on_the_floor;
} packet_player_update_t;

typedef struct packet_map_desc_s {
    packet_type_t type;
    char map[MAP_ROWS][MAP_COLS];
} packet_map_desc_t;

typedef struct packet_input_s {
    packet_type_t type;
    player_input_t input;
} packet_input_t;

typedef struct packet_player_stats_s {
    packet_type_t type;
    int dead;
    int score;
} packet_player_stats_t;

typedef struct packet_game_ended_s {
    packet_type_t type;
    int winner_id;
    int client_won;
} packet_game_ended_t;

static const size_t PACKET_SIZES[] = {
    0,
    sizeof(packet_generic_t) - sizeof(packet_type_t),
    sizeof(packet_generic_t) - sizeof(packet_type_t),
    sizeof(packet_player_update_t) - sizeof(packet_type_t),
    sizeof(packet_map_desc_t) - sizeof(packet_type_t),
    sizeof(packet_input_t) - sizeof(packet_type_t),
    sizeof(packet_player_stats_t) - sizeof(packet_type_t),
    sizeof(packet_game_ended_t) - sizeof(packet_type_t),
    sizeof(packet_generic_t) - sizeof(packet_type_t)
};

#endif //PACKETS_H
