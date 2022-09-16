#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dx12.h>
#include <cstdlib>
#include<cassert>
#include "WinApp.h"
//using namespace DirectX;
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
using namespace Microsoft::WRL;

class DirectXCommon
{
private:
	
	WinApp* winapp=nullptr;
	ComPtr<IDXGIFactory6>dxgiFactory;
	ComPtr<ID3D12Device> dev;
	ComPtr<IDXGISwapChain4> swapchain;
	ComPtr<IDXGISwapChain1> swapchain1;
	ComPtr<ID3D12CommandAllocator>cmdAllocator;
	ID3D12GraphicsCommandList* cmdList;
	ComPtr<ID3D12CommandQueue>cmdQueue;
	ComPtr<ID3D12DescriptorHeap>rtvHeaps;
	ComPtr<ID3D12DescriptorHeap>dsvHeap;
	ComPtr<ID3D12Fence>fence;
	std::vector<ComPtr<ID3D12Resource>>backbuffers;
	//���\�[�X�ݒ�
	ComPtr<ID3D12Resource>depthBuffer;
	CD3DX12_RESOURCE_DESC depthResDesc;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	//�[�xview�p�ŃX�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	UINT64 fenceVal = 0;
	//�E�B���h�E�T�C�Y�̎w��
	int window_width = 1820;
	int  window_height = 780;
	static UINT bbIndex;
	//�[�x�e�X�g�p
public:
	//float clearColor[] { 0.3,0.3,0.3,0 };
	//�e�평��������
	void Initialize(WinApp*winapp);
	void InitializeDevice();
	void InitializeCommand();
	void InitializeSwapchain();
	void InitializeRenderTargetView();
	void InitializeDepthBuffer();
	void InitializeFence();
	//�`��O�E�㏈��
	void BeginDraw();
	void EndDraw();
	//�Q�b�^�[
	ID3D12Device* GetDev() { return dev.Get(); }
	ID3D12GraphicsCommandList* GetCmdList() { return cmdList; }
	
};

