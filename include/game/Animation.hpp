#pragma once
#ifndef ANIMATION_H
#define ANIMATION_H

#include "../manager/RenderManager.hpp"
#include "../utils/include/Timer.hpp"

/**
 * @brief Classe qui contient au moins une image et qui peut les faires défiler.
 * Cette classe fonctionne également pour les objets avec un seul sprite.
 */
class Animation {
    public: 
        Animation() = default;
        Animation(float speed){
            timer = new Timer(speed);
        }
        Animation(Timer* timer){
            this->timer = timer;
        }

        void reset(){
            index = 0;
            timer->reset();
            finished = false;
        }
        void playAnimation();
        void addFrame(Texture texture){
            tileTextures.push_back(texture);
            nbFrames += 1;
        }

        void setTimer(Timer* timer){
            if(this->timer != nullptr) delete this->timer;
            this->timer = timer;
        }
        void setLoop(bool val){loop = val;}
        void setTextureHeight(int height){this->height = height;}
        void setTextureWidth(int width){this->width = width;}

        int getNbFrames(){return tileTextures.size();}
        int getTextureWidth(){return this->width;}
        int getTextureHeight(){return this->height;}
        Texture getCurrentFrame(){
            if(!loop){
                if(index == nbFrames - 1){
                    finished = true;
                }
            }

            // On regarde d'abord si on peut éviter le calcul du modulo.
            if(nbFrames == 1){
                return tileTextures[0];
            }

            if(timer->getTime() == 0) index = (index + 1) % nbFrames;
            return tileTextures[index];
        }
        int getFrame(){
            return index;
        }
        bool isFinished(){return finished;}
        Timer* getTimer(){return this->timer;}

    private:
        vector<Texture> tileTextures;
        Timer* timer = new Timer(0);

        int index = 0; // Par défaut l'index est à 0.
        int nbFrames = 0;
        int width;
        int height;
        // Champ permettant de savoir si l'animation doit se relancer.
        bool loop = true;
        bool finished = false;
};

#endif