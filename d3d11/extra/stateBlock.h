#pragma once

#include <d3d11_1.h>

namespace RENDER {

    namespace D3D11 {
        
        class stateBlock {
        public:
            void capture(ID3D11DeviceContext* context);
            void apply();

            stateBlock() = default;
            ~stateBlock();
        private:
            ID3D11DeviceContext* m_context = nullptr;

            UINT m_samplemask = 0;
            UINT m_stencilRef = 0;

            D3D11_RECT m_scissorRect = {};

            ID3D11VertexShader* m_vertexShader = nullptr;
            ID3D11PixelShader* m_pixelShader = nullptr;
            ID3D11ShaderResourceView* m_pixelShaderResource = nullptr;
            ID3D11RasterizerState* m_rasterizerState = nullptr;
            ID3D11DepthStencilState* m_depthStencilState = nullptr;
            ID3D11InputLayout* m_inputLayout = nullptr;
            ID3D11BlendState* m_blendState = nullptr;
            ID3D11SamplerState* m_samplerState = nullptr;
            ID3D11Buffer* m_vsShaderBuffer = nullptr;
        };
    }
}

