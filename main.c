#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

typedef struct game game;
typedef enum direction direction;
typedef struct queue queue;
typedef struct node node;

enum direction{LEFT = 1,RIGHT = -1,UP = 2,DOWN = -2,NONE = 0};

struct game{
	int width;
	int height;
	int loc_x;
	int loc_y;
	uint8_t **map;
	uint8_t ***memory;
	uint8_t memory_size;
	direction last_step;
};

game* create_game();
void print_game(game*);

void swap(game*,direction);
bool can_go(game*,direction);
int h(game*);

void play_1(game*,int);
void play_2(game*);
direction get_heuristic(game*,uint8_t***);
void save(game*);
uint8_t** copy(game*);
bool is_remembered(game*,uint8_t**);
bool is_same(game*,uint8_t**);

int main(){
	srand(time(NULL));
	game *g = create_game(7,7);
	print_game(g);
	for(int i = 0;i<10000000;i++){
		play_1(g,1);
		print_game(g);
		if(h(g) == 0){
			print_game(g);
			break;
		}
	}
}

game* create_game(int w,int h){
	game *g = malloc(sizeof(game));
	g->width = w;
	g->height = h;
	g->loc_x = 0;
	g->loc_y = 0;
	g->memory = NULL;
	g->memory_size = 0;
	g->map = malloc(sizeof(uint8_t*) * w);
	for(int i = 0;i < w;i++){
		g->map[i] = malloc(sizeof(uint8_t) * h);
	}

	for(int i = 0;i < w;i++){
		for(int j = 0;j < h;j++){
			g->map[i][j] = i + j * w;
		}
	}

	for(int i = 0;i < 10;i++){
		int directions[] = {LEFT,UP,RIGHT,DOWN};
		for(int i = 0;i < 1000;i++){
			int to;
			do{
				to = rand() % 4;
			}while(!can_go(g,directions[to]));
			swap(g,directions[to]);
		}
	}

	for(int i = 0;i < w;i++){
		for(int j = 0;j < h;j++){
			if(g->map[i][j] == 0){
				g->loc_x = i;
				g->loc_y = j;
				break;
			}
		}
	}

	return g;
}

void print_game(game *g){
	printf("--%d--\n",h(g));
	for(int i = 0;i < g->height;i++){
		for(int j = 0;j < g->width;j++){
			if(g->map[j][i]){
				if(g->map[j][i] < 10){
					printf("%d   ",g->map[j][i]);
				}else{
					printf("%d  ",g->map[j][i]);
				}
			}else{
				printf("    ");
			}
		}
		printf("\n\n");
	}
	printf("\n");
}

void swap(game *g,direction dir){
	int swap;
	switch(dir){
		case LEFT :
			swap = g->map[g->loc_x][g->loc_y];
			g->map[g->loc_x][g->loc_y] = g->map[g->loc_x - 1][g->loc_y];
			g->map[g->loc_x - 1][g->loc_y] = swap;
			g->loc_x--;
			break;
		case UP : 
			swap = g->map[g->loc_x][g->loc_y];
			g->map[g->loc_x][g->loc_y] = g->map[g->loc_x][g->loc_y - 1];
			g->map[g->loc_x][g->loc_y - 1] = swap;
			g->loc_y--;
			break;
		case RIGHT :
			swap = g->map[g->loc_x][g->loc_y];
			g->map[g->loc_x][g->loc_y] = g->map[g->loc_x + 1][g->loc_y];
			g->map[g->loc_x + 1][g->loc_y] = swap;
			g->loc_x++;
			break;
		case DOWN :
			swap = g->map[g->loc_x][g->loc_y];
			g->map[g->loc_x][g->loc_y] = g->map[g->loc_x][g->loc_y + 1];
			g->map[g->loc_x][g->loc_y + 1] = swap;	
			g->loc_y++;
			break;
	}
}

int h(game *g){
	int h_value = 0;
	for(int i = 0;i < g->width;i++){
		for(int j = 0;j < g->height;j++){
			int value = g->map[i][j];
			if(value != 0){
				h_value += abs(i - (value % g->width)) + abs(j - (value / g->width));
			}
		}
	}
	return h_value;
}

