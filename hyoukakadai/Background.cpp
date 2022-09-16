#include"Background.h"
#pragma region 3Dオブジェクトのパイプライン関数
PipelineSet createBackpipeline(ID3D12Device* dev) {
	int result;
	//シェーダオブジェクト
	ComPtr<ID3DBlob>vsBlob;
	ComPtr<ID3DBlob>psBlob;
	ComPtr<ID3DBlob>errorBlob;

	//頂点シェーダファイルの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/Shader/BackVS.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob, &errorBlob
	);

	//ピクセルシェーダファイルの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/Shader/BackPS.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob, &errorBlob
	);

	//シェーダのエラー内容
	if (FAILED(result)) {
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";

		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	//頂点シェーダに渡すための頂点データを整える
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	{
		"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0,
	},
	{
			"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
},
	{
		"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0,
	},
	/*{
		"EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,
	D3D12_APPEND_ALIGNED_ELEMENT,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0,
},*/
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};

	//頂点シェーダ、ピクセルシェーダをパイプライン
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//カリングしない
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//デプスステンシルステートの設定
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	//図形の形状を三角形に
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//その他の設定
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	//スタティックサンプラ
	auto samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	//レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = gpipeline.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	blenddesc.BlendEnable = true;
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	//blenddesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;//反転
	//blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;//減算
	//blenddesc.SrcBlend = D3D12_BLEND_ZERO;//加算

	//blenddesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;//反転
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;//半透明
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;//半透明
	//blenddesc.DestBlend = D3D12_BLEND_ZERO;
	//blenddesc.DestBlend = D3D12_BLEND_ONE;
	//result = texbuff->WriteToSubresource(0, nullptr, img->pixels, (UINT)img->rowPitch, (UINT)img->slicePitch);


	//でスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeCBV, descRangeSRV;
	descRangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	//ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[2];

	rootparams[0].InitAsConstantBufferView(0);
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	//スタティックサンプラ
	//auto samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	//ルートシグネチャの生成
	ComPtr<ID3D12RootSignature>rootsignature;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


	//パイプラインとルートシグネチャのセット
	PipelineSet pipelineset;

	ComPtr<ID3DBlob>rootSigBlob;
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);
	//ルートシグネチャの生成
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&pipelineset.rootsignature));

	//パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = pipelineset.rootsignature.Get();
	ComPtr<ID3D12PipelineState>pipelinestate;
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineset.pipelinestate));

	return pipelineset;
}
#pragma endregion
