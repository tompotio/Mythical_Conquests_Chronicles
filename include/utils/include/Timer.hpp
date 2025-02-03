#pragma once
#ifndef TIMEUP_H
#define TIMEUP_H

/**
 * @brief Le timer est basiquement un chronomètre que l'on peut configurer.
 */
class Timer
{
    public: 
        Timer() = default;
        Timer(float end){
            this->end = end;
            this->elapsed = 0;
        }

        void reset(){elapsed = 0; ended = false;}
        void update(float deltaT){
            elapsed += deltaT;
            if (elapsed >= end) {
                elapsed = 0;
                ended = true;
            }
        }
        void setEnd(float end){this->end=end;}

        float getEnd(){return end;}
        float getTime(){return elapsed;}
        bool getEnded(){return ended;}
        
    private:
        float elapsed = 0; // début du chrono
        float end = 1; // fin du chrono
        bool ended = false; 
};

#endif 