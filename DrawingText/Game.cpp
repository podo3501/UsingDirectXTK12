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

    // TODO: Add your game logic here.
    elapsedTime;

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

    ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap() };
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

    m_spriteBatch->Begin(commandList);

    //draw font
    /*const wchar_t* output = L"Hello World";
    Vector2 origin = m_font->MeasureString(output) / 2.f;
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos, Colors::White, 0.f, origin);*/

    //draw wstring
    //std::wstring output = std::wstring(L"Hello") + std::wstring(L" World");
    //Vector2 origin = m_font->MeasureString(output.c_str()) / 2.f;
    //m_font->DrawString(m_spriteBatch.get(), output.c_str(), m_fontPos, Colors::White, 0.f, origin);

    //draw ASCII text
    /*const char* ascii = "Hello World";
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring output = converter.from_bytes(ascii);

    Vector2 origin = m_font->MeasureString(output.c_str()) / 2.f;
    m_font->DrawString(m_spriteBatch.get(), output.c_str(), m_fontPos, Colors::White, 0.f, origin);*/

    //drop-shadow effect
    /*const wchar_t* output = L"Hello World";

    Vector2 origin = m_font->MeasureString(output) / 2.f;
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos + Vector2(1.f, 1.f), Colors::Black, 0.f, origin);
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos + Vector2(-1.f, 1.f), Colors::Black, 0.f, origin);
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos, Colors::White, 0.f, origin);*/

    //outline effect
    const wchar_t* output = L"Hello World";

    Vector2 origin = m_font->MeasureString(output) / 2.f;
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos + Vector2(1.f, 1.f), Colors::Black, 0.f, origin);
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos + Vector2(-1.f, 1.f), Colors::Black, 0.f, origin);
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos + Vector2(-1.f, -1.f), Colors::Black, 0.f, origin);
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos + Vector2(1.f, -1.f), Colors::Black, 0.f, origin);
    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos, Colors::White, 0.f, origin);

    m_spriteBatch->End();

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

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

     m_resourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);

     ResourceUploadBatch resourceUpload(device);

     resourceUpload.Begin();

     m_font = std::make_unique<SpriteFont>(device, resourceUpload, L"myfileHangle.spritefont",
         m_resourceDescriptors->GetCpuHandle(Descriptors::MyFont),
         m_resourceDescriptors->GetGpuHandle(Descriptors::MyFont));

     RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
         m_deviceResources->GetDepthBufferFormat());

     SpriteBatchPipelineStateDescription pd(rtState);
     m_spriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);

     auto uploadResourceFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());
     uploadResourceFinished.wait();
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto viewport = m_deviceResources->GetScreenViewport();
    m_spriteBatch->SetViewport(viewport);

    auto size = m_deviceResources->GetOutputSize();
    m_fontPos.x = float(size.right) / 2.f;
    m_fontPos.y = float(size.bottom) / 2.f;
}

void Game::OnDeviceLost()
{
    m_font.reset();
    m_resourceDescriptors.reset();
    m_spriteBatch.reset();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
     m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
