#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

/* Screen dimensions */
#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	200

typedef struct
{
	SDL_Renderer* renderer;
	int x;					/* Current position X						*/
	int y;					/* Current position Y						*/
	int color;				/* Current Color							*/
	int scale;				/* Current scale factor						*/
	float angle;			/* Current angle							*/
	bool penUp;				/* Current only move without draw			*/
	bool returnOrigin;		/* Current draw but return to origin point	*/
} stDrawingContext;

typedef struct
{
	Uint8 R;
	Uint8 G;
	Uint8 B;
} stRGB;

stRGB Pallete0[] =
{
	{0x00, 0x00, 0x00},
	{0x55, 0xFF, 0x55},
	{0xFF, 0x55, 0x55},
	{0xFF, 0xFF, 0x55}
};

stRGB Pallete1[] =
{
	{0x00, 0x00, 0x00},
	{0x55, 0xFF, 0xFF},
	{0xFF, 0x55, 0xFF},
	{0xFF, 0xFF, 0xFF}
};

int initGml(SDL_Window** window, SDL_Renderer** renderer, int WindowWidth, int WindowHeight, stDrawingContext* ctx)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return (-1);
	}

	*window = SDL_CreateWindow(
		"GML Draw",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WindowWidth,
		WindowHeight,
		SDL_WINDOW_SHOWN);
	if (NULL == *window)
	{
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		SDL_Quit();
		return (-1);
	}

	*renderer = SDL_CreateRenderer(
		*window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (NULL == *renderer)
	{
		SDL_DestroyWindow(*window);
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_Quit();
		return (-1);
	}

	/* Initialize drawing context */
	ctx->renderer = *renderer;
	ctx->x = WindowWidth / 2;
	ctx->y = WindowHeight / 2;
	ctx->color = 1;
	ctx->scale = 4;
	ctx->angle = 0.0;
	ctx->penUp = false;
	ctx->returnOrigin = false;
	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
	SDL_RenderClear(*renderer);

	return 0;
}

int destroyGml(SDL_Window** window, SDL_Renderer** renderer)
{
	SDL_DestroyRenderer(*renderer);
	SDL_DestroyWindow(*window);
	SDL_Quit();
	return 0;
}

void executeGmlCommandMove(stDrawingContext* ctx, int targetX, int targetY)
{
	if (false == ctx->penUp)
	{
		SDL_RenderDrawLine(ctx->renderer, ctx->x, ctx->y, targetX, targetY);
	}
	else
	{
		ctx->penUp = false;
	}

	if (false == ctx->returnOrigin)
	{
		ctx->x = targetX;
		ctx->y = targetY;
	}
	else
	{
		ctx->returnOrigin = false;
	}
}

void executeGmlCommand(stDrawingContext* ctx, char cmd, int param1, int param2)
{
	static bool bIsCurrentMoveButReturn = false;
	/* Execute command */
	switch (cmd)
	{
	case 'U':		/* Move up		*/
		executeGmlCommandMove(ctx, ctx->x, ctx->y - ctx->scale * param1);
		break;
	case 'D':		/* Move down	*/
		executeGmlCommandMove(ctx, ctx->x, ctx->y + ctx->scale * param1);
		break;
	case 'L':		/* Move left	*/
		executeGmlCommandMove(ctx, ctx->x - ctx->scale * param1, ctx->y);
		break;
	case 'R':		/* Move right	*/
		executeGmlCommandMove(ctx, ctx->x + ctx->scale * param1, ctx->y);
		break;
	case 'E':		/* Move diagonally up and right		*/
		executeGmlCommandMove(ctx, ctx->x + ctx->scale * param1, ctx->y - ctx->scale * param1);
		break;
	case 'F':		/* Move diagonally down and right	*/
		executeGmlCommandMove(ctx, ctx->x + ctx->scale * param1, ctx->y + ctx->scale * param1);
		break;
	case 'G':		/* Move diagonally down and left	*/
		executeGmlCommandMove(ctx, ctx->x - ctx->scale * param1, ctx->y + ctx->scale * param1);
		break;
	case 'H':		/* Move diagonally up and left		*/
		executeGmlCommandMove(ctx, ctx->x - ctx->scale * param1, ctx->y - ctx->scale * param1);
		break;
	case 'B':		/* Move but plot no points							*/
		ctx->penUp = true;
		break;
	case 'N':		/* Move but return to original position when done	*/
		ctx->returnOrigin = true;
		break;
	case 'C':		/* Set color	*/
		ctx->color = param1;
		//TODO COLOR
		SDL_SetRenderDrawColor(ctx->renderer, Pallete1[ctx->color].R, Pallete1[ctx->color].G, Pallete1[ctx->color].B, 255);
		break;
	case 'S':		/* Set scale	*/
		ctx->scale = param1 / 4;
		break;
	case 'M':
		executeGmlCommandMove(ctx, ctx->scale * param1, ctx->scale * param2);
		break;
	case 'A':		/* Set angle	*/
		ctx->angle = param1 * 90.0f;
		break;
	case 'T':		/* Turn angle	*/
		ctx->angle += param1;
		break;
	default:
		printf("Unkown command %c\n", cmd);
		break;
	}
}

