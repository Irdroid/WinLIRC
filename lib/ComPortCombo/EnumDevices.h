// Enumerate serial ports using the SetupAPI.
//
// Windows 95 and NT4 notes:
// The app may be linked to SetupAPI.lib even if Win 95 or NT4 must be supported when:
// - IE or specific service packs has been installed on the user's machine (SetupAPI.dll is present)
// - The app is build with delay loading for SetupAPI.dll and the OS version is checked
//   before calling any setup functions. No Setup functions are called when Init() didn't find the DLL. 
//
// Tested with Windows 2000 SP4, XP SP2, 7, 98SE (Virtual PC 2007)
//
// SetupAPI does not work with NT4!
// The SetupAPI DLL may be present, but the functions used here can't be used to enumerate the
//  serial ports. Windows NT4 contains a registry key with the ports GUID, but this key only
//  contains the value names "Class" and "Install32" and no subkeys with device information.
// Therefore, the provided function EnumPortsNT4() must be used with NT4.
//
// Original author: Jochen Arndt <jochen.arndt@t-online.de>

#pragma once

#if !defined(WINVER) || WINVER <= 0x0501
#error Windows XP and older are not supported.
#endif

#if !_UNICODE || !UNICODE
    #error Only unicode is supported.
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT WINVER
#endif


#include <cassert>
#include <cstdlib>
#include <functional>
#include <sstream>
#include <string>

#include <WinIoCtl.h>   // Pre-defined GUIDs
#include <setupapi.h>   // SetupAPI version and types

#pragma comment(lib, "setupapi")       // link setupapi of Win 95 and NT4 must be not supported

#include <tchar.h>

#define DATA_PORT_MASK		0x0000FFFF
#define DATA_VIRTUAL_MASK	0x00010000

#define INFO_PORT_NAME		0x00000001
#define INFO_NAME			0x00000002
#define INFO_SERVICE		0x00000004
#define INFO_MFG			0x00000008
#define INFO_PNP_NAME		0x00000010
#define INFO_DESCR			0x00000020
#define INFO_CLASS_NAME		0x00000040
#define INFO_CLASS_GUID		0x00000080
#define INFO_ALL			0x0FFFFFFF
#define INFO_NON_PRESENT	0x80000000

#define INFO_PORTS		(INFO_PORT_NAME | INFO_NAME | INFO_SERVICE)

class CEnumDevInfo
{
public:
    CEnumDevInfo()
        : m_nStatus(0)
        , m_nPortNum(-1)
    { }

    unsigned m_nStatus;
    int m_nPortNum;									// COM ports only
    std::wstring m_strPort;								// COM and LPT ports only
    std::wstring m_strName;								// user friendly name
    std::wstring m_strService;
    std::wstring m_strManufacturer;
    std::wstring m_strPnPName;
    std::wstring m_strDescription;
    std::wstring m_strClassName;
    std::wstring m_strClassGUID;
};

class CEnumDevices
{
public:
    // Max. number of COM ports supported by Windows versions:
    // Windows 9x: 256
    // Windows NT: 4096 (should be limited to 2000 with NT4 due to memory usage)
    static unsigned const MaxComPorts = 4096;

    typedef std::function<void(CEnumDevInfo const&)> EnumCallBack;

    static bool EnumSerialPorts(EnumCallBack const& callback, bool bPresent = true);
    static bool EnumDevices(unsigned nInfo, EnumCallBack const& pCallBack, GUID const& guid);

    static bool GetFileName(int nPortNum, std::wstring& str);
    static int  GetPortFromName(LPCTSTR lpszPort);
};


// Size for property buffer in bytes.
// NOTE: Max. string length is half size with Unicode! 
static size_t const PROP_BUF_SIZE = 2048;

static HDEVINFO GetClassDevs(GUID const* lpGUID, bool bPresent = true);
static bool GetPropertyString(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData, DWORD dwProperty, std::wstring& str);
static bool GetCustomPropertyString(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData, LPCTSTR lpszEntry, std::wstring& str);
static void Error(LPCTSTR lpszFunction, bool bMsg = false);

