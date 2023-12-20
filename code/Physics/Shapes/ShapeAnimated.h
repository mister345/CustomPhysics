#pragma once
#include "ShapeSphere.h"

class ShapeAnimated : public ShapeSphere {
    public:
        ShapeAnimated( const float size, const bool _isOrigin ) :
            ShapeSphere( size ),
            isOrigin( _isOrigin ) { 
            color = isOrigin ? RED : BLUE;
        }
        bool isOrigin = false;
};

