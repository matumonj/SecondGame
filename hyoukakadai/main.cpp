#pragma region include周りの宣言
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
//まっつんホーム汚すぎ
//それな
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
//円当たり判定
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
#pragma region DirectXの初期化
	const int window_width = 1820;
	const int window_height = 780;

	WinApp* winapp = nullptr;
	//初期化
	winapp = new WinApp();
	winapp->Createwindow();

	MSG msg{};
	//でバッグレイヤーのやつ
#ifdef _DEBUG
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif
#pragma region ポインタ置き場
	//DirectXの共通部分
	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winapp);
	HRESULT result;

	// キーボードなどの入力処理
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winapp);

#pragma endregion

	//頂点データ3点分の座標
#pragma region 描画初期化処理
#pragma region 頂点データ構造体と初期化(vertices)	
//頂点データ構造体
	//三角錐用
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	//スプライト用
	struct VertexPosUv
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	//背景用
	struct BackGround
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

	//const float topHeight = 10.0f;
	//const int Divs = 3;
	//const float radius = 10.0f;
	//三角錐の頂点データ
	const int division = 30;					// 分割数
	float radius = 10.0f;				// 底面の半径
	const float prizmHeight = 10.0f;			// 柱の高さ
	const int planeCount = division * 2 + division * 2;		// 面の数
	const int vertexCount = planeCount * 3;		// 頂点数
	Vertex vertices[vertexCount];
	unsigned short indices[planeCount * 3];
	// 頂点インデックス配列
	std::vector<Vertex> realVertices;
	// 頂点座標の計算（重複あり）
	{
		realVertices.resize((division + 1) * 2);
		int index = 0;
		float zValue;

		// 底面
		zValue = prizmHeight / 2.0f;
		for (int i = 0; i < division; i++)
		{
			XMFLOAT3 vertex;
			vertex.x = radius * sinf(XM_2PI / division * i);
			vertex.y = radius * cosf(XM_2PI / division * i);
			vertex.z = zValue;
			realVertices[index++].pos = vertex;
		}
		realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// 底面の中心点
		// 天面
		zValue = -prizmHeight / 2.0f;
		for (int i = 0; i < division; i++)
		{
			XMFLOAT3 vertex;
			vertex.x = radius * sinf(XM_2PI / division * i);
			vertex.y = radius * cosf(XM_2PI / division * i);
			vertex.z = zValue;
			realVertices[index++].pos = vertex;
		}
		realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// 天面の中心点
	}

	// 頂点座標の計算（重複なし）
	{
		int index = 0;
		// 底面
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = i + 1;
			unsigned short index1 = i;
			unsigned short index2 = division;

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2]; // 底面の中心点
		}
		// 底面の最後の三角形の1番目のインデックスを0に書き換え
		vertices[index - 3] = realVertices[0];

		int topStart = division + 1;
		// 天面
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = topStart + i;
			unsigned short index1 = topStart + i + 1;
			unsigned short index2 = topStart + division;

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2]; // 天面の中心点
		}
		// 天面の最後の三角形の1番目のインデックスを0に書き換え
		vertices[index - 2] = realVertices[topStart];

		// 側面
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

	// 頂点インデックスの設定
	{
		for (int i = 0; i < _countof(indices); i++)
		{
			indices[i] = i;
		}
	}

	// 法線方向の計算
	for (int i = 0; i < _countof(indices) / 3; i++)
	{// 三角形１つごとに計算していく
		// 三角形のインデックスを取得
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		// 三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		// p0→p1ベクトル、p0→p2ベクトルを計算
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		// 外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);
		// 正規化（長さを1にする)
		normal = XMVector3Normalize(normal);
		// 求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);
	}

	//背景用の板ポリゴン
	BackGround background[] = {
		{{-20.4f,0.0f,0.0f},{}, {0.0f,1.0f}},
		{{-20.4f,50.7f,0.0f},{}, {0.0f,0.0f}},
		{{20.0f,0.0f,0.0f},{}, {1.0f,1.0f}},
		{{20.0f,50.7f,0.0f},{}, {1.0f,0.0f}},
	};


#pragma endregion


#pragma region インデックスデータ
	//インデックスデータ


	//背景用の板ポリゴン　インデックスデータ
	unsigned short backgroundindices[] = {
		0,1,2,
		1,2,3,
	};
	//頂点バッファのサイズ
	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * _countof(vertices));

	ComPtr<ID3D12Resource> vertBuff;
	result = dxCommon->GetDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//仮想メモリを取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i];
	}
	//マップ解除
	vertBuff->Unmap(0, nullptr);

#pragma region 背景用の板ポリゴン
	//頂点バッファのサイズ
	UINT backsizeVB = static_cast<UINT>(sizeof(BackGround) * _countof(background));
	//頂点バッファへの生成
	ComPtr<ID3D12Resource>backvertBuff;
	result = dxCommon->GetDev()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&backvertBuff));

	//仮想メモリを取得
	BackGround* backvertMap = nullptr;
	result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
	for (int i = 0; i < _countof(background); i++) {
		backvertMap[i] = background[i];
	}
	//マップ解除
	backvertBuff->Unmap(0, nullptr);

#pragma endregion

