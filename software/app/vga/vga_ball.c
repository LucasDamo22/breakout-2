#include <hf-risc.h>
#include "vga_drv.h"

#define VERT_BAR_BASE	9
#define NUMBER_OF_BARS  13 * VERT_BAR_BASE
#define A_LEFT			0x2190
#define A_UP			0x2191
#define A_RIGHT			0x2192
#define A_DOWN			0x2193
#define A_UP_LEFT		0x2196
#define A_UP_RIGHT		0x2197
#define A_DOWN_LEFT		0x2199
#define A_DOWN_RIGHT	0x2198


struct ball_s {
	unsigned int ballx, bally;
	unsigned int last_ballx, last_bally;
	int dx, dy;
	int spx, spy;
	int spxcnt, spycnt;
};

struct paddle {
	unsigned int x, y;
	int cor;
	int spx;
	int div;
	int w, h;
};

struct bars {
	unsigned int x, y;
	int w, h;
	int cor;
};

// void init_bars(struct bars *b[NUMBER_OF_BARS])
// {
// 	char colors[9][10] = {{RED},{ LRED},{ YELLOW},{ GREEN},{ LGREEN},{ CYAN},{ LCYAN},{ BLUE},{ LBLUE}};

// 	for (int i = 0, l = 0; i < 280; i += 21) {
// 		int base = 5;
// 		for(int j = 0; j < 9 ; j++, l++) {
// 			display_frectangle	(5+i, base,	20, 8, colors[j]);
// 			b[l]->x = 5+i;
// 			b[l]->y = base;
// 			b[l]->w = 20;
// 			b[l]->h = 8;
// 			b[l]->cor = colors[j];
// 			base += 9;
// 		}
// 	}
// }

void init_bars(struct bars *b[NUMBER_OF_BARS])
{
 	for (int i = 0, l = 0; i < 280; i += 21) {
 		display_frectangle(5+i, 5, 20, 8, RED);
		b[l]->x = 5+i;
		b[l]->y = 5;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = RED;
 		display_frectangle(5+i, 14, 20, 8, LRED);
		b[l]->x = 5+i;
		b[l]->y = 14;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = LRED;
 		display_frectangle(5+i, 23, 20, 8, YELLOW);
		b[l]->x = 5+i;
		b[l]->y = 23;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = YELLOW;
 		display_frectangle(5+i, 32, 20, 8, GREEN);
		b[l]->x = 5+i;
		b[l]->y = 32;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = GREEN;
 		display_frectangle(5+i, 41, 20, 8, LGREEN);
		b[l]->x = 5+i;
		b[l]->y = 41;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = LGREEN;
 		display_frectangle(5+i, 50, 20, 8, CYAN);
		b[l]->x = 5+i;
		b[l]->y = 50;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = CYAN;
 		display_frectangle(5+i, 59, 20, 8, LCYAN);
		b[l]->x = 5+i;
		b[l]->y = 59;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = LCYAN;
 		display_frectangle(5+i, 68, 20, 8, BLUE);
		b[l]->x = 5+i;
		b[l]->y = 68;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = BLUE;
 		display_frectangle(5+i, 77, 20, 8, LBLUE);
		b[l]->x = 5+i;
		b[l]->y = 77;
		b[l]->w = 20;
		b[l]->h = 8;
		b[l++]->cor = LBLUE;
 	}
 }

void init_paddle(struct paddle *p) {
	p->x = 130;
	p->y = 170;
	p->cor = WHITE;
	p->spx = 1;
	p->div = 10;
	p->w = 20;
	p->h = 4;
	display_frectangle(p->x, p->y, p->w, p->h, p->cor); //paddle
}

void init_display(struct bars *b[NUMBER_OF_BARS])
{
	display_background(BLACK);
	init_bars(b);
	for (int i = 0; i < NUMBER_OF_BARS; i++) {
		printf("Barra %d, (%d, %d), w: %d, h: %d, cor: %d\n", i, b[i]->x, b[i]->y, b[i]->w, b[i]->h, b[i]->cor);
	}
}

void init_ball(struct ball_s *ball, int x, int y, int dx, int dy, int spx, int spy)
{
	ball->ballx = x;
	ball->bally = y;
	ball->last_ballx = ball->ballx;
	ball->last_bally = ball->bally;
	ball->dx = dx;
	ball->dy = dy;
	ball->spx = spx;
	ball->spxcnt = spx;
	ball->spy = spy;
	ball->spycnt = spy;
}

