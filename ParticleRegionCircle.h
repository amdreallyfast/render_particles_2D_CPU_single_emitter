#pragma once

#include "IParticleRegion.h"
#include "glm/vec2.hpp"
#include "Particle.h"
#include <vector>

/*-----------------------------------------------------------------------------------------------
Description:
    This object defines a circular region within which particles are considered active and the 
    logic to determine when a particle goes outside of its boundaries.
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
class ParticleRegionCircle : public IParticleRegion
{
public:
    ParticleRegionCircle(const glm::vec2 &center, const float radius);

    virtual bool OutOfBounds(const Particle &p) const;

private:
    glm::vec2 _center;
    float _radiusSqr;   // because radius is never used
};
