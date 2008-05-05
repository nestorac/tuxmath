/*
*  C Implementation: SDL_extras
*
* Description: a few handy functions for using SDL graphics.
*
*
* Author: David Bruce,,, <dbruce@tampabay.rr.com>, (C) 2007
*
* Copyright: GPL v3 or later
*
*/

#include "SDL_extras.h"
#include "tuxmath.h"

#ifdef SDL_Pango
#include "SDL_Pango.h"
#endif


/* DrawButton() creates and draws a translucent button with */
/* rounded ends.  All colors and alpha values are supported.*/
void DrawButton(SDL_Rect* target_rect,
                int radius,
                Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  /* NOTE - we use a 32-bit temp surface even if we have a 16-bit */
  /* screen - it gets converted during blitting.                  */
  SDL_Surface* tmp_surf = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
                                          target_rect->w,
                                          target_rect->h,
                                          32, 
                                          rmask, gmask, bmask, amask);
  Uint32 color = SDL_MapRGBA(tmp_surf->format, r, g, b, a);
  SDL_FillRect(tmp_surf, NULL, color);
  RoundCorners(tmp_surf, radius);

  SDL_BlitSurface(tmp_surf, NULL, screen, target_rect);
  SDL_FreeSurface(tmp_surf);
}



void RoundCorners(SDL_Surface* s, Uint16 radius)
{
  int y = 0;
  int x_dist, y_dist;
  Uint32* p = NULL;
  Uint32 alpha_mask;
  int bytes_per_pix;
  
  if (!s)
    return;
  if (SDL_LockSurface(s) == -1)
    return;

  bytes_per_pix = s->format->BytesPerPixel;
  if (bytes_per_pix != 4)
    return;

  /* radius cannot be more than half of width or height: */
  if (radius > (s->w)/2)
    radius = (s->w)/2;
  if (radius > (s->h)/2)
    radius = (s->h)/2;


  alpha_mask = s->format->Amask;

  /* Now round off corners: */
  /* upper left:            */
  for (y = 0; y < radius; y++) 
  {  
    p = (Uint32*)(s->pixels + (y * s->pitch));
    x_dist = radius;
    y_dist = radius - y;

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p++;
      x_dist--;
    }
  }

  /* upper right:            */
  for (y = 0; y < radius; y++) 
  {  
    /* start at end of top row: */
    p = (Uint32*)(s->pixels + ((y + 1) * s->pitch) - bytes_per_pix);

    x_dist = radius;
    y_dist = radius - y;

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p--;
      x_dist--;
    }
  }

  /* bottom left:            */
  for (y = (s->h - 1); y > (s->h - radius); y--) 
  {  
    /* start at beginning of bottom row */
    p = (Uint32*)(s->pixels + (y * s->pitch));
    x_dist = radius;
    y_dist = y - (s->h - radius);

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p++;
      x_dist--;
    }
  }

  /* bottom right:            */
  for (y = (s->h - 1); y > (s->h - radius); y--) 
  {  
    /* start at end of bottom row */
    p = (Uint32*)(s->pixels + ((y + 1) * s->pitch) - bytes_per_pix);
    x_dist = radius;
    y_dist = y - (s->h - radius);

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p--;
      x_dist--;
    }
  }
  SDL_UnlockSurface(s);
} 