void init_input()
{
	/* configure GPIOB pins 8 .. 12 as inputs */
	GPIOB->DDR &= ~(MASK_P8 | MASK_P9 | MASK_P10 | MASK_P11 | MASK_P12);
}

void test_limits(struct ball_s *ball, char *limits)
{
	unsigned int ballx, bally;
	
	ballx = ball->ballx;
	bally = ball->bally;
	
	limits[0] = display_getpixel(ballx-1, bally-1);		//D_left
	limits[1] = display_getpixel(ballx, bally-1);		//Down
	limits[2] = display_getpixel(ballx+1, bally-1);		//D_right
	limits[3] = display_getpixel(ballx-1, bally);		//Left
	limits[4] = 0;
	limits[5] = display_getpixel(ballx+1, bally);		//Right
	limits[6] = display_getpixel(ballx-1, bally+1);		//U_left
	limits[7] = display_getpixel(ballx, bally+1);		//Up
	limits[8] = display_getpixel(ballx+1, bally+1);		//U_right
}

char test_collision(struct ball_s *ball, char *dir, struct bars *b[NUMBER_OF_BARS])
{
	char hit = 0;
	char cor_hit;
	int i, j, pos_hit;
	char limits[9];
	
	if ((ball->spx == ball->spxcnt) || (ball->spy == ball->spycnt)) {
		test_limits(ball, limits);

		if ((ball->ballx < VGA_WIDTH-1) && (ball->ballx > 0) && (ball->bally < VGA_HEIGHT-1) && (ball->bally > 0)) {
			char directions[9] = {A_DOWN_LEFT, A_DOWN, A_DOWN_RIGHT, A_LEFT, 0, A_RIGHT, A_UP_LEFT, A_UP, A_UP_RIGHT};
			for (i = 0; i < 9; i++) {
				if (limits[i]) {
					printf("Hit! Cor: %d\n", limits[i]);
					hit = 1;
					cor_hit = limits[i];
					*dir = directions[i];
					break;
				}
			}
			
			switch (cor_hit) {
				case RED: i = 0; break;
				case LRED: i = 1; break;
				case YELLOW: i = 2; break;
				case GREEN: i = 3; break;
				case LGREEN: i = 4; break;
				case CYAN: i = 5; break;
				case LCYAN: i = 6; break;
				case BLUE: i = 7; break;
				case LBLUE: i = 8; break;
				default: i = -1; break;
			}

			if (i != -1) {
				for (j = 0; j < 13; i += VERT_BAR_BASE, j++) {
					if (ball->ballx <= (b[i]->x + b[i]->w) && ball->ballx >= b[i]->x) {
						//printf("Deletando bloco em (%d, %d)\n", b[i]->x, b[i]->y);
						display_frectangle(b[i]->x, b[i]->y, b[i]->w, b[i]->h, BLACK);
						break;
					}
				}
			}

			if (limits[1]) /*DOWN*/	{ 
				ball->dy = -ball->dy;
				/*Search for block under the ball*/
				// for (i = 0; i < NUMBER_OF_BARS; i++) {
				// 	if ((ball->ballx >= b[i]->x) && (ball->ballx <= b[i]->x + b[i]->w) && (ball->bally + 1 == b[i]->y)) {
				// 		display_frectangle(b[i]->x, b[i]->y, b[i]->w, b[i]->h, BLACK);
				// 		break;
				// 	}
				// }
			}

			if (limits[3]) /*LEFT*/ { 
				ball->dx = -ball->dx;
				/*Search for block to the left of the ball*/
				// for (i = 0; i < NUMBER_OF_BARS; i++) {
				// 	if ((ball->ballx - 1 >= b[i]->x) && (ball->ballx - 1 <= b[i]->x + b[i]->w) && (ball->bally  == b[i]->y)) {
				// 		display_frectangle(b[i]->x, b[i]->y, b[i]->w, b[i]->h, BLACK);
				// 		break;
				// 	}
				// }

			}
			if (limits[5])/*RIGHT*/{ 
				ball->dx = -ball->dx;
				/*Search for block to the right of the ball*/
				// for (i = 0; i < NUMBER_OF_BARS; i++) {
				// 	if ((ball->ballx + 1 >= b[i]->x) && (ball->ballx + 1 <= b[i]->x + b[i]->w) && (ball->bally  == b[i]->y))/*Talvez tenha que adicionar um && bally + h*/ {
				// 		display_frectangle(b[i]->x, b[i]->y, b[i]->w, b[i]->h, BLACK);
				// 		break;
				// 	}
				// }
			}			
			if (limits[7]) /*UP*/{ 
				ball->dy = -ball->dy;
				/*Search for block above the ball*/
				// for (i = 0; i < NUMBER_OF_BARS; i++) {
				// 	if ((ball->ballx >= b[i]->x) && (ball->ballx <= b[i]->x + b[i]->w) && (ball->bally - 1 == b[i]->y)) {
				// 		display_frectangle(b[i]->x, b[i]->y, b[i]->w, b[i]->h, BLACK);
				// 		break;
				// 	}
				// }
			}

			if (limits[0]) {								//D_left
				if (!limits[3]) ball->dx = -ball->dx;
				if (!limits[1]) ball->dy = -ball->dy;
			}
			if (limits[2]) {								//D_right
				if (!limits[5]) ball->dx = -ball->dx;
				if (!limits[1]) ball->dy = -ball->dy;
			}
			if (limits[6]) {								//U_left
				if (!limits[3]) ball->dx = -ball->dx;
				if (!limits[7]) ball->dy = -ball->dy;
			}
			if (limits[8]) {								//U_right
				if (!limits[5]) ball->dx = -ball->dx;
				if (!limits[7]) ball->dy = -ball->dy;
			}
		} else {
			if ((ball->ballx + ball->dx > VGA_WIDTH-1) || (ball->ballx + ball->dx < 1))
				ball->dx = -ball->dx;
			if ((ball->bally + ball->dy > VGA_HEIGHT-1) || (ball->bally + ball->dy < 1))
				ball->dy = -ball->dy;
		}
	}
	
	return hit;
}

