
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

	//namespace�ȗ�
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	//������
	void Initialize(WinApp* winapp);
	//�X�V
	void update();
	//�L�[�̉������`�F�b�N
	bool Pushkey(BYTE keyNumber);
	//�L�[�̃g���K�[����
	bool TriggerKey(BYTE keyNumber);
};