// Get COM port file name from port number.
/*static*/ inline bool CEnumDevices::GetFileName(int nPortNum, std::wstring& str)
{
    if (nPortNum <= 0)
    {
        str = L"";
    }
    else
    {
        std::wostringstream o;
        if (nPortNum >= 10)
            o << L"\\\\.\\";
        o << L"COM" << nPortNum;
        str = o.str();
    }
    return nPortNum >= 0;
}

// Get port number form COM port file name.
// Return value:
// -1 : Invalid name
//  0 : Empty name treated as "no port".
// >0 : Port number
/*static*/ inline int CEnumDevices::GetPortFromName(LPCTSTR lpszPort)
{
    assert(lpszPort != NULL);

    int nPortNum = -1;
    if (lpszPort != NULL)
    {
        if (*lpszPort == L'\0')
            nPortNum = 0;
        else
        {
            LPCTSTR s = lpszPort;
            if (_tcsncmp(s, _T("\\\\.\\"), 4) == 0)
                s += 4;
            if (_tcsnicmp(s, _T("COM"), 3) == 0)
                nPortNum = _tstoi(s + 3);
            if (nPortNum <= 0)
            {
                //ATLTRACE("Invalid COM port file name %s\n", lpszPort);
                nPortNum = -1;
            }
        }
    }
    return nPortNum;
}

inline void GetComDeviceStr(std::wstring& str, int nPort)
{
    assert(nPort > 0 && nPort <= CEnumDevices::MaxComPorts);
    std::wostringstream o;
    if (nPort >= 10)
        o << L"\\\\.\\";
    o << L"COM" << nPort;
    str = o.str();
}

// Generate error message for SetupAPI errors.
// NOTE: Must use special conversion macro HRESULT_FROM_SETUPAPI.
inline void Error(LPCTSTR lpszFunction, bool bMsg /*= false*/)
{
    //ATLASSERT(lpszFunction != NULL);

    DWORD dwErr = ::GetLastError();
    if (dwErr)
    {
        LPVOID lpMsgBuf;
        if (::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            HRESULT_FROM_SETUPAPI(dwErr),			// special conversion for SetupAPI errors
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default user language
            reinterpret_cast<LPTSTR>(&lpMsgBuf),
            0,
            NULL))
        {
            //ATLTRACE("SetupAPI error %d (%#X) in %s: %s\n",
            //    dwErr, dwErr, lpszFunction,
            //    reinterpret_cast<LPCTSTR>(lpMsgBuf));
            if (bMsg && dwErr != ERROR_CALL_NOT_IMPLEMENTED)
            {
                std::wostringstream strMsg;
                strMsg << L"SetupAPI error in "
                    << lpszFunction << L"L\n"
                    << reinterpret_cast<LPCTSTR>(lpMsgBuf);

                MessageBox(NULL, strMsg.str().c_str(), NULL, 0);
            }
            ::LocalFree(lpMsgBuf);
        }
    }
}

// With Windows 2K/XP and later we can use GUID_DEVINTERFACE_COMPORT with DIGCF_DEVICEINTERFACE.
// This does not work with Windows 9x where we must use
//  GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR which returns also LPT ports.
// We can always use the ANSI version here, because the PnP enum/name parameter isn't used.
inline HDEVINFO GetClassDevs(GUID const* lpGUID, bool bPresent /*= TRUE*/)
{
    DWORD dwFlags = bPresent ? DIGCF_PRESENT : 0;	// only present devices or all that has been seen
    if (lpGUID == &GUID_DEVINTERFACE_COMPORT)		// Fixed GUID for COM ports (NT only) 
        dwFlags |= DIGCF_DEVICEINTERFACE;
    else if (lpGUID == NULL)						// No specific GUID (all classes)
        dwFlags |= DIGCF_ALLCLASSES;
    HDEVINFO hDevInfo = ::SetupDiGetClassDevsA(lpGUID, NULL, NULL, dwFlags);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        Error(_T("SetupDiGetClassDevs"));
    return hDevInfo;
}

