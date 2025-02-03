#pragma once
#ifndef TILETYPE_H
#define TILETYPE_H

#include "Animation.hpp"
#include "Entity.hpp"

enum TileType {
    Ground,
    Wall,
    Water,
    Sand,
};

/**
* @brief Classe qui définit une tile.
 * Un tuple d'information d'une tuile dans le Json de la map, a la forme suivante :
 *      {NomTexture, type, {animation, nbFrames}}
 * 
 * Définitions :
 *      NomTexture = nom de la texture avec le format .png.
 *      type = Enum TileType.
 *      {animation,nbFrames}, si animation est true alors on regarde le nombre de frames qui compose l'animation.
 * 
 * Si une tile est de type Ground (enum) et sans animation, alors par défaut elle n'est pas une entité et compose simplement le fond d'écran (la texture du monde statique cf. mapWorldText dans la classe Map).
 * Par contre si une tile est de type Ground avec une animation ou alors d'un autre type, elle devient une entité du monde à part entière.
 * Cette tile dynamique doit être instanciée par cette classe.
 * Les tiles dynamiques pourront être réaffichées à chaque frame par dessus la texture du monde statique dans la méthode render() de la classe Map.
 * 
 * Commentaires : 
 *      Pour l'instant n'hérite pas de entity. (rajoute de la complexité, je ferai ça plus tard)
 */
class TileDynamique : public Entity
{
    public:
        TileDynamique(int x, int y,int w, int h, int d, int f, b2Shape* shape, BodyType type,ETypeInstance typeinstance) : Entity(x,y,w,h,d,f,0.0f,shape,type,this,TILECATEGORY,typeinstance, TILEINDEX), x(x), y(y) 
        {
            
        }

        void setX(int x){this->x = x;}
        void setY(int y){this->y = y;}
        void setType(TileType type){this->type = type;}
        
        int getX(){return x;}
        int getY(){return y;}
        
        TileType getType(){return type;}

    private:
        int x;
        int y;
        TileType type;
};

/**
 * @brief Simple Tile avec une animation. On s'en sert pour les sols.
 * 
 */
class TileStatique
{
    public:
        TileStatique(int x, int y){};
        void setX(int x){this->x = x;}
        void setY(int y){this->y = y;}
        int getX(){return x;}
        int getY(){return y;}

        void addAnimation(float speed){
            animation = Animation(speed);
        }

        void addAnimation(Timer* timer){
            animation = Animation(timer);
        }
        
        Animation& getAnimation(){return this->animation;}
    
    private:
        int x;
        int y;

        Animation animation;
};

/**
 * @brief Tile simple avec une texture.
 * 
 */
class Tile {
    public: 
        Tile(int x, int y, Texture texture) : x(x),y(y), texture(texture) {};
        void setX(int x){this->x = x;}
        void setY(int y){this->y = y;}
        void setTexture(Texture texture){this->texture = texture;}

        int getX(){return x;}
        int getY(){return y;}
        Texture getTexture(){return texture;}

    private:
        int x;
        int y;
        Texture texture;
};

class TileWall : public TileDynamique
{
    public:
        TileWall(int x, int y,int w, int h, int d, int f, b2Shape* shape, BodyType type) : TileDynamique(x, y, w, h, d, f, shape, type,TILEWALL){};
        void update(float deltaT) override;
        void onTouch(b2Contact* contact,float deltaT) override;
        void BeginContact(b2Contact* contact,float deltaT)override;
        void EndContact(b2Contact* contact,float deltaT) override;
    private:
};

class TileWater : public TileDynamique
{
    public:
        TileWater(int x, int y,int w, int h, int d, int f, b2Shape* shape, BodyType type) : TileDynamique(x, y, w, h, d, f, shape, type,TILEWATER){};
        void update(float deltaT) override;
        void onTouch(b2Contact* contact,float deltaT) override;
        void BeginContact(b2Contact* contact,float deltaT)override;
        void EndContact(b2Contact* contact,float deltaT) override;
    private:
};

#endif