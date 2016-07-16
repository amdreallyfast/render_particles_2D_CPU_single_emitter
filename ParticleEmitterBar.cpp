#include "ParticleEmitterBar.h"

#include "RandomToast.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.
Parameters:
    p1      The bar's origin point in window space (XY on range [-1,+1]).
    p2      The bar's end point.
    emitDir     Particles will be launched in this direction evenly along the bar.
    minVel      The minimum velocity for particles being emitted.
    maxVel      The maximum emission velocity.
Returns:    None
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
ParticleEmitterBar::ParticleEmitterBar(const glm::vec2 &p1, const glm::vec2 &p2, 
    const glm::vec2 &emitDir, float minVel, const float maxVel)
{
    _barStart = p1;
    _barStartToEnd = p2 - p1;
    _velocityCalculator.SetMinMaxVelocity(minVel, maxVel);

    //// rotate a 2D vector (x,y) by +90 degrees => (-y,x)
    //glm::vec2 plus90Degrees((-1.0f) * _barStartToEnd.y, _barStartToEnd.x);
    //_velocityCalculator.SetDir(plus90Degrees);
    _velocityCalculator.SetDir(emitDir);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Sets the given particle's starting position and velocity.  The position is set to a random 
    point along the var.
    
    Does NOT alter the "is active" flag.  That flag is altered only by the "particle updater" 
    object.
Parameters:
resetThis   Self-explanatory.
Returns:    None
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleEmitterBar::ResetParticle(Particle *resetThis) const
{
    //??when to turn it on??
    //resetThis->_isActive = true;

    // give it some flavor by making the particles be reset to within a range near the emitter 
    // bar's position instead of exactly on the bar, making it look like a particle hotspot
    // TODO: the random thing
    resetThis->_position = _barStart + (RandomOnRange0to1() * _barStartToEnd);


    resetThis->_velocity = _velocityCalculator.GetNew();
}