inline bool GetPropertyString(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData, DWORD dwProperty, std::wstring& str)
{
    assert(hDevInfo != INVALID_HANDLE_VALUE);
    assert(pDevInfoData != NULL);
    assert(dwProperty < SPDRP_MAXIMUM_PROPERTY);

    bool bResult = false;
    DWORD dwDataType = 0;							// registry data type (e.g. REG_SZ)
    DWORD dwReqSize = 0;							// retrieve required buffer size
    BYTE lpszBuf[PROP_BUF_SIZE];

    bResult = ::SetupDiGetDeviceRegistryProperty(
        hDevInfo,								// <- device info set handler
        pDevInfoData,							// <- enumerated device info data
        dwProperty,								// <- Description and name as displayed in the hardware config
        &dwDataType,							// -> registry data type (e.g. REG_SZ), optional
        lpszBuf,								// -> output buffer filled with registry data
        PROP_BUF_SIZE,							// <- output buffer size
        &dwReqSize);							// -> required size, optional
    if (bResult && dwDataType == REG_SZ)		// just to be sure ...
        str = reinterpret_cast<LPCTSTR>(lpszBuf);

    if (!bResult)
    {
        std::wostringstream s;
        s << L"SetupDiGetDeviceRegistryProperty(" << dwProperty << L")";
        Error(s.str().c_str());
    }
    else
    {
        assert(dwDataType == REG_SZ);            // not a string property
        //ATLTRACE(" Property %#X = %s\n", dwProperty, str);
    }
    return bResult;
}

// Get value from 'Device Parameters' key.
// Requires Windows XP or later (API version >= 5.01).
inline bool GetCustomPropertyString(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData, LPCTSTR lpszEntry, std::wstring& str)
{
    assert(hDevInfo != INVALID_HANDLE_VALUE);
    assert(pDevInfoData != NULL);
    assert(lpszEntry != NULL && *lpszEntry != _T('\0'));

    DWORD dwDataType = 0;                           // registry data type (e.g. REG_SZ)
    DWORD dwReqSize = 0;                            // retrieve required buffer size
    BYTE lpszBuf[PROP_BUF_SIZE];

    bool bResult = false;
    {
        bResult = SetupDiGetCustomDeviceProperty(        // get value from 'Device Parameters' key
            hDevInfo, pDevInfoData,
            lpszEntry,                              // get value from entry
            0, &dwDataType,
            lpszBuf, PROP_BUF_SIZE, &dwReqSize);
    }
    if (!bResult)
    {
        std::wostringstream o;
        o << L"SetupDiGetCustomDeviceProperty(" << lpszEntry << L")";
        Error(o.str().c_str());
    }
    else
    {
        assert(dwDataType == REG_SZ);             // not a string property
        if (dwDataType == REG_SZ)
        {
            str = reinterpret_cast<LPCTSTR>(lpszBuf);
            //ATLTRACE(" Custom property %s = %s\n", lpszEntry, str);
        }
    }
    return bResult;
}

// Enumerate serial ports and pass data to ListBox/ComboBox using call back function.
inline bool CEnumDevices::EnumSerialPorts(EnumCallBack const& pCallBack, bool bPresent /*= TRUE*/)
{
    return EnumDevices(
        bPresent ? INFO_PORTS : INFO_PORTS | INFO_NON_PRESENT,
        pCallBack,
        GUID_DEVINTERFACE_COMPORT);
}

