#include <hf-risc.h>
#include "vga_drv.h"
//#include <stdio.h>
#define RECT_WIDTH 14 
#define RECT_HEIGHT 8
#define PAD_WIDTH 26
#define PAD_HEIGHT 6
#define MAX_SPEED 8

int16_t ball_color = WHITE;
int16_t blockCount = 0;
int16_t deadBlocks = 0;
int16_t speed = 24;
int16_t xspeed = 24;
int16_t yspeed = 24;
int16_t pad_speed = 2;
int16_t points = 0;
int16_t point_coef = 1;

volatile uint8_t up    = 0;
volatile uint8_t down  = 0;
volatile uint8_t left  = 0;
volatile uint8_t right = 0;

struct rectangle{
    int16_t x0, y0, color;
    uint8_t alive;
};

struct ball{
    int16_t x0, y0;
    int16_t old_x0, old_y0;
};

struct paddle
{
    int16_t x0, y0;
    int16_t old_x0, old_y0;
};

char* int_to_string(int16_t points){
    char *str = malloc(7*sizeof(char));
    if(str == NULL){
        return NULL;
    }

    sprintf(str, "%d", points);
    return str;
}

char *concatenate_str(char* str1, char* str2){
    uint8_t totallenght = strlen(str1)+strlen(str2)+1;

    char *result = malloc(totallenght * sizeof(char));
    if(result == NULL){
        return NULL;
    }

    strcpy(result,str1);
    strcat(result,str2);


    return result;
}

void display_points(){
    char *points_str = int_to_string(points);
    char *points_header = "Points:";
    char *printable = concatenate_str(points_header,points_str);
    // printf("%s\n", printable);
    display_frectangle(260,210,40,8,BLACK);
    display_print(printable,200,210,1,WHITE);
    free(points_str);
    // free(points_header);
    free(printable);
}





void init_input()
{
	/* configure GPIOB pins 8 .. 12 as inputs */
	GPIOB->DDR &= ~(MASK_P8 | MASK_P9 | MASK_P10 | MASK_P11 | MASK_P12);
}



void get_input(struct paddle *ppad)
{
	if (GPIOB->IN & MASK_P10) { // left
        //printf("going left\n");
        
            ppad->old_x0 = ppad->x0;
            ppad->x0 -= pad_speed;
            if(ppad->x0 < 0){
                ppad->x0 = 0;
                ppad->old_x0 = ppad->x0 + pad_speed;
            }
        
        display_frectangle(ppad->old_x0 + PAD_WIDTH, ppad->y0,pad_speed,PAD_HEIGHT,BLACK);
        display_frectangle(ppad->x0, ppad->y0,pad_speed,PAD_HEIGHT,WHITE);
        //printf("paddle x %d, y %d\n",ppad->x0, ppad->y0);
		
	}	
	if (GPIOB->IN & MASK_P11) { // right
		//printf("going right\n");
        
            ppad->old_x0 = ppad->x0;
            ppad->x0 += pad_speed;
            if(ppad->x0 > VGA_WIDTH-PAD_WIDTH - pad_speed){
                ppad->x0 = VGA_WIDTH-PAD_WIDTH - pad_speed;
                ppad->old_x0 = ppad->x0 - pad_speed;
            }
        
        
        
        display_frectangle(ppad->old_x0, ppad->y0,pad_speed,PAD_HEIGHT,BLACK);
        display_frectangle(ppad->x0+PAD_WIDTH, ppad->y0,pad_speed,PAD_HEIGHT,WHITE);
        //printf("paddle x %d, y %d\n",ppad->x0, ppad->y0);
	}
    
}
void display_rects(struct rectangle *rect ){
    display_frectangle(rect->x0, rect->y0, RECT_WIDTH, RECT_HEIGHT, rect->color);
}


