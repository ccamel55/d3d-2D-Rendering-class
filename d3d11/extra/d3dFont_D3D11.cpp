#include "d3dFont_D3D11.h"

#include "../../extra/helpers.h"
#include "../../types/vertex.h"

#include "../renderD3D11.h"

RENDER::D3D11::d3dFont_D3D11::d3dFont_D3D11(const std::string& fontFamily, UINT fontHeight, UINT fontWeight, UINT defaultFlags = 0) {

    m_fontFamily = fontFamily;
    m_fontHeight = fontHeight;
    m_fontWeight = fontWeight;
    m_defaultFlags = defaultFlags;

    m_texWidth = 128;
    m_texHeight = 128;
}

RENDER::D3D11::d3dFont_D3D11::~d3dFont_D3D11() {
    destroy();
}

void RENDER::D3D11::d3dFont_D3D11::init(ID3D11Device* device) {

    if (m_init)
        return;

    auto checkTextureSize = [&](HDC GDIContext) -> bool {
        
        SIZE size;
        char chr[2] = "x";

        GetTextExtentPoint32A(GDIContext, chr, 1, &size);
        const auto m_spacing = static_cast<long>(ceil(size.cy * 0.3f));

        long x = m_spacing;
        long y = 0;

        for (char c = 32; c < 127; c++)
        {
            chr[0] = c;
            GetTextExtentPoint32A(GDIContext, chr, 1, &size);

            if (x + size.cx + m_spacing > m_texWidth)
            {
                x = m_spacing;
                y += size.cy + 1;
            }

            if (y + size.cy > m_texHeight)
                return false;

            x += size.cx + (2 * m_spacing);
        }

        return true;
    };

    auto generateTextureCoords = [&](HDC GDIContext) -> void {

        SIZE size;
        char chr[2] = "x";

        GetTextExtentPoint32A(GDIContext, chr, 1, &size);
        m_texSpacing = static_cast<long>(ceil(size.cy * 0.3f));

        long x = m_texSpacing;
        long y = 0;

        for (char c = 32; c < 127; c++)
        {
            chr[0] = c;
            GetTextExtentPoint32A(GDIContext, chr, 1, &size);

            if (x + size.cx + m_texSpacing > m_texWidth)
            {
                x = m_texSpacing;
                y += size.cy + 1;
            }

            ExtTextOutA(GDIContext, x + 0, y + 0, ETO_OPAQUE, nullptr, chr, 1, nullptr);

            m_texCoords[c - 32][0] = (static_cast<float>(x - m_texSpacing)) / m_texWidth;
            m_texCoords[c - 32][1] = (static_cast<float>(y)) / m_texHeight;
            m_texCoords[c - 32][2] = (static_cast<float>(x + size.cx + m_texSpacing)) / m_texWidth;
            m_texCoords[c - 32][3] = (static_cast<float>(y + size.cy)) / m_texHeight;

            x += size.cx + (2 * m_texSpacing);
        }
    };

    m_init = true;
}

void RENDER::D3D11::d3dFont_D3D11::destroy() {

    RENDER::HELPER::safeRelease(m_fontTexture);
    m_init = false;
}

