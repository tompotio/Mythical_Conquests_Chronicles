#pragma once
#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <cmath>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION

#include "../../libs/Nuklear-master/nuklear.h"
#include "../../libs/Nuklear-master/nuklear_sdl_renderer.h"
#include "../utils/include/constante.h"
#include "../../libs/Box2d/include/box2d/box2d.h"
#include "../utils/include/constante.h"

using namespace std;
typedef SDL_Texture* Texture;
typedef SDL_Rect Rect;
typedef b2Vec2 Vec2;

struct Camera {
    float x;
    float y;
    int w;
    int h;
    float wM;
    float hM;
};

/*
    Le renderManager gère toute la logique liée au rendu des éléments graphiques.
*/
class RenderManager : public b2Draw {
    public:
        ~RenderManager();
        // ----------------------------------- [Getters] ----------------------------------- //
        static RenderManager& getInstance(const char title[], int w, int h,bool fullscreen);
        static RenderManager& getInstance();
        SDL_Renderer* getRenderer() const {return renderer;}
        SDL_Window* getWindow() const {return window;}
        nk_context* getGuiContext(){return ctx;};
        int getWindowWidth(){return window_width;}
        int getWindowHeight(){return window_height;}
        void getFullWindowSize(int* w, int* h){
            *w = window_width;
            *h = window_height;
        }
        void updateScreenSize();
        Camera* getCamera(){return camera;}
        void setZoomFactor(float zoomFactor){this->zoomFactor = zoomFactor;}

        // ----------------------------------- [Utilities] ----------------------------------- //
        void draw_circle(float x, float y, float radius);
        void draw_polygon();
        void draw_filled_cirle();
        void draw_filled_polygon();
        void draw_filled_rectangle(int x, int y, int w, int h, int r, int g, int b, int a);
        void draw_BackgroundColor(int r, int g, int b, int a);
        void draw_BackgroundImage(Texture texture);
        void draw_PartImage(Texture texture,Rect srrect, Rect dstrect);
        void draw_Image(Texture texture, int x, int y, int w, int h, uint8 alpha, double angle);
        void drawImageByVerticalAxisSymmetry(Texture texture, int x, int y, int w, int h, uint8 alpha, double angle);
        void draw_Point(int x, int y);
        void renderScale();
        void parseTexture(vector<Texture>&tab,Texture texture, int w, int h, int n);

        // DebugDraw methods
        void DrawPolygon(const Vec2* vertices, int32 vertexCount, const b2Color& color) override;
        void DrawSolidPolygon(const Vec2* vertices, int32 vertexCount, const b2Color& color) override; 
        void DrawCircle(const Vec2& center, float radius, const b2Color& color) override;
        void DrawSolidCircle(const Vec2& center, float radius, const Vec2& axis, const b2Color& color) override; 
        void DrawSegment(const Vec2& p1, const Vec2& p2, const b2Color& color) override;
        void DrawTransform(const b2Transform& xf) override;
        void DrawPoint (const Vec2 &p, float size, const b2Color &color) override;
        void getTextureSize(Texture texture, int* w, int* h);

        void saveTexture(char fileName[], Texture texture);
        Texture loadTexture(string file);
        Texture GenerateQrCode(string svg,SDL_Renderer* renderer);
        void renderClear(){SDL_RenderClear(renderer);}
        void renderPresent(){SDL_RenderPresent(renderer);}
        void renderGui(){nk_sdl_render(NK_ANTI_ALIASING_ON);}

        Vec2 convertPoint(Camera* camera,const Vec2& point);
        Texture copyTexture(Texture textureSource);

    private:
        RenderManager(const char title[], int w, int h,bool fullscreen);
        RenderManager(const RenderManager&) = delete;
        RenderManager& operator=(const RenderManager&) = delete;

        // Platform
        static RenderManager* instance;
        int window_width;
        int window_height;
        SDL_Window* window;
        SDL_Surface* windowSurface;
        SDL_Renderer* renderer;
        Camera* camera;

        float zoomFactor = 1;

        // GUI
        struct nk_context *ctx;
        struct nk_colorf bg;
};

#endif