struct rectangle** create_blocks(){
    for(int i = 0; i < (VGA_WIDTH - RECT_WIDTH); i += (RECT_WIDTH + 1)) {
        for(int j = 0; j < (VGA_HEIGHT / 3) - RECT_HEIGHT; j += (RECT_HEIGHT + 1)) {
            blockCount++;
        }
    }
    struct rectangle** blocks = malloc(blockCount * sizeof(struct rectangle*));
    if (blocks == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    int index = 0;
    int z = 2;
    for(int i = 0; i < (VGA_WIDTH - RECT_WIDTH); i += (RECT_WIDTH + 1)) {
        for(int j = 0; j < (VGA_HEIGHT / 3) - RECT_HEIGHT; j += (RECT_HEIGHT + 1)) {
            if(i % 2 == 0) {
                z = YELLOW;
            } else {
                z = RED;
            }

            // Allocate memory for each rectangle struct
            blocks[index] = malloc(sizeof(struct rectangle));
            if (blocks[index] == NULL) {
                // Handle memory allocation failure for a rectangle
                // Ideally, you should also free previously allocated memory here
                return NULL;
            }

            // Initialize the struct as needed, e.g., setting its color
            // Assuming 'rectangle' has a 'color' field
            blocks[index]->color = z;
            blocks[index]->x0 = i;
            blocks[index]->y0 = j;
            blocks[index]->alive = 0xFF;

            display_rects(blocks[index]);

            // Increment index for the next rectangle
            index++;
        }
    }

    return blocks; // Return the vector of pointers
}




void init_ball(struct ball *ball){
    ball->x0 = 150;
    ball->y0 = 50;
    ball->old_x0 = ball->x0;
    ball->old_y0 = ball->y0;
}

void display_ball(struct ball *ball){
    //display_frectangle(ball->old_x0 -5, ball->old_y0-5, 1,1, BLACK);
    display_frectangle(ball->x0, ball->y0,1,1,ball_color);
}
void erase_ball(struct ball *ball){
    display_frectangle(ball->old_x0, ball->old_y0,1,1, BLACK);
}



void move_ball(struct ball *ball){

    ball->old_x0 = ball->x0;
    ball->old_y0 = ball->y0;
    //printf("\rx %d, y %d", ball->x0, ball->y0);

    if(left){
        ball->x0--;
    }
    if(right){
        ball->x0++;
    }
    if(up){
        ball->y0--;
    }
    if(down){
        ball->y0++;
    }
    if(299 <= ball->x0){
            right = !right;
            left  = !left;
        }
    if(0 >= ball->y0){
            up = !up;
            down = !down;
    }
    if(0 >= ball->x0){
            right = !right;
            left  = !left;
    }
    if(217 <= ball->y0){
            up = !up;
            down = !down;
    }
    erase_ball(ball);
    display_ball(ball);
}

// void move_snake(struct ball *ball){

//     while(1){
//         for (int i = 0; i < 10; i ++){
//             move_ball(0, 1, 0, 0, ball);
//             delay_ms(16);
//         }
//         for (int i = 0; i < 10; i ++){
//             move_ball(0, 0, 1, 0, ball);
//             delay_ms(16);
//         }
//         for (int i = 0; i < 10; i ++){
//             move_ball(0, 1, 0, 0, ball);
//             delay_ms(16);
//         }
//         for (int i = 0; i < 10; i ++){
//             move_ball(0, 0, 0, 1, ball);
//             delay_ms(16);
//         }

//     }
// }
void erase_rect(struct rectangle *rect){
    display_frectangle(rect->x0, rect->y0,RECT_WIDTH,RECT_HEIGHT, BLACK);
}


void collison_detector(struct ball *ball,struct rectangle **blocks, struct paddle *ppad){
    for(int i = 0; i < blockCount; i++){
        // if(( (blocks[i]->x0 < ball->x0) < blocks[i]->x0 + RECT_WIDTH ) && ( blocks[i]->y0 < ball->y0 < blocks[i]->y0 + RECT_HEIGHT)){
        if(blocks[i]->alive > 1){

            if(((blocks[i]->x0 <= ball->x0)&& (ball->x0 <= blocks[i]->x0 + RECT_WIDTH)) && ((blocks[i]->y0 <= ball->y0)&&(ball->y0 <= blocks[i]->y0 + RECT_HEIGHT))){
            
                if((ball->x0 == blocks[i]->x0) || (ball->x0 == blocks[i]->x0 + RECT_WIDTH)){
                    left = !left;
                    right = !right;
                    //printf("side collision x = %d  y = %d index%d\n",ball->x0, ball->y0, i);
                } else {
                    up    = !up;
                    down  = !down;
                    //printf("top/bottom collision x = %d  y = %d index%d\n",ball->x0, ball->y0, i);
                }
                //printf("\rblocos mortos %d blocos totais %d", deadBlocks, blockCount);
                blocks[i]->alive = 0x00;
                deadBlocks++;
                if(speed > MAX_SPEED){
                    speed--;
                }
                //printf("Collison! x = %d  y = %d index%d\n",ball->x0, ball->y0, i);
                //printf("%d\n",points);
                erase_rect(blocks[i]);
                //free(blocks[i]);
                points += point_coef;
                display_points();
            }
        }
    }
    if(((ppad->x0 <= ball->x0) && (ball->x0 <= ppad->x0  + PAD_WIDTH)) && ((ppad->y0 <= ball->y0+1)&&(ball->y0 <= ppad->y0 + PAD_HEIGHT))){
        if((ball->x0 == ppad->x0) || (ball->x0 == ppad->x0 + PAD_WIDTH)){
            left = !left;
            right = !right;
        } else {
            up    = !up;
            down  = !down;
        }
        
        
        
        //printf("paddle hit!\n");
    }
    move_ball(ball);
}
void new_collision(struct ball *ball, struct rectangle **blocks, struct paddle *ppad){

    // only possible collision areas
    move_ball(ball);
    if((ball->y0 < VGA_HEIGHT/3+10)){ 
        for(int i = 0; i < blockCount; i++){
            // checking if the block still in the game
            if(blocks[i]->alive > 1){
                // checking limits
                if(((ball->x0 + 1  >= blocks[i]->x0) && (ball->x0 -1 <= blocks[i]->x0 + RECT_WIDTH)) && ((ball->y0 + 1 >= blocks[i]->y0) && (ball->y0 -1  <= blocks[i]->y0 + RECT_HEIGHT))){
                    // finding what type of collision
                    if((ball->x0+1 == blocks[i]->x0) || (ball->x0  -1 == (blocks[i]->x0 + RECT_WIDTH))){
                        // hit on the side
                        left = !left;
                        right = !right;
                        // printf("side collision x = %d  y = %d index%d\n",ball->x0, ball->y0, i);
                        
                    } else {
                        // hit top or bottom
                        up = !up;
                        down = !down;
                        // printf("top/bottom collision x = %d  y = %d index%d\n",ball->x0, ball->y0, i);
                        
                    }
                    // printf("Collison! x = %d  y = %d index%d\n",ball->x0, ball->y0, i);
                    blocks[i]->alive = 0x00;
                    deadBlocks++;
                    if(speed > MAX_SPEED){
                        speed--;
                    }
                    erase_rect(blocks[i]);
                    points += point_coef;
                    display_points();
                    break;
                }
            }
        }
    } else if (ball->y0 > ppad->y0 -10) {
        // checking for paddle collision
        if(((ball->x0 +1 >= ppad->x0) &&(ball->x0 -1 <= ppad->x0 + PAD_WIDTH)) && ((ball->y0 +1 >= ppad->y0) && ball->y0 -1 <= ppad->y0 + PAD_HEIGHT)){
            
            if(ball->x0 + 1 == ppad->x0){
                left = !left;
                right = !right;
                ball->x0 = ppad->x0 - 1;
                printf("1\n");
            }
            else if(ball->x0 -1 == ppad->x0 + PAD_WIDTH){
                left = !left;
                right = !right;
                //ball->x0 = ppad->x0 + PAD_WIDTH + 1;
                printf("2\n");
            } else if (ball->y0 + 1 == ppad->y0){
                up    = !up;
                down  = !down;
                printf("3\n");
            } else if (ball->y0 -1  == ppad->y0 + PAD_HEIGHT){
            
                //ball->y0 = ppad->y0 -1;
                up    = !up;
                down  = !down;
                printf("4\n");
            } else {
                
                left = !left;
                right = !right;
                if((ball->x0-ppad->x0) < (ppad->x0 + PAD_WIDTH - ball->x0 )){
                    ball->x0 = ppad->x0 - 4 * pad_speed;
                } else {
                    ball->x0 = ppad->x0 + PAD_WIDTH + 4* pad_speed;
                }
                

            }
        //if (ball->y0 <= ppad->y0 && )
    }
    //move_ball(ball);
}
}


void init_paddle(struct paddle *ppad){
    ppad->x0 = 130;
    ppad->y0 = 200;
    ppad->old_x0 = ppad->x0;
    ppad->old_y0 = ppad->y0;
    display_frectangle(ppad->x0,ppad->y0,PAD_WIDTH,PAD_HEIGHT,WHITE);
}







int main(void){


    heap_init((uint32_t *)&_heap_start, 8192);
    display_background(BLACK);
    
    struct ball ball1;
    struct paddle pad;
    struct paddle *ppad = &pad;

    struct ball *pball1 = &ball1; 

    struct rectangle **blocks = create_blocks();

    
    init_ball(pball1);
    init_paddle(ppad);

    char* str = "VICTORY!";
    

    display_pixel(0,0, WHITE);
    display_pixel(299,0, WHITE);
    display_pixel(0, 217, WHITE);
    display_pixel(299, 217, WHITE);
    //display_frectangle(450, 300, 10, 10, RED);


    up = 0xff;
    left = 0xff;
    display_points();
    while(deadBlocks < blockCount)
    {
        
        //printf("ball x%d y%d, dead%d, blocks%d\n", pball1->x0,pball1->y0, deadBlocks, blockCount);
        get_input(ppad);
        // collison_detector(pball1, blocks, ppad);
        new_collision(pball1, blocks, ppad);
        delay_ms(speed);
    }
    display_background(WHITE);
    display_print(str, 45, 45,2, YELLOW);
    display_print(str, 47, 47,2, BLACK);
    return 0;
}