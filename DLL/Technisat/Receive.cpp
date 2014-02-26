#include <windows.h>
#include <tchar.h>
#include "../Common/Win32Helpers.h"
#include "Receive.h"
#include "Globals.h"

DWORD WINAPI IRReader(void *recieveClass) {

	((Receive*)recieveClass)->threadProc();
	return 0;
}

Receive::Receive() {
	m_pFilter = NULL;
	m_pIB2C2MPEG2TunerCtrl = NULL;
	m_pIB2C2MPEG2DataCtrl = NULL;
	m_pIB2C2MPEG2AvCtrl = NULL;

	DevID = -1;
	threadHandle = NULL;
	bufferStart		= 0;
	bufferEnd		= 0;
	// Initialize COM.
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
}

Receive::~Receive() {
	deinit();
	CoUninitialize ();
}

int Receive::init(int deviceID) {
	if (DevID!=-1) return 0;

	HRESULT hr = S_OK;

	if (!m_pFilter)
	{
		// Create B2C2 Filter, which is the upstream source filter.
		hr = CoCreateInstance (CLSID_B2C2MPEG2Filter,
										NULL,
										CLSCTX_INPROC,
										IID_IBaseFilter,
										(void **)&m_pFilter);

		if (FAILED (hr)) return FALSE;

		// Get Filter Data Control interface.
		hr = m_pFilter->QueryInterface (IID_IB2C2MPEG2DataCtrl6, (VOID **)&m_pIB2C2MPEG2DataCtrl);
		if (FAILED (hr))
		{
			SAFE_RELEASE(m_pFilter);
			return FALSE;
		}

		// Get B2C2 Filter Tuner Control interface in preparation for tuning.
		hr = m_pFilter->QueryInterface (IID_IB2C2MPEG2TunerCtrl4, (VOID **)&m_pIB2C2MPEG2TunerCtrl);
		if (FAILED (hr))
		{
			SAFE_RELEASE(m_pFilter);
			return FALSE;
		}

		// Get Filter Audio/Video Control interface.
		hr = m_pFilter->QueryInterface (IID_IB2C2MPEG2AVCtrl3, (VOID **)&m_pIB2C2MPEG2AvCtrl);
		if (FAILED (hr))
		{
			SAFE_RELEASE(m_pFilter);
			return FALSE;
		}

		if ( !m_pIB2C2MPEG2TunerCtrl || !m_pIB2C2MPEG2DataCtrl || !m_pIB2C2MPEG2AvCtrl)
		{
			SAFE_RELEASE(m_pFilter);
			return FALSE;
		}
	}

	tDEVICE_INFORMATION DevInfo[MAX_DEVICES_SUPPORTED];
	long lDevInfoSize = sizeof(DevInfo);
	m_dwDeviceCount = MAX_DEVICES_SUPPORTED;
	hr = m_pIB2C2MPEG2DataCtrl->GetDeviceList(DevInfo, &lDevInfoSize, &m_dwDeviceCount);
		
	if (!m_dwDeviceCount || deviceID>=m_dwDeviceCount) return FALSE;

	hr = m_pIB2C2MPEG2DataCtrl->SelectDevice(DevInfo[deviceID].dwDeviceID);
	if (FAILED (hr)) return FALSE;

	DevID = deviceID;
	threadHandle = CreateThread(NULL,0,IRReader,(void *)this,0,NULL);

	return threadHandle!=NULL;
}

void Receive::deinit() {

	DevID = -1;
	KillThread(exitEvent,threadHandle);

	SAFE_RELEASE(m_pIB2C2MPEG2TunerCtrl);
	SAFE_RELEASE(m_pIB2C2MPEG2DataCtrl);
	SAFE_RELEASE(m_pIB2C2MPEG2AvCtrl);
	SAFE_RELEASE(m_pFilter);
}

void Receive::setData(ir_code data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool Receive::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool Receive::getData(ir_code *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

bool Receive::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
	else evt=2;

	if(!dataReady())
	{
		ResetEvent(dataReadyEvent);
		int res;
		if(maxUSecs)
			res=WaitForMultipleObjects(evt,events,FALSE,(maxUSecs+500)/1000);
		else
			res=WaitForMultipleObjects(evt,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			return false;
		}
	}

	return true;

}

#define MAX_IR_DATA 4

void Receive::threadProc() {

	exitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	while(TRUE) {		
		BYTE ucIRData[MAX_IR_DATA * 2]; 
		long lIRDataCount = MAX_IR_DATA;
		
		if FAILED(m_pIB2C2MPEG2AvCtrl->GetIRData ((long*)ucIRData, &lIRDataCount))
			break;

		if (WaitForSingleObject (exitEvent,100) == WAIT_OBJECT_0)
			break;

		if (!lIRDataCount || ucIRData[0]==0xcc || ucIRData[1]==0xcc || ucIRData[0]==0xff || ucIRData[1]==0xff)
			continue;

		WORD code = MAKEWORD(ucIRData[1],ucIRData[0]);
		code |= 0x3000;

#ifdef _DEBUG
		char code_str[128];
		sprintf_s(code_str,"Technisat RC: 0x%04X\n",(DWORD)code);
		OutputDebugString(code_str);
#endif//RCCODE_DEBUGOUT

		setData(code);	// no conversion needed (hopefully)
		SetEvent(dataReadyEvent);		
	}

	SAFE_CLOSE_HANDLE(exitEvent);
}
