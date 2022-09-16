#include "Input.h"
#include"WinApp.h"
//#pragma comment(lib,"dxguid.lib")
void Input::Initialize(WinApp* winapp)
{
	HRESULT result;
	//winapp�̃C���X�^���X���L�^
	this->winapp = winapp;
	//DirectInput�̃C���X�^���X����
	ComPtr<IDirectInput8>dinput = nullptr;
	result = DirectInput8Create(winapp->GetInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	//�L�[�{�[�h�f�o�C������
	//ComPtr<IDirectInputDevice8>devkeyboard;
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);
	//���̓f�[�^�`���̃Z�b�g
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);
	//�r�����䃌�x���̃Z�b�g
	result = devkeyboard->SetCooperativeLevel(winapp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
}
void Input::update()
{
	HRESULT result;
	//�O��̃L�[����ۑ�
	memcpy(keypre, key, sizeof(key));
	//�L�[�{�[�h���̎擾�J�n
	result = devkeyboard->Acquire();
	//�S�L�[�̓��͏��擾
	//BYTE key[256] = {};
	result = devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::Pushkey(BYTE keyNumber)
{
	//�w��L�[�������Ă����true��Ԃ�
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

//�g���K�[
bool Input::TriggerKey(BYTE keyNumber)
{
	//�O��̌��ʂ��O�ō���̉�b���P�̂Ƃ�
	if (!keypre[keyNumber] && key[keyNumber]) {
		return true;
	}
	return false;
}