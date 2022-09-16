#include "EnemyArm.h"

#include <d3dcompiler.h>
#include <DirectXTex.h>
#include <fstream>
#include<sstream>
#include<string>
#include<vector>
#pragma comment(lib, "d3dcompiler.lib")
using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

/// <summary>
/// �ÓI�����o�ϐ��̎���
/// </summary>
int EnemyArm::Return_Position = 0;
XMFLOAT3 EnemyArm::Arm_Position = { -190, -140, 150 };
int EnemyArm::Return_Position_Left = 0;
XMFLOAT3 EnemyArm::Arm_Position_Left = { 214, -140, 150 };
float EnemyArm::Attack_Speed = 20;
const float EnemyArm::radius = 5.0f;				// ��ʂ̔��a
const float EnemyArm::prizmHeight = 8.0f;			// ���̍���
ID3D12Device* EnemyArm::device = nullptr;
UINT EnemyArm::descriptorHandleIncrementSize = 0;
ID3D12GraphicsCommandList* EnemyArm::cmdList = nullptr;
ComPtr<ID3D12RootSignature> EnemyArm::rootsignature;
ComPtr<ID3D12PipelineState> EnemyArm::pipelinestate;
ComPtr<ID3D12DescriptorHeap> EnemyArm::descHeap;
ComPtr<ID3D12Resource> EnemyArm::vertBuff;
ComPtr<ID3D12Resource> EnemyArm::indexBuff;
ComPtr<ID3D12Resource> EnemyArm::texbuff;
CD3DX12_CPU_DESCRIPTOR_HANDLE EnemyArm::cpuDescHandleSRV;
CD3DX12_GPU_DESCRIPTOR_HANDLE EnemyArm::gpuDescHandleSRV;
XMMATRIX EnemyArm::matView{};
XMMATRIX EnemyArm::matProjection{};
XMFLOAT3 EnemyArm::eye = { 0, 0, -50.0f };
XMFLOAT3 EnemyArm::target = { 0, 0, 0 };
XMFLOAT3 EnemyArm::up = { 0, 1, 0 };
D3D12_VERTEX_BUFFER_VIEW EnemyArm::vbView{};
D3D12_INDEX_BUFFER_VIEW EnemyArm::ibView{};
EnemyArm::Material EnemyArm::material;
//EnemyArm::VertexPosNormalUv EnemyArm::vertices[vertexCount];
//unsigned short EnemyArm::indices[planeCount * 3];
std::vector<EnemyArm::VertexPosNormalUv>EnemyArm::vertices;
std::vector<unsigned short>EnemyArm::indices;
bool EnemyArm::StaticInitialize(ID3D12Device* device, int window_width, int window_height)
{
	// nullptr�`�F�b�N
	assert(device);

	EnemyArm::device = device;

	// �f�X�N���v�^�q�[�v�̏�����
	InitializeDescriptorHeap();

	// �J����������
	InitializeCamera(window_width, window_height);

	// �p�C�v���C��������
	InitializeGraphicsPipeline();

	// �e�N�X�`���ǂݍ���
	//LoadTexture(directoryPath,material.textureFilename);

	// ���f������
	//CreateModel();

	return true;
}

