#pragma once

#include "IParticleRegion.h"
#include "glm/vec2.hpp"
#include "Particle.h"
#include <vector>


// num corners == num faces
const unsigned int MAX_POLYGON_FACES = 10;

/*-----------------------------------------------------------------------------------------------
Description:
    This object defines a polygonal region within which particles are considered active and the
    logic to determine when a particle goes outside of its boundaries.
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
class ParticleRegionPolygon : public IParticleRegion
{
public:
    ParticleRegionPolygon(const std::vector<glm::vec2> corners);
    virtual bool OutOfBounds(const Particle &p) const;

private:
    unsigned int _numFaces;

    // do NOT use a std::vector or any other container that allocates memory on the heap
    // Note: Tinkering last weekend discovered that using a std::vector<...> trashed the data 
    // cache when running the "out of bounds" check.  The continual need to run out to the heap 
    // just to find the corners' vectors created a major frame rate drop.
    // Solution: Use arrays with a max size.  This keeps up the flexibility of variable number of
    // sides while keeping all necessary info in the process' data memory.
    glm::vec2 _faceCenterPoints[MAX_POLYGON_FACES];
    glm::vec2 _faceNormals[MAX_POLYGON_FACES];

    //// used to unroll a loop in the "out of bounds" check, which became necessary when the frame
    //// rate dropped like a rock when going from 1000->10000 particles (the circle particle 
    //// region was fine and trucks along at a high frame rate)
    //unsigned int _numCorners;

    //glm::vec2 _cp1;
    //glm::vec2 _cp2;
    //glm::vec2 _cp3;
    //glm::vec2 _cp4;
    //glm::vec2 _n1;
    //glm::vec2 _n2;
    //glm::vec2 _n3;
    //glm::vec2 _n4;

    //std::vector<glm::vec2> _faceNormals;

    //// these centers are used as reference points to make a vector out of the particle's current 
    //// position that stems from the same base as the face normal
    //// Note: I actually could have used any point along the face, but the center point is 
    //// conceptually easy to imagine as the "source" of the surface normal since that is where it 
    //// is usually drawn in diagrams.
    //std::vector<glm::vec2> _faceCenterPoints;
};