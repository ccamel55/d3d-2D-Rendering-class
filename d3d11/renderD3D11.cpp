#include "renderD3D11.h"

#include "../extra/helpers.h"
#include "shaders/shaders.h"

void c_renderD3D11::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext) {

    if (m_hasInit)
        return;

    m_device = device;
    m_device->AddRef();

    m_context = deviceContext;
    m_context->AddRef();

    // compile our shaders and assign them to repsective buffers 
    {
        RENDER::HELPER::throwIfFailed(D3DCompile(vertexShader, std::size(vertexShader), NULL, NULL, NULL, "VS", "vs_4_0", NULL, NULL, &m_vertexShaderBuffer, NULL));
		RENDER::HELPER::throwIfFailed(m_device->CreateVertexShader(m_vertexShaderBuffer->GetBufferPointer(), m_vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader));

        // used when we arnt drawing a texture
		RENDER::HELPER::throwIfFailed(D3DCompile(pixelShader, std::size(pixelShader), NULL, NULL, NULL, "PSC", "ps_4_0", NULL, NULL, &m_pixelShaderBufferCol, NULL));
        RENDER::HELPER::throwIfFailed(m_device->CreatePixelShader(m_pixelShaderBufferCol->GetBufferPointer(), m_pixelShaderBufferCol->GetBufferSize(), NULL, &m_pixelShaderCol));

        // used when we are drawing a texture
		RENDER::HELPER::throwIfFailed(D3DCompile(pixelShader, std::size(pixelShader), NULL, NULL, NULL, "PST", "ps_4_0", NULL, NULL, &m_pixelShaderBufferTex, NULL));
        RENDER::HELPER::throwIfFailed(m_device->CreatePixelShader(m_pixelShaderBufferTex->GetBufferPointer(), m_pixelShaderBufferTex->GetBufferSize(), NULL, &m_pixelShaderTex));
    }

    // create input layout of each vertex
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0 }, // float float : VEC2D
			{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }, // float float float float : COLOUR
			{ "TEX_POS",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }, // FLOAT FLOAT : VEC2D
		};

		RENDER::HELPER::throwIfFailed(m_device->CreateInputLayout(layout, static_cast<UINT>(std::size(layout)), m_vertexShaderBuffer->GetBufferPointer(), m_vertexShaderBuffer->GetBufferSize(), &m_inputLayout));
	}

    // create vertex buffer
    {
		D3D11_BUFFER_DESC bufferDesc = {};
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));

		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(RENDER::R_VERTEX) * RENDER::MAX_VERTICES;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;

		RENDER::HELPER::throwIfFailed(m_device->CreateBuffer(&bufferDesc, NULL, &m_vertexBuffer));
	}

    // create screen projection matrix
    {
        // init screenProjectionBuffer
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));

            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.ByteWidth = sizeof(float[4][4]);
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.MiscFlags = 0;

            RENDER::HELPER::throwIfFailed(m_device->CreateBuffer(&bufferDesc, NULL, &m_screenProjectionBuffer));
        }

        UINT numViewports = 1;
		D3D11_VIEWPORT viewport = {};

        // make sure you set the viewport when you initialize, we will use scissor regions to cut items
        m_context->RSGetViewports(&numViewports, &viewport);

        // set scissor rect, we will move this to renderFrame once we add clipping to the renderer
        {
            const D3D11_RECT r = { 
                
                static_cast<LONG>(viewport.TopLeftX), 
                static_cast<LONG>(viewport.TopLeftY), 
                static_cast<LONG>(viewport.Width), 
                static_cast<LONG>(viewport.Height) 
                };

            m_context->RSSetScissorRects(1, &r);
        }

        // create projection matrix and map to buffer, credits IMGUI
        {
            const auto L = viewport.TopLeftX;
		    const auto R = viewport.TopLeftX + viewport.Width;
		    const auto T = viewport.TopLeftY;
		    const auto B = viewport.TopLeftY + viewport.Height;

            RENDER::RENDER_MATRIX projectionMatrix = {
                { 2.0f / (R - L),       0.0f,               0.0f,       0.0f },
                { 0.0f,                 2.0f / (T - B),     0.0f,       0.0f },
                { 0.0f,                 0.0f,               0.5f,       0.0f },
                { (R + L) / (L - R),    (T + B) / (B - T),  0.5f,       1.0f }
            };

            D3D11_MAPPED_SUBRESOURCE mappedResource = {};
            RENDER::HELPER::throwIfFailed(m_context->Map(m_screenProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

            std::memcpy(mappedResource.pData, &projectionMatrix, sizeof(float[4][4]));
            m_context->Unmap(m_screenProjectionBuffer, 0);
        }
    }

    // init m_blendState
	{
		D3D11_BLEND_DESC blendDesc = {};
		ZeroMemory(&blendDesc, sizeof(blendDesc));

		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		RENDER::HELPER::throwIfFailed(m_device->CreateBlendState(&blendDesc, &m_blendState));
	}

	// init samplerState
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = 0.f;

		RENDER::HELPER::throwIfFailed(m_device->CreateSamplerState(&samplerDesc, &m_samplerState));
	}

	// init rasterizerState
	{
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));

		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.DepthClipEnable = true;

		RENDER::HELPER::throwIfFailed(m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState));
	}

	// init depthStencilState
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilState = {};
		ZeroMemory(&depthStencilState, sizeof(depthStencilState));

		depthStencilState.DepthEnable = false;
		depthStencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilState.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilState.StencilEnable = false;

		depthStencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilState.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		depthStencilState.BackFace = depthStencilState.FrontFace;

		RENDER::HELPER::throwIfFailed(m_device->CreateDepthStencilState(&depthStencilState, &m_depthStencilState));
	}

    m_hasInit = true;
}

