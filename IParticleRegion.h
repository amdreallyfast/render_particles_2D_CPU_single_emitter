#pragma once

#include "Particle.h"

/*-----------------------------------------------------------------------------------------------
Description:
    The "particle updater" must be able to easily switch out particle regions (circle, polygon, 
    whatever else I come up with) without much trouble, so use an interface that defines the 
    basic functionality of each particle emitter.
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
class IParticleRegion
{
public:
    virtual ~IParticleRegion() {}
    virtual bool OutOfBounds(const Particle &p) const = 0;
};

