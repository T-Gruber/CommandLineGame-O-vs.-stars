

#include <stdlib.h> // system
#include <time.h>   // clock
#include <stdio.h>  // print
#include <string.h> // string
#include <termios.h>// kbhit
#include <unistd.h> // kbhit
#include <fcntl.h>  // kbhit

#define HEIGHT 44
#define WIDTH  80
#define MAX_COUNT 200
#define MIN_COUNT 1

#define ENTER 10
#define UP 65
#define DOWN 66
#define RIGHT 67
#define LEFT 68

#define SPEED 10000
#define STONE_SPEED 100000

#define STONES_CHAR '*'
#define PLAYER_CHAR 'O'

char playground[HEIGHT+2][WIDTH];
unsigned int w_stones[MAX_COUNT] = {0};
unsigned int h_stones[MAX_COUNT] = {0};

unsigned int stone_score = 0;
unsigned int stone_count = MIN_COUNT;
unsigned int high_score = 0;
unsigned int stone_speed = STONE_SPEED;
clock_t last_time = 0;


void read_highscore(){
  FILE* file = fopen ("highscore.txt", "r");
  if(file){
    fscanf(file, "%d", &high_score);  
    fclose (file);
  }
  else{
    high_score = 0;  
  } 
}

void write_highscore(){
  FILE* file = fopen ("highscore.txt", "w");
  if(file){
    fprintf(file, "%d", high_score);
  }    
}


int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
    ungetc(ch, stdin);
    return 1;
    }

    return 0;
}


void wait_for_clock(unsigned long delay){
    clock_t curr_clock = clock();
    clock_t end_clock = curr_clock + delay;
    while(clock() < end_clock){}    
}

void wait_for_key(char key_to_wait_on){
    char key_pressed = getchar();
    while(key_pressed != key_to_wait_on){}
}


void clear_terminal(){
    system("clear");
}

void init_playground(){

    for(unsigned int i = 0; i < HEIGHT; ++i){
        for(unsigned int j = 0; j < WIDTH; ++j){
            if(i == 0 || i == HEIGHT-3 || i == HEIGHT-1)
                playground[i][j] = '-';
            else if(j == 0 || j == WIDTH-1)
                playground[i][j] = '|';
            else
                playground[i][j] = ' ';
        }
    }

    char score_string[WIDTH];
    sprintf(score_string, " SCORE: %6d  |  HIGHSCORE: %6d", stone_score, high_score);
    for(unsigned int i = 0; i < sizeof(score_string)/sizeof(char); ++i){
        if(!score_string[i])
            break;
        playground[HEIGHT-2][1+i] = score_string[i];
    }
}

void draw_playground(){
    for(unsigned int i = 0; i < HEIGHT; ++i){
        for(unsigned int j = 0; j < WIDTH; ++j)
            printf("%c", playground[i][j]);        
        printf("\n");
    }
}

void string_to_playground(char* string){
    size_t len = strlen(string);
    int h_pos = HEIGHT/2;
    int w_pos = WIDTH/2 - len/2;
    for(size_t i = 0; i < len; ++i)
        playground[h_pos][w_pos + i] = string[i];
}

const unsigned int h_player = HEIGHT-4;
unsigned int w_player = WIDTH/2;
void move_player(char dir){
    if(dir == -1 && w_player > 1)
        --w_player;
    if(dir == +1 && w_player < (WIDTH -2))
        ++w_player;
}

void draw_player(){
    playground[h_player][w_player] = PLAYER_CHAR;
}


void move_stones(){ 
    if(clock()-stone_speed > last_time){
        for(unsigned int i = 0; i < stone_count; ++i){
            if(w_stones[i] == 0){
                w_stones[i] = (rand() % (WIDTH-2))+1;
            }
            if(h_stones[i] < HEIGHT-4)
                ++h_stones[i];
            else{
                w_stones[i] = 0;
                h_stones[i] = 1;
                ++stone_score;
            }
        }
        last_time = clock();
    }
}

void draw_stones(){
    for(unsigned int i = 0; i < stone_count; ++i){
        if(h_stones[i] != 0 && w_stones[i] != 0)
            playground[h_stones[i]][w_stones[i]] = STONES_CHAR;
    }
}

void inc_stones(){
    if(clock() > stone_count * 1000000)
        ++stone_count;

}

void inc_speed(){
    if(stone_speed > 50000 && clock() > stone_count * 1000000)
        stone_speed -= 100;

}

unsigned char detect_collision(){
    if(playground[h_player][w_player] == STONES_CHAR){
        return 0;
    }
    return 1;
}

_Bool new_highscore = 0;

int main(){

    // prepare & draw playground
    clear_terminal();
    init_playground();
    draw_playground();
    read_highscore();

    wait_for_clock(100000);

    string_to_playground("READY TO PLAY (press ENTER)?");
    clear_terminal();
    draw_playground();
    wait_for_key(ENTER);
    clear_terminal();

    char key_pressed;
    while(1){

        // Increase stones
        if(stone_count < MAX_COUNT)
            inc_stones();
        else
            inc_speed();

        // Keyboard routine
        if(kbhit()){
            key_pressed = getchar();
            if(key_pressed == RIGHT)
                move_player(1);
            else if(key_pressed == LEFT)
                move_player(-1);
            else if(key_pressed == ENTER)
                break;
        }

        // Move & draw rountines
        init_playground();
        move_stones();
        draw_stones();

         // Collision check
        if(!detect_collision()){
            write_highscore();
            init_playground();
            if(new_highscore)
                string_to_playground("NEW HIGHSCORE!!");
            else
                string_to_playground("YOU'RE A LOOSER!!");
            clear_terminal();
            draw_playground();
            break;
        }

        draw_player();
        clear_terminal();
        draw_playground();
        wait_for_clock(9000);

        if(high_score < stone_score){
            high_score = stone_score;
            new_highscore = 1;
        }
    }

    return 0;
}
