/*! \file empty.c
 *  \brief Empty gamestate.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "../common.h"
#include <math.h>
#include <libsuperderpy.h>

struct Person {
		int r, g, b;
		float x, y;
		bool active;
};

struct Pusher {
		int r, g, b;
		bool active;
};



struct GamestateResources {
		// This struct is for every resource allocated and used by your gamestate.
		// It gets created on load and then gets passed around to all other function calls.
		ALLEGRO_FONT *font;
		int blink_counter;

		ALLEGRO_BITMAP *bg, *clouds, *fg, *pendo, *person, *pusher;
		struct Character *huginn;

		int cloudspos;
		float pos, speed;
		int speedmod;

		bool up, down, left, right;

		int numpushers;
		int numpersons;
		struct Person persons[1024];
		struct Pusher pushers[1024];

		char *text;

};

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game *game, struct GamestateResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	data->blink_counter++;
	if (data->blink_counter >= 60) {
		data->blink_counter = 0;
	}

	data->cloudspos--;
	if (data->cloudspos == -320) data->cloudspos = 0;

	data->pos-=data->speed;
	if (data->pos <= -320) data->pos += 320;

	data->speed += 0.0002*data->speedmod;

	if (data->up) {
		MoveCharacter(game, data->huginn, 0, -1, 0);
	}
	if (data->down) {
		MoveCharacter(game, data->huginn, 0, 1, 0);
	}
	if (data->left) {
		MoveCharacter(game, data->huginn, -1, 0, 0);
	}
	if (data->right) {
		MoveCharacter(game, data->huginn, 1, 0, 0);
	}

	if (GetCharacterY(game, data->huginn) <= 50) {
		MoveCharacter(game, data->huginn, 0, 1, 0);
	}
	if (GetCharacterY(game, data->huginn) > 125) {
		MoveCharacter(game, data->huginn, 0, -1, 0);
	}

	if (GetCharacterX(game, data->huginn) < 0) {
		MoveCharacter(game, data->huginn, 1, 0, 0);
	}
	if (GetCharacterX(game, data->huginn) > 300) {
		MoveCharacter(game, data->huginn, -1, 0, 0);
	}

	if (rand()%(int)(fmax(0,(250/(data->speed+1)))+1)==0) {
		PrintConsole(game, "yay");

		data->persons[data->numpersons].active = true;
		data->persons[data->numpersons].r = rand()%255;
		data->persons[data->numpersons].g = rand()%255;
		data->persons[data->numpersons].b = rand()%255;
		data->persons[data->numpersons].x = 320;
		data->persons[data->numpersons].y = rand()%70 + 60;
		data->numpersons++;
	}

	if (data->numpersons >= 1024) {
		data->numpersons = 0;
	}
	if (data->numpushers >= 1024) {
		data->numpushers = 0;
	}

	for (int i=0; i<data->numpersons; i++) {
		if (data->persons[i].active) {
			data->persons[i].x -= 0.5 + 0.5 * data->speed;

			if (IsOnCharacter(game, data->huginn, data->persons[i].x, data->persons[i].y + 25)) {
				data->persons[i].active = false;
				data->speedmod++;
				data->pushers[data->numpushers].active = true;
				data->pushers[data->numpushers].r = data->persons[i].r;
				data->pushers[data->numpushers].g = data->persons[i].g;
				data->pushers[data->numpushers].b = data->persons[i].b;

				data->numpushers++;
			}
		}
	}
}

void Gamestate_Draw(struct Game *game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.
	al_draw_bitmap(data->bg, 0, 0, 0);
	al_draw_bitmap(data->clouds, data->cloudspos, 0, 0);
	al_draw_bitmap(data->clouds, data->cloudspos + 320, 0, 0);
	al_draw_bitmap(data->pendo, 0, 0, 0);
	for (int i=0; i<data->numpushers; i++) {
		if (data->pushers[i].active) {
			al_draw_tinted_bitmap(data->pusher, al_map_rgb(data->pushers[i].r, data->pushers[i].g, data->pushers[i].b), 27-i , 44, 0);
		}
	}

	al_draw_bitmap(data->fg, (int)data->pos, 0, 0);
	al_draw_bitmap(data->fg, (int)data->pos + 320, 0, 0);
	DrawCharacter(game, data->huginn, al_map_rgb(255, 255, 255), 0);

	for (int i=0; i<data->numpersons; i++) {
		if (data->persons[i].active) {
			al_draw_tinted_bitmap(data->person, al_map_rgb(0, 0, 0), (int)data->persons[i].x+1 , (int)data->persons[i].y+1, 0);
			al_draw_tinted_bitmap(data->person, al_map_rgb(data->persons[i].r, data->persons[i].g, data->persons[i].b), (int)data->persons[i].x , (int)data->persons[i].y, 0);
		}
	}


}

void Gamestate_ProcessEvent(struct Game *game, struct GamestateResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}

	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_DOWN)) {
		data->down = true;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		data->left = true;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		data->right = true;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_UP)) {
		data->up = true;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_DOWN)) {
		data->down = false;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		data->left = false;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		data->right = false;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_UP)) {
		data->up = false;
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources *data = malloc(sizeof(struct GamestateResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar

//	ALLEGRO_BITMAP *bg, *clouds, *fg, *huginn, *pendo, *person, *pusher;
	data->bg = al_load_bitmap(GetDataFilePath(game, "bg.png"));
	data->clouds = al_load_bitmap(GetDataFilePath(game, "clouds.png"));
	data->fg = al_load_bitmap(GetDataFilePath(game, "fg.png"));
	data->pendo = al_load_bitmap(GetDataFilePath(game, "pendo.png"));
	data->person = al_load_bitmap(GetDataFilePath(game, "person.png"));
	data->pusher = al_load_bitmap(GetDataFilePath(game, "pusher.png"));

	data->huginn = CreateCharacter(game, "huginn");
	RegisterSpritesheet(game, data->huginn, "huginn");
	LoadSpritesheets(game, data->huginn);

	return data;
}

void Gamestate_Unload(struct Game *game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	free(data);
}

void Gamestate_Start(struct Game *game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	data->blink_counter = 0;
	data->cloudspos = 0;
	data->pos = 0;
	data->speed= 0;
	data->speedmod = 0;
	data->numpersons=0;
	data->numpushers=0;
	data->text = NULL;
	SetCharacterPosition(game, data->huginn, 10, 80, 0);
	SelectSpritesheet(game, data->huginn, "huginn");

	data->up = false;
	data->down = false;
	data->left = false;
	data->right = false;
}

void Gamestate_Stop(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

void Gamestate_Pause(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets paused (so only Draw is being called, no Logic not ProcessEvent)
	// Pause your timers here.
}

void Gamestate_Resume(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets resumed. Resume your timers here.
}

// Ignore this for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct GamestateResources* data) {}
