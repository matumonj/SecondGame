#pragma region include����̐錾
#include<Windows.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<vector>
#include<string>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#define DIRECTiNPUT_VERSION 
#include<dinput.h>
#include"EnemyModel.h"
#include<DirectXTex.h>
#include<wrl.h>
#include"Field.h"
//�܂���z�[��������
//�����
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include"object3D.h"
#include"Sprites.h"
#include"Input.h"
#include"WinApp.h"
#include"DirectXCommon.h"
#include"Background.h"
#include"ObjectNew.h"
#include"MeteoModel.h"
#include"EnemyArm.h"
#include"Bullet.h"
#include"Audio.h"
#define PI 3.141592654
//#include"Device.h"

using namespace DirectX;
using namespace Microsoft::WRL;

static ID3D12Device* g_pd3dDevice = NULL;
static int const NUM_FRAMES_IN_FLIGHT = 3;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
#pragma endregion
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
//�~�����蔻��
float collision(float X1, float Y1, float Z1, float X2, float Y2, float Z2, float R1) {
	float a = X1 - X2;
	float b = Y1 - Y2;
	float c = Z1 - Z2;
	float distance = sqrtf(a * a + b * b + c * c);
	float radius = R1;
	if (distance <= radius) {
		return TRUE;
	} else {
		return FALSE;
	}
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#pragma region DirectX�̏�����
	const int window_width = 1820;
	const int window_height = 780;

	WinApp* winapp = nullptr;
	//������
	winapp = new WinApp();
	winapp->Createwindow();

	MSG msg{};
	//�Ńo�b�O���C���[�̂��
#ifdef _DEBUG
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif
#pragma region �|�C���^�u����
	//DirectX�̋��ʕ���
	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winapp);
	HRESULT result;

	// �L�[�{�[�h�Ȃǂ̓��͏���
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winapp);

#pragma endregion

	//���_�f�[�^3�_���̍��W
#pragma region �`�揉��������
#pragma region ���_�f�[�^�\���̂Ə�����(vertices)	
//���_�f�[�^�\����
	//�O�p���p
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	//�X�v���C�g�p
	struct VertexPosUv
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	//�w�i�p
	struct BackGround
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

	//const float topHeight = 10.0f;
	//const int Divs = 3;
	//const float radius = 10.0f;
	//�O�p���̒��_�f�[�^
	const int division = 30;					// ������
	float radius = 10.0f;				// ��ʂ̔��a
	const float prizmHeight = 10.0f;			// ���̍���
	const int planeCount = division * 2 + division * 2;		// �ʂ̐�
	const int vertexCount = planeCount * 3;		// ���_��
	Vertex vertices[vertexCount];
	unsigned short indices[planeCount * 3];
	// ���_�C���f�b�N�X�z��
	std::vector<Vertex> realVertices;
	// ���_���W�̌v�Z�i�d������j
	{
		realVertices.resize((division + 1) * 2);
		int index = 0;
		float zValue;

		// ���
		zValue = prizmHeight / 2.0f;
		for (int i = 0; i < division; i++)
		{
			XMFLOAT3 vertex;
			vertex.x = radius * sinf(XM_2PI / division * i);
			vertex.y = radius * cosf(XM_2PI / division * i);
			vertex.z = zValue;
			realVertices[index++].pos = vertex;
		}
		realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// ��ʂ̒��S�_
		// �V��
		zValue = -prizmHeight / 2.0f;
		for (int i = 0; i < division; i++)
		{
			XMFLOAT3 vertex;
			vertex.x = radius * sinf(XM_2PI / division * i);
			vertex.y = radius * cosf(XM_2PI / division * i);
			vertex.z = zValue;
			realVertices[index++].pos = vertex;
		}
		realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// �V�ʂ̒��S�_
	}

	// ���_���W�̌v�Z�i�d���Ȃ��j
	{
		int index = 0;
		// ���
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = i + 1;
			unsigned short index1 = i;
			unsigned short index2 = division;

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2]; // ��ʂ̒��S�_
		}
		// ��ʂ̍Ō�̎O�p�`��1�Ԗڂ̃C���f�b�N�X��0�ɏ�������
		vertices[index - 3] = realVertices[0];

		int topStart = division + 1;
		// �V��
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = topStart + i;
			unsigned short index1 = topStart + i + 1;
			unsigned short index2 = topStart + division;

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2]; // �V�ʂ̒��S�_
		}
		// �V�ʂ̍Ō�̎O�p�`��1�Ԗڂ̃C���f�b�N�X��0�ɏ�������
		vertices[index - 2] = realVertices[topStart];

		// ����
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = i + 1;
			unsigned short index1 = topStart + i + 1;
			unsigned short index2 = i;
			unsigned short index3 = topStart + i;

			if (i == division - 1)
			{
				index0 = 0;
				index1 = topStart;
			}

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2];

			vertices[index++] = realVertices[index2];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index3];
		}
	}

	// ���_�C���f�b�N�X�̐ݒ�
	{
		for (int i = 0; i < _countof(indices); i++)
		{
			indices[i] = i;
		}
	}

	// �@�������̌v�Z
	for (int i = 0; i < _countof(indices) / 3; i++)
	{// �O�p�`�P���ƂɌv�Z���Ă���
		// �O�p�`�̃C���f�b�N�X���擾
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		// �O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		// p0��p1�x�N�g���Ap0��p2�x�N�g�����v�Z
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		// �O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);
		// ���K���i������1�ɂ���)
		normal = XMVector3Normalize(normal);
		// ���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);
	}

	//�w�i�p�̔|���S��
	BackGround background[] = {
		{{-20.4f,0.0f,0.0f},{}, {0.0f,1.0f}},
		{{-20.4f,50.7f,0.0f},{}, {0.0f,0.0f}},
		{{20.0f,0.0f,0.0f},{}, {1.0f,1.0f}},
		{{20.0f,50.7f,0.0f},{}, {1.0f,0.0f}},
	};


#pragma endregion


#pragma region �C���f�b�N�X�f�[�^
	//�C���f�b�N�X�f�[�^


	//�w�i�p�̔|���S���@�C���f�b�N�X�f�[�^
	unsigned short backgroundindices[] = {
		0,1,2,
		1,2,3,
	};
	//���_�o�b�t�@�̃T�C�Y
	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * _countof(vertices));

	ComPtr<ID3D12Resource> vertBuff;
	result = dxCommon->GetDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//���z���������擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i];
	}
	//�}�b�v����
	vertBuff->Unmap(0, nullptr);

#pragma region �w�i�p�̔|���S��
	//���_�o�b�t�@�̃T�C�Y
	UINT backsizeVB = static_cast<UINT>(sizeof(BackGround) * _countof(background));
	//���_�o�b�t�@�ւ̐���
	ComPtr<ID3D12Resource>backvertBuff;
	result = dxCommon->GetDev()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&backvertBuff));

	//���z���������擾
	BackGround* backvertMap = nullptr;
	result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
	for (int i = 0; i < _countof(background); i++) {
		backvertMap[i] = background[i];
	}
	//�}�b�v����
	backvertBuff->Unmap(0, nullptr);

#pragma endregion

#pragma endregion

	//���_�o�b�t�@view�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbview{};
	// ���_�o�b�t�@�r���[�̍쐬
	vbview.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbview.SizeInBytes = sizeof(vertices);
	vbview.StrideInBytes = sizeof(vertices[0]);


	//vbview.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//vbview.SizeInBytes = sizeVB;
	//vbview.StrideInBytes = sizeof(Vertex);

	//�C���f�b�N�X�o�b�t�@�̐ݒ�	
	//�C���f�b�N�X
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * _countof(indices));
	//
	//
	//
	ComPtr<ID3D12Resource> indexBuff;
	result = dxCommon->GetDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff)
	);


	//�C���f�b�N�X�o�b�t�@�ւ̃f�[�^�]��
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);

	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}

	indexBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[�̍쐬
	D3D12_INDEX_BUFFER_VIEW ibView{};
	//ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	//ibView.Format = DXGI_FORMAT_R16_UINT;
	//ibView.SizeInBytes = sizeIB;
	// �C���f�b�N�X�o�b�t�@�r���[�̍쐬
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);
#pragma region �w�i�p�̔|���S��
	//���_�o�b�t�@view�̍쐬
	D3D12_VERTEX_BUFFER_VIEW backvbview{};

	backvbview.BufferLocation = backvertBuff->GetGPUVirtualAddress();
	backvbview.SizeInBytes = backsizeVB;
	backvbview.StrideInBytes = sizeof(BackGround);

	//�C���f�b�N�X�o�b�t�@�̐ݒ�	
	ComPtr<ID3D12Resource>backindexBuff;
	UINT backsizeIB = sizeof(backgroundindices);

	//�C���f�b�N�X�o�b�t�@�̐���
	result = dxCommon->GetDev()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeIB), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&backindexBuff));

	//���z���������擾
	unsigned short* backindexMap = nullptr;
	result = backindexBuff->Map(0, nullptr, (void**)&backindexMap);

	//�S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < _countof(backgroundindices); i++) {
		backindexMap[i] = backgroundindices[i];
	}
	backindexBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@view�̍쐬
	D3D12_INDEX_BUFFER_VIEW backibview{};
	backibview.BufferLocation = backindexBuff->GetGPUVirtualAddress();
	backibview.Format = DXGI_FORMAT_R16_UINT;
	backibview.SizeInBytes = sizeof(backgroundindices);

#pragma endregion

	//���z���������擾

	//�ˉe�ϊ��s��̍��
	XMMATRIX matprojection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)window_width / (float)window_height, 0.1f, 1000.0f);

	//�r���[�ϊ��s��
	XMMATRIX matview;
	XMFLOAT3 eye(0, 0, -10);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//�J�����̉�]�p
	float angle = -10.0f;

	//���[���h�ϊ��s��
	XMMATRIX matworld0;

	XMFLOAT3 position;
	position = { 0.0f,0.0f,0.0f };

	XMMATRIX matworld1;


	//�f�X�N���v�^�q�[�v�g���p�̕ϐ�
	const int constantBufferNum = 128;
	//�萔�o�b�t�@�p�ŃX�N���v�^�q�[�v�̐���
	ID3D12DescriptorHeap* basicDescHeap;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = constantBufferNum + 1;

	result = dxCommon->GetDev()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	auto basicHeapHandle0 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		0, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//2
	auto basicHeapHandle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		1, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//3d�I�u�W�F�N�g�̐�
#pragma region �I�u�W�F�N�g����(���f���ł͂Ȃ�)
	const int object_num = 15;

	Object3d object3ds[object_num];//�G�@�̂悤�Ȃ���
	//Object3d player;//�v���C���[
	Object3d backgrounds;//�ۗ�
	Object3d Arm_DamageArea;
	Object3d Arm_DamageArea_Left;
	Object3d Knock_DamageArea;
	Object3d Art_DamageArea;
	//Object3d backgroundsleft;//�ۗ�
	//Object3d backgroundsright;//�ۗ�
	Object3d L_Meteo_DamageArea[4];
	Object3d pmdmodel;
#pragma endregion
#pragma region �I�u�W�F�N�g�̏���������
	//player�p
	//�w�i�p�@���͕ۗ�
	InitializeObject3d(&backgrounds, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Arm_DamageArea, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Arm_DamageArea_Left, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Knock_DamageArea, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Art_DamageArea, 1, dxCommon->GetDev(), basicDescHeap);
	for (int i = 0; i < 4; i++) {
		InitializeObject3d(&L_Meteo_DamageArea[i], 1, dxCommon->GetDev(), basicDescHeap);
	}
#pragma endregion
	//�V�F�[�_���\�[�X�r���[�̃A�h���X�v�Z����
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

	//�n���h���̃A�h���X��i�߂�
	cpuDescHandleSRV.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	gpuDescHandleSRV.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV2 = basicDescHeap->GetGPUDescriptorHandleForHeapStart();
	gpuDescHandleSRV2.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#pragma region �X�v���C�g�̐���
	//���ʃf�[�^�̊֐��Ăяo��
	SpriteCommon spritecommon;
	//�X�v���C�g���ʃf�[�^����
	spritecommon = SpriteCommonCreate(dxCommon->GetDev(), window_width, window_height);
	//�X�v���C�g�p�e�N�X�`���ǂݍ��݂̊֐��Ăяo��
	SpriteCommonLoadTexture(spritecommon, 0, L"Resources/hart.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 1, L"Resources/bosshpb.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 2, L"Resources/bosshp2-export.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 3, L"Resources/title.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 4, L"Resources/SPACE-.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 5, L"Resources/whiteback.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 6, L"Resources/player-hp.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 7, L"Resources/playerhpb.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 8, L"Resources/rule1.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 9, L"Resources/rule2.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 10, L"Resources/Clear.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spritecommon, 11, L"Resources/gameover.png", dxCommon->GetDev());
	//�X�v���C�g�������֐��̌Ăяo��
	Sprite sprite[11];
	for (int i = 0; i < 5; i++) {
		sprite[i] = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	}
	Sprite Boss_HP = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Boss_HP2 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite title = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite space = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite ArtScene = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	//�X�v���C�g�̐���
	Boss_HP = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Player_HP = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Player_HP2 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Rule1 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Rule2 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Clear = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Gameover = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
#pragma endregion
	//WIC�e�N�X�`���̃��[�h
	//�摜�t�@�C���̗p��
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(L"Resources/damagearea-.png", WIC_FLAGS_NONE, &metadata, scratchImg);

	const Image* img = scratchImg.GetImage(0, 0, 0);


	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, metadata.width,
		static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	////3
	ComPtr<ID3D12Resource>texBuff = nullptr;
	result = dxCommon->GetDev()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0), D3D12_HEAP_FLAG_NONE, &texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texBuff));

	//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	result = texBuff->WriteToSubresource(0, nullptr, img->pixels, static_cast<UINT>(img->rowPitch),
		static_cast<UINT>(img->slicePitch));

	////3
	auto basicHeapHandle2 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		2, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));


	////�V�F�[�_���\�[�X�r���[�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	srvDesc.Format = metadata.format;
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	dxCommon->GetDev()->CreateShaderResourceView(texBuff.Get(), &srvDesc, cpuDescHandleSRV);

	auto gpuDescHandle0 = CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		0, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	////2
	auto gpuDescHandle1 = CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		1, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	////3
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle2 = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
		2, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	);
	float angles = 0;
	float angles2 = 0;
	//3D�I�u�W�F�N�g�p�p�C�v���C������
	PipelineSet obj3Dpipelineset = create3Dpipeline(dxCommon->GetDev());
	//�X�v���C�g�p�p�C�v���C���ݒ�
	PipelineSet spriteobj3Dpipelineset = create3DpipelineSprite(dxCommon->GetDev());
	//�w�i�p�p�C�v���C���Z�b�g
	PipelineSet Backpipelineset = createBackpipeline(dxCommon->GetDev());


#pragma region �X�v���C�g�̈ʒu��X�P�[����e�N�X�`���ԍ��̏������ݒ�
	for (int i = 0; i < 5; i++) {
		sprite[i].position = { (float)i * 64,680,0 };
	}
	//boss
	Boss_HP.position = { 60,16,0 };
	Boss_HP2.position.x = Boss_HP.position.x + 180.0;
	Boss_HP2.position.y = Boss_HP.position.y + 0.5;
	Boss_HP2.position.z = 0;
	Boss_HP.scale = { 21,0.7,1 };
	Boss_HP2.scale = { 18,0.7,1 };
	//player
	Player_HP.position = { 60,670,0 };
	Player_HP2.position.x = Player_HP.position.x + 90.0;
	Player_HP2.position.y = Player_HP.position.y + 0.5;
	Player_HP2.position.z = 0;
	Player_HP.scale = { 9,0.8,1 };
	Player_HP2.scale = { 7.53,0.8,1 };
	//�V�[���p
	title.position = { 0,0,0 };
	title.scale = { 29,13,0 };
	space.position = { 700,500,0 };
	space.scale = { 1,1,0 };
	ArtScene.position = { 0,0,0 };
	ArtScene.scale = { 100,100,1 };
	Rule1.position = { -30,0,0 };
	Rule1.scale = { 29,13,0 };
	Rule2.position = { -30,0,0 };
	Rule2.scale = { 29,13,0 };
	Clear.position = { 0,0,0 };
	Clear.scale = { 29,12,0 };
	Gameover.position = { 0,0,0 };
	Gameover.scale = { 29,12,0 };
	//Boss_HP2.position = {(double)Boss_HP.position.x+0.5,(double)Boss_HP.position.y + 0.8,0 };
	//�X�v���g���ƂɃe�N�X�`�����w��
	for (int i = 0; i < 5; i++) {
		sprite[i].texNumber = 0;
	}
	Boss_HP.texNumber = 1;
	Boss_HP2.texNumber = 2;
	Player_HP.texNumber = 7;
	Player_HP2.texNumber = 6;
	title.texNumber = 3;
	space.texNumber = 4;
	ArtScene.texNumber = 5;
	Rule1.texNumber = 8;
	Rule2.texNumber = 9;
	Clear.texNumber = 10;
	Gameover.texNumber = 11;
	//SpriteUpdate(sprite, spritecommon);
#pragma endregion
	//�v���C���[��Y����]
	float pRotation;
	int LorRFlag = 1;
	int scene = 1;
	int movesf = 0;
	int mtimer = 0;

	//���f�������Ƃ��̏���������
	// 3D�I�u�W�F�N�g����
