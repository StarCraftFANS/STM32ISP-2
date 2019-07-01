


////////////////////////////////////////////////////////////////////////////////////////////////////
// ˵��: ClassXP.c �ļ�
// ����: 2003-3-10
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ����Ԥ����
#if _WIN32_WINNT < 0x0400
#define _WIN32_WINNT 0x0400
#endif
#include <Windows.h>
#include "ClassXP.h"
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)
#pragma comment(lib, "Msimg32.lib")

// ��������
#ifdef CXP_DLLMODE
#pragma comment(linker, "/EXPORT:ClassXP=_ClassXP@8")
#endif // CXP_DLLMODE

// ǿ��ʹ�� C ���Է�ʽ����
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// �궨��
#define Bk_Color  0x00FFFFFF 


// ��������
#define CXPT_UNKNOWN		-1	// ���ܴ��������
#define CXPT_PUSHBUTTON		0	// ��ť
#define CXPT_CHECKBOX		1	// ��ѡ��
#define CXPT_RADIOBOX		2	// ��ѡ��
#define CXPT_EDITBOX		3	// �༭��
#define CXPT_COMBOBOX		4	// ��Ͽ�


// ����״̬
#define CXPS_DISABLED		0x00000001L		// ����״̬
#define CXPS_PRESSED		0x00000002L		// ����״̬
#define CXPS_HOTLIGHT		0x00000004L		// ����״̬ (����ڸô�����)
#define CXPS_FOCUS			0x00000008L		// ���м������뽹��
#define CXPS_DEFAULT		0x00000010L		// Ĭ��״̬ (���ڰ�ť)
#define CXPS_CHECKED		0x00000020L		// ѡ��״̬ (���ڸ�ѡ��)
#define CXPS_INDETERMINATE	0x00000040L		// δȷ��״̬ (���ڸ�ѡ��)
#define CXPS_READONLY		0x00000080L		// ֻ��״̬ (���ڱ༭��)


// ���ô���״̬
#define CXPM_SETSTATE(Data, Mask, IsSet)	((IsSet) ? (Data |= Mask) : (Data &= ~Mask))
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// CLASSXP �ṹ�����еĴ��붼��Χ������ṹ����д��
typedef struct tagCLASSXP
{
	HWND hWnd;					// ���ھ��
	DWORD dwType;				// ���ڵ�����
	DWORD dwState;				// ���ڵ�״̬
	WNDPROC wpPrev;				// ���໯֮ǰ�Ĵ��ڻص�������ַ
	struct tagCLASSXP * pNext;	// ָ����һ�� CLASSXP �ṹ��������õ�������ṹ
}
CLASSXP, * PCLASSXP;
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMDCXP �ṹ��Ϊ�˷���ʹ���ڴ�����豸���������
typedef struct tagMEMDCXP
{
	HWND hWnd;					// ���ھ�����������
	HDC hDC;					// �����豸�������������
	HDC hMemDC;					// �����ڴ�����豸�������������
	BOOL bTransfer;				// �Ƿ�Ҫ���� hDC �� hMemDC �䴫�����ݣ��������
	HBITMAP hBitmap;			// λͼ�����������������
}
MEMDCXP, * LPMEMDCXP;
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��������
PCLASSXP WINAPI CreateClassXP(HWND hWnd);
PCLASSXP WINAPI DeleteClassXP(HWND hWnd);
PCLASSXP WINAPI GetClassXP(HWND hWnd);

DWORD WINAPI GetWindowTypeXP(HWND hWnd);
HDC WINAPI GetMemDCXP(LPMEMDCXP pMdcxp);
VOID WINAPI ReleaseMemDCXP(LPMEMDCXP pMdcxp);
VOID WINAPI GradientRectXP(HDC hDC, LPRECT pRect,COLORREF crColor[4]);
VOID WINAPI DrawDropGripXP(HDC hDC, LPRECT pRect);