void EnemyArm::PreDraw(ID3D12GraphicsCommandList* cmdList)
{
	// PreDraw��PostDraw���y�A�ŌĂ΂�Ă��Ȃ���΃G���[
	assert(EnemyArm::cmdList == nullptr);

	// �R�}���h���X�g���Z�b�g
	EnemyArm::cmdList = cmdList;

	// �p�C�v���C���X�e�[�g�̐ݒ�
	cmdList->SetPipelineState(pipelinestate.Get());
	// ���[�g�V�O�l�`���̐ݒ�
	cmdList->SetGraphicsRootSignature(rootsignature.Get());
	// �v���~�e�B�u�`���ݒ�
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void EnemyArm::PostDraw()
{
	// �R�}���h���X�g������
	EnemyArm::cmdList = nullptr;
}

EnemyArm* EnemyArm::Create()
{
	// 3D�I�u�W�F�N�g�̃C���X�^���X�𐶐�
	EnemyArm* enemy_Arm = new EnemyArm();
	if (enemy_Arm == nullptr) {
		return nullptr;
	}

	// ������
	if (!enemy_Arm->Initialize()) {
		delete enemy_Arm;
		assert(0);
		return nullptr;
	}
	float scale_val = 5;
	enemy_Arm->scale = { scale_val , scale_val , scale_val };

	return enemy_Arm;
}

void EnemyArm::SetEye(XMFLOAT3 eye)
{
	EnemyArm::eye = eye;

	UpdateViewMatrix();
}

void EnemyArm::SetTarget(XMFLOAT3 target)
{
	EnemyArm::target = target;

	UpdateViewMatrix();
}

void EnemyArm::CameraMoveVector(XMFLOAT3 move)
{
	XMFLOAT3 eye_moved = GetEye();
	XMFLOAT3 target_moved = GetTarget();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	target_moved.x += move.x;
	target_moved.y += move.y;
	target_moved.z += move.z;

	SetEye(eye_moved);
	SetTarget(target_moved);
}

bool EnemyArm::InitializeDescriptorHeap()
{
	HRESULT result = S_FALSE;

	// �f�X�N���v�^�q�[�v�𐶐�	
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NumDescriptors = 1; // �V�F�[�_�[���\�[�X�r���[1��
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));//����
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// �f�X�N���v�^�T�C�Y���擾
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return true;
}

void EnemyArm::InitializeCamera(int window_width, int window_height)
{
	// �r���[�s��̐���
	matView = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));

	// ���s���e�ɂ��ˉe�s��̐���
	//constMap->mat = XMMatrixOrthographicOffCenterLH(
	//	0, window_width,
	//	window_height, 0,
	//	0, 1);
	// �������e�ɂ��ˉe�s��̐���
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),
		(float)window_width / window_height,
		0.1f, 1000.0f
	);
}

bool EnemyArm::InitializeGraphicsPipeline()
{
	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; // ���_�V�F�[�_�I�u�W�F�N�g
	ComPtr<ID3DBlob> psBlob;	// �s�N�Z���V�F�[�_�I�u�W�F�N�g
	ComPtr<ID3DBlob> errorBlob; // �G���[�I�u�W�F�N�g

	// ���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		//L"Resources/shaders/BasicVertexShader.hlsl",	// �V�F�[�_�t�@�C����
		L"OBJVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "vs_5_0",	// �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// �s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		//L"Resources/shaders/BasicPixelShader.hlsl",	// �V�F�[�_�t�@�C����
		L"OBJPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "ps_5_0",	// �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy���W(1�s�ŏ������ق������₷��)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // �@���x�N�g��(1�s�ŏ������ق������₷��)
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // uv���W(1�s�ŏ������ق������₷��)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// �O���t�B�b�N�X�p�C�v���C���̗����ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	// �T���v���}�X�N
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�
	// ���X�^���C�U�X�e�[�g
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// �f�v�X�X�e���V���X�e�[�g
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// RBGA�S�Ẵ`�����l����`��
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	// �u�����h�X�e�[�g�̐ݒ�
	gpipeline.BlendState.RenderTarget[0] = blenddesc;

	// �[�x�o�b�t�@�̃t�H�[�}�b�g
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// ���_���C�A�E�g�̐ݒ�
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	// �}�`�̌`��ݒ�i�O�p�`�j
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;	// �`��Ώۂ�1��
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0�`255�w���RGBA
	gpipeline.SampleDesc.Count = 1; // 1�s�N�Z���ɂ�1��T���v�����O

	// �f�X�N���v�^�����W
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 ���W�X�^

	// ���[�g�p�����[�^
	//CD3DX12_ROOT_PARAMETER rootparams[2];
	//rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	//rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// �X�^�e�B�b�N�T���v���[
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ���[�g�V�O�l�`���̐ݒ�
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// �o�[�W������������̃V���A���C�Y
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	// ���[�g�V�O�l�`���̐���
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	if (FAILED(result)) {
		return result;
	}

	gpipeline.pRootSignature = rootsignature.Get();

	// �O���t�B�b�N�X�p�C�v���C���̐���
	result = device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));

	if (FAILED(result)) {
		return result;
	}

	return true;
}

