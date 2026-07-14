#include <am.h>
#include <amdev.h>
#include <klib-macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//constrain all screen 窗口的分辨率为400x300
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 300

//static uint32_t *buffer;//全局像素缓冲区指针
//# 字符串化运算符
//#define NAMEINIT(key)  [ AM_KEY_##key ] = #key,
//static const char *names[] = {
 // AM_KEYS(NAMEINIT)
//};
#define N 32

static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}
//#define FPS 1

static inline uint8_t R(uint32_t p) { return p >> 16; }//add inline keyword can unfold the function in there
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }

static uint32_t canvas[N][N];
static uint32_t color_buf[N * N];

static uint32_t colors[8] = {
    0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff,
    0x00ffff00, 0x00ff00ff, 0x0000ffff, 0x00ffffff
};

void redraw() {
    int w = io_read(AM_GPU_CONFIG).width / N;
    int h = io_read(AM_GPU_CONFIG).height / N;
   // int w_plus = io_read(AM_GPU_CONFIG).width % N;
   // int h_plus = io_read(AM_GPU_CONFIG).height % N;
    int block_size = w * h;
    int x, y, k;

    for (y = 0; y < N; y++) {
        for (x = 0; x < N; x++) {
            for (k = 0; k < block_size; k++) {
                color_buf[k] = canvas[y][x];
            }
            io_write(AM_GPU_FBDRAW, x * w, y * h, color_buf, w, h, false);
        }
    }
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
}

void draw(uint32_t color_val) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            canvas[i][j] = color_val;
        }
    }
    redraw();
}

int main(const char *args) {
    srand(time(NULL));
    ioe_init();
    
    AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
    cfg.width = SCREEN_WIDTH;
    cfg.height = SCREEN_HEIGHT;
    io_write(AM_GPU_CONFIG, true, false,cfg.width,cfg.height,0);
    cfg = io_read(AM_GPU_CONFIG);
    printf("Screen: %dx%d\n", cfg.width, cfg.height);
    
    uint32_t current_color = colors[0];
    uint32_t target_color = colors[1];
    int speed = 1;
    int steps = 20;
    int current_step = 0;
    unsigned long last = 0;
    unsigned long fps_last = 0;
    int fps = 0;
    
    draw(current_color);
    printf("Initial draw done\n");
    
    while (1) {
        unsigned long upt = io_read(AM_TIMER_UPTIME).us / 1000;
        
        AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
        if (ev.keycode == AM_KEY_ESCAPE && ev.keydown) {
            printf("ESC pressed, exiting\n");
            return 0;
        }
        if (ev.keycode != AM_KEY_NONE && ev.keycode != AM_KEY_ESCAPE) {
            if (ev.keydown) {
                speed = 5;
            } else {
                speed = 1;
            }
        }
        
        unsigned long interval = 60 / speed;
        if (upt - last > interval) {
            if (current_step <= steps) {
                uint32_t r = R(current_color) + current_step * (R(target_color) - R(current_color)) / steps;
                uint32_t g = G(current_color) + current_step * (G(target_color) - G(current_color)) / steps;
                uint32_t b = B(current_color) + current_step * (B(target_color) - B(current_color)) / steps;
                draw(pixel(r, g, b));
                current_step++;
                fps++;
            } else {
                current_color = target_color;
                do {
                    target_color = colors[rand() % 8];
                } while (target_color == current_color);
                current_step = 0;
            }
            last = upt;
        }
        
        if (upt - fps_last > 1000) {
            printf("FPS = %d, speed = %dx\n", fps, speed);
            fps_last = upt;
            fps = 0;
        }
    }
    
    return 0;
}