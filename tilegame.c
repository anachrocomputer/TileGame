/* tilegame --- tile-based game in SDL                      2016-12-04 */

#include <SDL2/SDL.h> 
#include <stdio.h> 

#include "CHGUK01.h"

#define UK101_NCOLS    (48)
#define UK101_NROWS    (16)

#define UK101_CHARWD   (8)
#define UK101_CHARHT   (8)

#define XSCALE (2)
#define YSCALE (4)

/* Window size */
#define SCREEN_WIDTH   (UK101_NCOLS * UK101_CHARWD * XSCALE)
#define SCREEN_HEIGHT  (UK101_NROWS * UK101_CHARHT * YSCALE)

int CursorX = 0;
int CursorY = 0;

void printUK101(const SDL_Surface *screenSurface, const char str[], const SDL_Surface *chgen);
void drawUK101CharAt(const SDL_Surface *screenSurface, const int ch, const int col, const int row, const SDL_Surface *chgen);

int main(int argc, char *argv[])
{
   SDL_Joystick *joystick = NULL;   /* The SDL joystick */
   SDL_Window *window = NULL;       /* The SDL window */

   /* The surface contained by the window */
   SDL_Surface *screenSurface = NULL;

   SDL_Surface *pixelImage = NULL;
   Uint32 white, blue, black, grey; /* Precomputed colours */
   Uint32 red, yellow, green;       /* Precomputed colours */
   Uint32 chgWhite, chgBlack;
#if 0
   Uint32 transparent;
#endif
   int nb;
   int jb[16];
   SDL_Rect jbut[16];
   int na;
   int ja[16];
   SDL_Rect jaxis[16], jposn[16];
   int nh;
   SDL_Event e;
   int running = 1;
   int i;
   int hlx, hly;
   int ch, col, row;
   Uint32 rmask, gmask, bmask, amask;

   /* SDL interprets each pixel as a 32-bit number, so our masks must depend
      on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   rmask = 0xff000000;
   gmask = 0x00ff0000;
   bmask = 0x0000ff00;
   amask = 0x000000ff;
#else
   rmask = 0x000000ff;
   gmask = 0x0000ff00;
   bmask = 0x00ff0000;
   amask = 0xff000000;
#endif

   hlx = 0;
   hly = 0;
      
   /* Initialise Simple DirectMedia Layer */
   if (SDL_Init(SDL_INIT_EVERYTHING)) {
      printf("SDL could not initialise! SDL_Error: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
   }                    

   for (i = 0; i < 16; i++) {
      ja[i] = 0;
      jaxis[i].x = 10 + (16 * i);
      jaxis[i].y = 30;
      jaxis[i].w = 10;
      jaxis[i].h = 128 + 10;
      
      jposn[i].x = jaxis[i].x;
      jposn[i].y = jaxis[i].y;
      jposn[i].w = jaxis[i].w;
      jposn[i].h = 10;
   }
   
   for (i = 0; i < 16; i++) {
      jb[i] = 0;
      jbut[i].x = 10 + (16 * i);
      jbut[i].y = 10;
      jbut[i].w = 10;
      jbut[i].h = 10;
   }
   
   /* Check for joysticks */
   if (SDL_NumJoysticks() < 1) {
      printf("Warning: No joysticks connected!\n");
      na = nb = nh = 0;
   }
   else { 
      /* Open joystick */
      joystick = SDL_JoystickOpen(0);
      if (joystick == NULL) { 
         printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
         na = nb = nh = 0;
      }
      else {
#ifdef DB
         printf("Name: %s\n", SDL_JoystickNameForIndex(0));
         printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joystick));
         printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joystick));
         printf("Number of Hats: %d\n", SDL_JoystickNumHats(joystick));
#endif
         
         na = SDL_JoystickNumAxes(joystick);
         nb = SDL_JoystickNumButtons(joystick);
         nh = SDL_JoystickNumHats(joystick);
      }
   }

   /* Create SDL window */
   window = SDL_CreateWindow("Compukit UK101", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

   if (window == NULL) { 
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
      SDL_Quit();
      exit (EXIT_FAILURE);
   }               

   /* Get window surface */
   screenSurface = SDL_GetWindowSurface(window); 

   /* Allocate an image in the same format as our display */
#ifdef TOONEW
   if ((pixelImage = SDL_CreateRGBSurfaceWithFormat(0, 8, 256 * 8, screenSurface->format->BitsPerPixel, screenSurface->format->format)) == NULL) {
      printf("Error: could not allocate 'pixelImage': %s.\n", SDL_GetError());
   }
#else
   pixelImage = SDL_CreateRGBSurface(0, UK101_CHARWD * XSCALE, 256 * UK101_CHARHT * YSCALE, screenSurface->format->BitsPerPixel,
                                   rmask, gmask, bmask, amask);

#endif

   printf("Size of 'pixelImage' is %dx%d\n", pixelImage->w, pixelImage->h);
   
   /* Generate some colours */
   white  = SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0xff);
   blue   = SDL_MapRGB(screenSurface->format, 0x40, 0x40, 0xff);
   black  = SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00);
   grey   = SDL_MapRGB(screenSurface->format, 0x80, 0x80, 0x80);
   red    = SDL_MapRGB(screenSurface->format, 0xff, 0x40, 0x40);
   yellow = SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0x40);
   green  = SDL_MapRGB(screenSurface->format, 0x40, 0xff, 0x40);
   
