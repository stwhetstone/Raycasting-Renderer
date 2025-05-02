#include <cmath>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "Vec2.hpp"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

const int worldMap[MAP_WIDTH][MAP_HEIGHT] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


void handleInput(bool &quit, Vec2<float> &pos, Vec2<float> &dir, Vec2<float> &cameraPlane);
Vec2<int> getLineHeight(const int side,
                    const Vec2<float> &sideDist,
                    const Vec2<float> &rayUnitStepSize,
                    const Vec2<float> &dir, 
                    const Vec2<float> &rayDir
                    ); 
unsigned int getColor(const Vec2<int> &mapPos, const int side);

int main(int argc, char **argv) {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Raycasting", 1280, 720, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);

    if(!window || !renderer || !texture) {
        return -1;
    }

    SDL_SetRenderVSync(renderer, 1);

    Vec2<float> pos(4, 4), dir(-1, 0), cameraPlane(0, 1);

    bool quit = false;
    while(!quit) {   
        handleInput(quit, pos, dir, cameraPlane);

        SDL_SetRenderTarget(renderer, texture);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for(int x = 0; x < WINDOW_WIDTH; x++) {
            // current point on camera plane
            // makes left side of screen -1, center 0, right side 1
            const float cameraX = 2 * x / (float) WINDOW_WIDTH - 1;
            Vec2<float> rayDir = dir + cameraPlane * cameraX;
            
            // what square of the map are we in
            Vec2<int> mapPos((int)pos.x, (int)pos.y);
            // distance from original point in x/y direction to some side of a cell that the ray is currently intersecting
            Vec2<float> sideDist; 

            // distance to move if you wanted to go 1 space in the x/y direction
            // scale x/y so that when x/y = 1, a right triangle is made, then step size is that triangle's hypotenuse
            //
            // side dx = -1, side dy = -0.66
            // when dx = 1 => dy = -0.66 / -1 = 0.66
            // when dy = 1 => dx = -1 / -0.66 = 1.515
            //
            // rayUnitStepSize.x = sqrt( (dx/dx)^2 + (dy / dx)^2 ) = sqrt( 1 + (0.66)^2 )
            // rayUnitStepSize.y = sqrt( (dy/dy)^2 + (dx / dy)^2 ) = sqrt( 1 + (1.515) ^2 )
            Vec2<float> rayUnitStepSize(
                                    rayDir.x == 0 ? 1e30 : std::sqrt( 1 + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x) ),
                                    rayDir.y == 0 ? 1e30 : std::sqrt( 1 + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y) )
                                );
            // the direction that the ray moves in
            Vec2<int> step;

            // distance between nearest x side in direction of rayDir and current point
            if(rayDir.x < 0) {
                step.x = -1;
                sideDist.x = (pos.x - mapPos.x) * rayUnitStepSize.x;
            } else if (rayDir.x >= 0) {
                step.x = 1;
                sideDist.x = (mapPos.x + 1.0f - pos.x) * rayUnitStepSize.x;
            }

            // distance between nearest y side in direction of rayDir and current point
            if(rayDir.y < 0) {
                step.y = -1;
                sideDist.y = (pos.y - mapPos.y) * rayUnitStepSize.y;
            } else if (rayDir.y >= 0) {
                step.y = 1;
                sideDist.y = (mapPos.y + 1.0f - pos.y) * rayUnitStepSize.y;
            }


            bool hit = false;
            int side = 0;
            // DDA 
            while(!hit) {
                if(sideDist.x < sideDist.y) {
                    // move the ray 1 unit in the x direction and rayDir.y / rayDir.x in the y direction
                    sideDist.x += rayUnitStepSize.x;
                    // move mapPos to the next square in the x direction
                    mapPos.x += step.x;
                    side = 0;
                } else if(sideDist.x >= sideDist.y) {
                    // move the ray 1 unit in the y direction and rayDir.x / rayDir.y in the x direction
                    sideDist.y += rayUnitStepSize.y;
                    // move mapPos to the next square in the y direction
                    mapPos.y += step.y;
                    side = 1;
                }

                if(worldMap[mapPos.x][mapPos.y] > 0) {
                    hit = true;
                }
            }


            auto [lineStart, lineEnd] = getLineHeight(side, sideDist, rayUnitStepSize, dir, rayDir);
            unsigned int color = getColor(mapPos, side);
           

            SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
            SDL_RenderLine(renderer, (float)x, lineStart, (float)x, lineEnd);

        }

        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}


void handleInput(bool &quit, Vec2<float> &pos, Vec2<float> &dir, Vec2<float> &cameraPlane) {
    float rotSpeed = 0.05f;

    SDL_Event e;
    while(SDL_PollEvent(&e) != 0) {
        if(e.type == SDL_EVENT_QUIT) {
            quit = true;
        } else if(e.type == SDL_EVENT_KEY_DOWN) {
            switch(e.key.key) {
                case SDLK_UP:
                    pos += dir;
                    break;
                case SDLK_DOWN:
                    pos -= dir;
                    break;
                case SDLK_RIGHT:
                    Vec2<float>::rotate(dir, -rotSpeed);
                    Vec2<float>::rotate(cameraPlane, -rotSpeed);
                    break;
                case SDLK_LEFT:
                    Vec2<float>::rotate(dir, rotSpeed);
                    Vec2<float>::rotate(cameraPlane, rotSpeed);
                    break;

            }
        }
    }
}

Vec2<int> getLineHeight(
        const int side,
        const Vec2<float> &sideDist,
        const Vec2<float> &rayUnitStepSize,
        const Vec2<float> &dir, 
        const Vec2<float> &rayDir
) {
    float wallDist = 0.0f;
    if(side == 0) {
        wallDist = sideDist.x - rayUnitStepSize.x;
    } else if(side == 1) {
        wallDist = sideDist.y - rayUnitStepSize.y;
    }

    // angle to fix fisheye lens
    float angle = atan2(dir.x, dir.y) - atan2(rayDir.x, rayDir.y);
    int lineHeight = WINDOW_HEIGHT / (wallDist * cos(angle));

    int lineStart = -lineHeight / 2 + WINDOW_HEIGHT / 2;
    if(lineStart < 0) {
        lineStart = 0;
    }

    int lineEnd = lineHeight / 2 + WINDOW_HEIGHT / 2;
    if(lineEnd >= WINDOW_HEIGHT) {
        lineEnd = WINDOW_HEIGHT - 1;
    }

    return (Vec2<int>){lineStart, lineEnd};
}

unsigned int getColor(const Vec2<int> &mapPos, const int side) {
    unsigned int color = 0;
    int r = 0, g = 0, b = 0;
    switch(worldMap[mapPos.x][mapPos.y]) {
        case 1:
            r = 255;
            break;
        case 2:
            g = 255;
            break;
        case 3:
            b = 255;
            break;
        case 4:
            r = 255;
            g = 255;
            b = 255;
            break;
        default:
            r = 255;
            g = 255;
            break;
    }

    if(side == 1) {
        r /= 2;
        g /= 2;
        b /= 2;
    }

    color += r << 16;
    color += g << 8;
    color += b;

    return color;
}
