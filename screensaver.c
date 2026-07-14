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
        speed_factor = 10;
        return 1;
    }
    return 0;
}

void draw_color() {
    buffer = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    
    
    if(flag_qk) 
    {
        unsigned long last = 0;
    }else {
        unsigned long last = 200;
    }
    unsigned long fps_last = 0;
    int fps = 0;
    

    while (1) {
        unsigned long upt = io_read(AM_TIMER_UPTIME).us / 1000;//convert to ms
        uint32_t init_color = color[0];
        uint32_t target_color = color[rand() % 8];
        uint32_t color_buf[8];
        int i=0,k=0;
        for (i = 0; i < 8; i++) {
            if (color[i] == target_color) {
                k=i;
                break;
            }
        }
        for(i=1;i<k;i++){
            color_buf[i]=pixel(R(init_color)+i*(R(target_color)-R(init_color))/k,
                            G(init_color)+i*(G(target_color)-G(init_color))/k,
                            B(init_color)+i*(B(target_color)-B(init_color))/k);
           if(upt - last > 1000 / FPS) {
                draw(color_buf[i]);
                last = upt;
                fps ++;
           }
           if(upt - fps_last > 1000) {// ensure 1s display fps only once
                // display fps every 1s
                printf("fps: %d\n", fps);
                fps_last = upt;
                fps = 0;
           }
        }
    }
    free(buffer);
}



int keys() {
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    if (ev.keycode == AM_KEY_NONE || ev.keycode == AM_KEY_ESCAPE) return 0;
    printf("Got  (kbd): %s (%d) %s\n", names[ev.keycode], ev.keycode, ev.keydown ? "DOWN" : "UP");
    if(ev.keydown && ev.keycode != AM_KEY_ESCAPE) {
        draw_color(1);
        return 1;
    }
}

void key() {
    printf("Try to press  key \n");
    printf("If you press ESC ,this process will exit.\n");
    printf("And when you press any keys ,you should see the change be quickened.\n");
    while (1) {
      int ret = keys();
      if(ret) break;
      else {
        return;
      }
      yield();
    }
}



int main() {
    srand(time(NULL));
    ioe_init();
    key();
    draw_color(0);
    yield();
    return 0;
}