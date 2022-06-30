#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <vector>

#include "extra/stateBlock.h"
#include "extra/d3dFont_D3D11.h"

#include "../types/colour.h"
#include "../types/vec2d.h"
#include "../types/vertex.h"

namespace RENDER {

    namespace D3D11 {

        struct R_BATCH {

            R_BATCH() = default;
            R_BATCH(ID3D11ShaderResourceView* texture, D3D11_PRIMITIVE_TOPOLOGY topology) : m_texture(texture), m_topology(topology) { };

            ID3D11ShaderResourceView* m_texture = nullptr;
            D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

            UINT m_vertexCount = 0;
        };

        struct R_RENDER_LOT {
            
            R_RENDER_LOT() {
                
                m_vertices.reserve(RENDER::MAX_VERTICES);        
            }

            std::vector<RENDER::D3D11::R_BATCH> m_drawBatchs = {};
            std::vector<RENDER::R_VERTEX> m_vertices = {};
        };
    }
}

class c_renderD3D11 {
public:
    void init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    void destroy();
    void beginFrame();
    void finishFrame();
    void addVertexVector(const std::vector<RENDER::R_VERTEX>& vertices, D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11ShaderResourceView* texture);
private:
    bool m_hasInit = false;
    RENDER::D3D11::stateBlock m_oldRenderState = {};

    std::vector<RENDER::D3D11::R_RENDER_LOT> m_renderLots = {};
    std::vector<std::shared_ptr<RENDER::D3D11::d3dFont_D3D11>> m_fonts = {};

    ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_context = nullptr;
    ID3D11InputLayout* m_inputLayout = nullptr;

    // shaders
    ID3D10Blob* m_vertexShaderBuffer = nullptr;
	ID3D10Blob* m_pixelShaderBufferCol = nullptr;
	ID3D10Blob* m_pixelShaderBufferTex = nullptr;

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShaderCol = nullptr;
	ID3D11PixelShader* m_pixelShaderTex = nullptr;

    // information buffers
	ID3D11Buffer* m_screenProjectionBuffer = nullptr;
	ID3D11Buffer* m_vertexBuffer = nullptr;

    // render state stuff
	ID3D11RasterizerState* m_rasterizerState = nullptr;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11BlendState* m_blendState = nullptr;
	ID3D11SamplerState* m_samplerState = nullptr;
};

inline c_renderD3D11 renderD3D11;