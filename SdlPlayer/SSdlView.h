#pragma once
#include <SDL.h>

SNSBEGIN

#define UM_FLUSHSDL (WM_USER+100)
class SSdlHost : public SHostWnd
{
public:
struct IListener
{
	virtual BOOL OnHostCacheUpdated(SHostWnd *pHost,IBitmapS * pCache, LPCRECT pRect) = NULL;
	virtual BOOL OnHandleEvent(IEvtArgs *pEvt) = NULL;
	virtual void OnFlushSdl() = 0;
};

public:
	SSdlHost(IListener *pListener):m_listener(pListener){}

	IBitmapS * GetCache(){
		if(!m_memRT)
			return NULL;
		return (IBitmapS*) m_memRT->GetCurrentObject(OT_BITMAP);
	}
protected:
	BOOL OnCacheUpdated(IBitmapS * pCache, LPCRECT pRect){
		if(!m_listener) return FALSE;
		return m_listener->OnHostCacheUpdated(this,pCache,pRect);
	}

	virtual BOOL _HandleEvent(IEvtArgs *pEvt)
	{
		if(!m_listener)
			return FALSE;
		return m_listener->OnHandleEvent(pEvt);
	}
private:
	LRESULT OnFlushSdl(UINT uMsg,WPARAM,LPARAM)
	{
		m_listener->OnFlushSdl();
		return 0;
	}
	IListener * m_listener;
	BEGIN_MSG_MAP_EX(SSdlHost)
		MESSAGE_HANDLER_EX(UM_FLUSHSDL,OnFlushSdl)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()
};

class SSdlView : public SWindow, SSdlHost::IListener
{
	DEF_SOBJECT(SWindow,L"SdlView")

protected:
	SSdlHost   m_sdlHost;
	SDL_Window * m_sdlWnd;
	SDL_Renderer *m_sdlRenderer;
	SDL_Surface* m_wndSurface;
	SDL_Texture * m_wndTexture;
	BOOL m_bNeedFlush;
	SCriticalSection m_cs;
public:
	SSdlView(void);
	~SSdlView(void);

	static BOOL InitSdl();
	static void UninitSdl();

	BOOL StartSdlRender();
	void EndSdlRender();
	SDL_Renderer * GetSdlRenderer();
	void ReleaseSdlRenderer(SDL_Renderer *pRenderer);
public:
	SWindow * FindChildByID(int nID, int nDeep /* = -1 */) OVERRIDE;
	SWindow * FindChildByName(LPCWSTR strName, int nDeep /* = -1 */) OVERRIDE;

protected:
	BOOL OnHostCacheUpdated(SHostWnd *pHost,IBitmapS * pCache, LPCRECT pRect) OVERRIDE;
	BOOL OnHandleEvent(IEvtArgs *pEvt) OVERRIDE;
	void OnFlushSdl() OVERRIDE;

	void FlushSdl();
public:
	HWND GetHwnd() const{return m_sdlHost.m_hWnd;}
protected:
	STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE;
protected:
	int OnCreate(LPVOID);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SHOWWINDOW(OnShowWindow)
	SOUI_MSG_MAP_END()
};

SNSEND