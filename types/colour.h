#pragma once

namespace RENDER {
    
    struct R_COLOUR {

        R_COLOUR() = default;
        R_COLOUR(float r, float g, float b, float a = 1.f) : 
            m_r(r), 
            m_g(g), 
            m_b(b), 
            m_a(a) {}

        float m_r = 0.f;
        float m_g = 0.f;
        float m_b = 0.f;
        float m_a = 1.f;
    };
} 