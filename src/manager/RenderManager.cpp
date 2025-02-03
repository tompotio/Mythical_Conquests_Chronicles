#include "../../include/manager/RenderManager.hpp"
#include <fstream>

RenderManager::RenderManager(const char title[], int w, int h,bool fullscreen) : window_width(w), window_height(h) {
    int flags = 0;
    float font_scale = 1;
    int windowflags = 0;

    if (!SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0")){
        cerr << "Error setting SDL_Hint: " << SDL_GetError() << endl;
        exit(1);
    }

    if(fullscreen){
        windowflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        window_width = DM.w;
        window_height = DM.h;
        //cout << "res_x : " << window_width << " res_y : " << window_height << endl;
    }else{
        windowflags |= SDL_WINDOW_RESIZABLE;
    }

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window_width,
        window_height,
        windowflags
    );

    if (window == nullptr) {
        cerr << "Error creating window: " << SDL_GetError() << endl;
        SDL_Quit();
        exit(1);
    }

    flags |= SDL_RENDERER_ACCELERATED;
    flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer = SDL_CreateRenderer(window, -1, flags);

    if (renderer == nullptr) {
        cerr << "Error creating renderer: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    /* scale the renderer output for High-DPI displays */
    {
        int render_w, render_h;
        int window_w, window_h;
        float scale_x, scale_y;
        SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
        SDL_GetWindowSize(window, &window_w, &window_h);
        scale_x = (float)(render_w) / (float)(window_w);
        scale_y = (float)(render_h) / (float)(window_h);
        SDL_RenderSetScale(renderer, scale_x, scale_y);
        font_scale = scale_y;
    }

    windowSurface = SDL_GetWindowSurface(window);

    /* GUI */
    ctx = nk_sdl_init(window, renderer);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
        nk_sdl_font_stash_end();

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);
    }

   // Instancie la caméra.
   camera = new Camera();
}

RenderManager& RenderManager::getInstance(const char title[], int w, int h,bool fullscreen)
{
    if (!instance) {
        instance = new RenderManager(title, w, h,fullscreen);
    }
    return *instance;
}

RenderManager& RenderManager::getInstance()
{
    if (!instance) {
        cerr << "Trying to get instance without initialising it first" << SDL_GetError() << endl;
        exit(1);
    }
    return *instance;
}

void RenderManager::updateScreenSize()
{
    SDL_GetWindowSize(window, &window_width, &window_height);
}

RenderManager* RenderManager::instance = nullptr;

RenderManager::~RenderManager() {
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void RenderManager::saveTexture(char fileName[], Texture texture){

}

/**
 * @brief Dessine la partie d'une texture à un endroit du renderer.
 * En paramètre définit la partie de l'image source à afficher. Et où l'afficher.
 * @param texture
 * @param x
 * @param y
 * @param w
 * @param h
 */
void RenderManager::draw_PartImage(Texture texture,Rect srrect, Rect dstrect){
    SDL_RenderCopy(renderer, texture, &srrect, &dstrect);
}

/**
 * @brief Dessine une texture entière à un endroit du renderer.
 * 
 * @param texture 
 * @param x 
 * @param y 
 * @param w 
 * @param h 
 * @param alpha 
 * @param angle 
 */
void RenderManager::draw_Image(Texture texture,int x, int y, int w, int h, uint8 alpha, double angle){
    x = x * zoomFactor;
    y = y * zoomFactor;
    h = h * zoomFactor;
    w = w * zoomFactor;

    Rect rect = {x, y, w, h};
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
}

/**
 * @brief Dessine une texture entière à un endroit du renderer avec une symétrie horizontale.
 * 
 * @param texture 
 * @param x 
 * @param y 
 * @param w 
 * @param h 
 * @param alpha 
 * @param angle 
 */
void RenderManager::drawImageByVerticalAxisSymmetry(Texture texture, int x, int y, int w, int h, uint8 alpha, double angle) {
    x *= zoomFactor;
    y *= zoomFactor;
    h *= zoomFactor;
    w *= zoomFactor;

    SDL_SetTextureAlphaMod(texture, alpha);
    Rect rect = {x, y, w, h};
    SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_HORIZONTAL);
}


void RenderManager::renderScale(){
    SDL_RenderSetScale(renderer,zoomFactor, zoomFactor);
}