/**********************
 Flip:
   input: a SDL_Surface, x, y
   output: a copy of the SDL_Surface flipped via rules:

     if x is a positive value, then flip horizontally
     if y is a positive value, then flip vertically

     note: you can have it flip both
**********************/
SDL_Surface* Flip( SDL_Surface *in, int x, int y ) {
	SDL_Surface *out, *tmp;
	SDL_Rect from_rect, to_rect;
	Uint32	flags;
	Uint32  colorkey=0;

	/* --- grab the settings for the incoming pixmap --- */

	SDL_LockSurface(in);
	flags = in->flags;

	/* --- change in's flags so ignore colorkey & alpha --- */

	if (flags & SDL_SRCCOLORKEY) {
		in->flags &= ~SDL_SRCCOLORKEY;
		colorkey = in->format->colorkey;
	}
	if (flags & SDL_SRCALPHA) {
		in->flags &= ~SDL_SRCALPHA;
	}

	SDL_UnlockSurface(in);

	/* --- create our new surface --- */

	out = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		in->w, in->h, 32, rmask, gmask, bmask, amask);

	/* --- flip horizontally if requested --- */

	if (x) {
		from_rect.h = to_rect.h = in->h;
		from_rect.w = to_rect.w = 1;
		from_rect.y = to_rect.y = 0;
		from_rect.x = 0;
		to_rect.x = in->w - 1;

		do {
			SDL_BlitSurface(in, &from_rect, out, &to_rect);
			from_rect.x++;
			to_rect.x--;
		} while (to_rect.x >= 0);
	}

	/* --- flip vertically if requested --- */

	if (y) {
		from_rect.h = to_rect.h = 1;
		from_rect.w = to_rect.w = in->w;
		from_rect.x = to_rect.x = 0;
		from_rect.y = 0;
		to_rect.y = in->h - 1;

		do {
			SDL_BlitSurface(in, &from_rect, out, &to_rect);
			from_rect.y++;
			to_rect.y--;
		} while (to_rect.y >= 0);
	}

	/* --- restore colorkey & alpha on in and setup out the same --- */

	SDL_LockSurface(in);

	if (flags & SDL_SRCCOLORKEY) {
		in->flags |= SDL_SRCCOLORKEY;
		in->format->colorkey = colorkey;
		tmp = SDL_DisplayFormat(out);
		SDL_FreeSurface(out);
		out = tmp;
		out->flags |= SDL_SRCCOLORKEY;
		out->format->colorkey = colorkey;
	} else if (flags & SDL_SRCALPHA) {
		in->flags |= SDL_SRCALPHA;
		tmp = SDL_DisplayFormatAlpha(out);
		SDL_FreeSurface(out);
		out = tmp;
	} else {
		tmp = SDL_DisplayFormat(out);
		SDL_FreeSurface(out);
		out = tmp;
	}

	SDL_UnlockSurface(in);

	return out;
}

/* Blend two surfaces together. The third argument is between 0.0 and
   1.0, and represents the weight assigned to the first surface.  If
   the pointer to the second surface is NULL, this performs fading.

   Currently this works only with RGBA images, but this is largely to
   make the (fast) pointer arithmetic work out; it could be easily
   generalized to other image types. */
SDL_Surface* Blend(SDL_Surface *S1,SDL_Surface *S2,float gamma)
{
  SDL_PixelFormat *fmt1,*fmt2;
  Uint8 r1,r2,g1,g2,b1,b2,a1,a2;
  SDL_Surface *tmpS,*ret;
  Uint32 *cpix1,*epix1,*cpix2,*epix2;
  float gamflip;

  gamflip = 1.0-gamma;
  if (gamma < 0 || gamflip < 0) {
    perror("gamma must be between 0 and 1");
    exit(0);
  }
  fmt1 = S1->format;
  if (fmt1->BitsPerPixel != 32) {
    perror("This works only with RGBA images");
    return S1;
  }
  if (S2 != NULL) {
    fmt2 = S2->format;
    if (fmt2->BitsPerPixel != 32) {
    perror("This works only with RGBA images");
    return S1;
    }
    // Check that both images have the same width dimension
    if (S1->w != S2->w) {
      printf("S1->w %d, S2->w %d;  S1->h %d, S2->h %d\n",
	     S1->w,S2->w,S1->h,S2->h);
      printf("Both images must have the same width dimensions\n");
      return S1;
    }
  }

  tmpS = SDL_ConvertSurface(S1,fmt1,SDL_SWSURFACE);
  SDL_LockSurface(tmpS);
  // We're going to go through the pixels in reverse order, to start
  // from the bottom of each image. That way, we can blend things that
  // are not of the same height and have them align at the bottom.
  // So the "ending pixel" (epix) will be before the first pixel, and
  // the current pixel (cpix) will be the last pixel.
  epix1 = (Uint32*) tmpS->pixels-1;
  cpix1 = epix1 + tmpS->w*tmpS->h;
  if (S2 != NULL) {
    SDL_LockSurface(S2);
    epix2 = (Uint32*) S2->pixels-1;
    cpix2 = epix2 + S2->w*S2->h;
  } else {
    epix2 = epix1;
    cpix2 = cpix1;
  }

  for (; cpix1 > epix1; cpix1--,cpix2--) {
    SDL_GetRGBA(*cpix1,fmt1,&r1,&g1,&b1,&a1);
    a1 = gamma*a1;
    if (S2 != NULL && cpix2 > epix2) {
      SDL_GetRGBA(*cpix2,fmt2,&r2,&g2,&b2,&a2);
      r1 = gamma*r1 + gamflip*r2;
      g1 = gamma*g1 + gamflip*g2;
      b1 = gamma*b1 + gamflip*b2;
      a1 += gamflip*a2;
    }
    *cpix1 = SDL_MapRGBA(fmt1,r1,g1,b1,a1);
  }
  SDL_UnlockSurface(tmpS);
  if (S2 != NULL)
    SDL_UnlockSurface(S2);

  ret = SDL_DisplayFormatAlpha(tmpS);
  SDL_FreeSurface(tmpS);

  return ret;
}

