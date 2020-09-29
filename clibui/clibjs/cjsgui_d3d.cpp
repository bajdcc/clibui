//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsgui.h"
#include <render\Direct2D.h>

namespace clib {

    HRESULT cjsgui::draw_d3d() {
        HRESULT hr;

        auto D3D11Device = Direct2D::Singleton().GetDirect3DDevice();
        auto D3D11DeviceContext = Direct2D::Singleton().GetDirect3DDeviceContext();

        struct Vertex {
            float x, y;
            CColor color;
        }; // 点
        const Vertex vertices[] = { // 三个点
            {0.0f, 0.5f,{D2D1::ColorF::Red}},
            {0.5f, -0.5f,{D2D1::ColorF::Green}},
            {-0.5f, -0.5f,{D2D1::ColorF::Blue}}
        };
        CComPtr<ID3D11Buffer> buffer;
        D3D11_BUFFER_DESC bd{};
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.CPUAccessFlags = 0u;
        bd.MiscFlags = 0u;
        bd.ByteWidth = sizeof(vertices);
        bd.StructureByteStride = sizeof(Vertex);
        D3D11_SUBRESOURCE_DATA sd{};
        sd.pSysMem = vertices;
        hr = D3D11Device->CreateBuffer(&bd, &sd, &buffer); // 创建顶点缓存
        if (hr != S_OK) return hr;
        const UINT stride = sizeof(Vertex);
        const UINT offset = 0u;
        D3D11DeviceContext->IASetVertexBuffers(0u, 1u, &buffer.p, &stride, &offset); // 设置顶点缓存

        CComPtr<ID3DBlob> blob;
        CComPtr<ID3D11PixelShader> pshader;
        hr = D3DReadFileToBlob(LR"(.\Shader\PixelShader.cso)", &blob); // 加载像素着色器
        if (hr != S_OK) return hr;
        hr = D3D11Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pshader); // 创建像素着色器
        if (hr != S_OK) return hr;
        D3D11DeviceContext->PSSetShader(pshader.p, nullptr, 0u); // 设置像素着色器

        blob = nullptr;
        CComPtr<ID3D11VertexShader> vshader;
        hr = D3DReadFileToBlob(LR"(.\Shader\VertexShader.cso)", &blob); // 加载顶点着色器
        if (hr != S_OK) return hr;
        hr = D3D11Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vshader); // 创建顶点着色器
        if (hr != S_OK) return hr;
        D3D11DeviceContext->VSSetShader(vshader.p, nullptr, 0u); // 设置顶点着色器

        CComPtr<ID3D11InputLayout> layout; // 给顶点着色器设置入口参数
        const D3D11_INPUT_ELEMENT_DESC ied[] = {
            {"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"Color", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = D3D11Device->CreateInputLayout(ied, (UINT)std::size(ied), blob->GetBufferPointer(), blob->GetBufferSize(), &layout); // 创建入口参数
        if (hr != S_OK) return hr;
        D3D11DeviceContext->IASetInputLayout(layout.p); // 设置入口参数

        D3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 设置读取顶点缓存的方式

        D3D11_VIEWPORT vp{};
        auto size = GLOBAL_STATE.bound.Size();
        vp.Width = (float)size.cx;
        vp.Height = (float)size.cy;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        D3D11DeviceContext->RSSetViewports(1u, &vp); // 设置视口

        D3D11DeviceContext->Draw((UINT)std::size(vertices), 0u);
        if (hr != S_OK) return hr;

        return hr;
    }
}