#pragma region 3D�I�u�W�F�N�g�ÓI������
		// �I�[�f�B�I�̏�����
	Audio* audio = nullptr;
	audio = new Audio();
	if (!audio->Initialize()) {
		assert(0);
		return 1;
	}

	if (!ObjectNew::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//�G
	if (!EnemyModel::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//�t�B�[���h
	if (!Field::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//����
	if (!MeteoModel::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//�G�̘r
	if (!EnemyArm::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//�e
	if (!Bullet::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
#pragma endregion
#pragma region �e�I�u�W�F�N�g�̃|�C���^�u����
	ObjectNew* player = new ObjectNew();
	EnemyModel* enemy = new EnemyModel();
	Field* field = new Field();
	MeteoModel* meteo = new MeteoModel();
	MeteoModel* L_meteo[4];
	EnemyArm* arm = new EnemyArm();
	EnemyArm* arm_left = new EnemyArm();
	Bullet* bullet[15];
	for (int i = 0; i < _countof(bullet); i++) {
		bullet[i] = new Bullet();
	}
	for (int i = 0; i < 4; i++) {
		L_meteo[i] = new MeteoModel();
	}
	//�K�E�Z�����Ŏg��
	Bullet* Art_BackWall = new Bullet();
	Bullet* SafeZone[2];
	for (int i = 0; i < _countof(SafeZone); i++) {
		SafeZone[i] = new Bullet();
	}
	//�N���A���̂ݎg��
	Bullet* Clear_BackWall = new Bullet();
#pragma endregion
#pragma region �e�I�u�W�F�N�g�̃��f������
	//���@
	player->CreateModel();
	player = ObjectNew::Create();
	player->Update(matview, matprojection, { 1,1,1,1 });
	//�G
	enemy->CreateModel();
	enemy = EnemyModel::Create();
	enemy->Update(matview, matprojection, { 1,1,0,1 });
	//�t�B�[���h
	field->CreateModel();
	field = Field::Create();
	field->Update(matview, matprojection, { 1,1,1,1 });
	//����
	meteo->CreateModel();
	meteo = MeteoModel::Create();
	meteo->Update(matview, matprojection, { 1,1,1,1 });
	//�G�̘r
	arm->CreateModel();
	arm = EnemyArm::Create();
	arm->Update(matview, matprojection, { 1,1,1,1 });
	//��
	arm_left->CreateModel();
	arm_left = EnemyArm::Create();
	arm_left->Update(matview, matprojection, { 1,1,1,1 });
	//�e
	for (int i = 0; i < _countof(bullet); i++) {
		bullet[i]->CreateModel();
		bullet[i] = Bullet::Create();
		bullet[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	//�L�͈͗���
	for (int i = 0; i < _countof(L_meteo); i++) {
		L_meteo[i]->CreateModel();
		L_meteo[i] = MeteoModel::Create();
		L_meteo[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	//�K�E�Z�����Ŏg��
	Art_BackWall->CreateModel();
	Art_BackWall = Bullet::Create();
	Art_BackWall->Update(matview, matprojection, { 1,1,1,1 });
	//���u
	for (int i = 0; i < _countof(SafeZone); i++) {
		SafeZone[i]->CreateModel();
		SafeZone[i] = Bullet::Create();
		SafeZone[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	//�K�E�Z�����Ŏg��
	Clear_BackWall->CreateModel();
	Clear_BackWall = Bullet::Create();
	Clear_BackWall->Update(matview, matprojection, { 1,1,1,1 });

#pragma endregion
#pragma region �e�I�u�W�F�N�g�̍��W��]�X�P�[���̑���ϐ�
	//���@�p�̍��W���]�A�X�P�[��
	XMFLOAT3 Player_pos = player->GetPosition();
	XMFLOAT3 Player_rot = player->GetRotation();
	XMFLOAT3 Player_scl = player->GetScale();
	//�G�p�̍��W���]�A�X�P�[��
	XMFLOAT3 Enemy_pos = enemy->GetPosition();
	XMFLOAT3 Enemy_rot = enemy->GetRotation();
	XMFLOAT3 Enemy_scl = enemy->GetScale();
	//�t�B�[���h�p�̍��W���]�A�X�P�[��
	XMFLOAT3 Field_pos = field->GetPosition();
	XMFLOAT3 Field_rot = field->GetRotation();
	XMFLOAT3 Field_scl = field->GetScale();
	//���Ηp�̍��W���]�A�X�P�[��
	XMFLOAT3 Meteo_pos = meteo->GetPosition();
	XMFLOAT3 Meteo_rot = meteo->GetRotation();
	XMFLOAT3 Meteo_scl = meteo->GetScale();
	//�G�̘r�̍��W���]�A�X�P�[��
	XMFLOAT3 Arm_pos = arm->GetPosition();
	XMFLOAT3 Arm_rot = arm->GetRotation();
	XMFLOAT3 Arm_scl = arm->GetScale();
	//����
	XMFLOAT3 Arm_pos_Left = arm_left->GetPosition();
	XMFLOAT3 Arm_rot_Left = arm_left->GetRotation();
	XMFLOAT3 Arm_scl_Left = arm_left->GetScale();
	//�e
	XMFLOAT3 Bullet_pos[15];
	XMFLOAT3 Bullet_rot[15];
	XMFLOAT3 Bullet_scl[15];
	for (int i = 0; i < 15; i++) {
		Bullet_pos[i] = bullet[i]->GetPosition();
		Bullet_rot[i] = bullet[i]->GetRotation();
		Bullet_scl[i] = bullet[i]->GetScale();
		bullet[i]->Bullet_Initialize(player);
	}
	//�K�E�Z�����Ŏg��
	XMFLOAT3 Art_pos = Art_BackWall->GetPosition();
	XMFLOAT3 Art_rot = Art_BackWall->GetRotation();
	XMFLOAT3 Art_scl = Art_BackWall->GetScale();
	//�N���A���̂ݎg��
	XMFLOAT3 Clear_pos = Clear_BackWall->GetPosition();
	XMFLOAT3 Clear_rot = Clear_BackWall->GetRotation();
	XMFLOAT3 Clear_scl = Clear_BackWall->GetScale();

	//���u
	XMFLOAT3 Safe_pos[2];
	XMFLOAT3 Safe_rot[2];
	XMFLOAT3 Safe_scl[2];
	for (int i = 0; i < 2; i++) {
		Safe_pos[i] = bullet[i]->GetPosition();
		Safe_rot[i] = bullet[i]->GetRotation();
		Safe_scl[i] = bullet[i]->GetScale();
	}
#pragma endregion
#pragma region �J�����n
	//�����_���]�x�N�g���̏�����
	float d = 20;
	XMVECTOR v0 = { 0,15, -6,0 };
	XMMATRIX rotM;// = XMMatrixIdentity();
	XMVECTOR eye2;
	XMVECTOR target2 = { object3ds[0].position.x, object3ds[1].position.y, object3ds[2].position.z, 0 };
	XMVECTOR up2 = { 0,0.3f,0,0 };

	float speed = 1;
	float cam = 0.1;
	int timer = 0;

	target2.m128_f32[0] = 0;// +UnitVec_player_target.x;
	target2.m128_f32[1] = 8;// +UnitVec_player_target.y;
	target2.m128_f32[2] = -10;// +UnitVec_player_target.z;
#pragma endregion
	int Player_HPs = 5;
	float RotationAngle = 270;
	int keyf = 0;
	int enetimer = 0;
	int move = 1;
#pragma region �G�̍U���p�^�[�� flag
	int Meteo_Flag = 0;//����
	int Left_Up_Meteo_Flag = 0;
	int Left_Bottom_Meteo_Flag = 0;
	int Right_Up_Meteo_Flag = 0;
	int Right_Bottom_Meteo_Flag = 0;
	int ehp = 500;
	int Arm_Attack_Flag = 0;//�r�ːi
	int Inside_Attack_FLAG = 0;
	//���r
	int Arm_Attack_Flag_Left = 0;//�r�ːi
	int Inside_Attack_FLAG_Left = 0;

	int clearmotion = 0;
#pragma endregion
#pragma region �|�W�V�������]�̏������֌W
	//����
	Meteo_pos.y = Player_pos.y + 200;
	meteo->SetPosition(Meteo_pos);
	//�G�̘r�̈ʒu
	//���r
	XMFLOAT3 Left_Arm = { 214, -140, 150 };
	//�E�r
	XMFLOAT3 Right_Arm = { -190, -140, 150 };
	arm->Arm_Initialize(arm, Right_Arm);
	arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
	//���@�̏�����
	Player_pos.z = -150;
	Player_pos.y = -150;
	Player_rot.y = 270;
	Player_scl = { 10,10,10 };
	//�G�̏�����
	enemy->SetPosition(Enemy_pos);
	Enemy_pos = { 0,-850,150 };
	Enemy_scl = { 150,150,300 };
	Enemy_rot = { 0,0,0 };
	//�t�B�[���h�̏�����
	Field_pos = { 15,-235,-90 };
	Field_scl = { 130,25,80 };
	Field_rot = { 0,0,0 };
	//�G�̘r�̏�����
	arm->SetScale({ 30,25,31 });
	arm_left->SetScale({ 30,25,31 });
#pragma endregion
#pragma region �e�֌W
	int Shot_Flag[15];
	for (int i = 0; i < 15; i++)
	{
		Shot_Flag[i] = 0;
	}
	//
	int Enemy_Damage_Flag = 0;
	int Rot_Flag = 0;
#pragma endregion
#pragma region �K�E�Z�̂悤�Ȃ���
	int Art_Rimmit_Flag = 0;
	int Art_Rimmit_Timer = 0;
	int ran[object_num];
	int ranscl[object_num];
	int artflag = 0;
	//�L����Ƃ��̉����l
	float accel = 0;
	//������萔�L�т���P��
	int bllowflag = 0;
	//���l
	int Alpha_Flag = 0;
	int barriar = 0;
	//�n���ł����@
	Art_BackWall->SetScale({ 900,900,1 });
	Clear_BackWall->SetScale({ 900,900,1 });
	for (int i = 0; i < object_num; i++) {
		ran[i] = rand() % 150 - 150;
		ranscl[i] = rand() % 2 + 1;
		InitializeObject3d(&object3ds[i], 1, dxCommon->GetDev(), basicDescHeap);

		object3ds[i].scale = { 0.0f,0.0f,0.0f };
		object3ds[i].rotation = { 90.0f,0.0f,0.0f };
		object3ds[i].position = { (float)i * 30.0f - 190,-215.0f,(float)ran[i] };
	}
	//�K�E�Z���̈��u�ݒ�
	SafeZone[0]->SetPosition({ -150,-155,-162 });
	SafeZone[1]->SetPosition({ 132,-155,-162 });
	SafeZone[0]->SetScale({ 165,150, 95 });
	SafeZone[1]->SetScale({ 170,150,95 });
	//�_���͈�
	Art_DamageArea.position = { 1 ,Player_pos.y + 1 ,-250 };
	Art_DamageArea.scale = { 14,6.4,10 };
	//Art_DamageArea.scale = { 3.15,3.09996,10 };
	Art_DamageArea.rotation = { 90,0,0 };
#pragma endregion
#pragma region ���ʗ��΂ƍL�͈͗��΂̃_���[�W�͈͐ݒ�
	//�_���[�W�G���A�̐ݒ�
	XMFLOAT3 DamageArea = { Player_pos.x ,Player_pos.y + 5 ,Player_pos.z - 25 };
	//����
	backgrounds.rotation = { 90,0,0 };
	backgrounds.position = { Player_pos.x ,Player_pos.y + 5 ,Player_pos.z - 25 };
	backgrounds.scale = { 1,1,1 };

	L_Meteo_DamageArea[0].position = { -143.199,Player_pos.y + 1, -250.2 };
	L_Meteo_DamageArea[0].scale = { 3.15,3.09996 ,10 };
	L_Meteo_DamageArea[0].rotation = { 90,0,0 };
	//����- 143.198959, -250.199997, 3.150000, 3.099961
	L_Meteo_DamageArea[1].position = { -140.399,Player_pos.y + 1 ,-86.80 };
	L_Meteo_DamageArea[1].scale = { 3.55,3,10 };
	L_Meteo_DamageArea[1].rotation = { 90,0,0 };
	//����:-140.399002,-86.802490,3.550000,2.999960
	L_Meteo_DamageArea[2].position = { 149.6,Player_pos.y + 1 ,-250.8 };
	L_Meteo_DamageArea[2].scale = { 3.4,3.25,10 };
	L_Meteo_DamageArea[2].rotation = { 90,0,0 };
	//�E��149.598862,-250.800003,3.400000,3.249960
	L_Meteo_DamageArea[3].position = { 140.6, Player_pos.y + 1 ,-95.802 };
	L_Meteo_DamageArea[3].scale = { 3.7,3.25,10 };
	L_Meteo_DamageArea[3].rotation = { 90,0,0 };
#pragma endregion
#pragma region ������΂�
	//Arm_rot.y = 45;
	//Arm_rot.z = 90;
	//Arm_rot_Left.y = -45;
	//Arm_rot_Left.z = -90;
	//Arm_pos = { -214, -150, 150 };
	//Arm_pos_Left = { 214, -150, 150 };
	int Knock_Flag = 0;
	//�t���[��
	float frame = 0;
	float maxframe = 80.0f;
	float moves = 13;
	int motionflag = 0;
	Knock_DamageArea.position = { 18 ,Player_pos.y + 1 ,-250 };
	Knock_DamageArea.scale = { 12.8,3.7,10 };
	Knock_DamageArea.rotation = { 90,0,0 };
#pragma endregion
	//�V�[���֌W�̕ϐ�
	XMFLOAT4 Enemy_Color = { 1,1,1,1 };
	int Retry_Flag = 0;
	int Fall_Flag = 0;
	int Damage_Flag = 0;
	//�t�B�[���h���@
	int Under_Field = -215;
	float BackWall_Red = 1;
	float BackWall_Green = 1;
	float BackWall_Blue = 1;
	float BackWall_Alpha = 0;
	int ClearFlag = 0;
	//�G�̏�����
	enemy->SetPosition(Enemy_pos);
	Enemy_pos = { 0,-850,150 };
	Enemy_scl = { 150,150,300 };
	Enemy_rot = { 0,0,0 };
	int shake = 0;;
	int shaketime = 0;
	int shakex = 0;
	int shakey = 0;
	float alpha = 0;
	int fedin = 0;
	int gameoverflag = 0;
	float vol[3] = { 0.3 };
	int pushCount = 0;
	audio->LoopWave("Resources/loop100216.wav", vol[0]);
	int Attck_stanby[9] = { 0 };
	int Attack = 0;
	int bf = 0;

	int notCanon = 0;
#pragma region �X�V����
	while (true) {
		//�E�B���h�E���b�Z�[�W����
		if (winapp->Processmassage()) {
			break;
		}
		input->update();
#pragma region �V�[���P
		if (scene == 1) {
			//���r
			Left_Arm = { 214, -140, 150 };
			//�E�r
			Right_Arm = { -190, -140, 150 };
			arm->Arm_Initialize(arm, Right_Arm);
			arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
			Player_scl = { 10,10,10 };
			speed = 1;
			cam = 0.1;
			timer = 0;
			pRotation;
			LorRFlag = 1;
			movesf = 0;
			mtimer = 0;
			RotationAngle = 270;
			keyf = 0;
			enetimer = 0;
			move = 1;
			//�J�����̉�]�p
			angle = -10.0f;
			//���@�̏�����
			Player_pos.x = 0;
			Player_pos.z = -150;
			Player_pos.y = -150;
			Player_rot.y = 270;
			player->SetPosition(Player_pos);
			player->SetRotation(Player_rot);
			player->SetScale(Player_scl);

			//
			Meteo_Flag = 0;//����
			Left_Up_Meteo_Flag = 0;
			Left_Bottom_Meteo_Flag = 0;
			Right_Up_Meteo_Flag = 0;
			Right_Bottom_Meteo_Flag = 0;

			Arm_Attack_Flag = 0;//�r�ːi
			Inside_Attack_FLAG = 0;
			//���r
			Arm_Attack_Flag_Left = 0;//�r�ːi
			Inside_Attack_FLAG_Left = 0;
			arm->SetScale({ 25, 30, 25 });
			arm_left->SetScale({ 25, 30, 25 });

			L_meteo[3]->SetRight_Bottom_MeteoSet(0);
			L_meteo[2]->SetRight_Up_MeteoSet(0);
			L_meteo[1]->SetLeft_Bottom_MeteoSet(0);
			L_meteo[0]->SetLeft_Up_MeteoSet(0);
			//
			meteo->SetMeteoSet(0);
			for (int i = 0; i < 4; i++) {
				L_meteo[i]->SetPosition({ 900,900,900 });
			}
			meteo->SetPosition({ 900,900,900 });
			arm->SetReturnPos(0);
			arm_left->SetReturnPos_Left(0);
			arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
			arm->Arm_Initialize(arm, Right_Arm);
			Knock_Flag = 0;
			motionflag = 0;
			notCanon = 0;
			arm->SetKnock_Motion_Set(0);
			arm->settimer(0);
			arm->SetKnockFlag(0);
			bf = 0;
			Art_Rimmit_Flag = 0;
			Art_Rimmit_Timer = 0;

			for (int i = 0; i < object_num; i++) {
				ran[i] = rand() % 150 - 150;
				ranscl[i] = rand() % 2 + 1;

				object3ds[i].scale = { 0.0f,0.0f,0.0f };
				object3ds[i].rotation = { 90.0f,0.0f,0.0f };
				object3ds[i].position = { (float)i * 30.0f - 190,-215.0f,(float)ran[i] };
			}
			artflag = 0;
			//�L����Ƃ��̉����l
			accel = 0;
			//������萔�L�т���P��
			bllowflag = 0;
			Damage_Flag = 0;
			frame = 0;
			maxframe = 80.0f;
			moves = 13;
			motionflag = 0;
			Fall_Flag = 0;
			artflag = 0;
			//
			Boss_HP2.scale.x = 18;
			Player_HP2.scale.x = 7.4;
			ClearFlag = 0;
			BackWall_Alpha = 0;
			gameoverflag = 0;
			alpha = 0;
			Arm_rot = { 0,0,0 };
			Arm_rot_Left = { 0,0,0 };
			//Left_Arm = { 214, -140, 150 };
			//Right_Arm = { -190, -140, 150 };
			//arm->SetPosition(Right_Arm);
		   // arm_left->SetPosition(Left_Arm);
			arm->SetRotation({ 0,0,0 });
			arm_left->SetRotation({ 0,0,0 });
			Player_scl = { 10,10,10 };
			artflag = 0;
			for (int i = 0; i < object_num; i++) {
				ran[i] = rand() % 150 - 150;
				ranscl[i] = rand() % 2 + 1;
				object3ds[i].scale = { 0.0f,0.0f,0.0f };
				object3ds[i].rotation = { 90.0f,0.0f,0.0f };
				object3ds[i].position = { (float)i * 30.0f - 190,-215.0f,(float)ran[i] };
			}
			target2.m128_f32[0] = 0;// +UnitVec_player_target.x;
			target2.m128_f32[1] = 8;// +UnitVec_player_target.y;
			target2.m128_f32[2] = -10;// +UnitVec_player_target.z;

			v0 = { 0,15, -6,0 };
			rotM = XMMatrixRotationX(XMConvertToRadians(angle));

			XMVECTOR v;
			v = XMVector3TransformNormal(v0, rotM);
			eye2 = target2 + v;
			matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

			//�G�̏�����
			for (int i = 0; i < 9; i++) {
				Attck_stanby[i] = 0;
			}
			Enemy_pos = { 0,-850,150 };
			Enemy_scl = { 150,150,300 };
			Enemy_rot = { 0,0,0 };
			enemy->SetPosition(Enemy_pos);
			SpriteUpdate(title, spritecommon);
			SpriteUpdate(space, spritecommon);
			if ((input->Pushkey(DIK_SPACE))) {
				scene = 2;
			}
			if ((input->Pushkey(DIK_RETURN))) {
				scene = 3;
			}
		}
#pragma endregion

		if (scene == 3) {
			pushCount++;
			SpriteUpdate(Rule1, spritecommon);
			if (pushCount >= 100) {
				if ((input->Pushkey(DIK_RETURN))) {
					scene = 4;
					pushCount = 0;
				}
			}
		}

		if (scene == 4) {
			SpriteUpdate(Rule2, spritecommon);
			if ((input->Pushkey(DIK_Y))) {
				scene = 1;
			}
		}

#pragma region �V�[��2
		//�w�i������������悤��
		if (scene == 2) {
			//�G�̘r
			Arm_DamageArea.rotation = { 90,0,0 };
			Arm_DamageArea.position = { arm->GetPosition().x,Player_pos.y + 2,-260 };
			Arm_DamageArea.scale = { 3,6.2,10 };
			//��
			Arm_DamageArea_Left.rotation = { 90,0,0 };
			Arm_DamageArea_Left.position = { arm_left->GetPosition().x,Player_pos.y + 2,-260 };
			Arm_DamageArea_Left.scale = { 3,6.2,10 };

			backgrounds.position = DamageArea;

#pragma region �ړ������Ƃ� �ړ��̎��̃J����
			//�v���C���[�̈ړ��֌W
			if (keyf == 1) {
				//�K�E�Z���͓�����悤��
				if (artflag != 1 && Fall_Flag != 1) {
					if ((input->Pushkey(DIK_RIGHT))) {
						Player_pos.x += 2.0f;
						RotationAngle = 0;
						target2.m128_f32[0] = Player_pos.x;
						target2.m128_f32[1] = Player_pos.y + 32;
						target2.m128_f32[2] = Player_pos.z - 60;
						v0 = { 0,15, -10,0 };
						notCanon = 0;
					} else if ((input->Pushkey(DIK_LEFT))) {
						Player_pos.x -= 2.0f;
						RotationAngle = 180;
						target2.m128_f32[0] = Player_pos.x;
						target2.m128_f32[1] = Player_pos.y + 32;
						target2.m128_f32[2] = Player_pos.z - 60;
						v0 = { 0,15, -10,0 };
						notCanon = 0;
					}

					else if ((input->Pushkey(DIK_UP))) {
						if (player->GetPosition().z < 60) {
							Player_pos.z += 2.0f;
						}
						RotationAngle = -90;
						target2.m128_f32[0] = Player_pos.x;
						target2.m128_f32[2] = Player_pos.z - 60;
						target2.m128_f32[1] = Player_pos.y + 32;
						v0 = { 0,15, -10,0 };
						notCanon = 0;
					} else if ((input->Pushkey(DIK_DOWN))) {
						Player_pos.z -= 2.0f;
						RotationAngle = 90;
						target2.m128_f32[0] = Player_pos.x;
						target2.m128_f32[2] = Player_pos.z - 60;
						target2.m128_f32[1] = Player_pos.y + 32;
						v0 = { 0,15, -10,0 };
						notCanon = 0;
					} else {
						notCanon = 1;
					}
				}
#pragma endregion
#pragma region �G�̓���
				Enemy_pos.x += move;
				if (Enemy_pos.x >= 150) {
					move -= 2;
				}
				if (Enemy_pos.x <= -150) {
					move += 2;
				}
				enetimer++;
				if (enetimer >= 360) {
					Enemy_rot.y += 5;
					if (Enemy_rot.y >= 360) {
						Enemy_rot.y = 0;
						enetimer = 0;
					}
				}
			}
#pragma endregion
#pragma region �{�X�̌�����
			if (timer == 0) {
				if (Enemy_pos.y <= -500) {
					Enemy_pos.y += speed;
					Enemy_pos.z += speed;
					angle -= cam;
				} else {
					timer = 1;
				}
			}
			if (timer == 1) {
				target2.m128_f32[2] -= speed * 5;
				Enemy_pos.z -= speed * 2;
				if (target2.m128_f32[2] <= -410) {
					timer = 2;
				}
			} else if (timer == 2) {
				speed = 1;
				target2.m128_f32[2] += speed * 2;
				target2.m128_f32[1] -= speed;
				if (target2.m128_f32[2] >= Player_pos.z - 30) {
					target2.m128_f32[2] = Player_pos.z - 30;
					if (target2.m128_f32[1] <= Player_pos.y + 32) {
						target2.m128_f32[1] = Player_pos.y + 32;
						timer = 3;
					}
				}
			}
			if (timer == 3) {
				keyf = 1;
			}


			Player_rot.y = RotationAngle;
			//�e�I�u�W�F�N�g�̃p�����[�^���Z�b�g
#pragma endregion
#pragma region �|�W�V������X�P�[���̃Z�b�g
			player->SetPosition(Player_pos);
			player->SetRotation(Player_rot);
			player->SetScale(Player_scl);

			enemy->SetPosition(Enemy_pos);

			enemy->SetRotation(Enemy_rot);
			enemy->SetScale(Enemy_scl);

			field->SetPosition(Field_pos);
			field->SetRotation(Field_rot);
			field->SetScale(Field_scl);
#pragma endregion
#pragma region ���Ώ���
			//MeteoFlag���P�ŗ���
			if (Boss_HP2.scale.x <= 17) {
				Attck_stanby[0]++;
				if (Attck_stanby[0] >= 50) {
					//	if (input->Pushkey(DIK_K)) {
					Meteo_Flag = 1;
				}
				//	}
			}

			if (Boss_HP2.scale.x <= 8) {
				Attck_stanby[3]++;
				if (Attck_stanby[3] >= 400) {
					//if (input->Pushkey(DIK_6)) {
					Left_Bottom_Meteo_Flag = 1;
					//}
					//if (input->Pushkey(DIK_7)) {
					Left_Up_Meteo_Flag = 1;
					//}
					//if (input->Pushkey(DIK_8)) {
					Right_Up_Meteo_Flag = 1;
					//}
				//	if (input->Pushkey(DIK_9)) {
					Right_Bottom_Meteo_Flag = 1;
					//}
					Attck_stanby[3] = 0;
				}
			}
			if (Boss_HP2.scale.x <= 16 && Boss_HP2.scale.x >= 6) {
				Attck_stanby[4]++;
				if (Attck_stanby[4] >= 200) {
					if (Player_pos.x >= 0 && Player_pos.x <= 134) {
						Inside_Attack_FLAG_Left = 1;
						Attck_stanby[4] = 0;
					} else if (Player_pos.x >= 134 && Player_pos.x <= 269) {
						Arm_Attack_Flag_Left = 1;
						Attck_stanby[4] = 0;
					} else if (Player_pos.x >= -135 && Player_pos.x <= -1) {
						Inside_Attack_FLAG = 1;
						Attck_stanby[4] = 0;
					} else if (Player_pos.x >= -269 && Player_pos.x <= -135) {
						Arm_Attack_Flag = 1;
						Attck_stanby[4] = 0;
					} else {
						Attck_stanby[4] = 0;
					}
				}
			}

			if (Boss_HP2.scale.x <= 6) {
				Attck_stanby[4]++;
				if (Attck_stanby[4] >= 150) {
					if (Player_pos.x >= 0 && Player_pos.x <= 134) {
						Inside_Attack_FLAG = 1;
						Inside_Attack_FLAG_Left = 1;
						Attck_stanby[4] = 0;
					} else if (Player_pos.x >= 134 && Player_pos.x <= 269) {
						Arm_Attack_Flag_Left = 1;
						Arm_Attack_Flag = 1;
						Attck_stanby[4] = 0;
					} else if (Player_pos.x >= -135 && Player_pos.x <= -1) {
						Inside_Attack_FLAG = 1;
						Inside_Attack_FLAG_Left = 1;
						Attck_stanby[4] = 0;
					} else if (Player_pos.x >= -269 && Player_pos.x <= -135) {
						Arm_Attack_Flag = 1;
						Arm_Attack_Flag_Left = 1;
						Attck_stanby[4] = 0;
					} else {
						Attck_stanby[4] = 0;
					}
				}
			}
			if (Boss_HP2.scale.x <= 18) {
				Attck_stanby[2]++;
				if (Attck_stanby[2] >= 800) {
					if (arm->GetReturnpos() == 0 && arm_left->GetReturnpos() == 0 && Inside_Attack_FLAG != 1 && Arm_Attack_Flag != 1 && Arm_Attack_Flag_Left != 1 && Inside_Attack_FLAG_Left != 1) {
						Knock_Flag = 1;
						motionflag = 1;
						Attck_stanby[2] = 0;
					}
				}
			}
			if (Boss_HP2.scale.x <= 5) {
				Attck_stanby[1]++;
				if (Attck_stanby[1] >= 200) {
					//�����O
					Art_Rimmit_Flag = 1;
					Attck_stanby[1] = 0;
				}
			}

#pragma region ���Ώ���(�S����5��)
#pragma region ���ʂ̗���
			//�_���[�W�\���͈͂̃Z�b�g background=damagearea
			if (meteo->GetMeteoSet() == 0) {
				DamageArea = { Player_pos.x ,Player_pos.y + 5 ,Player_pos.z - 25 };
			}
			if (Meteo_Flag == 1) {
				meteo->Meteo(meteo, player);
				//���΂��t�B�[���h���ɍs������F�X���ɖ߂�
				if (meteo->GetMeteoSet() == 2) {
					meteo->SetMeteoSet(0);
					Damage_Flag = 0;
					Meteo_Flag = 0;
				}
			}
			//���΂ƃv���C���[�̓����蔻��
			if (Damage_Flag != 2) {
				if (collision(meteo->GetPosition().x, meteo->GetPosition().y, meteo->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 25) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//�X�P�[�����]�l���Z�b�g
			//position�͊֐�meteo()�̒��ɂ��邩�炢���
			meteo->SetScale({ 9.5,9.5,9.5 });
			meteo->SetRotation(Meteo_rot);
#pragma endregion
#pragma region �L�͈͗��΍���
			//�_���[�W�\���͈͂̃Z�b�g background=damagearea
			if (Left_Up_Meteo_Flag == 1) {
				L_meteo[0]->Left_Up_Meteo(L_meteo[0]);
				//���΂��t�B�[���h���ɍs������F�X���ɖ߂�
				if (L_meteo[0]->GetLeft_Up_MeteoSet() == 2) {
					L_meteo[0]->SetLeft_Up_MeteoSet(0);
					Damage_Flag = 0;
					Left_Up_Meteo_Flag = 0;
				}
			}
			//���΂ƃv���C���[�̓����蔻��
			if (Damage_Flag != 2) {
				if (collision(L_meteo[0]->GetPosition().x, L_meteo[0]->GetPosition().y, L_meteo[0]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//�X�P�[�����]�l���Z�b�g
			//position�͊֐�meteo()�̒��ɂ��邩�炢���
			L_meteo[0]->SetScale({ 43,43,43 });
			L_meteo[0]->SetRotation(Meteo_rot);
#pragma endregion
#pragma region �L�͈͗��΍���
			//�_���[�W�\���͈͂̃Z�b�g background=damagearea
			if (Left_Bottom_Meteo_Flag == 1) {
				L_meteo[1]->Left_Bottom_Meteo(L_meteo[1]);
				//���΂��t�B�[���h���ɍs������F�X���ɖ߂�
				if (L_meteo[1]->GetLeft_Bottom_MeteoSet() == 2) {
					L_meteo[1]->SetLeft_Bottom_MeteoSet(0);

					Damage_Flag = 0;
					Left_Bottom_Meteo_Flag = 0;
				}
			}
			//���΂ƃv���C���[�̓����蔻��
			if (Damage_Flag != 2) {
				if (collision(L_meteo[1]->GetPosition().x, L_meteo[1]->GetPosition().y, L_meteo[1]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//�X�P�[�����]�l���Z�b�g
			//position�͊֐�meteo()�̒��ɂ��邩�炢���
			L_meteo[1]->SetScale({ 43,43,43 });
			L_meteo[1]->SetRotation(Meteo_rot);
#pragma endregion
#pragma region �L�͈͗��ΉE��
			//�_���[�W�\���͈͂̃Z�b�g background=damagearea
			if (Right_Up_Meteo_Flag == 1) {
				L_meteo[2]->Right_Up_Meteo(L_meteo[2]);
				//���΂��t�B�[���h���ɍs������F�X���ɖ߂�
				if (L_meteo[2]->GetRight_Up_MeteoSet() == 2) {
					L_meteo[2]->SetRight_Up_MeteoSet(0);
					Damage_Flag = 0;
					Right_Up_Meteo_Flag = 0;
				}
			}
			//���΂ƃv���C���[�̓����蔻��
			if (Damage_Flag != 2) {
				if (collision(L_meteo[2]->GetPosition().x, L_meteo[2]->GetPosition().y, L_meteo[2]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//�X�P�[�����]�l���Z�b�g
			//position�͊֐�meteo()�̒��ɂ��邩�炢���
			L_meteo[2]->SetScale({ 43,43,43 });
			L_meteo[2]->SetRotation(Meteo_rot);
#pragma endregion

#pragma region �L�͈͗��ΉE��
			//�_���[�W�\���͈͂̃Z�b�g background=damagearea

			if (Right_Bottom_Meteo_Flag == 1) {
				L_meteo[3]->Right_Bottom_Meteo(L_meteo[3]);
				//���΂��t�B�[���h���ɍs������F�X���ɖ߂�
				if (L_meteo[3]->GetRight_Bottom_MeteoSet() == 2) {
					L_meteo[3]->SetRight_Bottom_MeteoSet(0);

					Damage_Flag = 0;
					Right_Bottom_Meteo_Flag = 0;
				}
			}

			//���΂ƃv���C���[�̓����蔻��
			if (Damage_Flag != 2) {
				if (collision(L_meteo[3]->GetPosition().x, L_meteo[3]->GetPosition().y, L_meteo[3]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//�X�P�[�����]�l���Z�b�g
			//position�͊֐�meteo()�̒��ɂ��邩�炢���
			L_meteo[3]->SetScale({ 43,43,43 });
			L_meteo[3]->SetRotation(Meteo_rot);
#pragma endregion
#pragma endregion
#pragma region �G�̘r
			//�r�ːi

			if (Arm_Attack_Flag == 1) {
				arm->Arm_Attack(arm);

				//������x���ɍs�����猳�̈ʒu��
				//if (arm->GetPosition().z < -700) {
				if (arm->GetReturnpos() == 2) {
					Arm_Attack_Flag = 0;
					arm->Arm_Initialize(arm, Right_Arm);
					Damage_Flag = 0;
					arm->SetReturnPos(0);
				}
			}
			//�r�ƃv���C���[�̓����蔻��
			if (Damage_Flag != 2) {
				if (collision(arm->GetPosition().x, arm->GetPosition().y, arm->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 59) == TRUE) {
					Damage_Flag = 1;
				}
			}

			//�G�̘r�@�����̓ːi
			if (Inside_Attack_FLAG == 1) {
				arm->Arm_Attack_Inside(arm);
				//������x���ɍs�����猳�̈ʒu��
			//	if (arm->GetPosition().z<-700) {
			//		Inside_Attack_FLAG = 0;
					//arm->Arm_Initialize(arm, Left_Arm);
			//		Damage_Flag = 0;
			//	}
				if (arm->GetReturnpos() == 2) {
					Inside_Attack_FLAG = 0;
					arm->Arm_Initialize(arm, Right_Arm);
					Damage_Flag = 0;
					arm->SetReturnPos(0);
				}
			}
#pragma endregion
#pragma region �G�̘r�E��
			//�r�ːi

			if (Arm_Attack_Flag_Left == 1) {
				arm_left->Arm_Attack_Left(arm_left);

				//������x���ɍs�����猳�̈ʒu��
				//if (arm->GetPosition().z < -700) {
				if (arm_left->GetReturnpos_Left() == 2) {
					Arm_Attack_Flag_Left = 0;
					arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
					Damage_Flag = 0;
					arm_left->SetReturnPos_Left(0);
				}
			}
			//�r�ƃv���C���[�̓����蔻��
			if (Damage_Flag != 2) {
				if (collision(arm_left->GetPosition().x, arm_left->GetPosition().y, arm_left->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 59) == TRUE) {
					Damage_Flag = 1;
				}
			}

			//�G�̘r�@�����̓ːi
			if (Inside_Attack_FLAG_Left == 1) {
				arm_left->Arm_Attack_Inside_Left(arm_left);
				//������x���ɍs�����猳�̈ʒu��
			//	if (arm->GetPosition().z<-700) {
			//		Inside_Attack_FLAG = 0;
					//arm->Arm_Initialize(arm, Left_Arm);
			//		Damage_Flag = 0;
			//	}
				if (arm_left->GetReturnpos_Left() == 2) {
					Inside_Attack_FLAG_Left = 0;
					arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
					Damage_Flag = 0;
					arm_left->SetReturnPos_Left(0);
				}
			}
			Arm_rot.z++;
			Arm_rot_Left.z--;
			if (Inside_Attack_FLAG == 1 || Arm_Attack_Flag == 1) {
				Arm_rot.z = 0;
				Arm_rot_Left.z = 0;
			}
			if (Inside_Attack_FLAG_Left == 1 || Arm_Attack_Flag_Left == 1) {
				Arm_rot_Left.z = 0;
			}
			arm->SetRotation(Arm_rot);
			arm_left->SetRotation(Arm_rot_Left);
#pragma endregion
#pragma region �_���[�W���� ���� ���A
			//damageflag==2:�A���Ń_���[�W�𕉂�Ȃ��悤��
			if (Damage_Flag == 1) {
				if (artflag == 1) {
					Player_HP2.scale.x -= 0.1;
					Damage_Flag = 2;
				} else {
					Player_HP2.scale.x -= 1;
					Damage_Flag = 2;
				}
			}
			if (Fall_Flag == 1) {
				for (int i = 0; i < 15; i++) {
					bullet[i]->Bullet_Initialize(player);
					Shot_Flag[i] = 0;
				}
				if (bf == 0) {
					bf = 1;
				}
				if (Player_HP2.scale.x >= 0) {
					Player_HP2.scale.x -= 0.08;
				}
			}
			if (bf == 1) {
				audio->PlayWave("Resources/se_sua07.wav", vol[0]);
				bf = 2;
			}
			//�G���_���[�W��炤����
			if (Enemy_Damage_Flag == 1) {
				Boss_HP2.scale.x -= 0.01f;
				//audio->PlayWave("Resources/bom.wav", vol[0]);
				Enemy_Damage_Flag = 0;
			}
			if (player->GetPosition().x > 284 || player->GetPosition().x < -254
				|| player->GetPosition().z < -250) {
				Fall_Flag = 1;
			}
			if (Fall_Flag == 1) {
				Player_pos.y--;
				//player->Fall(Player_pos);
			}
			if (Player_HP2.scale.x <= 0) {
				gameoverflag = 1;
			}
			//���X���Ƃɖ߂�

			if (Retry_Flag == 1) {
				bf = 0;
				Meteo_Flag = 0;//����
				Left_Up_Meteo_Flag = 0;
				Left_Bottom_Meteo_Flag = 0;
				Right_Up_Meteo_Flag = 0;
				Right_Bottom_Meteo_Flag = 0;

				Arm_Attack_Flag = 0;//�r�ːi
				Inside_Attack_FLAG = 0;
				//���r
				Arm_Attack_Flag_Left = 0;//�r�ːi
				Inside_Attack_FLAG_Left = 0;

				arm->SetScale({ 25, 30, 25 });
				arm_left->SetScale({ 25, 30, 25 });
				L_meteo[3]->SetRight_Bottom_MeteoSet(0);
				L_meteo[2]->SetRight_Up_MeteoSet(0);
				L_meteo[1]->SetLeft_Bottom_MeteoSet(0);
				L_meteo[0]->SetLeft_Up_MeteoSet(0);
				//
				meteo->SetMeteoSet(0);
				for (int i = 0; i < 4; i++) {
					L_meteo[i]->SetPosition({ 900,900,900 });
				}
				meteo->SetPosition({ 900,900,900 });
				arm->SetReturnPos(0);
				arm_left->SetReturnPos_Left(0);
				arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
				arm->Arm_Initialize(arm, Right_Arm);
				Knock_Flag = 0;
				motionflag = 0;
				arm->SetKnock_Motion_Set(0);
				arm->settimer(0);
				arm->SetKnockFlag(0);

				Art_Rimmit_Flag = 0;
				Art_Rimmit_Timer = 0;

				for (int i = 0; i < object_num; i++) {
					ran[i] = rand() % 150 - 150;
					ranscl[i] = rand() % 2 + 1;

					object3ds[i].scale = { 0.0f,0.0f,0.0f };
					object3ds[i].rotation = { 90.0f,0.0f,0.0f };
					object3ds[i].position = { (float)i * 30.0f - 190,-215.0f,(float)ran[i] };
				}
				for (int i = 0; i < 9; i++) {
					Attck_stanby[i] = 0;
				}
				for (int i = 0; i < 15; i++) {
					bullet[i]->Bullet_Initialize(player);
					Shot_Flag[i] = 0;
				}
				artflag = 0;
				//�L����Ƃ��̉����l
				accel = 0;
				//������萔�L�т���P��
				bllowflag = 0;
				Damage_Flag = 0;
				frame = 0;
				maxframe = 80.0f;
				moves = 13;
				motionflag = 0;
				Fall_Flag = 0;
				artflag = 0;
				Player_pos.x = 0;
				Player_pos.y = -150;
				Player_pos.z = -100;
				player->SetPosition(Player_pos);
				player->SetRotation(Player_rot);
				player->SetScale(Player_scl);
				Retry_Flag = 0;
				Arm_rot = { 0,0,0 };
				Arm_rot_Left = { 0,0,0 };
				Left_Arm = { 214, -140, 150 };
				Right_Arm = { -190, -140, 150 };
				gameoverflag = 0;
				arm->SetPosition(Right_Arm);
				for (int i = 0; i < 9; i++) {
					Attck_stanby[i] = 0;
				}
				arm_left->SetPosition(Left_Arm);
				//�E�r
				Enemy_Damage_Flag = 0;
				Boss_HP2.scale.x = 18;
				Player_HP2.scale.x = 7.4;
				ClearFlag = 0;
				BackWall_Alpha = 0;
				alpha = 0;
				//clearmotion = 0;
				target2.m128_f32[0] = Player_pos.x;
				target2.m128_f32[2] = Player_pos.z - 60;
				target2.m128_f32[1] = Player_pos.y + 32;
			}
			player->SetPosition(Player_pos);

			player->SetRotation(Player_rot);
			player->SetScale(Player_scl);

#pragma endregion
#pragma region �e�̏���
			if (ClearFlag == 0) {
				if (input->TriggerKey(DIK_SPACE) && notCanon == 1) {
					audio->PlayWave("Resources/se_mod05.wav", vol[0]);
					for (int i = 0; i < 15; i++) {
						if (Shot_Flag[i] == 0) {
							bullet[i]->SetPosition(player->GetPosition());
							Shot_Flag[i] = 1;
							break;
						}
					}
				}
			}
			for (int i = 0; i < 15; i++) {
				if (Shot_Flag[i] == 1) {
					Rot_Flag = 1;
					bullet[i]->Shot_Bullet();
					if (bullet[i]->GetBullet_Flag() == 0) {
						Enemy_Damage_Flag = 0;
						Shot_Flag[i] = 0;
					}
					if (bullet[i]->GetPosition().z > 120) {
						Enemy_Damage_Flag = 1;
					}
				}

			}
			if (Rot_Flag == 1) {
				player->SetRotation({ 0,-90,0 });
				if (input->Pushkey(DIK_UP) || input->Pushkey(DIK_DOWN) ||
					input->Pushkey(DIK_RIGHT) || input->Pushkey(DIK_LEFT)) {
					Rot_Flag = 0;
				}
			}
			if (Enemy_Damage_Flag == 1) {
				Enemy_Color = { 1,0,0,1 };
			} else {
				Enemy_Color = { 1,1,1,1 };
			}
#pragma endregion
#pragma region �K�E�Z
			//�t�F�[�h�C���p�́����J�����O�ɃZ�b�g
			Art_BackWall->SetPosition({ target2.m128_f32[0],target2.m128_f32[1] - 20,target2.m128_f32[2] });

			if (Art_Rimmit_Flag == 1) {
				Meteo_Flag = 0;
				for (int i = 0; i < 9; i++) {
					Attck_stanby[i] = 0;
				}
				Art_Rimmit_Timer++;
			}
			if (Art_Rimmit_Timer > 200) {
				artflag = 1;
				Art_Rimmit_Timer = 0;
				Art_Rimmit_Flag = 0;
			}
			//����
			if (artflag == 1) {
				Meteo_Flag = 0;
				for (int i = 0; i < 9; i++) {
					Attck_stanby[i] = 0;
				}
				//if (Enemy_rot.x> -20) {
					//Enemy_rot.x-=0.5;
				//}
				enemy->SetRotation({ Enemy_rot.x,0,0 });
				Alpha_Flag = 0;
				accel += 0.059;
				for (int i = 0; i < object_num; i++) {
					object3ds[i].scale.z += (float)ranscl[i] * 0.5f;
					//}
					if (object3ds[i].scale.z > 150) {
						bllowflag = 1;
					}
					//������x�L�т���x,y�g��
					if (bllowflag == 1) {
						Meteo_Flag = 0;
						for (int i = 0; i < 9; i++) {
							Attck_stanby[i] = 0;
						}
						object3ds[i].scale.y += 0.02 * accel;
						object3ds[i].scale.x += 0.02 * accel;
						if (BackWall_Alpha <= 1) {
							if (object3ds[i].scale.x < 90) {
								BackWall_Alpha += 0.001;
							}
						}
					} else {//���������L����
						object3ds[i].scale.y = 0.02;
						object3ds[i].scale.x = 0.02;// *accel;
					}
				}
			}
			//��萔�傫���Ȃ����珉����
			for (int i = 0; i < object_num; i++) {
				if (object3ds[i].scale.x > 90) {
					artflag = 0;
					bllowflag = 0;
					object3ds[i].scale = { 0.0f,0.0f,0.0f };
					accel = 0;
					Alpha_Flag = 1;
				}

			}
			if (Alpha_Flag == 1) {
				if (BackWall_Alpha >= 0) {
					BackWall_Alpha -= 0.01;
				}
			}
			if (artflag == 1) {
				if (collision(player->GetPosition().x, player->GetPosition().y, player->GetPosition().z,
					SafeZone[0]->GetPosition().x, SafeZone[0]->GetPosition().y, SafeZone[0]->GetPosition().z, 26) == TRUE
					|| collision(player->GetPosition().x, player->GetPosition().y, player->GetPosition().z,
						SafeZone[1]->GetPosition().x, SafeZone[1]->GetPosition().y, SafeZone[1]->GetPosition().z, 26) == TRUE)
				{
					barriar = 1;
				} else {
					barriar = 0;
				}
				if (barriar == 0 && object3ds[0].scale.x > 9 && object3ds[0].scale.x < 20) {
					Damage_Flag = 1;

				} else {
					Damage_Flag = 0;
				}

			} else if (artflag == 0) {
				barriar = 0;
			}

			if (barriar == 1) {
				notCanon = 0;
			}
#pragma endregion
#pragma region ������΂��U��

			if (Knock_Flag == 1) {
				for (int i = 0; i < 9; i++) {
					Attck_stanby[i] = 0;
				}
				target2.m128_f32[0] = Player_pos.x;
				target2.m128_f32[2] = Player_pos.z - 60;
				target2.m128_f32[1] = Player_pos.y + 32;
				if (arm->GetKnock_Motion_Set() == 1) {
					arm->KnockBack_Attack(Player_pos.z, frame, maxframe, moves);
					player->SetPosition(Player_pos);
				}
			}
			if (motionflag == 1) {
				arm->Knock_Attack_Motion(arm, arm_left);
			}
			//
			if (arm->GetKnockFlag() == 2) {
				arm->SetKnockFlag(0);
				Knock_Flag = 0;
			}
			if (arm->GetKnock_Motion_Set() == 4) {
				arm->SetKnock_Motion_Set(0);
				//arm->SetPosition({ -190,-150,150 });// = { -190,-150,150 };
				//arm_left->SetPosition({ 210,-150,150 });

				//= { 210,-150,150 };
				motionflag = 0;
			}

#pragma endregion
#pragma region �V�[���R
			Clear_BackWall->SetScale({ 2500,2000,1 });
			if (ClearFlag != 1) {
				Clear_BackWall->SetPosition({ target2.m128_f32[0],target2.m128_f32[1] - 30,target2.m128_f32[2] });
			}
			//�̗͂��O�؂�����
			if (Boss_HP2.scale.x <= 0) {
				Player_HP2.scale.x = 100;
				ClearFlag = 1;//�J�����ʒu��G�ʒu�̌Œ�p
				clearmotion = 1;//���[�V�����̊J�n�p
			}

			if (clearmotion == 1) {
				arm->SetScale({ 30,25,50 });
				arm_left->SetScale({ 30,25,50 });
				arm->Clear_Motion(arm, arm_left, Arm_rot, Arm_rot_Left);
				Boss_HP2.scale.x = 18;
			}

			//�r�J������Ƃ��Ƀt�F�[�h�C��
			if (arm->GetClear_Motion_Set() == 1) {
				fedin = 1;
			}
			if (fedin == 1) {
				alpha += 0.01f;
				if (alpha >= 1) {
					fedin = 0;
				}
				Clear_BackWall->SetScale({ 3000, 3000, 1 });
				Clear_BackWall->SetPosition({ 0,target2.m128_f32[1] - 50,target2.m128_f32[2] + 29 });
			}


			//���[�V�������Ō�܂ōs������
			if (arm->GetClear_Motion_Set() == 2) {
				clearmotion = 0;
				arm->setClear_Motion_Set(0);
			}

			if (ClearFlag == 1) {
				player->SetPosition({ 0,-150,-100 });
				//�J�����ɃV�F�C�N����
				shake = rand() % 5;
				shakex = rand() % 5;
				shakey = rand() % 5;
				shakex -= shake;
				shakey -= shake;

				enemy->SetPosition({ 0,-500,150 });
				//���l���@�t�F�[�h�C���݂�����

				target2.m128_f32[0] = enemy->GetPosition().x + shakex;
				target2.m128_f32[2] = target2.m128_f32[2] - 1;
				target2.m128_f32[1] = enemy->GetPosition().y + 700 + shakey;
				Clear_BackWall->SetPosition({ 0,target2.m128_f32[1] - 50,target2.m128_f32[2] + 29 });

				//���l�����s������@�V�[���R��

			}
			if (alpha >= 1) {
				scene = 5;
			}
			//�N���A���[�V�������͒e�łĂȂ��悤��
			//�t�B�[���h�X�P�[���������Ⴍ
			//
#pragma endregion 
			if (gameoverflag == 1) {
				scene = 6;
				//gameoverflag = 0;
			}

			//�J�����֌W�̏���
				//�����_�̐ݒ�
				// 
				//meteo->SetPosition(Meteopos);
				//�s�����蒼��
			rotM = XMMatrixRotationX(XMConvertToRadians(angle));
			XMVECTOR v;
			v = XMVector3TransformNormal(v0, rotM);
			eye2 = target2 + v;
			matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

#pragma region �I�u�W�F�N�g�X�V����update()
			//�J�����p�Ɋe���f������view�s�����
			player->Update(matview, matprojection, { 1,1,1,1 });
			enemy->Update(matview, matprojection, Enemy_Color);
			field->Update(matview, matprojection, { 1,1,1,1 });
			meteo->Update(matview, matprojection, { 1,1,1,1 });
			arm->Update(matview, matprojection, Enemy_Color);
			arm_left->Update(matview, matprojection, Enemy_Color);
			for (int i = 0; i < _countof(bullet); i++) {
				bullet[i]->Update(matview, matprojection, { 0.1,0.1,0.8,0.4 });
			}
			for (int i = 0; i < _countof(L_meteo); i++) {
				L_meteo[i]->Update(matview, matprojection, { 1,1,0.8,0.9 });
			}
			for (int i = 0; i < _countof(SafeZone); i++) {
				SafeZone[i]->Update(matview, matprojection, { 0.1,0.8,0.1,0.6 });
			}
			Art_BackWall->Update(matview, matprojection, { 1,1,1,BackWall_Alpha });
			Clear_BackWall->Update(matview, matprojection, { 1,1,1,alpha });
			//�G�p�̍X�V����
			for (int i = 0; i < object_num; i++) {
				UpdateObject3d(&object3ds[i], matview, matprojection, { 1,1,1,1 });
			}
			//�w�i�p�̍X�V�����@���͕ۗ�
			UpdateObject3d(&backgrounds, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Arm_DamageArea, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Arm_DamageArea_Left, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Knock_DamageArea, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Art_DamageArea, matview, matprojection, { 0.9,0.3,0.2,0.8 });
			for (int i = 0; i < 4; i++) {
				UpdateObject3d(&L_Meteo_DamageArea[i], matview, matprojection, { 0.9,0.5,0.5,0.8 });
			}
			//�X�v���C�g�̍X�V�����Ăяo��
			for (int i = 0; i < 5; i++) {
				SpriteUpdate(sprite[i], spritecommon);
			}
			SpriteUpdate(Boss_HP, spritecommon);
			SpriteUpdate(Boss_HP2, spritecommon);
			SpriteUpdate(Player_HP, spritecommon);
			SpriteUpdate(Player_HP2, spritecommon);
#pragma endregion		 
		}//�����܂ŃV�[���Q  �^�C�g����
#pragma endregion
		if (scene == 5) {
			SpriteUpdate(Clear, spritecommon);
			//SpriteUpdate(space, spritecommon);
			if ((input->TriggerKey(DIK_Z))) {
				scene = 1;
				//	Retry_Flag = 1;
			}
			//Retry_Flag = 1;
		}
		if (scene == 6) {
			SpriteUpdate(Gameover, spritecommon);
			if ((input->TriggerKey(DIK_Z))) {
				scene = 1;
				//	Retry_Flag = 1;
			}
			if ((input->TriggerKey(DIK_R))) {
				scene = 2;
				Retry_Flag = 1;
			}
		}
		//���z���������擾
		Vertex* vertMap = nullptr;
		result = vertBuff->Map(0, nullptr, (void**)&vertMap);
		for (int i = 0; i < _countof(vertices); i++) {
			vertMap[i] = vertices[i];
		}
		vertBuff->Unmap(0, nullptr);


#pragma region �w�i�p�̔|���S��
		//���z���������擾
		BackGround* backvertMap = nullptr;
		result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
		for (int i = 0; i < _countof(background); i++) {
			backvertMap[i] = background[i];
		}
		backvertBuff->Unmap(0, nullptr);
#pragma endregion
#pragma region x,y���W�̃f�o�b�O���O
		wchar_t str[256];

		//swprintf_s(str, L"rotation:%f\n", player.rotation.y);
		//�� 44.000000,-86.000000,59.000000
		//�E�@29.000000,82.000000,-57.000000
		OutputDebugString(str);
		swprintf_s(str, L"timer:%d,%f,%f,%f\n", scene, target2.m128_f32[1], target2.m128_f32[2], Boss_HP2.scale.x);
		OutputDebugString(str);
#pragma endregion
#pragma region �`��O����
		dxCommon->BeginDraw();//�`��R�}���h�̏��ւ��
#pragma endregion
#pragma region �`��R�}���h
		//�`��R�}���h
#pragma region �K�E�Z�p�̉~���`��
		dxCommon->GetCmdList()->SetPipelineState(obj3Dpipelineset.pipelinestate.Get());
		dxCommon->GetCmdList()->SetGraphicsRootSignature(obj3Dpipelineset.rootsignature.Get());

		//�X�v���C�g���ʃR�}���h
		//�v���e�B���`��̐ݒ�R�}���h
		dxCommon->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�`�揈���̌Ăяo��
		if (scene == 2) {
			for (int i = 0; i < object_num; i++) {
				DrawObject3d(&object3ds[i], dxCommon->GetCmdList(), basicDescHeap, vbview, ibView, gpuDescHandleSRV, _countof(indices));
			}
		}
#pragma endregion
#pragma region �X�v���C�g�`��
		dxCommon->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		SpriteCommonBeginDraw(spritecommon, dxCommon->GetCmdList());//,basicDescHeap);

		if (scene == 1) {
			SpriteDraw(title, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
		}
		if (scene == 2) {
			if (ClearFlag == 0) {
				for (int i = 0; i < Player_HPs; i++) {
					//SpriteDraw(sprite[i], dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
				}
				SpriteDraw(Boss_HP, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
				SpriteDraw(Boss_HP2, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
				SpriteDraw(Player_HP, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
				SpriteDraw(Player_HP2, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
			}
		}
		if (scene == 5) {
			SpriteDraw(Clear, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
		}
		if (scene == 3) {
			SpriteDraw(Rule1, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
		}
		if (scene == 4) {
			SpriteDraw(Rule2, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
		}
		if (scene == 6) {
			SpriteDraw(Gameover, dxCommon->GetCmdList(), spritecommon, dxCommon->GetDev());
		}
#pragma endregion
		//if (scene != 3) {
			//�`��R�}���h
#pragma region �e�_���[�W�G���A�̕`��
		dxCommon->GetCmdList()->SetPipelineState(Backpipelineset.pipelinestate.Get());
		dxCommon->GetCmdList()->SetGraphicsRootSignature(Backpipelineset.rootsignature.Get());
		if (Meteo_Flag == 1) {
			DrawObject3d(&backgrounds, dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (arm->GetAttack_Set() == 1) {
			DrawObject3d(&Arm_DamageArea, dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (arm_left->GetAttack_Set_Left() == 1) {
			DrawObject3d(&Arm_DamageArea_Left, dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (arm->gettimer() > 0) {
			DrawObject3d(&Knock_DamageArea, dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (Art_Rimmit_Flag == 1) {
			DrawObject3d(&Art_DamageArea, dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (Left_Up_Meteo_Flag == 1) {
			DrawObject3d(&L_Meteo_DamageArea[1], dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (Left_Bottom_Meteo_Flag == 1) {
			DrawObject3d(&L_Meteo_DamageArea[0], dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (Right_Up_Meteo_Flag == 1) {
			DrawObject3d(&L_Meteo_DamageArea[3], dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
		if (Right_Bottom_Meteo_Flag == 1) {
			DrawObject3d(&L_Meteo_DamageArea[2], dxCommon->GetCmdList(), basicDescHeap, backvbview, backibview, gpuDescHandleSRV2, 6);
		}
#pragma endregion
#pragma region ���f���`��
		//�e�I�u�W�F�N�g�̕`��
		//���@
		if (scene == 2) {
			player->PreDraw(dxCommon->GetCmdList());
			if (ClearFlag != 1) {
				player->Draw();
			}
			player->PostDraw();
			//�G
			enemy->PreDraw(dxCommon->GetCmdList());
			enemy->Draw();
			enemy->PostDraw();
			//�t�B�[���h
			field->PreDraw(dxCommon->GetCmdList());
			if (ClearFlag != 1) {
				field->Draw();
			}
			field->PostDraw();
			//����
			meteo->PreDraw(dxCommon->GetCmdList());
			meteo->Draw();
			meteo->PostDraw();
			//�G�̘r
			arm->PreDraw(dxCommon->GetCmdList());
			arm->Draw();
			arm->PostDraw();
			//�G�̘r�E��
			arm_left->PreDraw(dxCommon->GetCmdList());
			arm_left->Draw();
			arm_left->PostDraw();
			//�L�͈͗���
			for (int i = 0; i < _countof(L_meteo); i++) {
				L_meteo[i]->PreDraw(dxCommon->GetCmdList());
				if (ClearFlag != 1) {
					L_meteo[i]->Draw();
				}
				L_meteo[i]->PostDraw();
			}
			//�e
			for (int i = 0; i < _countof(bullet); i++) {
				bullet[i]->PreDraw(dxCommon->GetCmdList());
				if (Shot_Flag[i] == 1) {
					bullet[i]->Draw();
				}
				bullet[i]->PostDraw();
			}
			//���u
			for (int i = 0; i < _countof(SafeZone); i++) {
				SafeZone[i]->PreDraw(dxCommon->GetCmdList());
				if (Art_Rimmit_Timer > 1 && Art_Rimmit_Timer < 199) {
					SafeZone[i]->Draw();
				}
				SafeZone[i]->PostDraw();
			}

			//�N���A���̂�
			Clear_BackWall->PreDraw(dxCommon->GetCmdList());
			if (ClearFlag == 1) {
				Clear_BackWall->Draw();
			}
			Clear_BackWall->PostDraw();
			//�K�E�Z�����Ŏg��
			Art_BackWall->PreDraw(dxCommon->GetCmdList());
			Art_BackWall->Draw();
			Art_BackWall->PostDraw();
			//�`��R�}���h�@�����܂�
	//	}
		}
#pragma endregion
#pragma region �`��㏈��
		dxCommon->EndDraw();//���[�v���̖�����ւ��
#pragma endregion
	}
	//�����܂ōX�V
#pragma endregion

#pragma endregion
#pragma region �������
	for (int i = 0; i < 15; i++) {
		delete bullet[i];
	}
	for (int i = 0; i < 4; i++) {
		delete L_meteo[i];
	}
	delete Art_BackWall;
	delete meteo;
	delete player;
	delete enemy;
	delete input;
	delete arm;
	delete arm_left;
	delete dxCommon;
	winapp->Deletewindow();
	delete winapp;
	return 0;
}
#pragma endregion
#pragma region �t���O����
//meteo_flag �ӂ��̗��� player���W
//Right_Up_Meteo_Flag �L�͈͗��΁@�E��
//Right_Bottom_Meteo_Flag�@�L�͈͗��΁@�E��
// Left_Up_Meteo_Flag�@�L�͈͗��΁@����
// Left_bottom_Meteo_Flag�@�L�͈͗��΁@����
//Arm_Attack_Flag �E�r�̊O���U��
//Arm_Attack_Flag_Left ���r�̊O���U��
//Inside_Attack_Flag �E�r�̓����U��
//Inside_Attack_Flag_Left ���r�̓����U��
//knock_flag ������΂��U��(motion_flag�ƃZ�b�g�Ŏg��)
// Art_Flag �K�E�Z
//retry�@���A
