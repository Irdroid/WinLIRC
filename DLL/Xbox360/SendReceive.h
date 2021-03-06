#ifndef STREAMZAPAPI_H
#define STREAMZAPAPI_H

#include <Windows.h>
#include "../Common/LIRCDefines.h"
#include "XInput.h"

class SendReceive {

public:

	SendReceive();

	BOOL init					(HANDLE exit);
	void deinit					();
	void threadProc				();
	bool waitTillDataIsReady	(int maxUSecs);
	bool dataReady				();
	int	 decodeCommand			(char *out);

private:

	//================================
	XINPUT_STATE	m_controllerState;
	HANDLE			m_threadHandle;
	HANDLE			m_threadExitEvent;
	HANDLE			m_dataReadyEvent;
	BOOL			m_done;
	int				m_value;
	int				m_repeats;
	//================================
};

#endif