bool EnemyArm::LoadTexture(const std::string& directoryPath, const std::string& filename)
{
	HRESULT result = S_FALSE;

	// WIC�e�N�X�`���̃��[�h
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	string filepath = directoryPath + filename;

	wchar_t wfilepath[128];
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, _countof(wfilepath));
	directoryPath + filename;
	/*result = LoadFromWICFile(
		L"Resources/tex1.png", WIC_FLAGS_NONE,
		&metadata, scratchImg);*/
	result = LoadFromWICFile(wfilepath, WIC_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(result)) {
		return result;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // ���f�[�^���o

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// �e�N�X�`���p�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // �e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&texbuff));

	if (FAILED(result)) {
		return result;
	}

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	result = texbuff->WriteToSubresource(
		0,
		nullptr, // �S�̈�փR�s�[
		img->pixels,    // ���f�[�^�A�h���X
		(UINT)img->rowPitch,  // 1���C���T�C�Y
		(UINT)img->slicePitch // 1���T�C�Y
	);
	if (FAILED(result)) {
		return result;
	}

	// �V�F�[�_���\�[�X�r���[�쐬
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texbuff.Get(), //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //�e�N�X�`���ݒ���
		cpuDescHandleSRV
	);

	return true;
}

void  EnemyArm::LoadMaterial(const std::string& directoryPath, const std::string& filename)
{
	std::ifstream file;
	file.open(directoryPath + filename);
	if (file.fail()) {
		assert(0);
	}
	string line;
	while (getline(file, line)) {
		std::istringstream line_stream(line);
		string key;
		getline(line_stream, key, ' ');
		if (key[0] == '\t') {
			key.erase(key.begin());
		}
		if (key == "newmtl") {
			line_stream >> material.name;
		}
		if (key == "Ka") {
			line_stream >> material.ambient.x;
			line_stream >> material.ambient.y;
			line_stream >> material.ambient.z;
		}
		if (key == "Kd") {
			line_stream >> material.diffuse.x;
			line_stream >> material.diffuse.y;
			line_stream >> material.diffuse.z;
		}
		if (key == "Ks") {
			line_stream >> material.specular.x;
			line_stream >> material.specular.y;
			line_stream >> material.specular.z;
		}
		if (key == "map_Kd") {
			line_stream >> material.textureFilename;
			LoadTexture(directoryPath, material.textureFilename);
		}
	}
	file.close();
}
void EnemyArm::CreateModel()
{
	HRESULT result = S_FALSE;

	std::ifstream file;
	//obj�t�@�C�����J��
	//file.open(L"Resources/triangle_tex/triangle_tex.obj");
	const string modelname = "enemy-arm";
	const string filename = modelname + ".obj";
	const string directoryPath = "Resources/" + modelname + "/";
	file.open(directoryPath + filename);
	if (file.fail()) {
		assert(0);
	}

	vector<XMFLOAT3>positions;//���_���W
	vector<XMFLOAT3>normals;//�@���x�N�g��
	vector<XMFLOAT2>texcoords;//�e�N�X�`��UV
	//1�s���ǂݍ���

	string line;
	while (getline(file, line)) {
		//1�s���̕�������X�g���[���ɕϊ�
		std::istringstream line_stream(line);
		//���pSPACE��؂�ōs�̐擪��������擾
		string key;
		getline(line_stream, key, ' ');

		//�擪������v�Ȃ璸�_���W
		if (key == "v") {
			//X,Y,Z���W�ǂݍ���
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			//���W�f�[�^�ɒǉ�
			positions.emplace_back(position);
			//���_�f�[�^�ɒǉ�
			//VertexPosNormalUv vertex{};
			//vertex.pos = position;
			//vertices.emplace_back(vertex);

		}
		//�擪������vt�Ȃ�e�N�X�`��
		if (key == "vt") {
			//UV�����ǂݍ���
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			//v�������]
			texcoord.y = 1.0f - texcoord.y;
			//�e�N�X�`�����W�f�[�^�ɒǉ�
			texcoords.emplace_back(texcoord);
		}
		//�擪������vn�Ȃ�@���x�N�g��
		if (key == "vn") {
			//XYZ�����ǂݍ���
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			//�@���x�N�g���f�[�^�ɒǉ�
			normals.emplace_back(normal);
		}
		//�擪������mtllib�Ȃ�}�e���A��
		if (key == "mtllib")
		{
			//�}�e���A���̃t�@�C�����ǂݍ���
			string filename;
			line_stream >> filename;
			LoadMaterial(directoryPath, filename);
		}
		//�擪������f�Ȃ�|���S��
		if (key == "f") {
			//���p�X�y�[�X��؂�ōs�̑�����ǂݍ���
			string index_string;
			while (getline(line_stream, index_string, ' ')) {
				//���_�C���f�b�N�X1�s���̕�������X�g���[���ɕϊ�
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				index_stream >> indexPosition;
				index_stream.seekg(1, ios_base::cur);
				index_stream >> indexTexcoord;
				index_stream.seekg(1, ios_base::cur);
				index_stream >> indexNormal;
				//���_�C���f�b�N�X�ɒǉ�]
				VertexPosNormalUv vertex{};
				vertex.pos = positions[indexPosition - 1];
				vertex.normal = normals[indexNormal - 1];
				vertex.uv = texcoords[indexTexcoord - 1];
				vertices.emplace_back(vertex);

				indices.emplace_back((unsigned short)indices.size());
			}
		}
	}
	//�t�@�C������
	file.close();

	//std::vector<VertexPosNormalUv> realVertices;
	//// ���_���W�̌v�Z�i�d������j
	//{
	//	realVertices.resize((division + 1) * 2);
	//	int index = 0;
	//	float zValue;

	//	// ���
	//	zValue = prizmHeight / 2.0f;
	//	for (int i = 0; i < division; i++)
	//	{
	//		XMFLOAT3 vertex;
	//		vertex.x = radius * sinf(XM_2PI / division * i);
	//		vertex.y = radius * cosf(XM_2PI / division * i);
	//		vertex.z = zValue;
	//		realVertices[index++].pos = vertex;
	//	}
	//	realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// ��ʂ̒��S�_
	//	// �V��
	//	zValue = -prizmHeight / 2.0f;
	//	for (int i = 0; i < division; i++)
	//	{
	//		XMFLOAT3 vertex;
	//		vertex.x = radius * sinf(XM_2PI / division * i);
	//		vertex.y = radius * cosf(XM_2PI / division * i);
	//		vertex.z = zValue;
	//		realVertices[index++].pos = vertex;
	//	}
	//	realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// �V�ʂ̒��S�_
	//}

	//// ���_���W�̌v�Z�i�d���Ȃ��j
	//{
	//	int index = 0;
	//	// ���
	//	for (int i = 0; i < division; i++)
	//	{
	//		unsigned short index0 = i + 1;
	//		unsigned short index1 = i;
	//		unsigned short index2 = division;

	//		vertices[index++] = realVertices[index0];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index2]; // ��ʂ̒��S�_
	//	}
	//	// ��ʂ̍Ō�̎O�p�`��1�Ԗڂ̃C���f�b�N�X��0�ɏ�������
	//	vertices[index - 3] = realVertices[0];

	//	int topStart = division + 1;
	//	// �V��
	//	for (int i = 0; i < division; i++)
	//	{
	//		unsigned short index0 = topStart + i;
	//		unsigned short index1 = topStart + i + 1;
	//		unsigned short index2 = topStart + division;

	//		vertices[index++] = realVertices[index0];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index2]; // �V�ʂ̒��S�_
	//	}
	//	// �V�ʂ̍Ō�̎O�p�`��1�Ԗڂ̃C���f�b�N�X��0�ɏ�������
	//	vertices[index - 2] = realVertices[topStart];

	//	// ����
	//	for (int i = 0; i < division; i++)
	//	{
	//		unsigned short index0 = i + 1;
	//		unsigned short index1 = topStart + i + 1;
	//		unsigned short index2 = i;
	//		unsigned short index3 = topStart + i;

	//		if (i == division - 1)
	//		{
	//			index0 = 0;
	//			index1 = topStart;
	//		}

	//		vertices[index++] = realVertices[index0];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index2];

	//		vertices[index++] = realVertices[index2];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index3];
	//	}
	//}

	//// ���_�C���f�b�N�X�̐ݒ�
	//{
	//	for (int i = 0; i < 3; i++)
	//	{
	//		indices[i] = i;
	//	}
	//}

	//// �@�������̌v�Z
	//for (int i = 0; i < _countof(indices                                                                  ) / 3; i++)
	//{// �O�p�`�P���ƂɌv�Z���Ă���
	//	// �O�p�`�̃C���f�b�N�X���擾
	//	unsigned short index0 = indices[i * 3 + 0];
	//	unsigned short index1 = indices[i * 3 + 1];
	//	unsigned short index2 = indices[i * 3 + 2];
	//	// �O�p�`���\�����钸�_���W���x�N�g���ɑ��
	//	XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
	//	XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
	//	XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
	//	// p0��p1�x�N�g���Ap0��p2�x�N�g�����v�Z
	//	XMVECTOR v1 = XMVectorSubtract(p1, p0);
	//	XMVECTOR v2 = XMVectorSubtract(p2, p0);
	//	// �O�ς͗������琂���ȃx�N�g��
	//	XMVECTOR normal = XMVector3Cross(v1, v2);
	//	// ���K���i������1�ɂ���)
	//	normal = XMVector3Normalize(normal);
	//	// ���߂��@���𒸓_�f�[�^�ɑ��
	//	XMStoreFloat3(&vertices[index0].normal, normal);
	//	XMStoreFloat3(&vertices[index1].normal, normal);
	//	XMStoreFloat3(&vertices[index2].normal, normal);
	//}
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv) * vertices.size());
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	// ���_�o�b�t�@����
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		//&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(result)) {
		assert(0);
		return;
	}

	// �C���f�b�N�X�o�b�t�@����
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		//&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));
	if (FAILED(result)) {
		assert(0);
		return;
	}

	// ���_�o�b�t�@�ւ̃f�[�^�]��
	VertexPosNormalUv* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result)) {
		//memcpy(vertMap, vertices, sizeof(vertices));
		std::copy(vertices.begin(), vertices.end(), vertMap);
		vertBuff->Unmap(0, nullptr);
	}

	// �C���f�b�N�X�o�b�t�@�ւ̃f�[�^�]��
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	if (SUCCEEDED(result)) {

		// �S�C���f�b�N�X�ɑ΂���
		//for (int i = 0; i < _countof(indices); i++)
		//{
			//indexMap[i] = indices[i];	// �C���f�b�N�X���R�s�[
		//}
		std::copy(indices.begin(), indices.end(), indexMap);
		indexBuff->Unmap(0, nullptr);
	}

	// ���_�o�b�t�@�r���[�̍쐬
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//vbView.SizeInBytes = sizeof(vertices);
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	// �C���f�b�N�X�o�b�t�@�r���[�̍쐬
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
	//ibView.SizeInBytes = sizeof(indices);
}

