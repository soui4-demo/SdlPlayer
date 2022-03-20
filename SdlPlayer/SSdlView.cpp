#include "stdafx.h"
#include "SSdlView.h"

SNSBEGIN

static const uint32_t Rmask = 0x00ff0000;
static const uint32_t Gmask = 0x0000FF00;
static const uint32_t Bmask = 0x000000ff;
static const uint32_t Amask = 0xFF000000;

BOOL SSdlView::InitSdl()
{
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		SLOG_FATAL( "SDL could not initialize! SDL Error: "<< SDL_GetError() );
		return FALSE;
	}
	//Set texture filtering to linear
	if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
	{
		SLOG_INFO( "Warning: Linear texture filtering not enabled!" );
	}
	return TRUE;
}

void SSdlView::UninitSdl()
{
	SDL_Quit();
}

SSdlView::SSdlView(void)
:m_sdlHost(this)
,m_sdlWnd(NULL)
,m_sdlRenderer(NULL)
,m_wndSurface(NULL)
,m_wndTexture(NULL)
,m_bNeedFlush(FALSE)
{
}

SSdlView::~SSdlView(void)
{
}

int SSdlView::OnCreate(LPVOID)
{
	if(GetContainer()->IsTranslucent())
		return 1;
	__baseCls::OnCreate(NULL);
	m_sdlHost.CreateNative(NULL,WS_CHILD,0,0,0,0,0,GetContainer()->GetHostHwnd(),GetID(),NULL);
	return 0;
}

void SSdlView::OnDestroy()
{
	EndSdlRender();
	m_sdlHost.DestroyWindow();
}

void SSdlView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__baseCls::OnShowWindow(bShow,nStatus);
	m_sdlHost.ShowWindow(IsVisible(TRUE)?SW_SHOW:SW_HIDE);
}

BOOL SSdlView::InitFromXml(THIS_ IXmlNode *pNode)
{
	SXmlNode node(pNode);	
	SXmlDoc xmlDoc;
	SXmlNode nodeCopy=xmlDoc.root().append_copy(node);
	node.remove_children();
	__baseCls::InitFromXml(pNode);

	nodeCopy.set_name(L"root");
	nodeCopy.remove_attribute(L"size");
	nodeCopy.append_attribute(L"size").set_value(L"-2,-2");

	SStringW strXml;
	xmlDoc.root().ToString(&strXml);
	m_sdlHost.InitFromXml(xmlDoc.Root());
	return TRUE;
}

SWindow * SSdlView::_FindChildByID(int nID, int nDeep /* = -1 */)
{
	return m_sdlHost.FindChildByID(nID,nDeep);
}

SWindow * SSdlView::_FindChildByName(const SStringW &strName, int nDeep /* = -1 */)
{
	return m_sdlHost.FindChildByName(strName,nDeep);
}


BOOL SSdlView::StartSdlRender()
{
	m_sdlWnd = SDL_CreateWindowFrom(m_sdlHost.m_hWnd);
	if(!m_sdlWnd)
		return FALSE;
	m_sdlRenderer = SDL_CreateRenderer(m_sdlWnd, -1, 0);
	if(!m_sdlRenderer)
		return FALSE;

	//create texture for window cache
	CRect rc = m_sdlHost.GetWindowRect();

	IBitmapS *pCache = m_sdlHost.GetCache();
	LPBYTE pBit = (LPBYTE)pCache->GetPixelBits();
	m_wndSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,rc.Width(),rc.Height(),32,Rmask,Gmask,Bmask,Amask);
	memcpy(m_wndSurface->pixels,pBit,rc.Width()*4*rc.Height());

	m_wndTexture = SDL_CreateTextureFromSurface( m_sdlRenderer, m_wndSurface );
	SDL_SetTextureBlendMode( m_wndTexture, SDL_BLENDMODE_BLEND );


	return TRUE;
}