void play_1(game *g,int brain_power){
	int min_h = -1;
	direction dir = NONE;
	int direction_list[] = {LEFT,UP,RIGHT,DOWN};
	
	for(int to = 0;to < 4;to++){
		if(can_go(g,direction_list[to]) && direction_list[to] != -g->last_step){
			int heuristic = 0;
			uint8_t ***memories = calloc(brain_power + 1,sizeof(uint8_t**));

			direction *directs = malloc(sizeof(direction) * brain_power);
			swap(g,direction_list[to]);
			memories[0] = copy(g);

			directs[0] = get_heuristic(g,memories);
			swap(g,directs[0]);
			memories[1] = copy(g);

			for(int i = 1;i < brain_power;i++){
				directs[i] = get_heuristic(g,memories);
				swap(g,directs[i]);
				memories[i + 1] = copy(g);
			}
			for(int i = 0;i < brain_power + 2;i++){
				free(memories[i]);
			}
			free(memories);
	
			heuristic = h(g);

			for(int i = brain_power - 1;i >= 0;i--){
				swap(g,-directs[i]);
			}
			swap(g,-direction_list[to]);

			if(min_h == -1 || min_h > heuristic){
				dir = direction_list[to];
				min_h = heuristic;
			}else if(min_h == heuristic && rand() % 2){
				dir = direction_list[to];
				min_h = heuristic;
			}
		}
	}

	g->last_step = dir;
	swap(g,dir);
	save(g);
}

void play_2(game *g){
	int directions[] = {LEFT,UP,RIGHT,DOWN};
	for(int i = 0;i < 10000000;i++){
		print_game(g);
		if(h(g) == 0){
			print_game(g);
			break;
		}
		int to;
		do{
			to = rand() % 4;
		}while(!can_go(g,directions[to]));
		swap(g,directions[to]);
	}
}

direction get_heuristic(game *g,uint8_t ***memories){;
	direction dir = NONE;
	int min_h = -1;
	
	if(can_go(g,LEFT)){
		swap(g,LEFT);
		if(!is_remembered(g,copy(g))){
			bool check = 1;
			int i = 0;
			while(memories[i] != NULL){
				if(is_same(g,memories[i])){
					check = 0;
				}
				i++;
			}
			if(check){
				int h_now = h(g);
				if(min_h == -1 || min_h > h_now){
					min_h = h_now;
					dir = LEFT;
				}	
			}
		}	
		swap(g,RIGHT);
	}

	if(can_go(g,RIGHT)){
		swap(g,RIGHT);
		if(!is_remembered(g,copy(g))){
			bool check = 1;
			int i = 0;
			while(memories[i] != NULL){
				if(is_same(g,memories[i])){
					check = 0;
				}
				i++;
			}
			if(check){
				int h_now = h(g);
				if(min_h == -1 || min_h > h_now){
					min_h = h_now;
					dir = RIGHT;
				}	
			}
		}	
		swap(g,LEFT);
	}

	if(can_go(g,UP)){
		swap(g,UP);
		if(!is_remembered(g,copy(g))){
			bool check = 1;
			int i = 0;
			while(memories[i] != NULL){
				if(is_same(g,memories[i])){
					check = 0;
				}
				i++;
			}
			if(check){
				int h_now = h(g);
				if(min_h == -1 || min_h > h_now){
					min_h = h_now;
					dir = UP;
				}	
			}
		}	
		swap(g,DOWN);
	}

	if(can_go(g,DOWN)){
		swap(g,DOWN);
		if(!is_remembered(g,copy(g))){
			bool check = 1;
			int i = 0;
			while(memories[i] != NULL){
				if(is_same(g,memories[i])){
					check = 0;
				}
				i++;
			}
			int h_now = h(g);
			if(min_h == -1 || min_h > h_now){
				min_h = h_now;
				dir = DOWN;
			}	
		}	
		swap(g,UP);
	}

	return dir;
}

bool can_go(game *g,direction dir){
	switch(dir){
		case LEFT : return (g->loc_x > 0)? 1 : 0;
		case UP : return (g->loc_y > 0)? 1 : 0;
		case RIGHT : return (g->loc_x < g->width - 1)? 1 : 0;
		case DOWN : return (g->loc_y < g->height - 1)? 1 : 0;
	}
}	

uint8_t** copy(game *g){
	uint8_t **map = malloc(sizeof(uint8_t*) * g->width);
	for(int i = 0;i < g->width;i++){
		map[i] = malloc(sizeof(uint8_t) * g->height);
		for(int j = 0;j < g->height;j++){
			map[i][j] = g->map[i][j];
		}
	}
	return map;
}

void save(game *g){
	g->memory = realloc(g->memory,sizeof(uint8_t**) * (g->memory_size + 1));
	g->memory[g->memory_size] = copy(g);
	g->memory_size++;
}

bool is_remembered(game *g,uint8_t **state){
	bool check = 1;
	for(long i = 0;i < g->memory_size;i++){
		check = 1;
		for(uint8_t j = 0;j < g->width;j++){
			for(uint8_t k = 0;k < g->height;k++){
				if(g->memory[i][j][k] != state[j][k]){
					check = 0;
				}
			}
		}
		if(check){
			return 1;
		}
	}
	return 0;
}

bool is_same(game *g,uint8_t **state){
	for(int i = 0;i < g->width;i++){
		for(int j = 0;j < g->height;j++){
			if(g->map[i][j] != state[i][j]){
				return 0;
			}
		}
	}
	return 1;
}
