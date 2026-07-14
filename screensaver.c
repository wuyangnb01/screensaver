#include <am.h>
#include <amdev.h>
#include <klib-macros.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//constrain all screen 窗口的分辨率为400x300
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 300

static uint32_t *buffer;//全局像素缓冲区指针
//# 字符串化运算符
#define NAMEINIT(key)  [ AM_KEY_##key ] = #key,
static const char *names[] = {
  AM_KEYS(NAMEINIT)
};

#define FPS 1
static int speed_draw = 1;

static inline uint8_t R(uint32_t p) { return p >> 16; }//add inline keyword can unfold the function in there
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }


static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
};

static uint32_t color[8] = {0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00ffff00, 0x00ff00ff, 0x0000ffff, 0x00ffffff};


void draw(uint32_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        buffer[i] = color;
    }
    io_write(AM_GPU_FBDRAW, 0, 0, buffer, SCREEN_WIDTH, SCREEN_HEIGHT, true);
}

int check_key() {
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    if (ev.keycode == AM_KEY_NONE) return 0;
    if (ev.keycode == AM_KEY_ESCAPE && ev.keydown) {
        return -1;
    }
    if (ev.keydown && ev.keycode != AM_KEY_ESCAPE) {
        printf("Got key: %s (%d)\n", names[ev.keycode], ev.keycode);
        speed_draw = 10;
        return 1;
    }
    return 0;
}

void draw_color() {
    buffer = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    
    unsigned long last = 0;
    unsigned long fps_last = 0;
    int fps = 0;
    uint32_t current_color = color[0];
    
    while (1) {
        unsigned long upt = io_read(AM_TIMER_UPTIME).us / 1000;
        uint32_t target_color = color[rand() % 7 + 1];
        int key_ret = check_key();
        if (key_ret == -1) {
            free(buffer);
            return;
        }
        
        if (upt - last > 1000 / (FPS * speed_draw)) {
            int k=0,i=0;
            for(i=1;i<8;i++)
            {
                if(target_color == color[i])
                {
                    k=i;
                    break;
                }
            }
            for (int i = 1; i <= k; i++) {
                uint32_t r = R(current_color) + i * (R(target_color) - R(current_color)) / k;
                uint32_t g = G(current_color) + i * (G(target_color) - G(current_color)) / k;
                uint32_t b = B(current_color) + i * (B(target_color) - B(current_color)) / k;
                draw(pixel(r, g, b));
            }
            
            last = upt;
            fps++;
        }
        
        if (upt - fps_last > 1000) {
            printf("fps: %d, speed: %dx\n", fps, speed_draw);
            fps_last = upt;
            fps = 0;
            current_color = target_color;
        }
        if (speed_draw > 1 && upt - last > 5000) {
            speed_draw = 1; 
            printf("Speed restored to normal\n");
        }   
        yield();
    }
}

int main() {
    srand(time(NULL));
    ioe_init();
    printf("Screen saver started.\n");
    printf("Press any key to speed up, ESC to exit.\n");
    draw_color();
    free(buffer);
    yield();
    return 0;
}