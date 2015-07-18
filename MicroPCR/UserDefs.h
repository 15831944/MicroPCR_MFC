#pragma once		// once compile

#include "stdafx.h"
#include <windows.h>

// Debug mode �� ���۽�ų ���, UI �� Edit Box �� Ư�� ������ �����ֵ��� �Ѵ�.
// #define DEBUG_MODE

// ������ mmtimer �� �����ߴ� ����� ������ �ʰ� ����� �� �ְ� ���ξ���.
// ������, ��� ����� ���������Ƿ� ����ϱ� ���ؼ��� �߰��� �ؾ���.
// #define USE_MMTIMER

// Device VID, PID
#define LS4550EK_VID			0x04D8
#define LS4550EK_PID			0x0041

// Capacity max device number
#define DEVICE_MAX				0x08

// Message Map
#define WM_SET_SERIAL			WM_USER + 0x01

#define TIMER_MAIN				0x01

// Timer Duration
#ifdef USE_MMTIMER
#define TIMER_DURATION			10
#else
#define TIMER_DURATION			100
#endif

#define CONSTANTS_PATH			L"PCRConstants"

// Recent Protocol File Path
#define RECENT_PATH				L"Recent_Protocol.txt"

#define PID_CONSTANTS_MAX		5

// MAX Protocol Name
#define MAX_PROTOCOL_LENGTH		50


// ���� �� ���� ����� �ϰ� ������, ���κ��� ����� ���� �޾ƿ��� �Ǹ�
// ���� ����
#define	A_VAL					8.314242955712037e-004
#define B_VAL					2.620794578770541e-004
#define C_VAL					1.368534674434736e-007

#define Rref					1800.0
#define K						273.15


#define SET_BUTTON_IMAGE(button, img)	((CButton*)GetDlgItem(button))->SetBitmap((HBITMAP)img)

#define FAN_STOP_TEMPDIF		1
#define INTGRALMAX				2600.0

class PID{
public:
	float startTemp;
	float targetTemp;
	float kp;
	float kd;
	float ki;

	PID()
		: startTemp(0.)
		, targetTemp(0.)
		, kp(0.)
		, kd(0.)
		, ki(0.)
	{
	}

	PID(float startTemp, float targetTemp, float kp,
		float kd, float ki)
		: startTemp(startTemp)
		, targetTemp(targetTemp)
		, kp(kp)
		, kd(kd)
		, ki(ki)			
	{
	}
};

typedef struct _ACTION
{
	CString Label;			// Action Label or GOTO
	double Temp;			// Target Temperature
	double Time;			// Duration that the temperature is stable over
} Action;

typedef enum _CMD
{
	CMD_READY = 0x00,
	CMD_PID_WRITE,
	CMD_PID_END,
	CMD_PCR_RUN,
	CMD_PCR_STOP,
	CMD_REQUEST_LINE,
	CMD_BOOTLOADER = 0x55
} CMD;

typedef enum _STATE
{
	STATE_READY = 0x00,
	STATE_RUNNING,
	STATE_PROTOCOL_READING,
	STATE_PID_READING
} STATE;

typedef struct _RxBuffer
{
	BYTE reserved;
	BYTE state;
	BYTE chamber_h;
	BYTE chamber_l;

	// calculated temperature
	BYTE chamber_temp_1;
	BYTE chamber_temp_2;
	BYTE chamber_temp_3;
	BYTE chamber_temp_4;

	BYTE photodiode_h;
	BYTE photodiode_l;
	BYTE currentError;
	
	// For request command
	BYTE request_data;

	BYTE reserved_for_64byte[53];
} RxBuffer;

typedef struct _TxBuffer
{
	BYTE reserved;

	BYTE cmd;

	// �ø� �µ� ��
	BYTE currentTargetTemp;
	
	// for pid params
	BYTE startTemp;
	BYTE targetTemp;

	BYTE pid_p1;
	BYTE pid_p2;
	BYTE pid_p3;
	BYTE pid_p4;
	
	BYTE pid_i1;
	BYTE pid_i2;
	BYTE pid_i3;
	BYTE pid_i4;

	BYTE pid_d1;
	BYTE pid_d2;
	BYTE pid_d3;
	BYTE pid_d4;

	BYTE reserved_for_64byte[48];
} TxBuffer;

typedef enum _ERROR
{
	ERROR_NO = 0x00,
	ERROR_ASSERT,
} Error;

#define BACKGROUND_COLOR		RGB(2,130,200)