void EnemyArm::UpdateViewMatrix()
{
	// �r���[�s��̍X�V
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
}

bool EnemyArm::Initialize()
{
	// nullptr�`�F�b�N
	assert(device);

	HRESULT result;
	//// �萔�o�b�t�@�̐���
	//result = device->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// �A�b�v���[�h�\
	//	D3D12_HEAP_FLAG_NONE,
	//	&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(&constBuff));

	// �萔�o�b�t�@�̐���B0
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// �A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB0) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB0));
	// �萔�o�b�t�@�̐���B1
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// �A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB1) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB1));


	return true;
}

void EnemyArm::Update(XMMATRIX matview, XMMATRIX matprojection, XMFLOAT4 color)
{
	HRESULT result;
	XMMATRIX matScale, matRot, matTrans;

	// �X�P�[���A��]�A���s�ړ��s��̌v�Z
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	// ���[���h�s��̍���
	matWorld = XMMatrixIdentity(); // �ό`�����Z�b�g
	matWorld *= matScale; // ���[���h�s��ɃX�P�[�����O�𔽉f
	matWorld *= matRot; // ���[���h�s��ɉ�]�𔽉f
	matWorld *= matTrans; // ���[���h�s��ɕ��s�ړ��𔽉f
//	rotation.y = 90;
	//rotation.x = 50;
	// �e�I�u�W�F�N�g�������
	if (parent != nullptr) {
		// �e�I�u�W�F�N�g�̃��[���h�s����|����
		matWorld *= parent->matWorld;
	}

	// �萔�o�b�t�@�փf�[�^�]��
	ConstBufferDataB0* constMap = nullptr;
	result = constBuffB0->Map(0, nullptr, (void**)&constMap);
	constMap->color = color;
	constMap->mat = matWorld * matview * matprojection;	// �s��̍���
	constBuffB0->Unmap(0, nullptr);

	ConstBufferDataB1* constMap1 = nullptr;
	result = constBuffB1->Map(0, nullptr, (void**)&constMap1);
	constMap1->ambient = material.ambient;
	constMap1->diffuse = material.diffuse;
	constMap1->specular = material.specular;
	constMap1->alpha = material.alpha;
	constBuffB1->Unmap(0, nullptr);
}

