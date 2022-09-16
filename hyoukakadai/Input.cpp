#include "Input.h"
#include"WinApp.h"
//#pragma comment(lib,"dxguid.lib")
void Input::Initialize(WinApp* winapp)
{
	HRESULT result;
	//winappのインスタンスを記録
	this->winapp = winapp;
	//DirectInputのインスタンス生成
	ComPtr<IDirectInput8>dinput = nullptr;
	result = DirectInput8Create(winapp->GetInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	//キーボードデバイ氏生成
	//ComPtr<IDirectInputDevice8>devkeyboard;
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);
	//入力データ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);
	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(winapp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
}
void Input::update()
{
	HRESULT result;
	//前回のキー情報を保存
	memcpy(keypre, key, sizeof(key));
	//キーボード情報の取得開始
	result = devkeyboard->Acquire();
	//全キーの入力情報取得
	//BYTE key[256] = {};
	result = devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::Pushkey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

//トリガー
bool Input::TriggerKey(BYTE keyNumber)
{
	//前回の結果が０で今回の会話が１のとき
	if (!keypre[keyNumber] && key[keyNumber]) {
		return true;
	}
	return false;
}