BOOL CALLBACK EnumWndProcXP(HWND hWnd, LPARAM lParam);
LRESULT CALLBACK HookProcXP(int iCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProcXP(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

VOID WINAPI DrawPushButtonXP(PCLASSXP pCxp);
VOID WINAPI DrawCheckBoxXP(PCLASSXP pCxp);
VOID WINAPI DrawRadioBoxXP(PCLASSXP pCxp);
VOID WINAPI DrawEditBoxXP(PCLASSXP pCxp);
VOID WINAPI DrawComboBoxXP(PCLASSXP pCxp);
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ȫ�ֱ���
HHOOK g_hPrevHookXP = NULL;		// ������Ϣ HOOK ���
PCLASSXP g_pClassXP = NULL;		// ���ڵ� CLASSXP �ṹָ��

#ifdef CXP_DLLMODE
HINSTANCE g_hModuleXP = NULL;	// ��̬���ӿ�ģ����
#endif // CXP_DLLMODE
////////////////////////////////////////////////////////////////////////////////////////////////////



#ifdef CXP_DLLMODE
////////////////////////////////////////////////////////////////////////////////////////////////////
// ��̬���ӿ�������
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID pvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hModuleXP = hModule;
		DisableThreadLibraryCalls(hModule);
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // CXP_DLLMODE



////////////////////////////////////////////////////////////////////////////////////////////////////
// ���û�ȡ�����ڵ� ClassXP ���
BOOL WINAPI ClassXP(HWND hWnd, BOOL bEnable)
{
	BOOL bReturn;

	bReturn = FALSE;

	// �����Ӱ�쵱ǰ�����е����д���
	if (hWnd == NULL)
	{
		// �����ȡ����ǰ�����е����д���
		if ((bEnable == FALSE) && (g_hPrevHookXP != NULL))
		{
			// ö�ٵ�ǰ�̵߳Ĵ��ڲ�ȡ�� ClassXP ���
			EnumThreadWindows(GetCurrentThreadId(), EnumWndProcXP, FALSE);

			// ȡ��������Ϣ HOOK
			bReturn = UnhookWindowsHookEx(g_hPrevHookXP);
			g_hPrevHookXP = NULL;
		}
		// ��������õ�ǰ�����е����д���
		else if ((bEnable == TRUE) && (g_hPrevHookXP == NULL))
		{
			// ö�ٵ�ǰ�߳����Ѵ��ڵĴ��ڲ�����Ϊ ClassXP ���
			EnumThreadWindows(GetCurrentThreadId(), EnumWndProcXP, TRUE);

			// ��װ������Ϣ HOOK
			g_hPrevHookXP = SetWindowsHookEx(WH_CALLWNDPROC, HookProcXP, 0, GetCurrentThreadId());
			bReturn = (BOOL) g_hPrevHookXP;
		}
	}
	else
	{
		// �����ȡ��ָ�����ڵ� ClassXP ���
		if (bEnable == FALSE)
			bReturn = (BOOL) DeleteClassXP(hWnd);
		// ���������ָ�����ڵ� ClassXP ���
		else
			bReturn = (BOOL) CreateClassXP(hWnd);			
	}
	return bReturn;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��������ʼ�� CLASSXP ���ݽṹ�����໯����
// ������� NULL����ʾû�д��������򷵻��´����ڵ��ָ�룬ͬʱ g_pClassXP ָ���´����Ľڵ�
PCLASSXP WINAPI CreateClassXP(HWND hWnd)
{
	LONG lStyle;
	DWORD dwType;
	PCLASSXP pCxp;

	// �Ƿ��Ѿ��� ClassXP ���
	if (GetClassXP(hWnd) == NULL)
	{
		// ��ȡ�������ͣ�������ж��Ƿ�������Ϊ ClassXP ���
		dwType = GetWindowTypeXP(hWnd);
		if ((dwType >= CXPT_PUSHBUTTON) && (dwType <= CXPT_COMBOBOX))
		{
			lStyle = GetWindowLong(hWnd, GWL_STYLE);

			// ����洢�ռ䣬����һ���ڵ�
			pCxp = (PCLASSXP) HeapAlloc(GetProcessHeap(), 0, sizeof(CLASSXP));
			pCxp->pNext = g_pClassXP;
			g_pClassXP = pCxp;

			// ���໯���ڲ���ʼ�� CLASSXP ���ݽṹ
			pCxp->hWnd = hWnd;
			pCxp->dwType = dwType;
			pCxp->dwState = (lStyle & WS_DISABLED) ? CXPS_DISABLED : 0;
			if (hWnd == GetFocus())
				pCxp->dwState |= CXPS_FOCUS;
			pCxp->wpPrev = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (LONG) WindowProcXP);

			// ���������ͷֱ� CLASSXP ���ݽṹ
			switch (dwType)
			{
			case CXPT_PUSHBUTTON:
			case CXPT_CHECKBOX:
			case CXPT_RADIOBOX:
				if ((lStyle & SS_TYPEMASK) == BS_DEFPUSHBUTTON)
					pCxp->dwState |= CXPS_DEFAULT;

				lStyle = (LONG) SendMessage(hWnd, BM_GETCHECK, 0, 0);
				if (lStyle == BST_CHECKED)
					pCxp->dwState |= CXPS_CHECKED;
				else if (lStyle == BST_INDETERMINATE)
					pCxp->dwState |= CXPS_INDETERMINATE;
				break;

			case CXPT_EDITBOX:
				if (lStyle & ES_READONLY)
					pCxp->dwState |= CXPS_READONLY;
				break;
			}

			// �ػ�����
			RedrawWindow(hWnd, NULL, NULL,
				RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW);
			return pCxp;
		}
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ȡ���������໯�����ٴ��ڵ� CLASSXP ���ݽṹ
// �������ֵ��Ϊ NULL ��ʾ�ɹ�ɾ��������ֵΪָ����һ���ڵ�ָ�룻
// ������� NULL �� g_pClassXP Ϊ NULL����ȫ���ڵ㱻ɾ����
// �����ʾû���ҵ��ýڵ㡣
// ��л: ��л dREAMtHEATER �Ľ��˺���!
PCLASSXP WINAPI DeleteClassXP(HWND hWnd)
{
	PCLASSXP pDel;
	PCLASSXP pCxp;

	// ��ȡ��ɾ���Ľڵ�ָ��
	pDel = GetClassXP(hWnd);
	if (pDel != NULL)
	{
		// �����ɾ���Ľڵ���� g_pClassXP �ڵ�
		if (pDel == g_pClassXP)
			pCxp = g_pClassXP = pDel->pNext;
		else
		{
			// ѭ�����Ҵ�ɾ���ڵ����һ���ڵ�
			for (pCxp = g_pClassXP; pCxp != NULL; pCxp = pCxp->pNext)
			{
				// ����ҵ�
				if (pCxp->pNext == pDel)
				{
					// ʹ����������ɾ���Ľڵ�
					pCxp->pNext = pDel->pNext;
					break;
				}
			}
		}

		// ȡ���������໯���ػ�����
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG) pDel->wpPrev);

		// ɾ�����ڴ�
		HeapFree(GetProcessHeap(), 0, pDel);

		// �ػ�����
		RedrawWindow(hWnd, NULL, NULL,
			RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW);
		return pCxp;
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ȡ���ڵ� CLASSXP ���ݽṹ
// ������� NULL����ʾû���ҵ������򷵻ؽڵ��ָ��
PCLASSXP WINAPI GetClassXP(HWND hWnd)
{
	PCLASSXP pCxp;

	for (pCxp = g_pClassXP; pCxp != NULL; pCxp = pCxp->pNext)
	{
		if (pCxp->hWnd == hWnd)
			return pCxp;
	}
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ȡ��������
DWORD WINAPI GetWindowTypeXP(HWND hWnd)
{
	DWORD lReturn;
	char szTemp[MAX_PATH];
	static char s_szClass[][32] = 
	{
		"Button",					// ��ť��
		"Edit",						// �༭����
		"ComboBox",					// ��Ͽ���

#ifdef CXP_DLLMODE
		"TButton",					// VCL TButton ��
		"ThunderCommandButton",		// Visual Basic Command Button ��
		"ThunderRT6CommandButton",	// Visual Basic Command Button ��
		"TCheckBox",
		"ThunderCheckBox",
		"ThunderRT6CheckBox",
		"TEdit",
		"TNumberEdit",
		"ThunderTextBox",
		"ThunderRT6TextBox",
		"TComboBox",
		"ThunderComboBox",
		"ThunderRT6ComboBox"
#endif // CXP_DLLMODE
	};

	// �����ж�ƥ���������
	GetClassName(hWnd, szTemp, sizeof(szTemp));
	for (lReturn = 0; lReturn < (sizeof(s_szClass) / sizeof(s_szClass[0])); lReturn++)
		if (lstrcmpi(szTemp, s_szClass[lReturn]) == 0)
			break;

	switch (lReturn)
	{
	case 0:
		lReturn = GetWindowLong(hWnd, GWL_STYLE);
		switch (lReturn & SS_TYPEMASK)
		{
		case BS_DEFPUSHBUTTON:		// Ĭ�ϰ�ť
		case BS_PUSHBUTTON:			// ��ͨ��ť
			lReturn = CXPT_PUSHBUTTON;
			break;

		case BS_CHECKBOX:			// ��ѡ��
		case BS_AUTOCHECKBOX:		// �Զ���ѡ��
		case BS_3STATE:				// ��״̬��ѡ��
		case BS_AUTO3STATE:			// �Զ���״̬��ѡ��
			if (lReturn & BS_PUSHLIKE)
				lReturn = CXPT_PUSHBUTTON;
			else
				lReturn = CXPT_CHECKBOX;
			break;

		case BS_RADIOBUTTON:		// ��ѡ��
		case BS_AUTORADIOBUTTON:	// �Զ���ѡ��
			if (lReturn & BS_PUSHLIKE)
				lReturn = CXPT_PUSHBUTTON;
			else
				lReturn = CXPT_RADIOBOX;
			break;

		default:	// δ֪����
			lReturn = CXPT_UNKNOWN;
		}
		break;

	case 1:			// �༭��
		lReturn = CXPT_EDITBOX;
		break;
	
	case 2:			// ��Ͽ�
		if ((GetWindowLong(hWnd, GWL_STYLE) & 0x00000003) == CBS_SIMPLE)
			lReturn = CXPT_UNKNOWN;
		else
			lReturn = CXPT_COMBOBOX;
		break;

#ifdef CXP_DLLMODE
	// VB �� VCL �Ŀؼ���ֻ���ڶ�̬���ӿⷽʽ�²��п��ܳ����������
	case 3:
	case 4:
	case 5:
		lReturn = CXPT_PUSHBUTTON;
		break;

	case 6:
	case 7:
	case 8:
		lReturn = CXPT_CHECKBOX;
		break;

	case 9:
	case 10:
	case 11:
	case 12:
		lReturn = CXPT_EDITBOX;
		break;

	case 13:
	case 14:
	case 15:
		lReturn = CXPT_COMBOBOX;
		break;

#endif // CXP_DLLMODE

	default:		// δ֪����
		lReturn = CXPT_UNKNOWN;
	}

	return lReturn;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ȡ���ڵ��ڴ�����豸����
HDC WINAPI GetMemDCXP(LPMEMDCXP pMdcxp)
{
	RECT Rect;

	GetWindowRect(pMdcxp->hWnd, &Rect);

	// ���������ڴ�����豸����������Ϊ͸��ģʽ
	pMdcxp->hDC = GetWindowDC(pMdcxp->hWnd);
	pMdcxp->hMemDC = CreateCompatibleDC(pMdcxp->hDC);;
	SetBkMode(pMdcxp->hMemDC, TRANSPARENT);

	// �Ƿ��Ѿ�ָ����λͼ���
	if (pMdcxp->hBitmap)
	{
		// ѡ��λͼ����
		SelectObject(pMdcxp->hMemDC, pMdcxp->hBitmap);
	}
	else
	{
		// ������ѡ��λͼ����
		pMdcxp->hBitmap = (HBITMAP) SelectObject(pMdcxp->hMemDC,
			CreateCompatibleBitmap(pMdcxp->hDC, Rect.right - Rect.left, Rect.bottom - Rect.top));
	}

	// ���Ҫ��������
	if (pMdcxp->bTransfer == TRUE)
	{
		BitBlt(pMdcxp->hMemDC, 0, 0,
			Rect.right - Rect.left, Rect.bottom - Rect.top, pMdcxp->hDC, 0, 0, SRCCOPY);
	}

	return pMdcxp->hMemDC;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ȡ���ڵ��ڴ�����豸����
VOID WINAPI ReleaseMemDCXP(LPMEMDCXP pMdcxp)
{
	RECT Rect;

	// ���Ҫ��������
	if (pMdcxp->bTransfer == TRUE)
	{
		GetWindowRect(pMdcxp->hWnd, &Rect);
		BitBlt(pMdcxp->hDC, 0, 0,
			Rect.right - Rect.left, Rect.bottom - Rect.top, pMdcxp->hMemDC, 0, 0, SRCCOPY);
	}

	if (pMdcxp->hBitmap)
		DeleteObject(SelectObject(pMdcxp->hMemDC, pMdcxp->hBitmap));
	DeleteDC(pMdcxp->hMemDC);
	ReleaseDC(pMdcxp->hWnd, pMdcxp->hDC);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ���ƽ������
VOID WINAPI GradientRectXP(HDC hDC, LPRECT pRect,COLORREF crColor[4])
{
	int i;
	TRIVERTEX Tve[4]; 
	GRADIENT_RECT GRect;
	GRADIENT_TRIANGLE GTrg;

	for (i = 0; i < 4; i++)
	{
		Tve[i].Red = ((COLOR16) GetRValue(crColor[i])) << 8;
		Tve[i].Green = ((COLOR16) GetGValue(crColor[i])) << 8;
		Tve[i].Blue = ((COLOR16) GetBValue(crColor[i])) << 8;
		Tve[i].Alpha = ((COLOR16) (crColor[i] >> 24)) << 8;
	}

	Tve[0].x = pRect->left;
	Tve[0].y = pRect->top;
	Tve[1].x = pRect->right;
	Tve[1].y = pRect->top;
	Tve[2].x = pRect->left;
	Tve[2].y = pRect->bottom;
	Tve[3].x = pRect->right;
	Tve[3].y = pRect->bottom;

	if ((crColor[0] == crColor[2]) &&
		(crColor[1] == crColor[3]))
		i = GRADIENT_FILL_RECT_H;
	if ((crColor[0] == crColor[1]) &&
		(crColor[2] == crColor[3]))
		i = GRADIENT_FILL_RECT_V;
	else
		i = GRADIENT_FILL_TRIANGLE;

	if (i == GRADIENT_FILL_TRIANGLE)
	{
		GTrg.Vertex1 = 0;
		GTrg.Vertex2 = 1;
		GTrg.Vertex3 = 2;
	}
	else
	{
		GRect.UpperLeft = 0;
		GRect.LowerRight = 3;
	}
	GradientFill(hDC, Tve, 4,
		((i == GRADIENT_FILL_TRIANGLE) ? ((PVOID) &GTrg) : ((PVOID) &GRect)), 1, i);

	if (i == GRADIENT_FILL_TRIANGLE)
	{
		GTrg.Vertex1 = 3;
		GradientFill(hDC,Tve, 4, &GTrg, 1, GRADIENT_FILL_TRIANGLE);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ���ƽ������
VOID WINAPI DrawDropGripXP(HDC hDC, LPRECT pRect)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ö�ٴ��ڻص�����
BOOL CALLBACK EnumWndProcXP(HWND hWnd, LPARAM lParam)
{
	// �����ȡ��ָ�����ڵ� ClassXP ���
	if (lParam == FALSE)
		DeleteClassXP(hWnd);
	// ���������ָ�����ڵ� ClassXP ���
	else
		CreateClassXP(hWnd);	

	// ö���Ӵ���
	EnumChildWindows(hWnd, EnumWndProcXP, lParam);
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ������Ϣ HOOK �ص�����
LRESULT CALLBACK HookProcXP(int iCode, WPARAM wParam, LPARAM lParam)
{
	// �����½��Ĵ���Ϊ ClassXP ���
	if ((((CWPSTRUCT *) lParam)->message == WM_CREATE) && (iCode >= 0))
		CreateClassXP(((CWPSTRUCT *) lParam)->hwnd);

	return CallNextHookEx(g_hPrevHookXP, iCode, wParam, lParam);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// �������໯�ص�����
LRESULT CALLBACK WindowProcXP(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG lReturn;
	PCLASSXP pCxp;
	TRACKMOUSEEVENT Tme;
	typedef VOID (WINAPI *DRAWXP)(PCLASSXP);
	static DRAWXP s_DrawXP[] = 
	{
		DrawPushButtonXP,
		DrawCheckBoxXP,
		DrawRadioBoxXP,
		DrawEditBoxXP,
		DrawComboBoxXP
	};

	pCxp = GetClassXP(hWnd);

	switch (pCxp->dwType)
	{
	case CXPT_PUSHBUTTON:	// ��ť
	case CXPT_CHECKBOX:		// ��ѡ��
	case CXPT_RADIOBOX:		// ��ѡ��
		switch (uMsg)
		{
		case BM_SETSTYLE:	// ��ť���ı�
			CXPM_SETSTATE(pCxp->dwState, CXPS_DEFAULT, wParam & BS_DEFPUSHBUTTON);
			s_DrawXP[pCxp->dwType](pCxp);
			break;

		case BM_SETSTATE:	// ���ð�ť״̬
			lReturn = (LONG) CallWindowProc(pCxp->wpPrev, hWnd, uMsg, wParam, lParam);
			CXPM_SETSTATE(pCxp->dwState, CXPS_PRESSED, wParam);
			s_DrawXP[pCxp->dwType](pCxp);
			return lReturn;

		case BM_SETCHECK:	// ����ѡ��״̬
			lReturn = (LONG) CallWindowProc(pCxp->wpPrev, hWnd, uMsg, wParam, lParam);
			CXPM_SETSTATE(pCxp->dwState, CXPS_CHECKED, (wParam == BST_CHECKED));
			CXPM_SETSTATE(pCxp->dwState, CXPS_INDETERMINATE, (wParam == BST_INDETERMINATE));
			s_DrawXP[pCxp->dwType](pCxp);
			return lReturn;

		case WM_SETTEXT:	// ���ô����ı�
			lReturn = (LONG) DefWindowProc(hWnd, uMsg, wParam, lParam);
			s_DrawXP[pCxp->dwType](pCxp);
			return lReturn;

		case WM_PAINT:		// �����ػ�
			lReturn = (LONG) DefWindowProc(hWnd, uMsg, wParam, lParam);
			s_DrawXP[pCxp->dwType](pCxp);
			return lReturn;
		}
		break;

	case CXPT_EDITBOX:		// �༭��
		if (uMsg == WM_NCPAINT)
		{
			lReturn = (LONG) DefWindowProc(hWnd, uMsg, wParam, lParam);
			DrawEditBoxXP(pCxp);
			return lReturn;
		}
		else if (uMsg == EM_SETREADONLY)
		{
			CXPM_SETSTATE(pCxp->dwState, CXPS_READONLY, wParam);
			DrawEditBoxXP(pCxp);
		}
		break;

	case CXPT_COMBOBOX:		// ��Ͽ�
		switch (uMsg)
		{
		case WM_PAINT:
		case WM_NCPAINT:
			lReturn = (LONG) CallWindowProc(pCxp->wpPrev, hWnd, uMsg, wParam, lParam);
			DrawComboBoxXP(pCxp);
			return lReturn;

		case WM_LBUTTONDOWN:
			pCxp->dwState |= CXPS_PRESSED;
			break;

		case WM_LBUTTONUP:
			pCxp->dwState &= ~CXPS_PRESSED;
			break;
		}
		break;
	}

	// ����ԭ���Ļص�����
	lReturn = (LONG) CallWindowProc(pCxp->wpPrev, hWnd, uMsg, wParam, lParam);

	// �����д�����ͬ�Ĵ���
	switch (uMsg)
	{
	case WM_MOUSEMOVE:		// �����ƶ�
		if (((pCxp->dwState & CXPS_HOTLIGHT) == 0) && ((wParam & MK_LBUTTON) == 0))
		{			
			pCxp->dwState |= CXPS_HOTLIGHT;
			s_DrawXP[pCxp->dwType](pCxp);

			// ׷������Ƴ���Ϣһ��
			Tme.cbSize = sizeof(TRACKMOUSEEVENT);
			Tme.dwFlags = TME_LEAVE;
			Tme.hwndTrack = hWnd;
			TrackMouseEvent(&Tme);
		}
		break;

	case WM_MOUSELEAVE:		// ����Ƴ�
		if (pCxp->dwState & CXPS_HOTLIGHT)
		{
			pCxp->dwState &= ~CXPS_HOTLIGHT;
			s_DrawXP[pCxp->dwType](pCxp);
		}
		break;

	case WM_ENABLE:			// ���ڱ�����Ϊ���û����
		CXPM_SETSTATE(pCxp->dwState, CXPS_DISABLED, !wParam);
		s_DrawXP[pCxp->dwType](pCxp);
		break;

	case WM_SETFOCUS:		// ��ý���
		pCxp->dwState |= CXPS_FOCUS;
		s_DrawXP[pCxp->dwType](pCxp);
		break;

	case WM_KILLFOCUS:		// ��ʧ����
		pCxp->dwState &= ~CXPS_FOCUS;
		s_DrawXP[pCxp->dwType](pCxp);
		break;

	case WM_DESTROY:		// ��������
		DeleteClassXP(hWnd);
	}
	return lReturn;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ���ư�ť
VOID WINAPI DrawPushButtonXP(PCLASSXP pCxp)
{
	int i;
	RECT Rect;
	MEMDCXP Mdcxp;
	HANDLE hHandle;
	COLORREF crColor;
	char szTemp[256];

	static COLORREF s_crGradientXP[][4] =
	{
		{0x00C1CCD1, 0x00C1CCD1, 0x00EEF1F2, 0xFFEEF1F2},
		{0x00CFF0FF, 0x00CFF0FF, 0x000097E5, 0xFF0097E5},
		{0x00BDCBD6, 0x00C6CFD6, 0x00EFF3F7, 0xFFEFF3F7},
		{0x00FFE7CE, 0x00FFE7CE, 0x00EE8269, 0xFFEE8269},
		{0x00FFFFFF, 0x00FFFFFF, 0x00D6DFE2, 0xFFD6DFE2},

		{0x00DEE3E7, 0x00E7E7E7, 0x00DEE3E7, 0xFFDEE3E7},
		{0x00FBFCFC, 0x00FBFCFC, 0x00E6EBEC, 0xFFE6EBEC},
	};

	// ��ȡ�ڴ�����豸����
	Mdcxp.hWnd = pCxp->hWnd;
	Mdcxp.bTransfer = FALSE;
	Mdcxp.hBitmap = NULL;
	GetMemDCXP(&Mdcxp);

	// ��ȡ���ڴ�С
	GetWindowRect(pCxp->hWnd, &Rect);
	Rect.right -= Rect.left;
	Rect.bottom -= Rect.top;
	Rect.left = Rect.top = 0;

	// ��������Ŀ���ɫ��ϵͳ��ť��ɫһֱ
	FrameRect(Mdcxp.hMemDC, &Rect, GetSysColorBrush(COLOR_BTNFACE));

	// ����ڶ���߿�
	InflateRect(&Rect, -1, -1);
	hHandle = (HANDLE) CreateSolidBrush(
		(pCxp->dwState & CXPS_DISABLED) ? (GetSysColor(COLOR_BTNFACE) - 0x00202020) : 0x00733C00);
	FrameRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);
	DeleteObject((HGDIOBJ) hHandle);

	// ���ȵ�򽥱䱳��
	InflateRect(&Rect, -1, -1);
	if (pCxp->dwState & CXPS_DISABLED)
	{
		i = -1;
		hHandle = (HANDLE) CreateSolidBrush(0x00EAF4F5);
		FillRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);
		DeleteObject((HGDIOBJ) hHandle);
	}
	else
	{
		if (pCxp->dwState & CXPS_PRESSED)
			i = 0;
		else if (pCxp->dwState & CXPS_HOTLIGHT)
			i = 1;
		else if ((pCxp->dwState & CXPS_CHECKED) || (pCxp->dwState & CXPS_INDETERMINATE))
			i = 2;
		else if ((pCxp->dwState & CXPS_FOCUS) || (pCxp->dwState & CXPS_DEFAULT))
			i = 3;
		else
			i = 4;
		GradientRectXP(Mdcxp.hMemDC, &Rect, s_crGradientXP[i]);
	}

	// ���������򽥱䱳��
	InflateRect(&Rect, -2, -2);
	if ((pCxp->dwState & CXPS_PRESSED) ||
		(pCxp->dwState & CXPS_CHECKED) ||
		(pCxp->dwState & CXPS_INDETERMINATE))
		i = 5;
	else if (!(pCxp->dwState & CXPS_DISABLED))
		i = 6;
	if ((i == 5) || (i == 6))
		GradientRectXP(Mdcxp.hMemDC, &Rect, s_crGradientXP[i]);


	// ����ϵͳ��ťһֱ����ɫ�ڶ���߿���Ľ�����
	crColor = GetSysColor(COLOR_BTNFACE);
	SetPixel(Mdcxp.hMemDC, 1, 1, crColor);
	SetPixel(Mdcxp.hMemDC, 1, Rect.bottom + 2, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 2, Rect.bottom + 2, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 2, 1, crColor);

	// ���ڶ���߿�Ĺս����أ�̫�����ˣ�ǧ�����ϸ������
	crColor = (pCxp->dwState & CXPS_DISABLED) ?
		(GetSysColor(COLOR_BTNFACE) - 0x00151515) : 0x00A57D52;
	SetPixel(Mdcxp.hMemDC, 2, 2, crColor);
	SetPixel(Mdcxp.hMemDC, 2, Rect.bottom + 1, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 1, Rect.bottom + 1, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 1, 2, crColor);
	crColor = (pCxp->dwState & CXPS_DISABLED) ?
		(GetSysColor(COLOR_BTNFACE) - 0x00111111) : 0x00AD967B;
	SetPixel(Mdcxp.hMemDC, 1, 2, crColor);
	SetPixel(Mdcxp.hMemDC, 2, 1, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 1, 1, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 2, 2, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 1, Rect.bottom + 2, crColor);
	SetPixel(Mdcxp.hMemDC, Rect.right + 2, Rect.bottom + 1, crColor);
	SetPixel(Mdcxp.hMemDC, 2, Rect.bottom + 2, crColor);
	SetPixel(Mdcxp.hMemDC, 1, Rect.bottom + 1, crColor);

	// ����н��㣬���������
	if (pCxp->dwState & CXPS_FOCUS)
	{
		InflateRect(&Rect, 1, 1);
		DrawFocusRect(Mdcxp.hMemDC, &Rect);
	}

	// ������
	if (GetWindowText(pCxp->hWnd, szTemp, sizeof(szTemp)))
	{
		if (pCxp->dwState & CXPS_PRESSED)
			OffsetRect(&Rect, 1, 1);
		SetTextColor(Mdcxp.hMemDC,
			((pCxp->dwState & CXPS_INDETERMINATE) || (pCxp->dwState & CXPS_DISABLED)) ?
			0x0094A2A5: 0x00000000);
		hHandle = (HANDLE) SelectObject(Mdcxp.hMemDC,
			(HGDIOBJ) SendMessage(pCxp->hWnd, WM_GETFONT, 0, 0));
		DrawText(Mdcxp.hMemDC, szTemp, -1, &Rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		SelectObject(Mdcxp.hMemDC, (HGDIOBJ) hHandle);
	}

	Mdcxp.bTransfer = TRUE;
	ReleaseMemDCXP(&Mdcxp);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ���Ƹ�ѡ��
VOID WINAPI DrawCheckBoxXP(PCLASSXP pCxp)
{
	int i;
	RECT Rect;
	MEMDCXP Mdcxp;
	HANDLE hHandle;
	char szTemp[256];
	COLORREF crColor;
	static COLORREF s_crGradientXP[][4] =
	{
		{0x00A5B2B5, 0x00CED7D6, 0x00CED7D6, 0x00DEEFF7},
		{0x00CEF3FF, 0x0063CBFF, 0x0063CBFF, 0x0031B2FF},
		{0x00D6DFDE, 0x00EFF3F7, 0x00EFF3F7, 0x00FFFFFF}
	};

	// ��ȡ�ڴ�����豸����
	Mdcxp.hWnd = pCxp->hWnd;
	Mdcxp.bTransfer = FALSE;
	Mdcxp.hBitmap = NULL;
	GetMemDCXP(&Mdcxp);

	// ��ȡ���ڴ�С
	GetWindowRect(pCxp->hWnd, &Rect);
	Rect.right -= Rect.left;
	Rect.bottom -= Rect.top;
	Rect.left = Rect.top = 0;

	hHandle = (HANDLE) CreateSolidBrush(COLOR_BTNFACE);
	// ��䱳��
	FillRect(Mdcxp.hMemDC, &Rect, (HBRUSH)hHandle/*GetSysColorBrush(COLOR_BTNFACE)*/);

	// ��������Ŀ�
	Rect.left = 0;
	Rect.right = 13;
	Rect.top = (Rect.bottom - 13) / 2;
	Rect.bottom = Rect.top + 13;
	hHandle = (HANDLE) CreateSolidBrush(
		(pCxp->dwState & CXPS_DISABLED) ? (GetSysColor(COLOR_BTNFACE) - 0x00202020) : 0x00845118);
	FrameRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);
	DeleteObject((HGDIOBJ) hHandle);

	// ���ȵ�򽥱䱳��
	InflateRect(&Rect, -1, -1);
	if (pCxp->dwState & CXPS_DISABLED)
		FillRect(Mdcxp.hMemDC, &Rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	else
	{
		if (pCxp->dwState & CXPS_PRESSED)
			i = 0;
		else if (pCxp->dwState & CXPS_HOTLIGHT)
			i = 1;
		else
			i = 2;
		GradientRectXP(Mdcxp.hMemDC, &Rect, s_crGradientXP[i]);
	}

	// ���ڿ�
	InflateRect(&Rect, -2, -2);
	if ((pCxp->dwState & CXPS_INDETERMINATE) ||
		((pCxp->dwState & CXPS_HOTLIGHT) && (!(pCxp->dwState & CXPS_PRESSED))))
	{
		if (pCxp->dwState & CXPS_INDETERMINATE)
		{
			if (pCxp->dwState & CXPS_DISABLED)
				crColor = 0x00BDCBCE;
			else if (pCxp->dwState & CXPS_PRESSED)
				crColor = 0x00188A18;
			else if (pCxp->dwState & CXPS_HOTLIGHT)
				crColor = 0x0021A221;
			else
				crColor = 0x0073C373;
		}
		else if (pCxp->dwState & CXPS_CHECKED)
			crColor = 0x00F7F7F7;
		else
			crColor = 0x00E7E7E7;

		hHandle = (HANDLE) CreateSolidBrush(crColor);
		FillRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);
		DeleteObject((HGDIOBJ) hHandle);
	}


	// ������ѡ�б�־
	if (pCxp->dwState & CXPS_CHECKED)
	{
		hHandle = (HANDLE) SelectObject(Mdcxp.hMemDC,
			CreatePen(PS_SOLID, 1, (pCxp->dwState & CXPS_DISABLED) ? 0x000BDCBCE : 0x0021A221));
		for (i = 3; i < 10; i++)
		{
			MoveToEx(Mdcxp.hMemDC, i, Rect.top + ((i < 6) ? i - 1 : (9 - i)), NULL);
			LineTo(Mdcxp.hMemDC, i, Rect.top + ((i < 6) ? i + 2 : (12 - i)));
		}
		DeleteObject(SelectObject(Mdcxp.hMemDC, (HGDIOBJ) hHandle));
	}

	// ������
	if (GetWindowText(pCxp->hWnd, szTemp, sizeof(szTemp)))
	{
		SetTextColor(Mdcxp.hMemDC, GetSysColor((pCxp->dwState & CXPS_DISABLED) ?  COLOR_GRAYTEXT: COLOR_BTNTEXT));
		hHandle = (HANDLE) SelectObject(Mdcxp.hMemDC,
			(HGDIOBJ) SendMessage(pCxp->hWnd, WM_GETFONT, 0, 0));

		Rect.left = 18;
		Rect.top -= 2;
		Rect.bottom = Rect.top + 1 + DrawText(Mdcxp.hMemDC, szTemp, -1, &Rect,
			DT_CALCRECT | DT_SINGLELINE | DT_VCENTER);

		DrawText(Mdcxp.hMemDC, szTemp, -1, &Rect, DT_SINGLELINE | DT_VCENTER);
		SelectObject(Mdcxp.hMemDC, (HGDIOBJ) hHandle);

		// ����н��㣬���������
		if (pCxp->dwState & CXPS_FOCUS)
		{
			InflateRect(&Rect, 1, 1);
			DrawFocusRect(Mdcxp.hMemDC, &Rect);
		}
	}
	Mdcxp.bTransfer = TRUE;
	ReleaseMemDCXP(&Mdcxp);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ���Ƶ�ѡ��
VOID WINAPI DrawRadioBoxXP(PCLASSXP pCxp)
{
	int i;
	RECT Rect;
	MEMDCXP Mdcxp;
	HANDLE hHandle;
	char szTemp[256];
	COLORREF crColor;
	static COLORREF s_crGradientXP[][4] =
	{
		{0x00A5B2B5, 0x00CED7D6, 0x00CED7D6, 0x00DEEFF7},
		{0x00CEF3FF, 0x0063CBFF, 0x0063CBFF, 0x0031B2FF},
		{0x00D6DFDE, 0x00EFF3F7, 0x00EFF3F7, 0x00FFFFFF}
	};

	// ��ȡ�ڴ�����豸����
	Mdcxp.hWnd = pCxp->hWnd;
	Mdcxp.bTransfer = FALSE;
	Mdcxp.hBitmap = NULL;
	GetMemDCXP(&Mdcxp);

	// ��ȡ���ڴ�С
	GetWindowRect(pCxp->hWnd, &Rect);
	Rect.right -= Rect.left;
	Rect.bottom -= Rect.top;
	Rect.left = Rect.top = 0;

	hHandle = (HANDLE) CreateSolidBrush(Bk_Color);
	// ��䱳��
	FillRect(Mdcxp.hMemDC, &Rect,(HBRUSH) hHandle);

	// ��������Ŀ�
	Rect.left = 0;
	Rect.right = 13;
	Rect.top = (Rect.bottom - 13) / 2;
	Rect.bottom = Rect.top + 13;
	hHandle = (HANDLE) CreateSolidBrush(
		(pCxp->dwState & CXPS_DISABLED) ? (GetSysColor(COLOR_BTNFACE) - 0x00202020) : 0x00845118);
	FrameRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);
	DeleteObject((HGDIOBJ) hHandle);

	// ���ȵ�򽥱䱳��
	InflateRect(&Rect, -1, -1);
	if (pCxp->dwState & CXPS_DISABLED)
		FillRect(Mdcxp.hMemDC, &Rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	else
	{
		if (pCxp->dwState & CXPS_PRESSED)
			i = 0;
		else if (pCxp->dwState & CXPS_HOTLIGHT)
			i = 1;
		else
			i = 2;
		GradientRectXP(Mdcxp.hMemDC, &Rect, s_crGradientXP[i]);
	}

	// ���ڿ�
	InflateRect(&Rect, -2, -2);
	if ((pCxp->dwState & CXPS_INDETERMINATE) ||
		((pCxp->dwState & CXPS_HOTLIGHT) && (!(pCxp->dwState & CXPS_PRESSED))))
	{
		if (pCxp->dwState & CXPS_INDETERMINATE)
		{
			if (pCxp->dwState & CXPS_DISABLED)
				crColor = 0x00BDCBCE;
			else if (pCxp->dwState & CXPS_PRESSED)
				crColor = 0x00188A18;
			else if (pCxp->dwState & CXPS_HOTLIGHT)
				crColor = 0x0021A221;
			else
				crColor = 0x0073C373;
		}
		else if (pCxp->dwState & CXPS_CHECKED)
			crColor = 0x00F7F7F7;
		else
			crColor = 0x00E7E7E7;

		hHandle = (HANDLE) CreateSolidBrush(crColor);
		FillRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);
		DeleteObject((HGDIOBJ) hHandle);
	}


	// ������ѡ�б�־
	if (pCxp->dwState & CXPS_CHECKED)
	{
		hHandle = (HANDLE) SelectObject(Mdcxp.hMemDC,
			CreatePen(PS_SOLID, 1, (pCxp->dwState & CXPS_DISABLED) ? 0x000BDCBCE : 0x0021A221));
		for (i = 3; i < 10; i++)
		{
			MoveToEx(Mdcxp.hMemDC, i, Rect.top + ((i < 6) ? i - 1 : (9 - i)), NULL);
			LineTo(Mdcxp.hMemDC, i, Rect.top + ((i < 6) ? i + 2 : (12 - i)));
		}
		DeleteObject(SelectObject(Mdcxp.hMemDC, (HGDIOBJ) hHandle));
	}

	// ������
	if (GetWindowText(pCxp->hWnd, szTemp, sizeof(szTemp)))
	{
		SetTextColor(Mdcxp.hMemDC, GetSysColor((pCxp->dwState & CXPS_DISABLED) ?  COLOR_GRAYTEXT: COLOR_BTNTEXT));
		hHandle = (HANDLE) SelectObject(Mdcxp.hMemDC,
			(HGDIOBJ) SendMessage(pCxp->hWnd, WM_GETFONT, 0, 0));

		Rect.left = 18;
		Rect.top -= 2;
		Rect.bottom = Rect.top + 1 + DrawText(Mdcxp.hMemDC, szTemp, -1, &Rect,
			DT_CALCRECT | DT_SINGLELINE | DT_VCENTER);

		DrawText(Mdcxp.hMemDC, szTemp, -1, &Rect, DT_SINGLELINE | DT_VCENTER);
		SelectObject(Mdcxp.hMemDC, (HGDIOBJ) hHandle);

		// ����н��㣬���������
		if (pCxp->dwState & CXPS_FOCUS)
		{
			InflateRect(&Rect, 1, 1);
			DrawFocusRect(Mdcxp.hMemDC, &Rect);
		}
	}
	Mdcxp.bTransfer = TRUE;
	ReleaseMemDCXP(&Mdcxp);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ���Ʊ༭��
VOID WINAPI DrawEditBoxXP(PCLASSXP pCxp)
{
	HDC hDC;
	RECT Rect;
	LONG lExStyle;
	HANDLE hHandle;

	lExStyle = GetWindowLong(pCxp->hWnd, GWL_EXSTYLE);
	if ((GetWindowLong(pCxp->hWnd, GWL_STYLE) & WS_BORDER) ||
		(lExStyle & WS_EX_CLIENTEDGE) || (lExStyle & WS_EX_STATICEDGE))
	{
		// ���ڻ��ƵĶ������٣�����ֱ�ӻ��ƶ���ʹ�� MEMDCXP ��ʽ
		hDC = GetWindowDC(pCxp->hWnd);

		// ��ȡ���ڴ�С
		GetWindowRect(pCxp->hWnd, &Rect);
		Rect.right -= Rect.left;
		Rect.bottom -= Rect.top;
		Rect.top = Rect.left = 0;

		// �������
		hHandle = (HANDLE) CreateSolidBrush(
			(pCxp->dwState & CXPS_DISABLED) ? (GetSysColor(COLOR_BTNFACE) - 0x00202020) : 0x00BD9E7B);
		FrameRect(hDC, &Rect, (HBRUSH) hHandle);
		DeleteObject((HGDIOBJ) hHandle);

		// �����ڿ�
		if ((lExStyle & WS_EX_CLIENTEDGE) || (lExStyle & WS_EX_STATICEDGE))
		{
			InflateRect(&Rect, -1, -1);
			hHandle = (HANDLE) GetSysColorBrush(
				(pCxp->dwState & CXPS_DISABLED) || (pCxp->dwState & CXPS_READONLY) ? COLOR_BTNFACE : COLOR_WINDOW);
			FrameRect(hDC, &Rect, (HBRUSH) hHandle);
			if ((lExStyle & WS_EX_CLIENTEDGE) && (lExStyle & WS_EX_STATICEDGE))
			{
				InflateRect(&Rect, -1, -1);
				FrameRect(hDC, &Rect, (HBRUSH)hHandle);
			}
		}

		// �ͷ��豸����
		ReleaseDC(pCxp->hWnd, hDC);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ������Ͽ�
VOID WINAPI DrawComboBoxXP(PCLASSXP pCxp)
{
	int i;
	RECT Rect;
	MEMDCXP Mdcxp;
	HANDLE hHandle;
	static COLORREF s_crGradientXP[][4] =
	{
		{0x00EFF3F7, 0x00DEE7E7, 0x00DEE3E7, 0x00DEE3E7},
		{0x00DEAEA5, 0x00F7CBBD, 0x00DE8273, 0x00F7C7B5},
		{0x00EFC7B5, 0x00E7AE94, 0x00DEA284, 0x00DEA68C},
		{0x00FFE3D6, 0x00F7CBBD, 0x00F7C3AD, 0x00F7C7B5},

		{0x00F7F7F7, 0x00EFF3F7, 0x00EFF3F7, 0x00EFF3F7},
		{0x00DEC3BD, 0x00DEB6AD, 0x00FFE3DE, 0x00F7E3DE},
		{0x00EFDBCE, 0x00EFCFC6, 0x00E7CFC6, 0x00E7CBBD},
		{0x00FFEFE7, 0x00FFE7DE, 0x00FFE3DE, 0x00F7E3DE},

		{0x00F7F7F7, 0x00E7EFEF, 0x00E7EBEF, 0x00DEE7E7},
		{0x00F78E6B, 0x00F79684, 0x00EF9E8C, 0x00EFDFD6},
		{0x00FFFFFF, 0x00FFE3CE, 0x00FFDFC6, 0x00FFDBBD},
		{0x00FFEBE7, 0x00FFCFBD, 0x00FFCBB5, 0x00F7CBAD}
	};

	// ��ȡ�ڴ�����豸����
	Mdcxp.hWnd = pCxp->hWnd;
	Mdcxp.bTransfer = TRUE;
	Mdcxp.hBitmap = NULL;
	GetMemDCXP(&Mdcxp);

	// ��ȡ���ڴ�С
	GetWindowRect(pCxp->hWnd, &Rect);
	Rect.right -= Rect.left;
	Rect.bottom -= Rect.top;
	Rect.top = Rect.left = 0;

	/*if ((GetWindowLong(hWnd, GWL_STYLE) & 0x00000003) == CBS_SIMPLE)
	{
		GetWindow(hWnd, GW_CHILD);
		// ��ԭ���ͷ��ڴ��豸����
		Mdcxp.bTransfer = TRUE;
		ReleaseMemDCXP(&Mdcxp);		
	}*/

	// �������
	hHandle = (HANDLE) CreateSolidBrush(
		(pCxp->dwState & CXPS_DISABLED) ? (GetSysColor(COLOR_BTNFACE) - 0x00202020) : 0x00BD9E7B);
	FrameRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);
	DeleteObject((HGDIOBJ) hHandle);

	// �����ڿ�
	InflateRect(&Rect, -1, -1);
	hHandle = (HANDLE) GetSysColorBrush((pCxp->dwState & CXPS_DISABLED) ? COLOR_BTNFACE : COLOR_WINDOW);
	FrameRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);

	InflateRect(&Rect, -1, -1);
	Rect.left = Rect.right - GetSystemMetrics(SM_CYVTHUMB);
	FrameRect(Mdcxp.hMemDC, &Rect, (HBRUSH) hHandle);

	Rect.left++;
	if (pCxp->dwState & CXPS_DISABLED)
		i = 0;
	else if (pCxp->dwState & CXPS_PRESSED)
		i = 1;
	else if (pCxp->dwState & CXPS_HOTLIGHT)
		i = 2;
	else
		i = 3;

	// �����������
	GradientRectXP(Mdcxp.hMemDC, &Rect, s_crGradientXP[i]);

	// �����������ս�����
	SetPixel(Mdcxp.hMemDC, Rect.left, Rect.top, s_crGradientXP[i + 4][0]);
	SetPixel(Mdcxp.hMemDC, Rect.right - 1, Rect.top, s_crGradientXP[i + 4][1]);
	SetPixel(Mdcxp.hMemDC, Rect.left, Rect.bottom - 1, s_crGradientXP[i + 4][2]);
	SetPixel(Mdcxp.hMemDC, Rect.right - 1, Rect.bottom - 1, s_crGradientXP[i + 4][3]);		

	// ���������ڿ�
	InflateRect(&Rect, -1, -1);
	GradientRectXP(Mdcxp.hMemDC, &Rect, s_crGradientXP[i + 8]);

	// ����������־
	Rect.left += (Rect.right - Rect.left) / 2;
	Rect.top += (Rect.bottom - Rect.top) / 2;
	hHandle = (HANDLE) SelectObject(Mdcxp.hMemDC,
		CreatePen(PS_SOLID, 1, (pCxp->dwState & CXPS_DISABLED) ? 0x00C6CBCE : 0x0084614A));
	MoveToEx(Mdcxp.hMemDC, Rect.left - 4, Rect.top - 2, NULL);
	LineTo(Mdcxp.hMemDC, Rect.left, Rect.top + 2);
	LineTo(Mdcxp.hMemDC, Rect.left + 5, Rect.top - 3);
	MoveToEx(Mdcxp.hMemDC, Rect.left - 3, Rect.top - 2, NULL);
	LineTo(Mdcxp.hMemDC, Rect.left, Rect.top + 1);
	LineTo(Mdcxp.hMemDC, Rect.left + 4, Rect.top - 3);
	MoveToEx(Mdcxp.hMemDC, Rect.left - 3, Rect.top - 3, NULL);
	LineTo(Mdcxp.hMemDC, Rect.left, Rect.top);
	LineTo(Mdcxp.hMemDC, Rect.left + 4, Rect.top - 4);

	DeleteObject(SelectObject(Mdcxp.hMemDC, (HGDIOBJ) hHandle));

	// ��ԭ���ͷ��ڴ��豸����
	Mdcxp.bTransfer = TRUE;
	ReleaseMemDCXP(&Mdcxp);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif // __cplusplus
////////////////////////////////////////////////////////////////////////////////////////////////////


