#include "ParticleUpdater.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
ParticleUpdater::ParticleUpdater() :
    _pRegion(0),
    _pEmitter(0)
{
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple assignment. 
Parameters: 
    pRegion     A pointer to a "particle region" interface.
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleUpdater::SetRegion(const IParticleRegion *pRegion)
{
    _pRegion = pRegion;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple assignment.
Parameters: 
    pEmitter    A pointer to a "particle emitter" interface.
    maxParticlesEmittedPerFrame     A restriction on the provided emitter to prevent all 
        particles from being emitted at once.
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleUpdater::SetEmitter(const IParticleEmitter *pEmitter, const int maxParticlesEmittedPerFrame)
{
    _pEmitter = pEmitter;
    _maxParticlesEmittedPerFrame = maxParticlesEmittedPerFrame;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Checks if each particle is out of bounds, and if so, tells the emitter to reset it.  If the 
    emitter hasn't reached its quota for emitted particles, then the particle is sent back out 
    again.  Lastly, if the particle is active, then its position is updated with its velocity and
    the provided delta time.
Parameters:
    particleCollection  The particle collection that will be updated.
    startIndex          Used in case the user wanted to adapat the updater to use multiple 
                        emitters and then wanted to split the number of particles between these 
                        emitters.
    numToUpdate         Same idea as "start index".
    deltatimeSec        Self-explanatory
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleUpdater::Update(std::vector<Particle> &particleCollection, 
    const unsigned int startIndex, const unsigned int numToUpdate, const float deltaTimeSec) const
{
    // ??throw a fit is endIndex >= particle collection size??

    // for all particles:
    // - if it has gone out of bounds, reset it and deactivate it
    // - if it is inactive and the emitter hasn't used up its quota for emitted particles this frame, reactivate it
    // - if it is active, update its position with its velocity
    // Note: If if() statements are used for each situation, then a particle has a chance to go 
    // out of bounds and get reset, get reactivated, and emit again in the same frame.  If 
    // else-if() statements are used, then only one of those situations will be run per frame.  
    // I did the former, but it doesn't really matter which approach is chosen.

    int emitCounter = 0;
    //for (size_t particleIndex = 0; particleIndex < particleCollection.size(); particleIndex++)
    for (size_t particleIndex = startIndex; particleIndex < numToUpdate; particleIndex++)
    {

        Particle &particleRef = particleCollection[particleIndex];
        if (_pRegion->OutOfBounds(particleRef))
        {
            particleRef._isActive = false;
            Particle pCopy = (particleCollection[particleIndex]);
            _pEmitter->ResetParticle(&pCopy);
            particleCollection[particleIndex] = pCopy;
        }

        // TODO: ?a way to make these conditions into assignments to avoid the pipeline thrashing? perhaps take advantage of "is active" being an integer??

        // if vs else-if()? eh
        if (!particleRef._isActive && emitCounter < _maxParticlesEmittedPerFrame)
        {
            particleRef._isActive = true;
            emitCounter++;
        }

        if (particleRef._isActive)
        {
            particleRef._position = particleRef._position + 
                (particleRef._velocity * deltaTimeSec);
        }
    }
}