// Enumerate devices and pass data using call back function.
inline bool CEnumDevices::EnumDevices(unsigned nInfo, EnumCallBack const& pCallBack, GUID const& guid)
{
    DWORD nNumDev = 0;								// count interfaces
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;		// device infoset handle

    SP_DEVINFO_DATA	DevInfoData = { 0 };			// device info data for enumerated device
    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    //ATLTRACE("EnumDevices(%#X)\n", nInfo);
    hDevInfo = GetClassDevs(&guid, !(nInfo & INFO_NON_PRESENT));
    bool bResult(hDevInfo != INVALID_HANDLE_VALUE);

    try
    {
        while (bResult)                             // enumerate devices
        {
            bResult = ::SetupDiEnumDeviceInfo(      // get device info
                hDevInfo, nNumDev++, &DevInfoData);
            if (!bResult)
            {
                if (::GetLastError() == ERROR_NO_MORE_ITEMS)
                    bResult = true;                 // no more devices
                else
                    Error(_T("SetupDiEnumDeviceInfo()"));
                break;
            }

            CEnumDevInfo Info;

            if ((nInfo & INFO_NAME) &&
                GetPropertyString(hDevInfo, &DevInfoData, SPDRP_FRIENDLYNAME, Info.m_strName))
                Info.m_nStatus |= INFO_NAME;
            if ((nInfo & INFO_SERVICE) &&
                GetPropertyString(hDevInfo, &DevInfoData, SPDRP_SERVICE, Info.m_strService))
                Info.m_nStatus |= INFO_SERVICE;
            if ((nInfo & INFO_MFG) &&
                GetPropertyString(hDevInfo, &DevInfoData, SPDRP_MFG, Info.m_strManufacturer))
                Info.m_nStatus |= INFO_MFG;
            if ((nInfo & INFO_DESCR) &&
                GetPropertyString(hDevInfo, &DevInfoData, SPDRP_DEVICEDESC, Info.m_strDescription))
                Info.m_nStatus |= INFO_DESCR;
            if ((nInfo & INFO_CLASS_NAME) &&
                GetPropertyString(hDevInfo, &DevInfoData, SPDRP_CLASS, Info.m_strClassName))
                Info.m_nStatus |= INFO_CLASS_NAME;
            if ((nInfo & INFO_CLASS_GUID) &&
                GetPropertyString(hDevInfo, &DevInfoData, SPDRP_CLASSGUID, Info.m_strClassName))
                Info.m_nStatus |= INFO_CLASS_GUID;
            if ((nInfo & INFO_PNP_NAME) &&
                GetPropertyString(hDevInfo, &DevInfoData, SPDRP_ENUMERATOR_NAME, Info.m_strPnPName))
                Info.m_nStatus |= INFO_PNP_NAME;
            if ((nInfo & INFO_PORT_NAME) &&
                GetCustomPropertyString(hDevInfo, &DevInfoData, _T("PortName"), Info.m_strPort))
                Info.m_nStatus |= INFO_PORT_NAME;
            // Serial port specific handling:
            //  Get the port name 'COMx' and the user friendly name.
            //  The used function for the port name requires a API version >= 5.01 (Windows XP).
            //  With older versions, we will extract the port name from the user friendly name.
            //  Extraction assumes that the port name is provided in parentheses as '(COMx)'.
            //  Looking for 'COM' only does not always work (e.g. with German Win 98 where
            //  the user friendly name is 'COM Anschluﬂ (COM1)')!
            if ((Info.m_nStatus & INFO_NAME) &&
                !(Info.m_nStatus & INFO_PORT_NAME))
            {
                auto nStartPos = Info.m_strName.find(L"(COM");
                if (nStartPos != std::wstring::npos)
                {
                    auto nEndPos = Info.m_strName.find(L')', ++nStartPos);
                    if (nEndPos != std::wstring::npos)
                    {
                        Info.m_strPort = Info.m_strName.substr(nStartPos, nEndPos - nStartPos);
                        Info.m_nStatus |= INFO_PORT_NAME;
                    }
                }
            }
            if ((Info.m_nStatus & INFO_PORT_NAME) &&	// found a serial port
                Info.m_strPort.substr(0, 3) == _T("COM"))
            {
                Info.m_nPortNum = _tstoi(Info.m_strPort.c_str() + 3);
                // Getting the service type fails with Windows 9x.
                // If the service type is not "Serial", it is probably a virtual
                //  COM port (e.g. USB to serial converter or internal modem card).
                // This is indicated by adding a special bit to the port number.
                if (Info.m_nStatus & INFO_SERVICE)
                {
                    if (Info.m_nPortNum &&
                        _tcsicmp(Info.m_strService.c_str(), _T("Serial")))
                        Info.m_nPortNum |= DATA_VIRTUAL_MASK;
                }
                else
                    Info.m_strService = _T("Serial");
            }
            pCallBack(Info);
        } // while (bResult)
    }
    catch (...)
    {
        bResult = false;
    }
    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        ::SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    //ATLTRACE(" %d devices found\n", --nNumDev);
    return bResult;
}