void EnemyArm::Draw()
{
	// nullptr�`�F�b�N
	assert(device);
	assert(EnemyArm::cmdList);

	// ���_�o�b�t�@�̐ݒ�
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	// �C���f�b�N�X�o�b�t�@�̐ݒ�
	cmdList->IASetIndexBuffer(&ibView);

	// �f�X�N���v�^�q�[�v�̔z��
	ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// �萔�o�b�t�@�r���[���Z�b�g
	//cmdList->SetGraphicsRootConstantBufferView(0, constBuffB0->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(0, constBuffB0->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(1, constBuffB1->GetGPUVirtualAddress());

	// �V�F�[�_���\�[�X�r���[���Z�b�g
	cmdList->SetGraphicsRootDescriptorTable(2, gpuDescHandleSRV);
	//cmdList->SetGraphicsRootDescriptorTable(1, gpuDescHandleSRV);
	// �`��R�}���h
	cmdList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
	//cmdList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
}
void EnemyArm::Arm_Initialize(EnemyArm* arm, XMFLOAT3 _Arm_Position)
{
	arm->SetScale({ 25,30,25 });
	Arm_Position = _Arm_Position;
	//= _Arm_Position;// { 190, -140, 150 };
	Attack_Timer = 0;
	Attack_Set = 0;
	arm->SetPosition(Arm_Position);
	//arm->SetPosition(Arm_Position);
}
void EnemyArm::Arm_Attack(EnemyArm* arm)
{
	//�U���O�̃^�C�}�[�@�������悤��
	Attack_Timer++;
	if (Attack_Timer < 49) {
		Attack_Set = 1;
	} else if (Attack_Timer >= 49) {
		Attack_Set = 2;//�U���@�n
	}

	if (Attack_Set == 2) {
		Arm_Position.z -= Attack_Speed;
	}

	if (Arm_Position.z < -700) {
		Return_Position = 1;
		Arm_Position = { -190, 140, 150 };
	}
	//else {
		//Return_Position = 0;
	//}
	if (Return_Position == 1) {
		arm->Arm_Initialize_Move(arm);
	}
	arm->SetPosition(Arm_Position);
}

