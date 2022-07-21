#include "stateBlock.h"

#include "../../extra/helpers.h"

RENDER::D3D11::stateBlock::~stateBlock() {

    m_context = nullptr;

	RENDER::HELPER::safeRelease(m_vertexShader);
	RENDER::HELPER::safeRelease(m_pixelShader);
	RENDER::HELPER::safeRelease(m_pixelShaderResource);
	RENDER::HELPER::safeRelease(m_rasterizerState);
	RENDER::HELPER::safeRelease(m_depthStencilState);
	RENDER::HELPER::safeRelease(m_inputLayout);
	RENDER::HELPER::safeRelease(m_blendState);
	RENDER::HELPER::safeRelease(m_samplerState);
	RENDER::HELPER::safeRelease(m_vsShaderBuffer);
}

void RENDER::D3D11::stateBlock::capture(ID3D11DeviceContext* context) {

    // we should be storing the state of many other things but eh
	m_context = context;

    if (m_context == nullptr)
        return;

    auto scissorCount = (UINT)D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    m_context->RSGetScissorRects(&scissorCount, &m_scissorRect);

	m_context->VSGetShader(&m_vertexShader, NULL, 0);
	m_context->VSGetConstantBuffers(0, 1, &m_vsShaderBuffer);

	m_context->PSGetShader(&m_pixelShader, NULL, 0);
	m_context->PSGetSamplers(0, 1, &m_samplerState);
	m_context->PSGetShaderResources(0, 1, &m_pixelShaderResource);

	m_context->OMGetBlendState(&m_blendState, NULL, &m_samplemask);
	m_context->OMGetDepthStencilState(&m_depthStencilState, &m_stencilRef);

	m_context->IAGetInputLayout(&m_inputLayout);
	m_context->RSGetState(&m_rasterizerState);
}

void RENDER::D3D11::stateBlock::apply() {

	if (m_context == nullptr)
        return;

    auto scissorCount = (UINT)D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    m_context->RSSetScissorRects(scissorCount, &m_scissorRect);

	m_context->VSSetShader(m_vertexShader, NULL, 0);
	m_context->VSSetConstantBuffers(0, 1, &m_vsShaderBuffer);

	m_context->PSSetShader(m_pixelShader, NULL, 0);
	m_context->PSSetSamplers(0, 1, &m_samplerState);
	m_context->PSSetShaderResources(0, 1, &m_pixelShaderResource);

	m_context->OMSetBlendState(m_blendState, NULL, m_samplemask);
	m_context->OMSetDepthStencilState(m_depthStencilState, m_stencilRef);

	m_context->IASetInputLayout(m_inputLayout);
	m_context->RSSetState(m_rasterizerState);
}