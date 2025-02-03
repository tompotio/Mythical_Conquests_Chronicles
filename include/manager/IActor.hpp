#pragma once
#ifndef IACTOR_H
#define IACTOR_H

/**
 * @brief Définit le comportement d'un acteur.
 * Un acteur de game doit pouvoir s'il le souhaite implémenter, selon ses données les méthodes update et render.
 */
class IActor {
    public:
        virtual void render() = 0;
        virtual void update(float deltaT) = 0;
        virtual void updateScreenSize() = 0;
};

#endif