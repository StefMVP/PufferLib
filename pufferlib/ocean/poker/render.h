#ifndef POKER_RENDER_H
#define POKER_RENDER_H

#include "raylib.h"
#include "constants.h"

typedef struct Client {
    float width;
    float height;
    float card_width;
    float card_height;
    int font_size;
} Client;

typedef struct Poker Poker;

static Client* make_client(float width, float height);
void c_close_client(Client* client);
void c_render(Poker* env);

#endif