void RenderManager::getTextureSize(Texture texture, int* w, int* h){
    SDL_QueryTexture(texture, NULL, NULL, w, h);
}

/**
 * @brief Change le background.
 *
 * @param texture
 */
void RenderManager::draw_BackgroundImage(Texture texture){
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void RenderManager::draw_BackgroundColor(int r, int g, int b, int a){
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void RenderManager::draw_filled_rectangle(int x, int y, int w, int h, int r, int g, int b, int a){
    x = x * zoomFactor;
    y = y * zoomFactor;
    w = w * zoomFactor;
    h = h * zoomFactor;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void RenderManager::draw_Point(int x, int y){
    SDL_RenderDrawPoint(renderer,x,y);
}

void RenderManager::draw_circle(float centreX, float centreY, float radius)
{
    radius *= zoomFactor;
    centreX *= zoomFactor;
    centreY *= zoomFactor;

    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

/**
 * @brief Parse une texture en fonction de ses dimensions.
 *
 * @param tab tableau dans lequel insérer les textures récupérées.
 * @param texture
 * @param w width.
 * @param h height.
 * @param n Maximum de textures à récupérer.
 */
void RenderManager::parseTexture(vector<Texture>&tab, Texture texture, int w, int h, int n){
    Texture rtex = SDL_GetRenderTarget(renderer);
    int largeurTexture, hauteurTexture;
    SDL_QueryTexture(texture, NULL, NULL, &largeurTexture, &hauteurTexture);
    const int col = largeurTexture / w;
    const int row = hauteurTexture / h;
    int i = 0;
    for(int stepY = 0; stepY < row; stepY++){
        for(int stepX = 0; stepX < col; stepX++){
            SDL_Rect srcRect = {stepX * w, stepY * h, w, h};
            Texture frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
            SDL_SetRenderTarget(renderer, frame);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, &srcRect, NULL);
            tab.push_back(frame);
            ++i;
        }
        if(i == n) break;
    }
    SDL_SetRenderTarget(renderer, rtex);
}

Texture RenderManager::loadTexture(string file){
    return IMG_LoadTexture(renderer, file.c_str());
};

Texture RenderManager::GenerateQrCode(string svg,SDL_Renderer* renderer){
  SDL_RWops *rw = SDL_RWFromConstMem(svg.c_str(), svg.size());
  SDL_Surface *surface = IMG_Load_RW(rw, 1);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  return texture;
}
/**
 * @brief Effectue la copie d'une texture.
 *
 * @param renderer
 * @param textureSource
 * @return Texture
 */
Texture RenderManager::copyTexture(Texture textureSource) {
    SDL_Texture* rtex = SDL_GetRenderTarget(renderer);
    int largeurTexture, hauteurTexture;
    SDL_QueryTexture(textureSource, NULL, NULL, &largeurTexture, &hauteurTexture);
    SDL_Texture* textureDestination = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, largeurTexture, hauteurTexture);    SDL_SetRenderTarget(renderer, textureDestination);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, textureSource, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderTarget(renderer, rtex);
    return textureDestination;
}

/**
 * @brief Convertit un point du monde dans le référentiel de la caméra, en pixel.
 *
 * @param camera
 * @param point
 * @return Vec2
 */
Vec2 RenderManager::convertPoint(Camera* camera, const Vec2& point){
    //cout << "x:" << point.x << "y:" << point.y << endl;
    return Vec2((point.x - camera->x) * PPM, (point.y - camera->y) * PPM);
}

void RenderManager::DrawPolygon(const Vec2* vertices, int32 vertexCount, const b2Color& color) {
	int i;
	bool estDedans = true;
		SDL_SetRenderDrawColor(renderer,255, 0, 0, 0);
		for(i=0;i<vertexCount-1;i++){
			Vec2 vec1 = zoomFactor * convertPoint(camera,vertices[i]);
			Vec2 vec2 = zoomFactor * convertPoint(camera,vertices[i+1]);
			SDL_RenderDrawLine(renderer,vec1.x,vec1.y,vec2.x,vec2.y);
		}
        Vec2 vec1 = zoomFactor * convertPoint(camera,vertices[0]);
        Vec2 vec2 = zoomFactor * convertPoint(camera,vertices[vertexCount-1]);
        SDL_RenderDrawLine(renderer,vec1.x,vec1.y,vec2.x,vec2.y);
}


void RenderManager::DrawSolidPolygon(const Vec2* vertices, int32 vertexCount, const b2Color& color) {
	int i;
	bool estDedans = true;
    
		SDL_SetRenderDrawColor(renderer,255, 0, 0, 0);
        for(i=0;i<vertexCount-1;i++){
            cout << "je dessine le polygon" << endl;
            cout << zoomFactor << endl;
			Vec2 vec1 = zoomFactor * convertPoint(camera,vertices[i]);
			Vec2 vec2 = zoomFactor * convertPoint(camera,vertices[i+1]);
            
			//SDL_RenderDrawLine(renderer,vec1.x,vec1.y,vec2.x,vec2.y);
            SDL_RenderDrawLine(renderer,vec1.x,vec1.y,vec2.x,vec2.y);
		}
        Vec2 vec1 = zoomFactor * convertPoint(camera,vertices[0]);
        Vec2 vec2 = zoomFactor * convertPoint(camera,vertices[vertexCount-1]);
        SDL_RenderDrawLine(renderer,vec1.x,vec1.y,vec2.x,vec2.y);
}


void RenderManager::DrawCircle(const Vec2& center, float radius, const b2Color& color) {
	//calcul d'appartenace abs(CE) - 1/2(l,h)-r
	SDL_SetRenderDrawColor(renderer,255,0,0,0);
	radius *= PPM * zoomFactor;
	Vec2 centerr = zoomFactor * convertPoint(camera,center);
	const int32_t diameter = (radius * 2);

	int32_t x = ((radius - 1));
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		SDL_RenderDrawPoint(renderer, centerr.x + x, centerr.y - y);
		SDL_RenderDrawPoint(renderer, centerr.x + x, centerr.y + y);
		SDL_RenderDrawPoint(renderer, centerr.x - x, centerr.y - y);
		SDL_RenderDrawPoint(renderer, centerr.x - x, centerr.y + y);
		SDL_RenderDrawPoint(renderer, centerr.x + y, centerr.y - x);
		SDL_RenderDrawPoint(renderer, centerr.x + y, centerr.y + x);
		SDL_RenderDrawPoint(renderer, centerr.x - y, centerr.y - x);
		SDL_RenderDrawPoint(renderer, centerr.x - y, centerr.y + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

void RenderManager::DrawSolidCircle(const Vec2& center, float radius, const Vec2& axis, const b2Color& color) {
	// calcul d'appartenace abs(CE) - 1/2(l,h)-r
	SDL_SetRenderDrawColor(renderer,255,0,0,0);
    
	radius *= PPM * zoomFactor;
	Vec2 centerr = zoomFactor * convertPoint(camera,center);
	const int32_t diameter = (radius * 2);

	int32_t x = ((radius - 1));
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		SDL_RenderDrawPoint(renderer, centerr.x + x, centerr.y - y);
		SDL_RenderDrawPoint(renderer, centerr.x + x, centerr.y + y);
		SDL_RenderDrawPoint(renderer, centerr.x - x, centerr.y - y);
		SDL_RenderDrawPoint(renderer, centerr.x - x, centerr.y + y);
		SDL_RenderDrawPoint(renderer, centerr.x + y, centerr.y - x);
		SDL_RenderDrawPoint(renderer, centerr.x + y, centerr.y + x);
		SDL_RenderDrawPoint(renderer, centerr.x - y, centerr.y - x);
		SDL_RenderDrawPoint(renderer, centerr.x - y, centerr.y + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

void RenderManager::DrawSegment(const Vec2& p1, const Vec2& p2, const b2Color& color) {
	SDL_SetRenderDrawColor(renderer,color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(
        renderer,
        p1.x * zoomFactor,
        p1.y * zoomFactor,
        p2.x * zoomFactor,
        p2.y * zoomFactor
    );
}

void RenderManager::DrawTransform(const b2Transform& xf){}

void RenderManager::DrawPoint (const Vec2 &p, float size, const b2Color &color) {
	if(p.x > camera->x && p.x < camera->x + camera->w/PPM && p.y > camera->y && p.y < camera->y + camera->w/PPM){
		SDL_SetRenderDrawColor(renderer,color.r, color.g, color.b, color.a);
		Vec2 vec1 = convertPoint(camera,p);
		SDL_RenderDrawPoint(renderer,vec1.x,vec1.y);
	}
}