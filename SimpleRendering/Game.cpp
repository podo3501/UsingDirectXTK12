//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    //   Add DX::DeviceResources::c_ReverseDepth to optimize depth buffer clears for 0 instead of 1.
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());
    elapsedTime;

    auto time = static_cast<float>(m_timer.GetTotalSeconds());

    float yaw = time * 0.4f;
    float pitch = time * 0.7f;
    float roll = time * 1.1f;

    auto quat = Quaternion::CreateFromYawPitchRoll(pitch, yaw, roll);
    auto light = XMVector3Rotate(g_XMOne, quat);

    m_effect->SetLightDirection(0, light);

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap(), m_states->Heap() };
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

    m_effect->Apply(commandList);

    m_batch->Begin(commandList);

    /*VertexPositionColor v1(Vector3(0.f, 0.5f, 0.5f), Colors::Yellow);
    VertexPositionColor v2(Vector3(0.5f, -0.5f, 0.5f), Colors::Yellow);
    VertexPositionColor v3(Vector3(-0.5f, -0.5f, 0.5f), Colors::Yellow);*/

    /*VertexPositionColor v1(Vector3(400.f, 150.f, 0.f), Colors::Red);
    VertexPositionColor v2(Vector3(600.f, 450.f, 0.f), Colors::Green);
    VertexPositionColor v3(Vector3(200.f, 450.f, 0.f), Colors::Blue);*/

    /*VertexPositionTexture v1(Vector3(400.f, 150.f, 0.f), Vector2(.5f, 0));
    VertexPositionTexture v2(Vector3(600.f, 450.f, 0.f), Vector2(1, 1));
    VertexPositionTexture v3(Vector3(200.f, 450.f, 0.f), Vector2(0, 1));*/

    VertexPositionNormalTexture v1(Vector3(400.f, 150.f, 0.f), -Vector3::UnitZ, Vector2(.5f, 0));
    VertexPositionNormalTexture v2(Vector3(600.f, 450.f, 0.f), -Vector3::UnitZ, Vector2(1, 1));
    VertexPositionNormalTexture v3(Vector3(200.f, 450.f, 0.f), -Vector3::UnitZ, Vector2(0, 1));

    m_batch->DrawTriangle(v1, v2, v3);
   
    m_batch->End(); 

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

    PIXEndEvent();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto const rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto const dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto const viewport = m_deviceResources->GetScreenViewport();
    auto const scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // If using the DirectX Tool Kit for DX12, uncomment this line:
     m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

     m_states = std::make_unique<CommonStates>(device);
     m_batch = std::make_unique<PrimitiveBatch<VertexType>>(device);

     RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
         m_deviceResources->GetDepthBufferFormat());

     m_resourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);

     ResourceUploadBatch resourceUpload(device);

     resourceUpload.Begin();

     DX::ThrowIfFailed(
         CreateWICTextureFromFile(device, resourceUpload, L"rocks.jpg", m_texture.ReleaseAndGetAddressOf()));

     CreateShaderResourceView(device, m_texture.Get(), m_resourceDescriptors->GetCpuHandle(Descriptors::Rocks));

     DX::ThrowIfFailed(
         CreateDDSTextureFromFile(device, resourceUpload, L"rocks_normalmap.dds", m_normalMap.ReleaseAndGetAddressOf()));

     CreateShaderResourceView(device, m_normalMap.Get(), m_resourceDescriptors->GetCpuHandle(Descriptors::NormalMap));

     auto uploadResourcesFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());

     uploadResourcesFinished.wait();

     EffectPipelineStateDescription pd(
         &VertexType::InputLayout,
         CommonStates::Opaque,
         CommonStates::DepthDefault,
         CommonStates::CullCounterClockwise,
         rtState);

     m_effect = std::make_unique<NormalMapEffect>(device, EffectFlags::None, pd);
     m_effect->SetTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::Rocks), m_states->LinearClamp());
     m_effect->SetNormalTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::NormalMap));

     m_effect->EnableDefaultLighting();
     m_effect->SetLightDiffuseColor(0, Colors::Gray);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();

    Matrix proj = Matrix::CreateScale(2.f / float(size.right),
        -2.f / float(size.bottom), 1.f) * Matrix::CreateTranslation(-1.f, 1.f, 0.f);
    m_effect->SetProjection(proj);
}

void Game::OnDeviceLost()
{
    m_effect.reset();
    m_batch.reset();

    m_texture.Reset();
    m_resourceDescriptors.reset();

    m_normalMap.Reset();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