#pragma endregion

	//頂点バッファviewの作成
	D3D12_VERTEX_BUFFER_VIEW vbview{};
	// 頂点バッファビューの作成
	vbview.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbview.SizeInBytes = sizeof(vertices);
	vbview.StrideInBytes = sizeof(vertices[0]);


	//vbview.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//vbview.SizeInBytes = sizeVB;
	//vbview.StrideInBytes = sizeof(Vertex);

	//インデックスバッファの設定	
	//インデックス
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


	//インデックスバッファへのデータ転送
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);

	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}

	indexBuff->Unmap(0, nullptr);

	//インデックスバッファビューの作成
	D3D12_INDEX_BUFFER_VIEW ibView{};
	//ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	//ibView.Format = DXGI_FORMAT_R16_UINT;
	//ibView.SizeInBytes = sizeIB;
	// インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);
#pragma region 背景用の板ポリゴン
	//頂点バッファviewの作成
	D3D12_VERTEX_BUFFER_VIEW backvbview{};

	backvbview.BufferLocation = backvertBuff->GetGPUVirtualAddress();
	backvbview.SizeInBytes = backsizeVB;
	backvbview.StrideInBytes = sizeof(BackGround);

	//インデックスバッファの設定	
	ComPtr<ID3D12Resource>backindexBuff;
	UINT backsizeIB = sizeof(backgroundindices);

	//インデックスバッファの生成
	result = dxCommon->GetDev()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(backsizeIB), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&backindexBuff));

	//仮想メモリを取得
	unsigned short* backindexMap = nullptr;
	result = backindexBuff->Map(0, nullptr, (void**)&backindexMap);

	//全インデックスに対して
	for (int i = 0; i < _countof(backgroundindices); i++) {
		backindexMap[i] = backgroundindices[i];
	}
	backindexBuff->Unmap(0, nullptr);

	//インデックスバッファviewの作成
	D3D12_INDEX_BUFFER_VIEW backibview{};
	backibview.BufferLocation = backindexBuff->GetGPUVirtualAddress();
	backibview.Format = DXGI_FORMAT_R16_UINT;
	backibview.SizeInBytes = sizeof(backgroundindices);

#pragma endregion

	//仮想メモリを取得

	//射影変換行列の作り
	XMMATRIX matprojection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)window_width / (float)window_height, 0.1f, 1000.0f);

	//ビュー変換行列
	XMMATRIX matview;
	XMFLOAT3 eye(0, 0, -10);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//カメラの回転角
	float angle = -10.0f;

	//ワールド変換行列
	XMMATRIX matworld0;

	XMFLOAT3 position;
	position = { 0.0f,0.0f,0.0f };

	XMMATRIX matworld1;


	//デスクリプタヒープ拡張用の変数
	const int constantBufferNum = 128;
	//定数バッファ用でスクリプタヒープの生成
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

	//3dオブジェクトの数
#pragma region オブジェクト生成(モデルではない)
	const int object_num = 15;

	Object3d object3ds[object_num];//敵機のようなもの
	//Object3d player;//プレイヤー
	Object3d backgrounds;//保留
	Object3d Arm_DamageArea;
	Object3d Arm_DamageArea_Left;
	Object3d Knock_DamageArea;
	Object3d Art_DamageArea;
	//Object3d backgroundsleft;//保留
	//Object3d backgroundsright;//保留
	Object3d L_Meteo_DamageArea[4];
	Object3d pmdmodel;
#pragma endregion
#pragma region オブジェクトの初期化処理
	//player用
	//背景用　今は保留
	InitializeObject3d(&backgrounds, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Arm_DamageArea, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Arm_DamageArea_Left, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Knock_DamageArea, 1, dxCommon->GetDev(), basicDescHeap);
	InitializeObject3d(&Art_DamageArea, 1, dxCommon->GetDev(), basicDescHeap);
	for (int i = 0; i < 4; i++) {
		InitializeObject3d(&L_Meteo_DamageArea[i], 1, dxCommon->GetDev(), basicDescHeap);
	}
#pragma endregion
	//シェーダリソースビューのアドレス計算処理
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

	//ハンドルのアドレスを進める
	cpuDescHandleSRV.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	gpuDescHandleSRV.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV2 = basicDescHeap->GetGPUDescriptorHandleForHeapStart();
	gpuDescHandleSRV2.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#pragma region スプライトの生成
	//共通データの関数呼び出し
	SpriteCommon spritecommon;
	//スプライト共通データ生成
	spritecommon = SpriteCommonCreate(dxCommon->GetDev(), window_width, window_height);
	//スプライト用テクスチャ読み込みの関数呼び出し
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
	//スプライト初期化関数の呼び出し
	Sprite sprite[11];
	for (int i = 0; i < 5; i++) {
		sprite[i] = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	}
	Sprite Boss_HP = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Boss_HP2 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite title = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite space = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite ArtScene = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	//スプライトの生成
	Boss_HP = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Player_HP = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Player_HP2 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Rule1 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Rule2 = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Clear = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
	Sprite Gameover = SpriteCreate(dxCommon->GetDev(), window_width, window_height);
#pragma endregion
	//WICテクスチャのロード
	//画像ファイルの用意
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

	//テクスチャバッファにデータ転送
	result = texBuff->WriteToSubresource(0, nullptr, img->pixels, static_cast<UINT>(img->rowPitch),
		static_cast<UINT>(img->slicePitch));

	////3
	auto basicHeapHandle2 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		2, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));


	////シェーダリソースビュー設定
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
	//3Dオブジェクト用パイプライン生成
	PipelineSet obj3Dpipelineset = create3Dpipeline(dxCommon->GetDev());
	//スプライト用パイプライン設定
	PipelineSet spriteobj3Dpipelineset = create3DpipelineSprite(dxCommon->GetDev());
	//背景用パイプラインセット
	PipelineSet Backpipelineset = createBackpipeline(dxCommon->GetDev());