void SSdlView::EndSdlRender()
{
	if(m_wndTexture)
	{
		SDL_DestroyTexture(m_wndTexture);
		m_wndTexture = NULL;
	}
	if(m_wndSurface)
	{
		SDL_FreeSurface(m_wndSurface);
		m_wndSurface = NULL;
	}

	if(m_sdlRenderer)
	{
		SDL_DestroyRenderer(m_sdlRenderer);
		m_sdlRenderer=NULL;
	}
	if(m_sdlWnd)
	{
		SDL_DestroyWindow(m_sdlWnd);
		m_sdlWnd = NULL;
	}
}

BOOL SSdlView::OnHostCacheUpdated(SHostWnd *pHost,IBitmapS * pCache, LPCRECT pRect)
{
	BOOL bRet = FALSE;
	//update texture for window
	m_cs.Enter();
	CRect rc = m_sdlHost.GetWindowRect();
	if(m_wndSurface && m_wndTexture)
	{
		LPBYTE pBit = (LPBYTE)pCache->GetPixelBits();
		SDL_Rect rcSdl={pRect->left,pRect->top,RectWidth(pRect),RectHeight(pRect)};
		SDL_UpdateTexture(m_wndTexture,&rcSdl,pBit,rc.Width()*4);
		bRet = TRUE;
	}
	m_cs.Leave();
	return bRet;
}


void SSdlView::OnSize(UINT nType, CSize size)
{
	__baseCls::OnSize(nType,size);
	if(m_sdlHost.IsWindow())
	{
		CRect rc = GetClientRect();
		int w = rc.Width();
		int h = rc.Height();
		m_sdlHost.MoveWindow(rc.left,rc.top,w,h);

		//update window texture
		m_cs.Enter();
		if(m_sdlRenderer)
		{
			if(m_wndTexture)
			{
				SDL_DestroyTexture(m_wndTexture);
				m_wndTexture = NULL;
			}
			if(m_wndSurface)
			{
				SDL_FreeSurface(m_wndSurface);
				m_wndSurface = NULL;
			}
			m_bNeedFlush = TRUE;
			IBitmapS *pCache = m_sdlHost.GetCache();
			LPBYTE pBit = (LPBYTE)pCache->GetPixelBits();
			m_wndSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,Rmask,Gmask,Bmask,Amask);
			memcpy(m_wndSurface->pixels,pBit,w*4*h);

			m_wndTexture = SDL_CreateTextureFromSurface( m_sdlRenderer, m_wndSurface );
			SDL_SetTextureBlendMode( m_wndTexture, SDL_BLENDMODE_BLEND );
		}
		m_cs.Leave();
	}
}


SDL_Renderer * SSdlView::GetSdlRenderer()
{
	m_cs.Enter();
	if(m_bNeedFlush)
	{
		m_cs.Leave();
		FlushSdl();
		m_cs.Enter();
	}
	return m_sdlRenderer;
}

void SSdlView::ReleaseSdlRenderer(SDL_Renderer *pRenderer)
{
	if(pRenderer && m_wndTexture)
	{
		CRect rc = m_sdlHost.GetWindowRect();
		SDL_Rect sdlRect={rc.left,rc.top,rc.Width(),rc.Height()};
		SDL_RenderCopy(pRenderer,m_wndTexture,NULL,&sdlRect);
		SDL_RenderPresent(pRenderer);
	}
	m_cs.Leave();
}

BOOL SSdlView::OnHandleEvent(IEvtArgs *pEvt)
{
	return GetContainer()->OnFireEvent(pEvt);
}

void SSdlView::FlushSdl()
{
	m_sdlHost.SendMessage(UM_FLUSHSDL);
}

void SSdlView::OnFlushSdl()
{
	m_cs.Enter();
	SDL_RenderClear(m_sdlRenderer);
	SDL_RenderFlush(m_sdlRenderer);
	m_cs.Leave();
}

SNSEND
