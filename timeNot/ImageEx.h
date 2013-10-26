// GDIPlusHelper.h: interface for the CGDIPlusHelper class.
//
//////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <GdiPlus.h>
#include <string>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;
using namespace std;

#if !defined(AFX_GDIPLUSHELPER_H__BD5F6266_5686_43E2_B146_5EA1217A56FE__INCLUDED_)
#define AFX_GDIPLUSHELPER_H__BD5F6266_5686_43E2_B146_5EA1217A56FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef	UNICODE
typedef	wstring TString;
#else
typedef	string TString;
#endif

class ImageEx : public Image
{
public:
	ImageEx():m_hWnd(NULL), m_hInst(NULL){}
	ImageEx(HINSTANCE hInst, IN LPCTSTR  sResourceType, IN LPCTSTR  sResource);
	ImageEx(const WCHAR* filename, BOOL useEmbeddedColorManagement = FALSE);

	~ImageEx();
public:

	//void	Draw(CDC* pDC);
	void	Draw(HDC hdc);
	void	Draw(HWND hWnd, POINT pt);
	void	Draw(HWND hWnd, int x, int y);
	void	Draw(HWND hWnd, POINT pt, int nWidth, int nHeight); 
	void	Draw(HWND hWnd, int x, int y, int nWidth, int nHeight);

	//CSize	GetSize();
	SIZE	GetSize();

	bool	IsAnimatedGIF() { return m_nFrameCount > 1; }
	void	SetPause(bool bPause);
	bool	IsPaused() { return m_bPause; }
	//bool	InitAnimation(HWND hWnd, CPoint pt);
	bool	InitAnimation(HWND hWnd, POINT pt);
	void	Destroy();

	bool Load(HINSTANCE hInst, const TCHAR *sResourceType, const TCHAR *sResource);

protected:

	bool		TestForAnimatedGIF();
	void		Initialize();
	bool		DrawFrameGIF();

	IStream*	m_pStream;
	
	bool LoadFromBuffer(BYTE* pBuff, int nSize);
	bool GetResource(LPCTSTR lpName, LPCTSTR lpType, void* pResource, int& nBufSize);
	//bool Load(CString sResourceType, CString sResource);
	bool Load(TString sResourceType, TString sResource);

	void ThreadAnimation();

	static UINT WINAPI _ThreadAnimationProc(LPVOID pParam);

	HANDLE			m_hThread;
	HANDLE			m_hPause;
	HANDLE			m_hExitEvent;
	HINSTANCE		m_hInst;
	HWND			m_hWnd;
	UINT			m_nFrameCount;
	UINT			m_nFramePosition;
	bool			m_bIsInitialized;
	bool			m_bPause;
	PropertyItem*	m_pPropertyItem;
	//CPoint			m_pt;
	POINT			m_pt;


};


#endif // !defined(AFX_GDIPLUSHELPER_H__BD5F6266_5686_43E2_B146_5EA1217A56FE__INCLUDED_)