void RENDER::D3D11::d3dFont_D3D11::drawFont(float x, float y, const RENDER::R_COLOUR &colour, const std::string& text, UINT flags) {

    // add our custom flags
    auto curFlags = m_defaultFlags | flags;

    if (curFlags & RENDER::E_FONT_FLAGS::FONT_CENTERED_X)
    {
        x -= getTextWidth(text) * 0.5f;
        x = roundf(x) + 1.f;
    }

    if (curFlags & RENDER::E_FONT_FLAGS::FONT_CENTERED_Y)
    {
        y -= getTextHeight() * 0.5f;
        y = roundf(y) + 1.f;
    }

    if (curFlags & RENDER::E_FONT_FLAGS::FONT_RIGHT_ALIGNED)
    {
        x -= getTextWidth(text);
        x = roundf(x) + 1.f;
    }

    x -= m_texSpacing;

    for (const auto& c : text)
    {
        if (c < ' ')
            continue;

        float tx1 = m_texCoords[c - 32][0];
        float ty1 = m_texCoords[c - 32][1];
        float tx2 = m_texCoords[c - 32][2];
        float ty2 = m_texCoords[c - 32][3];

        float w = (tx2 - tx1) * m_texWidth;
        float h = (ty2 - ty1) * m_texHeight;

        if (c != ' ')
        {
            // outline right
            std::vector<RENDER::R_VERTEX> verts =
            {
                RENDER::R_VERTEX(x,     y + h, {0.f, 0.f, 0.f, 1.f},   tx1,    ty2),
                RENDER::R_VERTEX(x,     y,     {0.f, 0.f, 0.f, 1.f},   tx1,    ty1),
                RENDER::R_VERTEX(x + w, y + h, {0.f, 0.f, 0.f, 1.f},   tx2,    ty2),

                RENDER::R_VERTEX(x + w, y,     {0.f, 0.f, 0.f, 1.f},   tx2,    ty1),
                RENDER::R_VERTEX(x + w, y + h, {0.f, 0.f, 0.f, 1.f},   tx2,    ty2),
                RENDER::R_VERTEX(x,     y,     {0.f, 0.f, 0.f, 1.f},   tx1,    ty1)
            };

            if (curFlags & RENDER::E_FONT_FLAGS::FONT_OUTLINE)
            {
                renderD3D11.addVertexVector(verts, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_fontTexture);

                // outline left
                verts[0].m_position.m_x -= 2.f; verts[1].m_position.m_x -= 2.f; verts[2].m_position.m_x -= 2.f;
                verts[3].m_position.m_x -= 2.f; verts[4].m_position.m_x -= 2.f; verts[5].m_position.m_x -= 2.f;

                renderD3D11.addVertexVector(verts, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_fontTexture);

                // outline top
                verts[0].m_position.m_x += 1.f; verts[1].m_position.m_x += 1.f; verts[2].m_position.m_x += 1.f;
                verts[3].m_position.m_x += 1.f; verts[4].m_position.m_x += 1.f; verts[5].m_position.m_x += 1.f;

                verts[0].m_position.m_y += 1.f; verts[1].m_position.m_y += 1.f; verts[2].m_position.m_y += 1.f;
                verts[3].m_position.m_y += 1.f; verts[4].m_position.m_y += 1.f; verts[5].m_position.m_y += 1.f;

                renderD3D11.addVertexVector(verts, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_fontTexture);

                // outline bottom
                verts[0].m_position.m_y -= 2.f; verts[1].m_position.m_y -= 2.f; verts[2].m_position.m_y -= 2.f;
                verts[3].m_position.m_y -= 2.f; verts[4].m_position.m_y -= 2.f; verts[5].m_position.m_y -= 2.f;

                renderD3D11.addVertexVector(verts, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_fontTexture);
            }

            // main font
            verts[0].m_position.m_y += 1.f; verts[1].m_position.m_y += 1.f; verts[2].m_position.m_y += 1.f;
            verts[3].m_position.m_y += 1.f; verts[4].m_position.m_y += 1.f; verts[5].m_position.m_y += 1.f;

            verts[0].m_colour = colour; verts[1].m_colour = colour; verts[2].m_colour = colour;
            verts[3].m_colour = colour; verts[4].m_colour = colour; verts[5].m_colour = colour;

            renderD3D11.addVertexVector(verts, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_fontTexture);
        }

        x += w - (2.f * m_texSpacing);
    }
}

float RENDER::D3D11::d3dFont_D3D11::getTextWidth(const std::string& text) {

    float width = 0.f;
    float rowWidth = 0.f;

    for (const auto& c : text)
    {
        if (c < ' ')
            continue;

        float tx1 = m_texCoords[c - 32][0];
        float tx2 = m_texCoords[c - 32][2];

        rowWidth += (tx2 - tx1) * m_texWidth - 2.f * m_texSpacing;

        if (rowWidth > width)
            width = rowWidth;
    }

    return width;
}

float RENDER::D3D11::d3dFont_D3D11:: getTextHeight() {
    return (m_texCoords[0][3] - m_texCoords[0][1]) * static_cast<float>(m_texHeight);
}