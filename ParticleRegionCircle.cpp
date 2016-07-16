#include "ParticleRegionCircle.h"

#include "glm/detail/func_geometric.hpp"    // glm::dot

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.
Parameters: 
    center  The central point of the circle.  Self-explanatory.
    radius  Specified (in this demo, at least) in window coordinates (X and Y on range [-1,+1]).
Returns:    None
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
ParticleRegionCircle::ParticleRegionCircle(const glm::vec2 &center, const float radius)
{
    _center = center;
    _radiusSqr = radius * radius;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Checks if the provided particle has gone outside the circle.
Parameters:
    p   A const reference to a Particle object.
Returns:    
    True if the particle's position is outside the circle's boundaries, otherwise false.
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
bool ParticleRegionCircle::OutOfBounds(const Particle &p) const
{
    glm::vec2 centerToParticle = p._position - _center;
    float distSqr = glm::dot(centerToParticle, centerToParticle);
    if (distSqr > _radiusSqr)
    {
        return true;
    }
    else
    {
        return false;
    }
}
