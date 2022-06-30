#pragma once

constexpr char vertexShader[] = R"(

cbuffer screenProjectionBuffer : register(b0)
{
    matrix projection;
};
 
struct VS_INPUT
{
    float2 m_pos : POSITION;
    float4 m_col : COLOR;
    float2 m_tex : TEX_POS;
};

struct PS_INPUT
{
    float4 m_pos : SV_POSITION;
    float4 m_col : COLOR;
    float2 m_tex : TEX_POS;
};
 
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
 
    output.m_pos = mul(projection, float4(input.m_pos, 0.f, 1.f));
    output.m_col = input.m_col;
    output.m_tex = input.m_tex;
 
    return output;
}

)";

constexpr char pixelShader[] = R"(

Texture2D texObj : register(t0);
sampler samplerStateObj : register(s0);

struct PS_INPUT
{
    float4 m_pos : SV_POSITION;
    float4 m_col : COLOR;
    float2 m_tex : TEX_POS;
};

float4 PSC(PS_INPUT input) : SV_TARGET
{
    return input.m_col;
}

float4 PST(PS_INPUT input) : SV_TARGET
{
    return texObj.Sample(samplerStateObj, input.m_tex) * input.m_col;
}

)";