//�r�����̈ʒu�ɕԂ����߂̏����@�r���~��Ă�����
void EnemyArm::Arm_Initialize_Move(EnemyArm* arm)
{
	Attack_Timer = 0;
	Attack_Set = 0;

	if (Arm_Position.y > -150) {
		Arm_Position.y -= 10;
	} else {//2����0�ɖ߂��̂�main��
		Return_Position = 2;
	}
	//}
	arm->SetPosition(Arm_Position);
}
//�����̍U��
void EnemyArm::Arm_Attack_Inside(EnemyArm* arm)
{
	//�U�����I����Ė߂��Ă���ʒu�̐ݒ�
	if (Return_Position == 1) {
		Arm_Position.x = -190;
		Arm_Attack(arm);
	}
	//�U���O�ɘr������֊񂹂鏈��
	if (Arm_Position.x <= -50) {
		Arm_Position.x += 10;
	} else {
		Arm_Attack(arm);
	}
	arm->SetPosition(Arm_Position);
}
#pragma region ����
void EnemyArm::Arm_Initialize_Left(EnemyArm* arm, XMFLOAT3 _Arm_Position)
{

	XMFLOAT3 Arm_Scale = arm->GetScale();
	Arm_Position_Left = _Arm_Position;// { 190, -140, 150 };
	Attack_Timer = 0;
	Attack_Set_Left = 0;
	//if (arm->GetScale().x == 0) {

	//}
	//arm->SetScale(Arm_Scale);
	arm->SetPosition(Arm_Position_Left);
}
void EnemyArm::Arm_Attack_Left(EnemyArm* arm)
{
	//�U���O�̃^�C�}�[�@�������悤��
	Attack_Timer++;
	if (Attack_Timer < 49) {
		Attack_Set_Left = 1;
	} else if (Attack_Timer >= 49) {
		Attack_Set_Left = 2;//�U���@�n
	}

	if (Attack_Set_Left == 2) {
		Arm_Position_Left.z -= Attack_Speed;
	}

	if (Arm_Position_Left.z < -400) {
		Return_Position_Left = 1;
		Arm_Position_Left = { 214, 140, 150 };
	}
	if (Return_Position_Left == 1) {
		arm->Arm_Initialize_Move_Left(arm);
	}
	arm->SetPosition(Arm_Position_Left);
}

