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
private: // エイリアス
// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	//	model* model;
		//model* model2;
public: // サブクラス
	// 頂点データ構造体
	struct VertexPosNormalUv
	{
		XMFLOAT3 pos; // xyz座標
		XMFLOAT3 normal; // 法線ベクトル
		XMFLOAT2 uv;  // uv座標
	};

	// 定数バッファ用データ構造体
	//struct ConstBufferData
	//{
		//XMFLOAT4 color;	// 色 (RGBA)
		//XMMATRIX mat;	// ３Ｄ変換行列
	//};

	//マテリアル
	struct Material
	{
		std::string name;//マテリアル名
		XMFLOAT3 ambient;//アンビエント影響度
		XMFLOAT3 diffuse;//ディヒューズ影響度
		XMFLOAT3 specular;//スペキュラ影響度
		float alpha;//
		std::string textureFilename;//テクスチャファイル名
		//コンストラクタ
		Material() {
			ambient = { 0.3f,0.3f,0.3f };
			diffuse = { 0.0f,0.0f,0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};
	//定数バッファ用データ構造体
	struct ConstBufferDataB0
	{
		XMFLOAT4 color;
		XMMATRIX mat;
	};
	//定数バッファ用データ構造体B1
	struct ConstBufferDataB1
	{
		XMFLOAT3 ambient;
		float pad1;
		XMFLOAT3 diffuse;
		float pad2;
		XMFLOAT3 specular;
		float alpha;
	};
private: // 定数

	static const int division = 50;					// 分割数
	static const float radius;				// 底面の半径
	static const float prizmHeight;			// 柱の高さ
	static const int planeCount = division * 2 + division * 2;		// 面の数
	static const int vertexCount = planeCount * 3;		// 頂点数

public: // 静的メンバ関数
	/// <summary>
	/// 静的初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="window_width">画面幅</param>
	/// <param name="window_height">画面高さ</param>
	/// <returns>成否</returns>
	static bool StaticInitialize(ID3D12Device* device, int window_width, int window_height);

	/// <summary>
	/// 描画前処理
	/// </summary>
	/// <param name="cmdList">描画コマンドリスト</param>
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// 描画後処理
	/// </summary>
	static void PostDraw();

	/// <summary>
	/// 3Dオブジェクト生成
	/// </summary>
	/// <returns></returns>
	static EnemyArm* Create();

	/// <summary>
	/// 視点座標の取得
	/// </summary>
	/// <returns>座標</returns>
	static const XMFLOAT3& GetEye() { return eye; }

	/// <summary>
	/// 視点座標の設定
	/// </summary>
	/// <param name="position">座標</param>
	static void SetEye(XMFLOAT3 eye);

	/// <summary>
	/// 注視点座標の取得
	/// </summary>
	/// <returns>座標</returns>
	static const XMFLOAT3& GetTarget() { return target; }

	/// <summary>
	/// 注視点座標の設定
	/// </summary>
	/// <param name="position">座標</param>
	static void SetTarget(XMFLOAT3 target);

	/// <summary>
	/// ベクトルによる移動
	/// </summary>
	/// <param name="move">移動量</param>
	static void CameraMoveVector(XMFLOAT3 move);

private: // 静的メンバ変数
	// デバイス
	static ID3D12Device* device;
	// デスクリプタサイズ
	static UINT descriptorHandleIncrementSize;
	// コマンドリスト
	static ID3D12GraphicsCommandList* cmdList;
	// ルートシグネチャ
	static ComPtr<ID3D12RootSignature> rootsignature;
	// パイプラインステートオブジェクト
	static ComPtr<ID3D12PipelineState> pipelinestate;
	// デスクリプタヒープ
	static ComPtr<ID3D12DescriptorHeap> descHeap;
	// 頂点バッファ
	static ComPtr<ID3D12Resource> vertBuff;
	// インデックスバッファ
	static ComPtr<ID3D12Resource> indexBuff;
	// テクスチャバッファ
	static ComPtr<ID3D12Resource> texbuff;
	// シェーダリソースビューのハンドル(CPU)
	static CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// シェーダリソースビューのハンドル(CPU)
	static CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	// ビュー行列
	static XMMATRIX matView;
	// 射影行列
	static XMMATRIX matProjection;
	// 視点座標
	static XMFLOAT3 eye;
	// 注視点座標
	static XMFLOAT3 target;
	// 上方向ベクトル
	static XMFLOAT3 up;
	// 頂点バッファビュー
	static D3D12_VERTEX_BUFFER_VIEW vbView;
	// インデックスバッファビュー
	static D3D12_INDEX_BUFFER_VIEW ibView;
	// 頂点データ配列
	static std::vector<VertexPosNormalUv>vertices;
	//static VertexPosNormalUv vertices[vertexCount];
	// 頂点インデックス配列
	static std::vector<unsigned short>indices;
	//static unsigned short indices[planeCount * 3];
	//マテリアル
	static Material material;
private:// 静的メンバ関数
	/// <summary>
	/// デスクリプタヒープの初期化
	/// </summary>
	/// <returns></returns>
	static bool InitializeDescriptorHeap();

	/// <summary>
	/// カメラ初期化
	/// </summary>
	/// <param name="window_width">画面横幅</param>
	/// <param name="window_height">画面縦幅</param>
	static void InitializeCamera(int window_width, int window_height);

	/// <summary>
	/// グラフィックパイプライン生成
	/// </summary>
	/// <returns>成否</returns>
	static bool InitializeGraphicsPipeline();

	/// <summary>
	/// テクスチャ読み込み
	/// </summary>
	/// <returns>成否</returns>
	static bool LoadTexture(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// モデル作成
	/// </summary>


	/// <summary>
	/// ビュー行列を更新
	/// </summary>
	static void UpdateViewMatrix();

	static void LoadMaterial(const std::string& directoryPath, const std::string& filename);
public: // メンバ関数
	bool Initialize();
	void CreateModel();
	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update(XMMATRIX matview, XMMATRIX matprojection, XMFLOAT4 color);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 座標の取得
	/// </summary>
	/// <returns>座標</returns>
	const XMFLOAT3& GetPosition() { return position; }
	const XMFLOAT3& GetRotation() { return rotation; }
	const XMFLOAT3& GetScale() { return scale; }

	/// <summary>
	/// 座標の設定
	/// </summary>
	/// <param name="position">座標</param>
	void SetPosition(XMFLOAT3 position) { this->position = position; }
	void SetRotation(XMFLOAT3 rotation) { this->rotation = rotation; }
	void SetScale(XMFLOAT3 scale) { this->scale = scale; }

private: // メンバ変数
	ComPtr<ID3D12Resource> constBuffB0; // 定数バッファ
	ComPtr<ID3D12Resource> constBuffB1;
	// 色
	//XMFLOAT4 color = { 1,1,1,1 };
	// ローカルスケール
	XMFLOAT3 scale = { 1,1,1 };
	// X,Y,Z軸回りのローカル回転角
	XMFLOAT3 rotation = { 0,0,0 };
	// ローカル座標
	XMFLOAT3 position = { 0,0,0 };
	// ローカルワールド変換行列
	XMMATRIX matWorld;
	// 親オブジェクト
	EnemyArm* parent = nullptr;
	//敵の行動パターン
	//MeteoModel* meteo = new MeteoModel();

	int Attack_Timer = 0;
	//右側
	static XMFLOAT3 Arm_Position;
	static float Attack_Speed;
	static int Return_Position;
	int Attack_Set = 0;
	//左側
	static XMFLOAT3 Arm_Position_Left;
	static int Return_Position_Left;
	int Attack_Set_Left = 0;
public:
	//右側
	int GetReturnpos() { return Return_Position; }
	int GetAttack_Set() { return Attack_Set; }
	void SetReturnPos(int returnpos) { this->Return_Position = returnpos; }

	void Arm_Initialize(EnemyArm* arm, XMFLOAT3 _Arm_Position);
	void Arm_Initialize_Move(EnemyArm* arm);
	//左右　-190:214
	void Arm_Attack(EnemyArm* arm);
	void Arm_Attack_Inside(EnemyArm* arm);
	//左側
	int GetReturnpos_Left() { return Return_Position_Left; }
	int GetAttack_Set_Left() { return Attack_Set_Left; }
	void SetReturnPos_Left(int returnpos_Left) { this->Return_Position_Left = returnpos_Left; }

	void Arm_Initialize_Left(EnemyArm* arm, XMFLOAT3 _Arm_Position);
	void Arm_Initialize_Move_Left(EnemyArm* arm);
	//左右　-190:214
	void Arm_Attack_Left(EnemyArm* arm);
	void Arm_Attack_Inside_Left(EnemyArm* arm);

	//ノックバック攻撃

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


