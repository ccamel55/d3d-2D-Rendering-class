#pragma once

namespace RENDER {
    
    struct R_VEC2D {

        R_VEC2D() = default;
        R_VEC2D(float x, float y) : 
            m_x(x),
            m_y(y) {}

        float m_x = 0.f;
        float m_y = 0.f;
    };
} 