#ifdef SDL_Pango
SDLPango_Context *context = NULL;
void init_SDLPango_Context()
{
   context =  SDLPango_CreateContext_GivenFontDesc(DEFAULT_FONT_NAME);
}  
void free_SDLPango_Context() 
{
  if(context != NULL)
    SDLPango_FreeContext(context);
  context = NULL;
}
#endif
/* BlackOutline() creates a surface containing text of the designated */
/* foreground color, surrounded by a black shadow, on a transparent    */
/* background.  The appearance can be tuned by adjusting the number of */
/* background copies and the offset where the foreground text is       */
/* finally written (see below).                                        */
SDL_Surface* BlackOutline(unsigned char *t, TTF_Font *font, SDL_Color *c)
{
  SDL_Surface* out = NULL;
  SDL_Surface* black_letters = NULL;
  SDL_Surface* white_letters = NULL;
  SDL_Surface* bg = NULL;
  SDL_Rect dstrect;
  Uint32 color_key;

  if (!t || !font || !c)
  {
    fprintf(stderr, "BlackOutline(): invalid ptr parameter, returning.");
    return NULL;
  }

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nEntering BlackOutline(): \n");
  fprintf( stderr, "BlackOutline of \"%s\"\n", t );
#endif

#ifndef SDL_Pango
  black_letters = TTF_RenderUTF8_Blended(font, t, black);
#else
  if( context != NULL)
  {
    SDLPango_SetDefaultColor(context, MATRIX_TRANSPARENT_BACK_BLACK_LETTER);
    SDLPango_SetText(context, t, -1);
    black_letters = SDLPango_CreateSurfaceDraw(context);
  }
  else {
    black_letters = TTF_RenderUTF8_Blended(font, t, black);
  }
#endif

  if (!black_letters)
  {
    fprintf (stderr, "Warning - BlackOutline() could not create image for %s\n", t);
    return NULL;
  }

  bg = SDL_CreateRGBSurface(SDL_SWSURFACE,
                            (black_letters->w) + 5,
                            (black_letters->h) + 5,
                             32,
                             rmask, gmask, bmask, amask);
  /* Use color key for eventual transparency: */
  color_key = SDL_MapRGB(bg->format, 01, 01, 01);
  SDL_FillRect(bg, NULL, color_key);

  /* Now draw black outline/shadow 2 pixels on each side: */
  dstrect.w = black_letters->w;
  dstrect.h = black_letters->h;

  /* NOTE: can make the "shadow" more or less pronounced by */
  /* changing the parameters of these loops.                */
  for (dstrect.x = 1; dstrect.x < 4; dstrect.x++)
    for (dstrect.y = 1; dstrect.y < 3; dstrect.y++)
      SDL_BlitSurface(black_letters , NULL, bg, &dstrect );

  SDL_FreeSurface(black_letters);

  /* --- Put the color version of the text on top! --- */
#ifndef SDL_Pango
  white_letters = TTF_RenderUTF8_Blended(font, t, *c);
#else
  if( context != NULL)
  {
    SDLPango_SetDefaultColor(context, MATRIX_TRANSPARENT_BACK_WHITE_LETTER);
    white_letters = SDLPango_CreateSurfaceDraw(context);
  }
  else {
    white_letters = TTF_RenderUTF8_Blended(font, t, *c);
  }
#endif

  dstrect.x = 1;
  dstrect.y = 1;
  SDL_BlitSurface(white_letters, NULL, bg, &dstrect);
  SDL_FreeSurface(white_letters);

  /* --- Convert to the screen format for quicker blits --- */
  SDL_SetColorKey(bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, color_key);
  out = SDL_DisplayFormatAlpha(bg);
  SDL_FreeSurface(bg);

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nLeaving BlackOutline(): \n");
#endif

  return out;
}


