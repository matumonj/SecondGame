
#pragma once
#include<Windows.h>
#include<wrl.h>
#include<dinput.h>
#include"WinApp.h"
//#define DIRECTINPUT_VERSION 0x0800
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
using namespace Microsoft::WRL;


class Input
{
private:
	ComPtr<IDirectInputDevice8> devkeyboard;
	BYTE key[256] = {};
	BYTE keypre[256] = {};
	WinApp* winapp = nullptr;
public:

	//namespace省略
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	//初期化
	void Initialize(WinApp* winapp);
	//更新
	void update();
	//キーの押下をチェック
	bool Pushkey(BYTE keyNumber);
	//キーのトリガー判定
	bool TriggerKey(BYTE keyNumber);
};

