#pragma once

#include <string>
#include <d3d11_1.h>

// CD3DFont but converted to d3d11 coz its simple, fast and just works.

namespace RENDER {

    namespace D3D11 {

        class d3dFont_D3D11 {
        public:
            d3dFont_D3D11(const std::string& fontFamily, UINT fontHeight, UINT fontWeight, UINT defaultFlags = 0);
            ~d3dFont_D3D11();
            void init(ID3D11Device* device);
            void destroy();
            void drawFont(float x, float y, const RENDER::R_COLOUR &colour, const std::string& text, UINT flags = 0);
            float getTextWidth(const std::string& text);
            float getTextHeight();
        private:
            bool m_init = false;
            UINT m_defaultFlags = 0;

            std::string m_fontFamily = "";

            UINT m_fontHeight = 0;
            UINT m_fontWeight = 0;

            long m_texWidth = 0;
            long m_texHeight = 0;
            long m_texSpacing = 0;

            float m_texCoords[96][4] = {};

     	    ID3D11ShaderResourceView* m_fontTexture = nullptr;
        };
    }
}