int inRect( SDL_Rect r, int x, int y) {
	if ((x < r.x) || (y < r.y) || (x > r.x + r.w) || (y > r.y + r.h))
		return 0;
	return 1;
}

/* Darkens the screen by a factor of 2^bits */
void DarkenScreen(Uint8 bits)
{
#if PIXEL_BITS == 32
  Uint32 rm = screen->format->Rmask;
  Uint32 gm = screen->format->Gmask;
  Uint32 bm = screen->format->Bmask;
  Uint32* p; 
#elif PIXEL_BITS == 16
  Uint16 rm = screen->format->Rmask;
  Uint16 gm = screen->format->Gmask;
  Uint16 bm = screen->format->Bmask;
  Uint16* p; 
#else
  return;
#endif

  int x, y;

  /* (realistically, 1 and 2 are the only useful values) */
  if (bits > 8)
    return;

  p = screen->pixels;

  for (y = 0; y < RES_Y; y++)
  { 
    for (x = 0; x < RES_X; x++)
    {
      *p = (((*p&rm)>>bits)&rm)
         | (((*p&gm)>>bits)&gm)
         | (((*p&bm)>>bits)&bm);
      p++;
    }
  }
}


void SwitchScreenMode(void)
{
  SDL_Surface *tmp;
  SDL_Rect src, dst;

  int window = 0;

  src.x = 0;
  src.y = 0;
  src.w = RES_X;
  src.h = RES_Y;
  dst.x = 0;
  dst.y = 0;

  tmp = SDL_CreateRGBSurface(
      SDL_SWSURFACE,
      RES_X,
      RES_Y,
      PIXEL_BITS,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      0xff000000,
      0x00ff0000,
      0x0000ff00,
      0x000000ff
#else
      0x000000ff,
      0x0000ff00,
      0x00ff0000,
      0xff000000
#endif
      );

  if (screen->flags & SDL_FULLSCREEN)
  {
    window = 1;
  }

  SDL_BlitSurface(screen,&src,tmp,&dst);
  SDL_UpdateRect(tmp, 0, 0, RES_X, RES_Y);
  SDL_FreeSurface(screen);
  screen = NULL;

  if (window)
  {
    screen = SDL_SetVideoMode(RES_X,
                              RES_Y,
                              PIXEL_BITS,
                              SDL_SWSURFACE|SDL_HWPALETTE);
  }
  else
  {
    screen = SDL_SetVideoMode(RES_X,
                              RES_Y,
                              PIXEL_BITS,
                              SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
  }

  SDL_BlitSurface(tmp,&src,screen,&dst);
  SDL_UpdateRect(tmp,0,0,RES_X,RES_Y);
  SDL_FreeSurface(tmp);
}