#if 0
   transparent = SDL_MapRGB(pixelImage->format, 0xff, 0x00, 0xff);
   
   if (SDL_SetColorKey(pixelImage, SDL_TRUE, transparent) < 0) {
      printf("SDL_SetColorKey failed: %s\n", SDL_GetError());
   }
#endif
   
   chgWhite  = SDL_MapRGB(pixelImage->format, 0xff, 0xff, 0xff);
   chgBlack  = SDL_MapRGB(pixelImage->format, 0x00, 0x00, 0x00);
   
   /* Generate the UK101 font */
   for (ch = 0; ch < 256; ch++) {
      for (col = 0; col < UK101_CHARHT; col++) {
         const int bits = TileGlyph[ch][col];
         SDL_Rect dest;
         int b;
         
         for (b = 0; b < UK101_CHARWD; b++) {
             dest.x = col * XSCALE;
             dest.y = ((ch * UK101_CHARHT) + b) * YSCALE;
             dest.w = XSCALE;
             dest.h = YSCALE;
             
             if (bits & (1 << b)) {
                 SDL_FillRect(pixelImage, &dest, chgWhite);
             }
             else {
                 SDL_FillRect(pixelImage, &dest, chgBlack);
             }
         }
      }
   }

   /* Main loop */
   do {
      while (SDL_PollEvent(&e) != 0) {
#ifdef DB
         printf("EVENT: %d ", e.type);
#endif

         if (e.type == SDL_QUIT) {
#ifdef DB
            printf("QUIT\n");
#endif
            running = 0;
         }
         else if (e.type == SDL_WINDOWEVENT) {
#ifdef DB
            printf("WINDOW EVENT %d\n", e.window.event);
#endif
         }
         else if (e.type == SDL_MOUSEMOTION) {
#ifdef DB
            printf("MOUSE %d TO %d,%d\n", e.motion.which, e.motion.x, e.motion.y);
#endif
         }
         else if (e.type == SDL_MOUSEBUTTONDOWN) {
#ifdef DB
            printf("MOUSE %d BUTTON %d DOWN\n", e.button.which, e.button.button);
#endif
         }
         else if (e.type == SDL_MOUSEBUTTONUP) {
#ifdef DB
            printf("MOUSE %d BUTTON %d UP\n", e.button.which, e.button.button);
#endif
         }
         else if (e.type == SDL_JOYAXISMOTION) {
#ifdef DB
            printf("JOYSTICK %d MOVED AXIS %d TO %d\n", e.jaxis.which, e.jaxis.axis, e.jaxis.value);
#endif
            if (e.jaxis.which == 0) {
               ja[e.jaxis.axis] = e.jaxis.value;
               
               switch (e.jaxis.axis) {
               case 0:
                  if (e.jaxis.value < -8000) {
                     if (hlx > 0)
                        hlx--;
                  }
                  else if (e.jaxis.value > 8000) {
                     if (hlx < 7)
                        hlx++;
                  }
                  break;
               case 1:
                  if (e.jaxis.value < -8000) {
                     if (hly < 7)
                        hly++;
                  }
                  else if (e.jaxis.value > 8000) {
                     if (hly > 0)
                        hly--;
                  }
                  break;
               case 2:
                  break;
               case 3:
                  break;
               }
            }
         }
         else if (e.type == SDL_JOYHATMOTION) {
#ifdef DB
            printf("JOYSTICK %d HAT %d TO %d\n", e.jhat.which, e.jhat.hat, e.jhat.value);
#endif
         }
         else if (e.type == SDL_JOYBUTTONDOWN) {
#ifdef DB
            printf("JOYSTICK %d BUTTON %d DOWN\n", e.jbutton.which, e.jbutton.button);
#endif
            jb[e.jbutton.button] = 1;
            switch (e.jbutton.button) {
            case 0:
               if (hlx < 7)
                  hlx++;
               
               if (hly < 7)
                  hly++;

               break;
            case 1:
               if (hlx < 7)
                  hlx++;
               
               if (hly > 0)
                  hly--;
                  
               break;
            case 2:
               if (hlx > 0)
                  hlx--;
                  
               if (hly > 0)
                  hly--;
                  
               break;
            case 3:
               if (hlx > 0)
                  hlx--;

               if (hly < 7)
                  hly++;
                  
               break;
            case 4:
               break;
            case 8:
               running = 0;
               break;
            }
         }
         else if (e.type == SDL_JOYBUTTONUP) {
#ifdef DB
            printf("JOYSTICK %d BUTTON %d UP\n", e.jbutton.which, e.jbutton.button);
#endif
            jb[e.jbutton.button] = 0;
         }
         else if (e.type == SDL_KEYDOWN) {
#ifdef DB
            printf("KEYBOARD %d DOWN\n", e.key.keysym.sym);
#endif
            
            switch (e.key.keysym.sym) {
            case SDLK_UP:
               if (hly < 7)
                  hly++;

               break;
            case SDLK_DOWN:
               if (hly > 0)
                  hly--;

               break;
            case SDLK_LEFT:
               if (hlx > 0)
                  hlx--;
                  
               break;
            case SDLK_RIGHT:
               if (hlx < 7)
                  hlx++;
                  
               break;
            case SDLK_q:
               running = 0;
               break;
            }
         }
         else if (e.type == SDL_KEYUP) {
#ifdef DB
            printf("KEYBOARD %d UP\n", e.key.keysym.sym);
#endif
         }
         else {
#ifdef DB
            printf("UNHANDLED\n");
#endif
         }
      }
      
#ifdef DB
      /* Fill the surface white */
      SDL_FillRect(screenSurface, NULL, white);
      
      /* Display joystick buttons */
      for (i = 0; i < 16; i++) {
         if (i < nb) {
            if (jb[i])
               SDL_FillRect(screenSurface, &jbut[i], blue);
            else
               SDL_FillRect(screenSurface, &jbut[i], black);
         }
         else
            SDL_FillRect(screenSurface, &jbut[i], grey);
      }
      
      /* Display joystick axes */
      for (i = 0; i < 16; i++) {
         if (i < na) {
            SDL_FillRect(screenSurface, &jaxis[i], black);
            jposn[i].y = jaxis[i].y + (ja[i] / 512) + 64;
            SDL_FillRect(screenSurface, &jposn[i], blue);
         }
         else
            SDL_FillRect(screenSurface, &jaxis[i], grey);
      }
#endif
      
      /* Home cursor */
      CursorX = 0;
      CursorY = 0;
      
      /* Clear screen */
      for (row = 0; row < UK101_NROWS; row++) {
          for (col = 0; col < UK101_NCOLS; col++) {
             const int asc = ' ';
             
             drawUK101CharAt(screenSurface, asc, col, row, pixelImage);
          }
      }

      /* Draw some text */
      printUK101(screenSurface, "MEMORY SIZE?", pixelImage);
      printUK101(screenSurface, "TERMINAL WIDTH?", pixelImage);
      printUK101(screenSurface, "", pixelImage);
      printUK101(screenSurface, " 7423 BYTES FREE", pixelImage);
      printUK101(screenSurface, "", pixelImage);
      printUK101(screenSurface, "C O M P U K I T  U K 1 0 1", pixelImage);
      printUK101(screenSurface, "", pixelImage);
      printUK101(screenSurface, "Personal Computer", pixelImage);
      printUK101(screenSurface, "", pixelImage);
      printUK101(screenSurface, "8K Basic Copyright1979", pixelImage);
      printUK101(screenSurface, "OK", pixelImage);
      
      /* Draw complete UK101 character set */
      for (row = 0; row < 16; row++) {
          for (col = 0; col < 16; col++) {
             const int asc = row * 16 + col;
             
             drawUK101CharAt(screenSurface, asc, col + 32, row, pixelImage);
          }
      }

      /* Update the surface */
      SDL_UpdateWindowSurface(window);

      SDL_Delay(40);
   } while (running);
   
   /* Destroy bitmaps */
   SDL_FreeSurface(pixelImage);
   
   /* Destroy window */
   SDL_DestroyWindow(window);

   SDL_JoystickClose(joystick);
   
   /* Quit SDL subsystem */
   SDL_Quit();

   return (EXIT_SUCCESS);
}


void printUK101(const SDL_Surface *screenSurface, const char str[], const SDL_Surface *chgen)
{
    int i;

    CursorX = 0;
        
    for (i = 0; str[i] != '\0'; i++) {
        drawUK101CharAt(screenSurface, str[i], CursorX++, CursorY, chgen);
    }
    
    CursorY++;
}


void drawUK101CharAt(const SDL_Surface *screenSurface, const int ch, const int col, const int row, const SDL_Surface *chgen)
{
   SDL_Rect src, dest;

   src.x = 0;
   src.y = ch * UK101_CHARHT * YSCALE;
   src.w = UK101_CHARWD * XSCALE;
   src.h = UK101_CHARHT * YSCALE;

   dest.x = col * UK101_CHARWD * XSCALE;
   dest.y = (SCREEN_HEIGHT - 512) + (row * UK101_CHARHT * YSCALE);

   SDL_BlitSurface(chgen, &src, screenSurface, &dest);
}