void c_renderD3D11::destroy() {

    if (m_hasInit == false)
        return;

    RENDER::HELPER::safeRelease(m_device);
	RENDER::HELPER::safeRelease(m_context);
	RENDER::HELPER::safeRelease(m_inputLayout);

    // shader
    RENDER::HELPER::safeRelease(m_vertexShader);
	RENDER::HELPER::safeRelease(m_pixelShaderCol);
	RENDER::HELPER::safeRelease(m_pixelShaderTex);

	RENDER::HELPER::safeRelease(m_vertexShaderBuffer);
	RENDER::HELPER::safeRelease(m_pixelShaderBufferCol);
	RENDER::HELPER::safeRelease(m_pixelShaderBufferTex);

    // buffers
	RENDER::HELPER::safeRelease(m_vertexBuffer);
    RENDER::HELPER::safeRelease(m_screenProjectionBuffer);

    // states
	RENDER::HELPER::safeRelease(m_rasterizerState);
	RENDER::HELPER::safeRelease(m_depthStencilState);
	RENDER::HELPER::safeRelease(m_blendState);
	RENDER::HELPER::safeRelease(m_samplerState);

    m_hasInit = false;
}

void c_renderD3D11::beginFrame() {

    // create a lot to draw into
    m_renderLots.emplace_back();

    // get old render state, so we dont fuck with anything after drawing our stuff
    m_oldRenderState.capture(m_context);

    m_context->VSSetShader(m_vertexShader, NULL, 0);
	m_context->VSSetConstantBuffers(0, 1, &m_screenProjectionBuffer);

	m_context->PSSetSamplers(0, 1, &m_samplerState);

	const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	m_context->OMSetBlendState(m_blendState, blend_factor, 0xffffffff);
	m_context->OMSetDepthStencilState(m_depthStencilState, 0);

	UINT offset = 0;
	UINT stride = sizeof(RENDER::R_VERTEX);

	m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_context->IASetInputLayout(m_inputLayout);

	m_context->RSSetState(m_rasterizerState);
}

void c_renderD3D11::finishFrame() {

    for (const auto& lot : m_renderLots)
	{
		if (lot.m_vertices.empty())
			continue;

		// map vertexs from our array to the vertex buffer
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		m_context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		std::memcpy(mappedResource.pData, lot.m_vertices.data(), sizeof(RENDER::R_VERTEX) * lot.m_vertices.size());
		m_context->Unmap(m_vertexBuffer, 0);

		UINT offset = 0;

		// draw from vertex array with topology and texture
		for (const auto& batch : lot.m_drawBatchs)
		{
			if (batch.m_texture == nullptr)
			{
				// use normal color shader
				m_context->PSSetShader(m_pixelShaderCol, NULL, 0);
			}
			else
			{
				// use texture shader
				m_context->PSSetShader(m_pixelShaderTex, NULL, 0);
				m_context->PSSetShaderResources(0, 1, &batch.m_texture);
			}

			m_context->IASetPrimitiveTopology(batch.m_topology);
			m_context->Draw(batch.m_vertexCount, offset);

			// move to next set of vertices
			offset += batch.m_vertexCount;
		}
	}

    // restore
	m_renderLots.clear();
	m_oldRenderState.apply();
}

void c_renderD3D11::addVertexVector(const std::vector<RENDER::R_VERTEX>& vertices, D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11ShaderResourceView* texture)
{
	// make sure we arnt overloading our vertex buffer
	if (m_renderLots.back().m_vertices.size() + vertices.size() >= RENDER::MAX_VERTICES - 1)
	{
		// create new lot of so
		// we should also check if verts >= maxVertices
		m_renderLots.emplace_back();
	}

	auto& backLot = m_renderLots.back();

	if (backLot.m_drawBatchs.empty() ||
		backLot.m_drawBatchs.back().m_topology != topology ||
		backLot.m_drawBatchs.back().m_texture != texture)
	{
		// create new render batch if we changed topology or texutre
		backLot.m_drawBatchs.emplace_back(texture, topology);
	}

	// write vector to our vertices vector
	backLot.m_vertices.resize(backLot.m_vertices.size() + vertices.size());
	std::memcpy(&backLot.m_vertices.at(backLot.m_vertices.size() - vertices.size()), &vertices.at(0), vertices.size() * sizeof(RENDER::R_VERTEX));

	backLot.m_drawBatchs.back().m_vertexCount += static_cast<UINT>(vertices.size());

	switch (topology)
	{
	case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
	case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
	{
		// we need to make a dummy batch to clear the renderer
		backLot.m_drawBatchs.emplace_back(nullptr, D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);

		backLot.m_vertices.emplace_back();
		backLot.m_drawBatchs.back().m_vertexCount += 1;

		break;
	}
	default:
		break;
	}
}