//�r�����̈ʒu�ɕԂ����߂̏����@�r���~��Ă�����
void EnemyArm::Arm_Initialize_Move_Left(EnemyArm* arm)
{
	Attack_Timer = 0;
	Attack_Set_Left = 0;

	if (Arm_Position_Left.y > -150) {
		Arm_Position_Left.y -= 10;
	} else {//2����0�ɖ߂��̂�main��
		Return_Position_Left = 2;
	}
	//}
	arm->SetPosition(Arm_Position_Left);
}
//�����̍U��
void EnemyArm::Arm_Attack_Inside_Left(EnemyArm* arm)
{
	//�U�����I����Ė߂��Ă���ʒu�̐ݒ�
	if (Return_Position_Left == 1) {
		Arm_Position_Left.x = 214;
		Arm_Attack_Left(arm);
	}
	//�U���O�ɘr������֊񂹂鏈��
	if (Arm_Position_Left.x >= 70) {
		Arm_Position_Left.x -= 10;
	} else {
		Arm_Attack_Left(arm);
	}
	arm->SetPosition(Arm_Position_Left);
}

//������΂�
void EnemyArm::KnockBack_Attack(float& posz, float& frame, float& maxframe, float& move)
{
	Knock_Flag = 0;
	float StartPos;
	if (Knock_Flag == 0) {
		StartPos = posz;
		Knock_Flag = 1;
		//frame = 0;
	}
	if (Knock_Flag == 1) {
		frame = frame + 3.5;
		//���[������
		if (frame >= -50 && frame <= maxframe) {
			posz = StartPos - move * sqrt((frame / maxframe));
		} else if (frame > maxframe) {
			Knock_Flag = 2;
			frame = 0;
		}
	}
}

