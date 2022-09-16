#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include<string>
#include"Input.h"
#include"ObjectNew.h"
class EnemyArm
{
private: // �G�C���A�X
// Microsoft::WRL::���ȗ�
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::���ȗ�
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	//	model* model;
		//model* model2;
public: // �T�u�N���X
	// ���_�f�[�^�\����
	struct VertexPosNormalUv
	{
		XMFLOAT3 pos; // xyz���W
		XMFLOAT3 normal; // �@���x�N�g��
		XMFLOAT2 uv;  // uv���W
	};

	// �萔�o�b�t�@�p�f�[�^�\����
	//struct ConstBufferData
	//{
		//XMFLOAT4 color;	// �F (RGBA)
		//XMMATRIX mat;	// �R�c�ϊ��s��
	//};

	//�}�e���A��
	struct Material
	{
		std::string name;//�}�e���A����
		XMFLOAT3 ambient;//�A���r�G���g�e���x
		XMFLOAT3 diffuse;//�f�B�q���[�Y�e���x
		XMFLOAT3 specular;//�X�y�L�����e���x
		float alpha;//
		std::string textureFilename;//�e�N�X�`���t�@�C����
		//�R���X�g���N�^
		Material() {
			ambient = { 0.3f,0.3f,0.3f };
			diffuse = { 0.0f,0.0f,0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};
	//�萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferDataB0
	{
		XMFLOAT4 color;
		XMMATRIX mat;
	};
	//�萔�o�b�t�@�p�f�[�^�\����B1
	struct ConstBufferDataB1
	{
		XMFLOAT3 ambient;
		float pad1;
		XMFLOAT3 diffuse;
		float pad2;
		XMFLOAT3 specular;
		float alpha;
	};
private: // �萔

	static const int division = 50;					// ������
	static const float radius;				// ��ʂ̔��a
	static const float prizmHeight;			// ���̍���
	static const int planeCount = division * 2 + division * 2;		// �ʂ̐�
	static const int vertexCount = planeCount * 3;		// ���_��

public: // �ÓI�����o�֐�
	/// <summary>
	/// �ÓI������
	/// </summary>
	/// <param name="device">�f�o�C�X</param>
	/// <param name="window_width">��ʕ�</param>
	/// <param name="window_height">��ʍ���</param>
	/// <returns>����</returns>
	static bool StaticInitialize(ID3D12Device* device, int window_width, int window_height);

	/// <summary>
	/// �`��O����
	/// </summary>
	/// <param name="cmdList">�`��R�}���h���X�g</param>
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// �`��㏈��
	/// </summary>
	static void PostDraw();

	/// <summary>
	/// 3D�I�u�W�F�N�g����
	/// </summary>
	/// <returns></returns>
	static EnemyArm* Create();

	/// <summary>
	/// ���_���W�̎擾
	/// </summary>
	/// <returns>���W</returns>
	static const XMFLOAT3& GetEye() { return eye; }

	/// <summary>
	/// ���_���W�̐ݒ�
	/// </summary>
	/// <param name="position">���W</param>
	static void SetEye(XMFLOAT3 eye);

	/// <summary>
	/// �����_���W�̎擾
	/// </summary>
	/// <returns>���W</returns>
	static const XMFLOAT3& GetTarget() { return target; }

	/// <summary>
	/// �����_���W�̐ݒ�
	/// </summary>
	/// <param name="position">���W</param>
	static void SetTarget(XMFLOAT3 target);

	/// <summary>
	/// �x�N�g���ɂ��ړ�
	/// </summary>
	/// <param name="move">�ړ���</param>
	static void CameraMoveVector(XMFLOAT3 move);

private: // �ÓI�����o�ϐ�
	// �f�o�C�X
	static ID3D12Device* device;
	// �f�X�N���v�^�T�C�Y
	static UINT descriptorHandleIncrementSize;
	// �R�}���h���X�g
	static ID3D12GraphicsCommandList* cmdList;
	// ���[�g�V�O�l�`��
	static ComPtr<ID3D12RootSignature> rootsignature;
	// �p�C�v���C���X�e�[�g�I�u�W�F�N�g
	static ComPtr<ID3D12PipelineState> pipelinestate;
	// �f�X�N���v�^�q�[�v
	static ComPtr<ID3D12DescriptorHeap> descHeap;
	// ���_�o�b�t�@
	static ComPtr<ID3D12Resource> vertBuff;
	// �C���f�b�N�X�o�b�t�@
	static ComPtr<ID3D12Resource> indexBuff;
	// �e�N�X�`���o�b�t�@
	static ComPtr<ID3D12Resource> texbuff;
	// �V�F�[�_���\�[�X�r���[�̃n���h��(CPU)
	static CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// �V�F�[�_���\�[�X�r���[�̃n���h��(CPU)
	static CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	// �r���[�s��
	static XMMATRIX matView;
	// �ˉe�s��
	static XMMATRIX matProjection;
	// ���_���W
	static XMFLOAT3 eye;
	// �����_���W
	static XMFLOAT3 target;
	// ������x�N�g��
	static XMFLOAT3 up;
	// ���_�o�b�t�@�r���[
	static D3D12_VERTEX_BUFFER_VIEW vbView;
	// �C���f�b�N�X�o�b�t�@�r���[
	static D3D12_INDEX_BUFFER_VIEW ibView;
	// ���_�f�[�^�z��
	static std::vector<VertexPosNormalUv>vertices;
	//static VertexPosNormalUv vertices[vertexCount];
	// ���_�C���f�b�N�X�z��
	static std::vector<unsigned short>indices;
	//static unsigned short indices[planeCount * 3];
	//�}�e���A��
	static Material material;
private:// �ÓI�����o�֐�
	/// <summary>
	/// �f�X�N���v�^�q�[�v�̏�����
	/// </summary>
	/// <returns></returns>
	static bool InitializeDescriptorHeap();

	/// <summary>
	/// �J����������
	/// </summary>
	/// <param name="window_width">��ʉ���</param>
	/// <param name="window_height">��ʏc��</param>
	static void InitializeCamera(int window_width, int window_height);

	/// <summary>
	/// �O���t�B�b�N�p�C�v���C������
	/// </summary>
	/// <returns>����</returns>
	static bool InitializeGraphicsPipeline();

	/// <summary>
	/// �e�N�X�`���ǂݍ���
	/// </summary>
	/// <returns>����</returns>
	static bool LoadTexture(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// ���f���쐬
	/// </summary>


	/// <summary>
	/// �r���[�s����X�V
	/// </summary>
	static void UpdateViewMatrix();

	static void LoadMaterial(const std::string& directoryPath, const std::string& filename);
public: // �����o�֐�
	bool Initialize();
	void CreateModel();
	/// <summary>
	/// ���t���[������
	/// </summary>
	void Update(XMMATRIX matview, XMMATRIX matprojection, XMFLOAT4 color);

	/// <summary>
	/// �`��
	/// </summary>
	void Draw();

	/// <summary>
	/// ���W�̎擾
	/// </summary>
	/// <returns>���W</returns>
	const XMFLOAT3& GetPosition() { return position; }
	const XMFLOAT3& GetRotation() { return rotation; }
	const XMFLOAT3& GetScale() { return scale; }

	/// <summary>
	/// ���W�̐ݒ�
	/// </summary>
	/// <param name="position">���W</param>
	void SetPosition(XMFLOAT3 position) { this->position = position; }
	void SetRotation(XMFLOAT3 rotation) { this->rotation = rotation; }
	void SetScale(XMFLOAT3 scale) { this->scale = scale; }

private: // �����o�ϐ�
	ComPtr<ID3D12Resource> constBuffB0; // �萔�o�b�t�@
	ComPtr<ID3D12Resource> constBuffB1;
	// �F
	//XMFLOAT4 color = { 1,1,1,1 };
	// ���[�J���X�P�[��
	XMFLOAT3 scale = { 1,1,1 };
	// X,Y,Z�����̃��[�J����]�p
	XMFLOAT3 rotation = { 0,0,0 };
	// ���[�J�����W
	XMFLOAT3 position = { 0,0,0 };
	// ���[�J�����[���h�ϊ��s��
	XMMATRIX matWorld;
	// �e�I�u�W�F�N�g
	EnemyArm* parent = nullptr;
	//�G�̍s���p�^�[��
	//MeteoModel* meteo = new MeteoModel();

	int Attack_Timer = 0;
	//�E��
	static XMFLOAT3 Arm_Position;
	static float Attack_Speed;
	static int Return_Position;
	int Attack_Set = 0;
	//����
	static XMFLOAT3 Arm_Position_Left;
	static int Return_Position_Left;
	int Attack_Set_Left = 0;
public:
	//�E��
	int GetReturnpos() { return Return_Position; }
	int GetAttack_Set() { return Attack_Set; }
	void SetReturnPos(int returnpos) { this->Return_Position = returnpos; }

	void Arm_Initialize(EnemyArm* arm, XMFLOAT3 _Arm_Position);
	void Arm_Initialize_Move(EnemyArm* arm);
	//���E�@-190:214
	void Arm_Attack(EnemyArm* arm);
	void Arm_Attack_Inside(EnemyArm* arm);
	//����
	int GetReturnpos_Left() { return Return_Position_Left; }
	int GetAttack_Set_Left() { return Attack_Set_Left; }
	void SetReturnPos_Left(int returnpos_Left) { this->Return_Position_Left = returnpos_Left; }

	void Arm_Initialize_Left(EnemyArm* arm, XMFLOAT3 _Arm_Position);
	void Arm_Initialize_Move_Left(EnemyArm* arm);
	//���E�@-190:214
	void Arm_Attack_Left(EnemyArm* arm);
	void Arm_Attack_Inside_Left(EnemyArm* arm);

	//�m�b�N�o�b�N�U��

	void KnockBack_Attack(float& posx, float& frame, float& maxframe, float& move);

private:
	int Knock_Flag;
	int Knock_Timer = 0;
	int Knock_Motion_Set = 0;
	//
	int returnpos_Knock = 0;
	int returnpos_Knock_Left = 0;
public:
	int gettimer() { return Knock_Timer; }
	void settimer(int timer) { this->Knock_Timer = timer; }
	int getreturnpos_Knock() { return returnpos_Knock; }
	int getreturnpos_Knock_Left() { return returnpos_Knock_Left; }
	void SetKnockFlag(int flag) { this->Knock_Flag = flag; }
	int GetKnockFlag() { return Knock_Flag; }
	void Knock_Attack_Motion(EnemyArm* RightArm, EnemyArm* LeftArm);
	int GetKnock_Motion_Set() { return Knock_Motion_Set; }
	void SetKnock_Motion_Set(int set) { this->Knock_Motion_Set = set; }
	int timer = 0;
	void Clear_Motion(EnemyArm* pright, EnemyArm* pleft, XMFLOAT3& right, XMFLOAT3& left);
	int Clear_Motion_Set = 0;
	int GetClear_Motion_Set() { return Clear_Motion_Set; }
	void setClear_Motion_Set(int set) { this->Clear_Motion_Set = set; }
};