void update_ball(struct ball_s *ball, char hit)
{
	ball->last_ballx = ball->ballx;
	ball->last_bally = ball->bally;
	
	if (hit) {
		ball->spxcnt = 1;
		ball->spycnt = 1;
	}
	
	if (--ball->spxcnt == 0) {
		ball->spxcnt = ball->spx;
		ball->ballx = ball->ballx + ball->dx;
	}
	if (--ball->spycnt == 0) {
		ball->spycnt = ball->spy;
		ball->bally = ball->bally + ball->dy;
	}

	if ((ball->spx == ball->spxcnt) || (ball->spy == ball->spycnt)) {
		display_pixel(ball->last_ballx, ball->last_bally, BLACK);
		display_pixel(ball->ballx, ball->bally, WHITE);
	}
}

void get_input(struct paddle *p)
{
	if (GPIOB->IN & MASK_P10) { // left
		int temp = p->x;
		p->x = (p->x - p->spx < 0 ? 0 : p->x - p->spx);
		
		display_frectangle(temp, p->y, 1, p->h, BLACK);
		display_frectangle(p->x, p->y, p->w, p->h, p->cor);
		
	}	
	if (GPIOB->IN & MASK_P11) { // right
		int temp = p->x;
		p->x = (p->x + p->spx > 300 ? 300 : p->x + p->spx);
		display_frectangle(temp+p->w, p->y, 1, p->h, BLACK);
		display_frectangle(p->x, p->y, p->w, p->h, p->cor);
		
	}
}

int main(void)
{
	struct ball_s	ball1, ball2, ball3;
	struct paddle	pad;
	struct ball_s	*pball1 = &ball1;
	struct ball_s	*pball2 = &ball2;
	struct ball_s	*pball3 = &ball3;
	struct paddle	*pad_pointer = &pad;
	struct bars		*b[NUMBER_OF_BARS];
	char hit;
	char dir;
	
	heap_init((uint32_t *)&_heap_start, 8192);

	for (int i = 0; i < NUMBER_OF_BARS; i++) b[i] = malloc(sizeof(struct bars));

	init_display(b);
	init_paddle(pad_pointer);
	init_ball(pball1, 20, 200, 1, -1, -30, -12);
	init_input();

	while (1) {
		if (pball1->bally <= 85 || pball1->bally >= 5) hit = test_collision(pball1, &dir, b);
		update_ball(pball1, hit);
		delay_ms(15);
		get_input(pad_pointer);
	}

	return 0;
}