void executeGml(stDrawingContext* ctx, const char* commands)
{
	size_t len = strlen(commands);
	for (size_t i = 0; i < len; i++)
	{
		char cmd = commands[i];
		int n = 0;

		if (isalpha(cmd) != 0)
		{
			cmd = toupper(cmd);
		}

		if ((cmd == ';') || (cmd == ' ') || (cmd == ','))
		{
			/* Do nothing */
			/* Ignore spaces */
			/* Ignore commas */
		}
		else if ((cmd == 'B') || (cmd == 'N'))
		{
			executeGmlCommand(ctx, cmd, -1, -1);
		}
		else if (cmd == 'M')
		{
			int x = 0;
			int y = 0;
			int nTotal = 0;
			char sign = ' ';

			if (('+' == commands[i + 1]) || ('-' == commands[i + 1]))
			{
				sign = commands[i + 1];
				i++;
			}
			/* Last parameter of sscanf %n holds the amount of characters matched */
			if (sscanf(&(commands[i + 1]), "%d,%d %n", &x, &y, &nTotal) == 2)
			{
				/* Check if a there was a sign before the x number */
				if (('+' == sign) || ('-' == sign))
				{
					/* If required convert it to negative */
					if ('-' == sign)
					{
						x *= -1;
					}

					/* Divide by scale since context x and y are already scaled after drawing */
					x += (ctx->x / ctx->scale);
					y += (ctx->y / ctx->scale);
				}
				i += nTotal;
				executeGmlCommand(ctx, cmd, x, y);
			}
		}
		else if (isdigit(commands[i + 1]) != 0)	/* Parse numeric argument if present */
		{
			char* tempPtr = (char *)&commands[i + 1];
			n = strtol(&(commands[i + 1]), (char**)&tempPtr, 10);
			executeGmlCommand(ctx, cmd, n, -1);
			i += tempPtr - &(commands[i + 1]);
		}
		else
		{
			printf("Unexpected GML character %c\n", commands[i + 1]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	stDrawingContext Context;

	if (initGml(&window, &renderer, SCREEN_WIDTH, SCREEN_HEIGHT, &Context) != 0)
	{
		return (-1);
	}

	// Draw Car using GML
	executeGml(&Context, "S32C3");
	executeGml(&Context, "BM12,1r3m+1,3d2R1ND2u1r2d4l2u1l1");
	executeGml(&Context, "d7R1nd2u2r3d6l3u2l1d3m-1,1l3");
	executeGml(&Context, "m-1,-1u3l1d2l3u6r3d2nd2r1u7l1d1l2");
	executeGml(&Context, "u4r2d1nd2R1U2");
	executeGml(&Context, "M+1,-3");
	executeGml(&Context, "BD10D2R3U2M-1,-1L1M-1,1");
	executeGml(&Context, "BD3D1R1U1L1BR2R1D1L1U1");
	executeGml(&Context, "BD2BL2D1R1U1L1BR2R1D1L1U1");
	executeGml(&Context, "BD2BL2D1R1U1L1BR2R1D1L1U1");

	// Draw Donkey using GML
	//executeGml(&Context, "S32C3");
	//executeGml(&Context, "BM14,18");
	//executeGml(&Context, "M+2,-4R8M+1,-1U1M+1,+1M+2,-1");
	//executeGml(&Context, "M-1,1M+1,3M-1,1M-1,-2M-1,2");
	//executeGml(&Context, "D3L1U3M-1,1D2L1U2L3D2L1U2M-1,-1");
	//executeGml(&Context, "D3L1U5M-2,3U1");

	SDL_RenderPresent(renderer);

	SDL_Event event;
	bool bQuit = false;

	/* Poll for events */
	while (false == bQuit)
	{
		while (SDL_PollEvent(&event)) 
		{
			switch (event.type)
			{
				case SDL_KEYUP:
				case SDL_QUIT:
					bQuit = true;

				default:
					break;
			}		
		}
	}

	return destroyGml(&window, &renderer);
}