#pragma region スプライトの位置やスケールやテクスチャ番号の初期化設定
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
	//シーン用
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
	//スプラトごとにテクスチャを指定
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
	//プレイヤーのY軸回転
	float pRotation;
	int LorRFlag = 1;
	int scene = 1;
	int movesf = 0;
	int mtimer = 0;

	//モデル生成とその初期化周り
	// 3Dオブジェクト生成
#pragma region 3Dオブジェクト静的初期化
		// オーディオの初期化
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
	//敵
	if (!EnemyModel::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//フィールド
	if (!Field::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//落石
	if (!MeteoModel::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//敵の腕
	if (!EnemyArm::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//弾
	if (!Bullet::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
#pragma endregion
#pragma region 各オブジェクトのポインタ置き場
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
	//必殺技だけで使う
	Bullet* Art_BackWall = new Bullet();
	Bullet* SafeZone[2];
	for (int i = 0; i < _countof(SafeZone); i++) {
		SafeZone[i] = new Bullet();
	}
	//クリア時のみ使う
	Bullet* Clear_BackWall = new Bullet();
#pragma endregion
#pragma region 各オブジェクトのモデル生成
	//自機
	player->CreateModel();
	player = ObjectNew::Create();
	player->Update(matview, matprojection, { 1,1,1,1 });
	//敵
	enemy->CreateModel();
	enemy = EnemyModel::Create();
	enemy->Update(matview, matprojection, { 1,1,0,1 });
	//フィールド
	field->CreateModel();
	field = Field::Create();
	field->Update(matview, matprojection, { 1,1,1,1 });
	//落石
	meteo->CreateModel();
	meteo = MeteoModel::Create();
	meteo->Update(matview, matprojection, { 1,1,1,1 });
	//敵の腕
	arm->CreateModel();
	arm = EnemyArm::Create();
	arm->Update(matview, matprojection, { 1,1,1,1 });
	//左
	arm_left->CreateModel();
	arm_left = EnemyArm::Create();
	arm_left->Update(matview, matprojection, { 1,1,1,1 });
	//弾
	for (int i = 0; i < _countof(bullet); i++) {
		bullet[i]->CreateModel();
		bullet[i] = Bullet::Create();
		bullet[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	//広範囲落石
	for (int i = 0; i < _countof(L_meteo); i++) {
		L_meteo[i]->CreateModel();
		L_meteo[i] = MeteoModel::Create();
		L_meteo[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	//必殺技だけで使う
	Art_BackWall->CreateModel();
	Art_BackWall = Bullet::Create();
	Art_BackWall->Update(matview, matprojection, { 1,1,1,1 });
	//安置
	for (int i = 0; i < _countof(SafeZone); i++) {
		SafeZone[i]->CreateModel();
		SafeZone[i] = Bullet::Create();
		SafeZone[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	//必殺技だけで使う
	Clear_BackWall->CreateModel();
	Clear_BackWall = Bullet::Create();
	Clear_BackWall->Update(matview, matprojection, { 1,1,1,1 });

#pragma endregion
#pragma region 各オブジェクトの座標回転スケールの代入変数
	//自機用の座標や回転、スケール
	XMFLOAT3 Player_pos = player->GetPosition();
	XMFLOAT3 Player_rot = player->GetRotation();
	XMFLOAT3 Player_scl = player->GetScale();
	//敵用の座標や回転、スケール
	XMFLOAT3 Enemy_pos = enemy->GetPosition();
	XMFLOAT3 Enemy_rot = enemy->GetRotation();
	XMFLOAT3 Enemy_scl = enemy->GetScale();
	//フィールド用の座標や回転、スケール
	XMFLOAT3 Field_pos = field->GetPosition();
	XMFLOAT3 Field_rot = field->GetRotation();
	XMFLOAT3 Field_scl = field->GetScale();
	//落石用の座標や回転、スケール
	XMFLOAT3 Meteo_pos = meteo->GetPosition();
	XMFLOAT3 Meteo_rot = meteo->GetRotation();
	XMFLOAT3 Meteo_scl = meteo->GetScale();
	//敵の腕の座標や回転、スケール
	XMFLOAT3 Arm_pos = arm->GetPosition();
	XMFLOAT3 Arm_rot = arm->GetRotation();
	XMFLOAT3 Arm_scl = arm->GetScale();
	//左側
	XMFLOAT3 Arm_pos_Left = arm_left->GetPosition();
	XMFLOAT3 Arm_rot_Left = arm_left->GetRotation();
	XMFLOAT3 Arm_scl_Left = arm_left->GetScale();
	//弾
	XMFLOAT3 Bullet_pos[15];
	XMFLOAT3 Bullet_rot[15];
	XMFLOAT3 Bullet_scl[15];
	for (int i = 0; i < 15; i++) {
		Bullet_pos[i] = bullet[i]->GetPosition();
		Bullet_rot[i] = bullet[i]->GetRotation();
		Bullet_scl[i] = bullet[i]->GetScale();
		bullet[i]->Bullet_Initialize(player);
	}
	//必殺技だけで使う
	XMFLOAT3 Art_pos = Art_BackWall->GetPosition();
	XMFLOAT3 Art_rot = Art_BackWall->GetRotation();
	XMFLOAT3 Art_scl = Art_BackWall->GetScale();
	//クリア時のみ使う
	XMFLOAT3 Clear_pos = Clear_BackWall->GetPosition();
	XMFLOAT3 Clear_rot = Clear_BackWall->GetRotation();
	XMFLOAT3 Clear_scl = Clear_BackWall->GetScale();

	//安置
	XMFLOAT3 Safe_pos[2];
	XMFLOAT3 Safe_rot[2];
	XMFLOAT3 Safe_scl[2];
	for (int i = 0; i < 2; i++) {
		Safe_pos[i] = bullet[i]->GetPosition();
		Safe_rot[i] = bullet[i]->GetRotation();
		Safe_scl[i] = bullet[i]->GetScale();
	}
#pragma endregion
#pragma region カメラ系
	//注視点や回転ベクトルの初期化
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
#pragma region 敵の攻撃パターン flag
	int Meteo_Flag = 0;//落石
	int Left_Up_Meteo_Flag = 0;
	int Left_Bottom_Meteo_Flag = 0;
	int Right_Up_Meteo_Flag = 0;
	int Right_Bottom_Meteo_Flag = 0;
	int ehp = 500;
	int Arm_Attack_Flag = 0;//腕突進
	int Inside_Attack_FLAG = 0;
	//左腕
	int Arm_Attack_Flag_Left = 0;//腕突進
	int Inside_Attack_FLAG_Left = 0;

	int clearmotion = 0;
#pragma endregion
#pragma region ポジションや回転の初期化関係
	//落石
	Meteo_pos.y = Player_pos.y + 200;
	meteo->SetPosition(Meteo_pos);
	//敵の腕の位置
	//左腕
	XMFLOAT3 Left_Arm = { 214, -140, 150 };
	//右腕
	XMFLOAT3 Right_Arm = { -190, -140, 150 };
	arm->Arm_Initialize(arm, Right_Arm);
	arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
	//自機の初期化
	Player_pos.z = -150;
	Player_pos.y = -150;
	Player_rot.y = 270;
	Player_scl = { 10,10,10 };
	//敵の初期化
	enemy->SetPosition(Enemy_pos);
	Enemy_pos = { 0,-850,150 };
	Enemy_scl = { 150,150,300 };
	Enemy_rot = { 0,0,0 };
	//フィールドの初期化
	Field_pos = { 15,-235,-90 };
	Field_scl = { 130,25,80 };
	Field_rot = { 0,0,0 };
	//敵の腕の初期化
	arm->SetScale({ 30,25,31 });
	arm_left->SetScale({ 30,25,31 });
#pragma endregion
#pragma region 弾関係
	int Shot_Flag[15];
	for (int i = 0; i < 15; i++)
	{
		Shot_Flag[i] = 0;
	}
	//
	int Enemy_Damage_Flag = 0;
	int Rot_Flag = 0;
#pragma endregion
#pragma region 必殺技のようなもの
	int Art_Rimmit_Flag = 0;
	int Art_Rimmit_Timer = 0;
	int ran[object_num];
	int ranscl[object_num];
	int artflag = 0;
	//広がるときの加速値
	float accel = 0;
	//ｚが一定数伸びたら１に
	int bllowflag = 0;
	//α値
	int Alpha_Flag = 0;
	int barriar = 0;
	//馬鹿でかく　
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
	//必殺技時の安置設定
	SafeZone[0]->SetPosition({ -150,-155,-162 });
	SafeZone[1]->SetPosition({ 132,-155,-162 });
	SafeZone[0]->SetScale({ 165,150, 95 });
	SafeZone[1]->SetScale({ 170,150,95 });
	//ダメ範囲
	Art_DamageArea.position = { 1 ,Player_pos.y + 1 ,-250 };
	Art_DamageArea.scale = { 14,6.4,10 };
	//Art_DamageArea.scale = { 3.15,3.09996,10 };
	Art_DamageArea.rotation = { 90,0,0 };
#pragma endregion
#pragma region 普通落石と広範囲落石のダメージ範囲設定
	//ダメージエリアの設定
	XMFLOAT3 DamageArea = { Player_pos.x ,Player_pos.y + 5 ,Player_pos.z - 25 };
	//落石
	backgrounds.rotation = { 90,0,0 };
	backgrounds.position = { Player_pos.x ,Player_pos.y + 5 ,Player_pos.z - 25 };
	backgrounds.scale = { 1,1,1 };

	L_Meteo_DamageArea[0].position = { -143.199,Player_pos.y + 1, -250.2 };
	L_Meteo_DamageArea[0].scale = { 3.15,3.09996 ,10 };
	L_Meteo_DamageArea[0].rotation = { 90,0,0 };
	//左下- 143.198959, -250.199997, 3.150000, 3.099961
	L_Meteo_DamageArea[1].position = { -140.399,Player_pos.y + 1 ,-86.80 };
	L_Meteo_DamageArea[1].scale = { 3.55,3,10 };
	L_Meteo_DamageArea[1].rotation = { 90,0,0 };
	//左上:-140.399002,-86.802490,3.550000,2.999960
	L_Meteo_DamageArea[2].position = { 149.6,Player_pos.y + 1 ,-250.8 };
	L_Meteo_DamageArea[2].scale = { 3.4,3.25,10 };
	L_Meteo_DamageArea[2].rotation = { 90,0,0 };
	//右下149.598862,-250.800003,3.400000,3.249960
	L_Meteo_DamageArea[3].position = { 140.6, Player_pos.y + 1 ,-95.802 };
	L_Meteo_DamageArea[3].scale = { 3.7,3.25,10 };
	L_Meteo_DamageArea[3].rotation = { 90,0,0 };
#pragma endregion
#pragma region 吹き飛ばし
	//Arm_rot.y = 45;
	//Arm_rot.z = 90;
	//Arm_rot_Left.y = -45;
	//Arm_rot_Left.z = -90;
	//Arm_pos = { -214, -150, 150 };
	//Arm_pos_Left = { 214, -150, 150 };
	int Knock_Flag = 0;
	//フレーム
	float frame = 0;
	float maxframe = 80.0f;
	float moves = 13;
	int motionflag = 0;
	Knock_DamageArea.position = { 18 ,Player_pos.y + 1 ,-250 };
	Knock_DamageArea.scale = { 12.8,3.7,10 };
	Knock_DamageArea.rotation = { 90,0,0 };
#pragma endregion
	//シーン関係の変数
	XMFLOAT4 Enemy_Color = { 1,1,1,1 };
	int Retry_Flag = 0;
	int Fall_Flag = 0;
	int Damage_Flag = 0;
	//フィールド下　
	int Under_Field = -215;
	float BackWall_Red = 1;
	float BackWall_Green = 1;
	float BackWall_Blue = 1;
	float BackWall_Alpha = 0;
	int ClearFlag = 0;
	//敵の初期化
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
#pragma region 更新処理
	while (true) {
		//ウィンドウメッセージ処理
		if (winapp->Processmassage()) {
			break;
		}
		input->update();
#pragma region シーン１
		if (scene == 1) {
			//左腕
			Left_Arm = { 214, -140, 150 };
			//右腕
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
			//カメラの回転角
			angle = -10.0f;
			//自機の初期化
			Player_pos.x = 0;
			Player_pos.z = -150;
			Player_pos.y = -150;
			Player_rot.y = 270;
			player->SetPosition(Player_pos);
			player->SetRotation(Player_rot);
			player->SetScale(Player_scl);

			//
			Meteo_Flag = 0;//落石
			Left_Up_Meteo_Flag = 0;
			Left_Bottom_Meteo_Flag = 0;
			Right_Up_Meteo_Flag = 0;
			Right_Bottom_Meteo_Flag = 0;

			Arm_Attack_Flag = 0;//腕突進
			Inside_Attack_FLAG = 0;
			//左腕
			Arm_Attack_Flag_Left = 0;//腕突進
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
			//広がるときの加速値
			accel = 0;
			//ｚが一定数伸びたら１に
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

			//敵の初期化
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

#pragma region シーン2
		//背景がこっち来るように
		if (scene == 2) {
			//敵の腕
			Arm_DamageArea.rotation = { 90,0,0 };
			Arm_DamageArea.position = { arm->GetPosition().x,Player_pos.y + 2,-260 };
			Arm_DamageArea.scale = { 3,6.2,10 };
			//左
			Arm_DamageArea_Left.rotation = { 90,0,0 };
			Arm_DamageArea_Left.position = { arm_left->GetPosition().x,Player_pos.y + 2,-260 };
			Arm_DamageArea_Left.scale = { 3,6.2,10 };

			backgrounds.position = DamageArea;

#pragma region 移動処理とか 移動の時のカメラ
			//プレイヤーの移動関係
			if (keyf == 1) {
				//必殺技中は動けんように
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
#pragma region 敵の動き
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
#pragma region ボスの見せ方
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
			//各オブジェクトのパラメータをセット
#pragma endregion
#pragma region ポジションやスケールのセット
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
#pragma region 落石処理
			//MeteoFlagを１で落石
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
					//発動前
					Art_Rimmit_Flag = 1;
					Attck_stanby[1] = 0;
				}
			}

#pragma region 落石処理(全部で5つ)
#pragma region 普通の落石
			//ダメージ予兆範囲のセット background=damagearea
			if (meteo->GetMeteoSet() == 0) {
				DamageArea = { Player_pos.x ,Player_pos.y + 5 ,Player_pos.z - 25 };
			}
			if (Meteo_Flag == 1) {
				meteo->Meteo(meteo, player);
				//落石がフィールド下に行ったら色々元に戻す
				if (meteo->GetMeteoSet() == 2) {
					meteo->SetMeteoSet(0);
					Damage_Flag = 0;
					Meteo_Flag = 0;
				}
			}
			//落石とプレイヤーの当たり判定
			if (Damage_Flag != 2) {
				if (collision(meteo->GetPosition().x, meteo->GetPosition().y, meteo->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 25) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//スケールや回転値をセット
			//positionは関数meteo()の中にあるからいらん
			meteo->SetScale({ 9.5,9.5,9.5 });
			meteo->SetRotation(Meteo_rot);
#pragma endregion
#pragma region 広範囲落石左上
			//ダメージ予兆範囲のセット background=damagearea
			if (Left_Up_Meteo_Flag == 1) {
				L_meteo[0]->Left_Up_Meteo(L_meteo[0]);
				//落石がフィールド下に行ったら色々元に戻す
				if (L_meteo[0]->GetLeft_Up_MeteoSet() == 2) {
					L_meteo[0]->SetLeft_Up_MeteoSet(0);
					Damage_Flag = 0;
					Left_Up_Meteo_Flag = 0;
				}
			}
			//落石とプレイヤーの当たり判定
			if (Damage_Flag != 2) {
				if (collision(L_meteo[0]->GetPosition().x, L_meteo[0]->GetPosition().y, L_meteo[0]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//スケールや回転値をセット
			//positionは関数meteo()の中にあるからいらん
			L_meteo[0]->SetScale({ 43,43,43 });
			L_meteo[0]->SetRotation(Meteo_rot);
#pragma endregion
#pragma region 広範囲落石左下
			//ダメージ予兆範囲のセット background=damagearea
			if (Left_Bottom_Meteo_Flag == 1) {
				L_meteo[1]->Left_Bottom_Meteo(L_meteo[1]);
				//落石がフィールド下に行ったら色々元に戻す
				if (L_meteo[1]->GetLeft_Bottom_MeteoSet() == 2) {
					L_meteo[1]->SetLeft_Bottom_MeteoSet(0);

					Damage_Flag = 0;
					Left_Bottom_Meteo_Flag = 0;
				}
			}
			//落石とプレイヤーの当たり判定
			if (Damage_Flag != 2) {
				if (collision(L_meteo[1]->GetPosition().x, L_meteo[1]->GetPosition().y, L_meteo[1]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//スケールや回転値をセット
			//positionは関数meteo()の中にあるからいらん
			L_meteo[1]->SetScale({ 43,43,43 });
			L_meteo[1]->SetRotation(Meteo_rot);
#pragma endregion
#pragma region 広範囲落石右上
			//ダメージ予兆範囲のセット background=damagearea
			if (Right_Up_Meteo_Flag == 1) {
				L_meteo[2]->Right_Up_Meteo(L_meteo[2]);
				//落石がフィールド下に行ったら色々元に戻す
				if (L_meteo[2]->GetRight_Up_MeteoSet() == 2) {
					L_meteo[2]->SetRight_Up_MeteoSet(0);
					Damage_Flag = 0;
					Right_Up_Meteo_Flag = 0;
				}
			}
			//落石とプレイヤーの当たり判定
			if (Damage_Flag != 2) {
				if (collision(L_meteo[2]->GetPosition().x, L_meteo[2]->GetPosition().y, L_meteo[2]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//スケールや回転値をセット
			//positionは関数meteo()の中にあるからいらん
			L_meteo[2]->SetScale({ 43,43,43 });
			L_meteo[2]->SetRotation(Meteo_rot);
#pragma endregion

#pragma region 広範囲落石右下
			//ダメージ予兆範囲のセット background=damagearea

			if (Right_Bottom_Meteo_Flag == 1) {
				L_meteo[3]->Right_Bottom_Meteo(L_meteo[3]);
				//落石がフィールド下に行ったら色々元に戻す
				if (L_meteo[3]->GetRight_Bottom_MeteoSet() == 2) {
					L_meteo[3]->SetRight_Bottom_MeteoSet(0);

					Damage_Flag = 0;
					Right_Bottom_Meteo_Flag = 0;
				}
			}

			//落石とプレイヤーの当たり判定
			if (Damage_Flag != 2) {
				if (collision(L_meteo[3]->GetPosition().x, L_meteo[3]->GetPosition().y, L_meteo[3]->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 80) == TRUE) {
					Damage_Flag = 1;
				}
			}
			//スケールや回転値をセット
			//positionは関数meteo()の中にあるからいらん
			L_meteo[3]->SetScale({ 43,43,43 });
			L_meteo[3]->SetRotation(Meteo_rot);
#pragma endregion
#pragma endregion
#pragma region 敵の腕
			//腕突進

			if (Arm_Attack_Flag == 1) {
				arm->Arm_Attack(arm);

				//ある程度後ろに行ったら元の位置へ
				//if (arm->GetPosition().z < -700) {
				if (arm->GetReturnpos() == 2) {
					Arm_Attack_Flag = 0;
					arm->Arm_Initialize(arm, Right_Arm);
					Damage_Flag = 0;
					arm->SetReturnPos(0);
				}
			}
			//腕とプレイヤーの当たり判定
			if (Damage_Flag != 2) {
				if (collision(arm->GetPosition().x, arm->GetPosition().y, arm->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 59) == TRUE) {
					Damage_Flag = 1;
				}
			}

			//敵の腕　内側の突進
			if (Inside_Attack_FLAG == 1) {
				arm->Arm_Attack_Inside(arm);
				//ある程度後ろに行ったら元の位置へ
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
#pragma region 敵の腕・左
			//腕突進

			if (Arm_Attack_Flag_Left == 1) {
				arm_left->Arm_Attack_Left(arm_left);

				//ある程度後ろに行ったら元の位置へ
				//if (arm->GetPosition().z < -700) {
				if (arm_left->GetReturnpos_Left() == 2) {
					Arm_Attack_Flag_Left = 0;
					arm_left->Arm_Initialize_Left(arm_left, Left_Arm);
					Damage_Flag = 0;
					arm_left->SetReturnPos_Left(0);
				}
			}
			//腕とプレイヤーの当たり判定
			if (Damage_Flag != 2) {
				if (collision(arm_left->GetPosition().x, arm_left->GetPosition().y, arm_left->GetPosition().z,
					player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, 59) == TRUE) {
					Damage_Flag = 1;
				}
			}

			//敵の腕　内側の突進
			if (Inside_Attack_FLAG_Left == 1) {
				arm_left->Arm_Attack_Inside_Left(arm_left);
				//ある程度後ろに行ったら元の位置へ
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
#pragma region ダメージ処理 落下 復帰
			//damageflag==2:連続でダメージを負わないように
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
			//敵がダメージ喰らう処理
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
			//諸々もとに戻す

			if (Retry_Flag == 1) {
				bf = 0;
				Meteo_Flag = 0;//落石
				Left_Up_Meteo_Flag = 0;
				Left_Bottom_Meteo_Flag = 0;
				Right_Up_Meteo_Flag = 0;
				Right_Bottom_Meteo_Flag = 0;

				Arm_Attack_Flag = 0;//腕突進
				Inside_Attack_FLAG = 0;
				//左腕
				Arm_Attack_Flag_Left = 0;//腕突進
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
				//広がるときの加速値
				accel = 0;
				//ｚが一定数伸びたら１に
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
				//右腕
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
#pragma region 弾の処理
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
#pragma region 必殺技
			//フェードイン用の□をカメラ前にセット
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
			//発動
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
					//ある程度伸びたらx,y拡大
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
					} else {//横方向を広げる
						object3ds[i].scale.y = 0.02;
						object3ds[i].scale.x = 0.02;// *accel;
					}
				}
			}
			//一定数大きくなったら初期化
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
#pragma region 吹き飛ばし攻撃

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
#pragma region シーン３
			Clear_BackWall->SetScale({ 2500,2000,1 });
			if (ClearFlag != 1) {
				Clear_BackWall->SetPosition({ target2.m128_f32[0],target2.m128_f32[1] - 30,target2.m128_f32[2] });
			}
			//体力が０切ったら
			if (Boss_HP2.scale.x <= 0) {
				Player_HP2.scale.x = 100;
				ClearFlag = 1;//カメラ位置や敵位置の固定用
				clearmotion = 1;//モーションの開始用
			}

			if (clearmotion == 1) {
				arm->SetScale({ 30,25,50 });
				arm_left->SetScale({ 30,25,50 });
				arm->Clear_Motion(arm, arm_left, Arm_rot, Arm_rot_Left);
				Boss_HP2.scale.x = 18;
			}

			//腕開放するときにフェードイン
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


			//モーションが最後まで行ったら
			if (arm->GetClear_Motion_Set() == 2) {
				clearmotion = 0;
				arm->setClear_Motion_Set(0);
			}

			if (ClearFlag == 1) {
				player->SetPosition({ 0,-150,-100 });
				//カメラにシェイク足す
				shake = rand() % 5;
				shakex = rand() % 5;
				shakey = rand() % 5;
				shakex -= shake;
				shakey -= shake;

				enemy->SetPosition({ 0,-500,150 });
				//α値を　フェードインみたいに

				target2.m128_f32[0] = enemy->GetPosition().x + shakex;
				target2.m128_f32[2] = target2.m128_f32[2] - 1;
				target2.m128_f32[1] = enemy->GetPosition().y + 700 + shakey;
				Clear_BackWall->SetPosition({ 0,target2.m128_f32[1] - 50,target2.m128_f32[2] + 29 });

				//α値が一定行ったら　シーン３に

			}
			if (alpha >= 1) {
				scene = 5;
			}
			//クリアモーション字は弾打てないように
			//フィールドスケールちっちゃく
			//
#pragma endregion 
			if (gameoverflag == 1) {
				scene = 6;
				//gameoverflag = 0;
			}

			//カメラ関係の処理
				//注視点の設定
				// 
				//meteo->SetPosition(Meteopos);
				//行列を作り直す
			rotM = XMMatrixRotationX(XMConvertToRadians(angle));
			XMVECTOR v;
			v = XMVector3TransformNormal(v0, rotM);
			eye2 = target2 + v;
			matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

#pragma region オブジェクト更新処理update()
			//カメラ用に各モデルごとview行列を代入
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
			//敵用の更新処理
			for (int i = 0; i < object_num; i++) {
				UpdateObject3d(&object3ds[i], matview, matprojection, { 1,1,1,1 });
			}
			//背景用の更新処理　今は保留
			UpdateObject3d(&backgrounds, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Arm_DamageArea, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Arm_DamageArea_Left, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Knock_DamageArea, matview, matprojection, { 1,0,0,1 });
			UpdateObject3d(&Art_DamageArea, matview, matprojection, { 0.9,0.3,0.2,0.8 });
			for (int i = 0; i < 4; i++) {
				UpdateObject3d(&L_Meteo_DamageArea[i], matview, matprojection, { 0.9,0.5,0.5,0.8 });
			}
			//スプライトの更新処理呼び出し
			for (int i = 0; i < 5; i++) {
				SpriteUpdate(sprite[i], spritecommon);
			}
			SpriteUpdate(Boss_HP, spritecommon);
			SpriteUpdate(Boss_HP2, spritecommon);
			SpriteUpdate(Player_HP, spritecommon);
			SpriteUpdate(Player_HP2, spritecommon);
#pragma endregion		 
		}//ここまでシーン２  タイトルへ
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
		//仮想メモリを取得
		Vertex* vertMap = nullptr;
		result = vertBuff->Map(0, nullptr, (void**)&vertMap);
		for (int i = 0; i < _countof(vertices); i++) {
			vertMap[i] = vertices[i];
		}
		vertBuff->Unmap(0, nullptr);


#pragma region 背景用の板ポリゴン
		//仮想メモリを取得
		BackGround* backvertMap = nullptr;
		result = backvertBuff->Map(0, nullptr, (void**)&backvertMap);
		for (int i = 0; i < _countof(background); i++) {
			backvertMap[i] = background[i];
		}
		backvertBuff->Unmap(0, nullptr);
#pragma endregion
#pragma region x,y座標のデバッグログ
		wchar_t str[256];

		//swprintf_s(str, L"rotation:%f\n", player.rotation.y);
		//左 44.000000,-86.000000,59.000000
		//右　29.000000,82.000000,-57.000000
		OutputDebugString(str);
		swprintf_s(str, L"timer:%d,%f,%f,%f\n", scene, target2.m128_f32[1], target2.m128_f32[2], Boss_HP2.scale.x);
		OutputDebugString(str);
#pragma endregion
#pragma region 描画前処理
		dxCommon->BeginDraw();//描画コマンドの上らへんに
#pragma endregion
#pragma region 描画コマンド
		//描画コマンド
#pragma region 必殺技用の円柱描画
		dxCommon->GetCmdList()->SetPipelineState(obj3Dpipelineset.pipelinestate.Get());
		dxCommon->GetCmdList()->SetGraphicsRootSignature(obj3Dpipelineset.rootsignature.Get());

		//スプライト共通コマンド
		//プリティ部形状の設定コマンド
		dxCommon->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//描画処理の呼び出し
		if (scene == 2) {
			for (int i = 0; i < object_num; i++) {
				DrawObject3d(&object3ds[i], dxCommon->GetCmdList(), basicDescHeap, vbview, ibView, gpuDescHandleSRV, _countof(indices));
			}
		}
#pragma endregion
#pragma region スプライト描画
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
			//描画コマンド
#pragma region 各ダメージエリアの描画
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
#pragma region モデル描画
		//各オブジェクトの描画
		//自機
		if (scene == 2) {
			player->PreDraw(dxCommon->GetCmdList());
			if (ClearFlag != 1) {
				player->Draw();
			}
			player->PostDraw();
			//敵
			enemy->PreDraw(dxCommon->GetCmdList());
			enemy->Draw();
			enemy->PostDraw();
			//フィールド
			field->PreDraw(dxCommon->GetCmdList());
			if (ClearFlag != 1) {
				field->Draw();
			}
			field->PostDraw();
			//落石
			meteo->PreDraw(dxCommon->GetCmdList());
			meteo->Draw();
			meteo->PostDraw();
			//敵の腕
			arm->PreDraw(dxCommon->GetCmdList());
			arm->Draw();
			arm->PostDraw();
			//敵の腕・左
			arm_left->PreDraw(dxCommon->GetCmdList());
			arm_left->Draw();
			arm_left->PostDraw();
			//広範囲落石
			for (int i = 0; i < _countof(L_meteo); i++) {
				L_meteo[i]->PreDraw(dxCommon->GetCmdList());
				if (ClearFlag != 1) {
					L_meteo[i]->Draw();
				}
				L_meteo[i]->PostDraw();
			}
			//弾
			for (int i = 0; i < _countof(bullet); i++) {
				bullet[i]->PreDraw(dxCommon->GetCmdList());
				if (Shot_Flag[i] == 1) {
					bullet[i]->Draw();
				}
				bullet[i]->PostDraw();
			}
			//安置
			for (int i = 0; i < _countof(SafeZone); i++) {
				SafeZone[i]->PreDraw(dxCommon->GetCmdList());
				if (Art_Rimmit_Timer > 1 && Art_Rimmit_Timer < 199) {
					SafeZone[i]->Draw();
				}
				SafeZone[i]->PostDraw();
			}

			//クリア時のみ
			Clear_BackWall->PreDraw(dxCommon->GetCmdList());
			if (ClearFlag == 1) {
				Clear_BackWall->Draw();
			}
			Clear_BackWall->PostDraw();
			//必殺技だけで使う
			Art_BackWall->PreDraw(dxCommon->GetCmdList());
			Art_BackWall->Draw();
			Art_BackWall->PostDraw();
			//描画コマンド　ここまで
	//	}
		}
#pragma endregion
#pragma region 描画後処理
		dxCommon->EndDraw();//ループ内の末尾らへんに
#pragma endregion
	}
	//ここまで更新
#pragma endregion

#pragma endregion
#pragma region 解放処理
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
#pragma region フラグ説明
//meteo_flag ふつうの落石 player座標
//Right_Up_Meteo_Flag 広範囲落石　右上
//Right_Bottom_Meteo_Flag　広範囲落石　右下
// Left_Up_Meteo_Flag　広範囲落石　左上
// Left_bottom_Meteo_Flag　広範囲落石　左下
//Arm_Attack_Flag 右腕の外側攻撃
//Arm_Attack_Flag_Left 左腕の外側攻撃
//Inside_Attack_Flag 右腕の内側攻撃
//Inside_Attack_Flag_Left 左腕の内側攻撃
//knock_flag 吹き飛ばし攻撃(motion_flagとセットで使う)
// Art_Flag 必殺技
//retry　復帰
