#include "ParticleRegionPolygon.h"

#include "glm/detail/func_geometric.hpp"    // for dot and normalize

/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates the rotating of a 2D vector by -90 degrees (+90 degrees not used in this demo).
Parameters:
    v   A const 2D vector.
Returns:    
    A 2D vector rotated -90 degrees from the provided one.
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
static glm::vec2 RotateNeg90(const glm::vec2 &v)
{
    return glm::vec2(v.y, -(v.x));
}

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.  Restricted to number of 
    corners equivalent to MAX_POLYGON_FACES.
Parameters:
    corners     A counterclockwise collection of 2D points in window space (XY on range[-1,+1]).
                If the number of corners exceeds MAX_POLYGON_FACES, extra corners are ignored.
                ??throw exception instead??
Returns:    None
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
ParticleRegionPolygon::ParticleRegionPolygon(const std::vector<glm::vec2> corners)
{

    //_numCorners = corners.size();
    
    // 3 corners -> triangle -> 3 faces
    // 4 corners -> square -> 4 faces
    // ...
    // num corners == num faces
    _numFaces = corners.size();

    // calculating the face centers does not depend on clockwise-ness
    for (size_t cornerIndex = 0; cornerIndex < _numFaces; cornerIndex++)
    {
        glm::vec2 corner1 = corners[cornerIndex];
        glm::vec2 corner2;
        if (cornerIndex + 1 == corners.size())
        {
            // wrap around
            corner2 = corners[0];
        }
        else
        {
            corner2 = corners[cornerIndex + 1];
        }

        // the center if halfway between these two points
        _faceCenterPoints[cornerIndex] = (corner1 + corner2) * 0.5f;
    }

    // the normals should point outwards
    // Note: This means that, if the corners are provided clockwise, then each corner->corner
    // vector needs to rotate +90 degrees, but if the corners are provided counterclockwise, 
    // then each corner->corner vector needs to rotate -90 degrees.  Try drawing it out or 
    // working it out with yoru fingers (I did the latter).
    for (size_t cornerIndex = 0; cornerIndex < _numFaces; cornerIndex++)
    {
        glm::vec2 corner1 = corners[cornerIndex];
        glm::vec2 corner2;
        if (cornerIndex + 1 == corners.size())
        {
            // wrap around
            corner2 = corners[0];
        }
        else
        {
            corner2 = corners[cornerIndex + 1];
        }

        _faceNormals[cornerIndex] = RotateNeg90(corner2 - corner1);
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    Checks if the provided particle has gone outside of the polygonal boundaries.
Parameters:
    p   A const reference to a particle object.
Returns:
    True if the particle has outside of the region's boundaries, otherwise false.
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
bool ParticleRegionPolygon::OutOfBounds(const Particle &p) const
{
    // "above" used in a relative sense
    //bool aboveFace[MAX_POLYGON_FACES];
    bool outsidePolygon = false;
    //Particle pCopy = p;
    for (size_t faceIndex = 0; faceIndex < _numFaces; faceIndex++)
    {
        // if any of these checks are true, then this flag will be true as well
        outsidePolygon |= (glm::dot(p._position - _faceCenterPoints[faceIndex], _faceNormals[faceIndex]) > 0);
    }

    return outsidePolygon;
}
