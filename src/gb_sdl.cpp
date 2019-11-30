#include "gb_sdl.h"
#include "gb_cpu.h"

struct timeval t1, t2;
struct display display;
struct sdl_joypad joypad;
Sdl_emu_keys emu_keys;
unsigned int frames;

using namespace std;

unsigned char buttons;
unsigned char direction;

void sdl_init(Sdl_params p) {
  /* video init */
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_VIDEO);

  // init window
  display.screen = SDL_CreateWindow("GBcon", SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, p.scale * LCD_Width,
                                    p.scale * LCD_Height, SDL_WINDOW_OPENGL);
  if (display.screen == nullptr) {
    cerr << "failed to create sdl window" << endl;
    return;
  }

  // init renderer
  display.renderer = SDL_CreateRenderer(display.screen, -1, 0);
  if (display.renderer == nullptr) {
    cerr << "failed to create sdl renderer" << endl;
    return;
  }

  if (p.scale > 1) {
    if (SDL_RenderSetLogicalSize(display.renderer, LCD_Width, LCD_Height) < 0) {
      cerr << "failed to set SDL renderer size" << endl;
    }
  }

  // init frameBuffer
  display.frameBuffer =
      SDL_CreateTexture(display.renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, LCD_Width, LCD_Height);
  if (display.frameBuffer == nullptr) {
    cerr << "failed to create sdl texture" << endl;
    return;
  }

  frames = 0;

  /* emu keys */
  joypad.up = false;
  joypad.up = false;
  joypad.left = false;
  joypad.left = false;
  joypad.start = false;
  joypad.select = false;
  joypad.a = false;
  joypad.b = false;

  /* emu control keys */
  emu_keys.pause = false;
}

void sdl_uninit(void) {

  SDL_DestroyTexture(display.frameBuffer);
  SDL_DestroyRenderer(display.renderer);
  SDL_DestroyWindow(display.screen);

  SDL_Quit();
}

void sdl_set_frame(void) {
// print FPS
//if (frames == 0) {
//  gettimeofday(&t1, NULL);
//}

//frames++;
//if (frames % 128 == 0) {
//  gettimeofday(&t2, NULL);
//  printf("FPS: %i\n", frames / ((int)t2.tv_sec - (int)t1.tv_sec));
//}

  SDL_UpdateTexture(display.frameBuffer, NULL, display.pixels,
                    LCD_Width * sizeof(uint32_t));
  SDL_RenderClear(display.renderer);
  SDL_RenderCopy(display.renderer, display.frameBuffer, NULL, NULL);
  SDL_RenderPresent(display.renderer);
}

int sdl_update(void) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {

    switch (event.type) {
    case SDL_QUIT:
      return 1;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_UP:
        joypad.up = true;
        break;
      case SDLK_DOWN:
        joypad.down = true;
        break;
      case SDLK_LEFT:
        joypad.left = true;
        break;
      case SDLK_RIGHT:
        joypad.right = true;
        break;
      case SDLK_RETURN:
        joypad.start = true;
        break;
      case SDLK_RSHIFT:
        joypad.select = true;
        break;
      case SDLK_a:
        joypad.a = true;
        break;
      case SDLK_s:
        joypad.b = true;
        break;
      case SDLK_ESCAPE:
        return 1;
      }
      break;
    case SDL_KEYUP:
      switch (event.key.keysym.sym) {
      case SDLK_UP:
        joypad.up = false;
        break;
      case SDLK_DOWN:
        joypad.down = false;
        break;
      case SDLK_LEFT:
        joypad.left = false;
        break;
      case SDLK_RIGHT:
        joypad.right = false;
        break;
      case SDLK_RETURN:
        joypad.start = false;
        break;
      case SDLK_RSHIFT:
        joypad.select = false;
        break;
      case SDLK_a:
        joypad.a = false;
        break;
      case SDLK_s:
        joypad.b = false;
        break;

      /* emu control keys */
      case SDLK_p:
        emu_keys.pause = !(emu_keys.pause);
        break;
      case SDLK_w:
        emu_keys.save_ram = true;
        break;
      case SDLK_l:
        emu_keys.load_ram = true;
        break;
      case SDLK_ESCAPE:
        return 1;
      }
      break;
    }

    buttons = (joypad.a << 0) | (joypad.b << 1) | (joypad.select << 2) |
              (joypad.start << 3);
    direction = (joypad.right << 0) | (joypad.left << 1) | (joypad.up << 2) |
                (joypad.down << 3);

    if (event.type == SDL_QUIT)
      return 1;
  }
  return 0;
}

void set_sdl_pixels(const unsigned char *gb_pixels) {
  for (int i = 0; i < LCD_Width * LCD_Height; i++) {
    display.pixels[i] = Palette[gb_pixels[i]];
  }
}
