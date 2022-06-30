#pragma once

#include "colour.h"
#include "vec2d.h"

namespace RENDER {
    
    struct R_VERTEX {

        R_VERTEX() = default;
        R_VERTEX(float posX, float posY, const RENDER::R_COLOUR& colour, float texPosX = 0.f, float texPosY = 0.f) : 
            m_position(posX, posY), 
            m_colour(colour), 
            m_texturePosition(texPosX, texPosY) { }

        RENDER::R_VEC2D m_position = {};  
        RENDER::R_COLOUR m_colour = {};
        RENDER::R_VEC2D m_texturePosition = {};
    };
} 
