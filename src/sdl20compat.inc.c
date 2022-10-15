#include<assert.h>

//dummy values
enum
{
    SDL_PHYSPAL     = 1,
    SDL_LOGPAL      = 2,
    SDL_SRCCOLORKEY = 4,
    SDL_HWPALETTE   = 8,
};

static SDL_Surface*  sdl2_screen     = NULL;
static SDL_Texture*  sdl2_screen_tex = NULL;
static SDL_Window*   sdl2_window     = NULL;
static SDL_Renderer* sdl2_rendr      = NULL;

static SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int
        rmask = 0xff000000,
        gmask = 0x00ff0000,
        bmask = 0x0000ff00,
        amask = 0x000000ff;
#else
    int
        rmask = 0x000000ff,
        gmask = 0x0000ff00,
        bmask = 0x00ff0000,
        amask = 0xff000000;
#endif
    if (!sdl2_window)
    {
#if defined (__NGAGE__) || defined (NGAGE_DEBUG)
        sdl2_window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
#else
        sdl2_window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE);
#endif
        if (!sdl2_window)
        {
            goto die;
        }
#if defined (__NGAGE__) || defined (NGAGE_DEBUG)
        sdl2_rendr = SDL_CreateRenderer(sdl2_window, -1, SDL_RENDERER_SOFTWARE);
#else
        sdl2_rendr = SDL_CreateRenderer(sdl2_window, -1, 0);
        SDL_RenderSetLogicalSize(sdl2_rendr, width, height);
#endif
        if (!sdl2_rendr)
        {
            goto die;
        }
        sdl2_screen_tex = SDL_CreateTexture(sdl2_rendr, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

        if (0)
        {
        die:
            if (sdl2_window)
            {
                SDL_DestroyWindow(sdl2_window);
            }
            if (sdl2_screen_tex)
            {
                SDL_DestroyTexture(sdl2_screen_tex);
            }
            if (sdl2_rendr)
            {
                SDL_DestroyRenderer(sdl2_rendr);
            }
            return NULL;
        }
    }
    sdl2_screen = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
    assert(sdl2_screen && sdl2_screen->format->BitsPerPixel == bpp);
    return sdl2_screen;
}

static void SDL_SetPalette(SDL_Surface* surf, int flag, SDL_Color* pal, int begin, int count)
{
    (void)surf;
    (void)flag;
    (void)pal;
    (void)begin;
    (void)count;
}
static void SDL_WM_SetCaption(const char* title, const char* icon)
{
    assert(sdl2_window != NULL);
    SDL_SetWindowTitle(sdl2_window, title);
    (void)icon;
}

static int SDL_WM_ToggleFullScreen(SDL_Surface* screen)
{
    static int fullscreen = 0;
    assert(screen == sdl2_screen);
    assert(sdl2_window != NULL);
    fullscreen = !fullscreen;
    return SDL_SetWindowFullscreen(sdl2_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) == 0;
}

static SDL_Surface* SDL_GetVideoSurface(void)
{
    return sdl2_screen;
}

static void SDL_Flip(SDL_Surface* screen)
{
#if defined (__NGAGE__)
    SDL_Rect source = { 0, 0, 128, 128 };
    SDL_Rect dest   = { 24, 40, 128, 128 };
#elif defined (NGAGE_DEBUG)
    SDL_Rect source = { 0, 0, 384, 384 };
    SDL_Rect dest   = { 72, 120, 384, 384 };
#endif

    assert(screen == sdl2_screen);
    assert(sdl2_window != NULL);
    SDL_UpdateTexture(sdl2_screen_tex, NULL, screen->pixels, screen->pitch);
    SDL_SetRenderDrawColor(sdl2_rendr, 0, 0, 0, 255);
    SDL_RenderClear(sdl2_rendr);
#if defined (__NGAGE__) || defined (NGAGE_DEBUG)
    SDL_RenderCopy(sdl2_rendr, sdl2_screen_tex, &source, &dest);
#else
    SDL_RenderCopy(sdl2_rendr, sdl2_screen_tex, NULL, NULL);
#endif
    SDL_RenderPresent(sdl2_rendr);
}

#define SDL_GetKeyState SDL_GetKeyboardState
//the above function now returns array indexed by scancodes, so we need to use those constants
#if defined (__NGAGE__)
// Reset
#define SDLK_F9         SDL_SCANCODE_BACKSPACE
// Pause
#define SDLK_ESCAPE     SDL_SCANCODE_SOFTRIGHT
// Quit
#define SDLK_DELETE     SDL_SCANCODE_SOFTLEFT
// Save state
#define SDLK_s          SDL_SCANCODE_1
// Load state
#define SDLK_d          SDL_SCANCODE_2
// Toggle screenshake
#define SDLK_e          SDL_SCANCODE_3
// Jump
#define SDLK_z          SDL_SCANCODE_7
// Dash
#define SDLK_x          SDL_SCANCODE_5
// Directional controls
#define SDLK_LEFT       SDL_SCANCODE_LEFT
#define SDLK_RIGHT      SDL_SCANCODE_RIGHT
#define SDLK_UP         SDL_SCANCODE_UP
#define SDLK_DOWN       SDL_SCANCODE_DOWN
#else
#define SDLK_F9         SDL_SCANCODE_F9
#define SDLK_ESCAPE     SDL_SCANCODE_ESCAPE
#define SDLK_DELETE     SDL_SCANCODE_DELETE
#define SDLK_F11        SDL_SCANCODE_F11
#define SDLK_LSHIFT     SDL_SCANCODE_LSHIFT
#define SDLK_LEFT       SDL_SCANCODE_LEFT
#define SDLK_RIGHT      SDL_SCANCODE_RIGHT
#define SDLK_UP         SDL_SCANCODE_UP
#define SDLK_DOWN       SDL_SCANCODE_DOWN
#define SDLK_5          SDL_SCANCODE_5
#define SDLK_s          SDL_SCANCODE_S
#define SDLK_d          SDL_SCANCODE_D
#define SDLK_c          SDL_SCANCODE_C
#define SDLK_x          SDL_SCANCODE_X
#define SDLK_n          SDL_SCANCODE_N
#define SDLK_a          SDL_SCANCODE_A
#define SDLK_b          SDL_SCANCODE_B
#define SDLK_z          SDL_SCANCODE_Z
#define SDLK_e          SDL_SCANCODE_E
#define SDLK_v          SDL_SCANCODE_V
#endif

#define sym scancode // SDL_Keysym.sym -> SDL_Keysym.scancode