void EnemyArm::Knock_Attack_Motion(EnemyArm* RightArm, EnemyArm* LeftArm)
{
	XMFLOAT3 Right_Rot = RightArm->GetRotation();
	XMFLOAT3 Left_Rot = LeftArm->GetRotation();
	XMFLOAT3 Right_Scl = RightArm->GetScale();
	XMFLOAT3 Left_Scl = LeftArm->GetScale();

	if (Knock_Motion_Set != 0) {
		Right_Scl = { 25,30,31 };
		Left_Scl = { 25,30,31 };
	}
	//�����ݒ�
	if (Knock_Motion_Set == 0) {
		Right_Rot.y = 45;
		Right_Rot.z = 90;
		Right_Rot.x = 20;
		Left_Rot.x = 20;
		Left_Rot.y = -45;
		Left_Rot.z = -90;
		if (Right_Scl.z < 50 || Left_Scl.z < 50) {
			Right_Scl.z++;
			Left_Scl.z++;
		}
		//Arm_Position = { -214, -100, 130 };
		//Arm_Position_Left = { 214, -100, 130 };

		Knock_Timer++;
		if (Knock_Timer > 150) {
			Knock_Motion_Set = 1;
		}
	}
	//int Knock_Flag = 0;
	//�r�U�苓��
	if (Knock_Motion_Set == 1) {
		Knock_Timer = 0;
		Arm_Position_Left.z -= 5;
		Arm_Position.z -= 5;
		Arm_Position_Left.y--;
		Arm_Position.y--;
		if (Right_Rot.y > -65) {
			Right_Rot.y -= 10;
		}
		if (Left_Rot.y < 65) {
			Left_Rot.y += 10;
		}
		//if (Arm_Position.x < 190) {
		Arm_Position.x += 25;
		Arm_Position_Left.x -= 25;
		if (Arm_Position.x >= 525 || Arm_Position_Left.x <= -525) {
			Knock_Motion_Set = 2;
		}
		//�r�������O�ʂ�
		Right_Rot.z = 90;
		Left_Rot.z = -90;
	}
	if (Knock_Motion_Set == 2) {
		Arm_Position = { -190,250,150 };
		Arm_Position_Left = { 210,250,150 };
		Knock_Motion_Set = 3;
	}
	if (Knock_Motion_Set == 3) {
		if (Arm_Position.y > -140) {
			Arm_Position.y -= 5;
		} else {
			Knock_Motion_Set = 4;
		}
		if (Arm_Position_Left.y > -140) {
			Arm_Position_Left.y -= 5;

		} else {
			RightArm->SetScale(Right_Scl);
			LeftArm->SetScale(Left_Scl);
			Knock_Motion_Set = 4;
		}

	}
	RightArm->SetPosition(Arm_Position);
	RightArm->SetRotation(Right_Rot);
	RightArm->SetScale(Right_Scl);
	LeftArm->SetScale(Left_Scl);
	LeftArm->SetPosition(Arm_Position_Left);
	LeftArm->SetRotation(Left_Rot);

}

void EnemyArm::Clear_Motion(EnemyArm* pright, EnemyArm* pleft, XMFLOAT3& right, XMFLOAT3& left)
{
	//XMFLOAT3 right_rot=right->GetRotation();
	//XMFLOAT3 left_rot=left->GetRotation();;
	//timer++;
	if (Clear_Motion_Set == 0) {
		timer++;
		//�E�r����ver
		Arm_Position_Left = { 130,-60,150 };
		Arm_Position = { -130,-60,150 };

		right = { 44.000000,-86.000000,59.000000 };
		//right->SetRotation({ 44.000000,-86.000000,59.000000 });
		//���r����ver
		left = { 29.000000,82.000000,-57.000000 };
		//left->SetRotation({ 29.000000,82.000000,-57.000000 });
		if (timer > 200) {
			Clear_Motion_Set = 1;
		}
	}
	if (Clear_Motion_Set == 1) {
		//�E�r�L����ver
		timer = 0;
		if (right.y < 65) {
			right.y += 6;
		}
		if (right.z < 128) {
			right.z += 5.5;
		}
		if (right.z > 23) {
			right.z += 1.5;
		}

		//�E�r����ver
		//arm->SetRotation({ 23.000000,65.000000,128.000000 });
		//arm_left->SetRotation({ 43.000000, -56.000000, -155.000000 });

		//arm->SetRotation({ 43.000000,7.000000,74.000000 });
		//left->SetRotation({ 29.000000,82.000000,-57.000000 });
		//���r�L����ver
		if (left.x < 43) {
			left.x += 6.5;
		}
		if (left.y > -56) {
			left.y -= 5.5;

		} else {
			Clear_Motion_Set = 2;
		}
		if (left.z > -155) {
			left.z -= 6;
		}
		Arm_Position.y--;
		Arm_Position_Left.y--;
		Arm_Position.x -= 5;
		Arm_Position_Left.x += 5;
		//arm_left->SetRotation({ 64.000000,-31.000000,-107.000000 });
	}
	pright->SetPosition(Arm_Position);
	pleft->SetPosition(Arm_Position_Left);
}