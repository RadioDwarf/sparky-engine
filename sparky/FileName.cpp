#include "raylib.h"
#include "raymath.h"
#include <map>
#include <tuple>
#include <iostream>
#include <cinttypes>
#define MAPWIDTH 1024
#define MAPHEIGHT 256
#define PIXELSIZE 10
#define LOOKUP 5
namespace ColorEditor {
	Color ColorAdd(Color color1, Color color2) {		
		return { uint8_t(Clamp(color1.r + color2.r,0,255)), uint8_t(Clamp(color1.g + color2.g,0,255)), uint8_t(Clamp(color1.b + color2.b,0,255)), uint8_t(Clamp(color1.a + color2.a,0,255)) };
	}
	Color ColorSubtract(Color color1, Color color2) {
		return { uint8_t(Clamp(color1.r - color2.r,0,255)), uint8_t(Clamp(color1.g - color2.g,0,255)), uint8_t(Clamp(color1.b - color2.b,0,255)), uint8_t(Clamp(color1.a - color2.a,0,255)) };
	}
	Color ColorMultiply(Color color1, Color color2) {
		return { uint8_t(Clamp(color1.r * color2.r,0,255)), uint8_t(Clamp(color1.g * color2.g,0,255)), uint8_t(Clamp(color1.b * color2.b,0,255)), uint8_t(Clamp(color1.a * color2.a,0,255)) };
	}
	Color ColorDivide(Color color1, Color color2) {
		return { uint8_t(Clamp(color1.r / color2.r,0,255)), uint8_t(Clamp(color1.g / color2.g,0,255)), uint8_t(Clamp(color1.b / color2.b,0,255)), uint8_t(Clamp(color1.a / color2.a,0,255)) };
	}

}
namespace sparky {
	struct tile_rules {
		Color color;
		bool goes_around;
		bool physics;
		bool goes_up;
		int weight;
		bool position_static;
		std::string name;
		int charing_type;
		int burning_type;
		int left_over;
		int burning_time;
	};
	tile_rules generate_rule(Color color, bool goes_around, bool physics, bool goes_up, bool position_static, int weight, std::string name) { //manual addition
		return { color,goes_around,physics, goes_up,weight,position_static,name,-1,-2,0};
	}
	struct tile {
		int direction = 1;
		int type = 0;
		Color offset = { 0,0,0,0 };
		int life_time;
		int old_type;
		bool updated=false;
		void Update(struct game_data* data, Vector2 position);
	};
	struct game_data {
		Vector2 camera;
		tile_rules rules[10];
		tile tiles[MAPWIDTH][MAPHEIGHT];
		int tile_types = 0;
		void add_tile(Color color, bool goes_around, bool physics,  bool goes_up, bool position_static,int weight, std::string name) { //automatic addition
			this->rules[this->tile_types++] = generate_rule(color,goes_around, physics, goes_up, position_static, weight, name);
		}
		void set_burning_time(int tile_id, int burning_time) {
			this->rules[tile_id].burning_time = burning_time;
		}
		void set_burner(int tile_id, int burn_type, int char_type, int left_over) {
			this->rules[tile_id].burning_type = burn_type;
			this->rules[tile_id].charing_type = char_type;
			this->rules[tile_id].left_over = left_over;
		}
		void setup() {

			for (int x = 0; x < MAPWIDTH; x++) {
				for (int y = 0; y < MAPHEIGHT; y++) {
					tiles[x][y] = {
						1,
						0,
						{uint8_t(GetRandomValue(0,30)),uint8_t(GetRandomValue(0,30)),uint8_t(GetRandomValue(0,30)),0},
						false,
					};
					
					
				}
			}
		}
	};
	bool falling_update(int x, int y, game_data* data, int gravity=1) {
		bool can_continue = true;
		tile* base = &data->tiles[x][y];
		if (y + gravity < MAPHEIGHT && y+gravity>-1) {
			tile* tile = &data->tiles[x][y + gravity];
			if (!tile->updated) {
				if (tile->type == 0 || (tile->type!=0 && data->rules[tile->type].weight<data->rules[base->type].weight && !data->rules[tile->type].position_static)) {
					int ogtype = tile->type;
					tile->type = base->type;
					base->type = ogtype;
					tile->updated = true;
					return true;
				}

			}

			for (int cx = -1; cx < 2; cx += 2) {
				tile = &data->tiles[cx + x][y + gravity];
				if (cx+x>-1 && cx+x<MAPWIDTH && !tile->updated && data->tiles[cx + x][y].type == 0) {

					if (tile->type == 0 || (tile->type != 0 && data->rules[tile->type].weight < data->rules[base->type].weight && !data->rules[tile->type].position_static)) {
						int ogtype = tile->type;
						tile->type = base->type;
						base->type = ogtype;
						tile->updated = true;
						return true;
					}
				}
			}
		}
		return false;
	}
	bool gas_update(int x, int y, game_data* data) {
		return falling_update(x, y, data, -1);
	}
	bool burner_update(int x, int y, game_data* data) {
		for (int cx = -1; cx < 2; cx += 2) {
			for (int cy = -1; cy < 2; cy += 2) {
				if (x + cx > -1 && x + cx<MAPWIDTH && y + cy>-1 && y + cy < MAPHEIGHT) {
					if (data->rules[data->tiles[x + cx][y + cy].type].burning_type == data->rules[data->tiles[x][y].type].charing_type) {
						data->tiles[x][y].type = data->rules[data->tiles[x][y].type].left_over;
						std::cout << "a";
						data->tiles[x + cx][y + cy].type = 0;
						return true;
					}
				}
			}
		}
		return false;
	}
	bool fluid_update(int x, int y, game_data* data) {
		tile* base = &data->tiles[x][y];
		
		tile* direction_tile = &data->tiles[x + base->direction][y];
		if (base->direction != 1 && base->direction!=-1) {
			base->direction = 1;
		}

		if (!base->updated && x + base->direction > -1 && x + base->direction < MAPWIDTH && !direction_tile->updated && direction_tile->type==0) {
			direction_tile->type = base->type;
			base->type = 0;
			direction_tile->updated = true;
			return true;
		}
		if (direction_tile->type != 0) {
			base->direction *= -1;
			return true;
		}
		return false;
	}
	void tile::Update(game_data* data, Vector2 position) {
		
		if (!this->updated) {
			if (position.x * PIXELSIZE - data->camera.x > -10 && position.x * PIXELSIZE - data->camera.x<1200 && position.y * PIXELSIZE - data->camera.y>-10 && position.y * PIXELSIZE - data->camera.y < 800 && this->type!=0) {
				Color col = ColorEditor::ColorAdd(data->rules[this->type].color, this->offset);
				DrawRectangle(position.x * PIXELSIZE - data->camera.x, position.y * PIXELSIZE - data->camera.y, PIXELSIZE, PIXELSIZE, col);
			}
			if (burner_update(position.x, position.y, data)) goto END;
			if (data->rules[this->type].physics) {
				if (falling_update(position.x, position.y, data)) goto END;
			}
			if (data->rules[this->type].goes_up) {
				if (gas_update(position.x, position.y, data)) goto END;
			}
			if (data->rules[this->type].goes_around) {
				if (fluid_update(position.x, position.y, data)) goto END;
			}
		}
	END:
		this->old_type = this->type;
		this->updated = false;
	}
	void update_engine(game_data* data) {
		Vector2 base = { data->camera.x / PIXELSIZE,data->camera.y / PIXELSIZE };
		int render_count = 0;
		for (int x = int(base.x); x < 1024 + int(base.x); x++) {
			for (int y = int(base.y); y < 1024 + int(base.y); y++) {
				if (x > -1 && y > -1 && x < MAPWIDTH && y < MAPHEIGHT) {
					data->tiles[x][y].Update(data, { float(x),float(y) });
					render_count++;
				}
			}
		}
		DrawText(TextFormat("\n %d \n %d", GetFPS(), render_count), 0, 0, 25, BLACK);

	}
	class engine {
	private:
		struct game_data data;
	public:
		engine(int width, int height, const char* title) {
			InitWindow(width, height, title);
			float percent = 0;
			data.camera = { 0,0 };
			data.setup();
			data.add_tile(SKYBLUE, false, false, false,true,0, "air");
			data.add_tile(YELLOW, false, true, false, false, 6, "sand");
			data.add_tile(BLUE, true, true,  false, false,5, "water");
			data.add_tile(BROWN, false, false, false, true, 4, "wood");
			data.add_tile(RED, true, true, false, false, 4, "lava");
			data.add_tile(DARKGRAY, true, false, true, false, 3, "smoke");
			data.set_burner(2, 3, 4, 5);
			data.set_burner(4, 4, -1, 5);
			data.set_burner(3, -2, 4, 5);
			SetTargetFPS(60);
		}
		void run() {
			while (!WindowShouldClose()) {
				BeginDrawing();
				ClearBackground(SKYBLUE);
				this->data.camera.x -= IsKeyDown(KEY_A) * 5 + IsKeyDown(KEY_D) * -5;
				this->data.camera.y -= IsKeyDown(KEY_W) * 5 + IsKeyDown(KEY_S) * -5;
				
				update_engine(&this->data);
				Vector2 mouse_pos = Vector2Add(GetMousePosition(), this->data.camera);
				mouse_pos = Vector2Divide(mouse_pos, { PIXELSIZE,PIXELSIZE });
				if (IsMouseButtonDown(0)) {
					for (int x = 0; x < 8; x++) {
						for (int y = 0; y < 8; y++) {
							if (mouse_pos.x+x>-1&&mouse_pos.y+y>-1) this->data.tiles[int(mouse_pos.x)+x][int(mouse_pos.y)+y].type = IsKeyDown(KEY_ONE) + IsKeyDown(KEY_TWO) * 2 + IsKeyDown(KEY_THREE) * 3 + IsKeyDown(KEY_FOUR) * 4;
						}
					}
					
				}
				if (IsMouseButtonDown(1)) {
					this->data.tiles[int(mouse_pos.x)][int(mouse_pos.y)].type = 0;
				}
				EndDrawing();
			}
		}
	};
}
sparky::engine enigne(1200, 800, "sparky engine");

int main() {
	enigne.run();
}