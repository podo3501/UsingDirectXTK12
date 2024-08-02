#pragma once

namespace DX
{
	class SkyboxEffect : public DirectX::IEffect, public DirectX::IEffectMatrices
	{
	public:
		SkyboxEffect(
			ID3D12Device* device,
			const DirectX::EffectPipelineStateDescription& pipelineStateDesc);

		void Apply(ID3D12GraphicsCommandList* commandList) override;

		void SetTexture(
			D3D12_GPU_DESCRIPTOR_HANDLE srvDescriptor,
			D3D12_GPU_DESCRIPTOR_HANDLE samplerDescriptor);

		void XM_CALLCONV SetWorld(DirectX::FXMMATRIX value) override;
		void XM_CALLCONV SetView(DirectX::FXMMATRIX value) override;
		void XM_CALLCONV SetProjection(DirectX::FXMMATRIX value) override;
		void XM_CALLCONV SetMatrices(
			DirectX::FXMMATRIX world,
			DirectX::CXMMATRIX,
			DirectX::CXMMATRIX projection) override;

	private:
		enum Descriptors
		{
			InputSRV,
			InputSampler,
			ConstantBuffer,
			Count
		};

		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSig;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

		D3D12_GPU_DESCRIPTOR_HANDLE m_texture;
		D3D12_GPU_DESCRIPTOR_HANDLE m_textureSampler;

		DirectX::SimpleMath::Matrix m_view;
		DirectX::SimpleMath::Matrix m_proj;
		DirectX::SimpleMath::Matrix m_worldViewProj;

		uint32_t m_dirtyFlags;

		DirectX::GraphicsResource m_constantBuffer;
	};
}
