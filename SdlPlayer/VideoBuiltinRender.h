#pragma once

#include <transvod-i.h>
#include <SDL.h>

#include <list>
#include "base/unknown.h"
#include "SSdlView.h"

using namespace SOUI;

VOD_NS_BEGIN


class CVideoBuiltinRender: public IUnknown, public CUnknown {
 public:
	 CVideoBuiltinRender(ITransVod* pTranVod, 
		 SSdlView* h, VideoRenderMode mode, 
		 AVPixelFormat format);

  virtual ~CVideoBuiltinRender(){}

  virtual void UpdateRenderRect(const IAVframe* frame) = 0;

  void onRenderFrame(IAVframe* frame);
  int expectWidth(void) { return m_expectW; }
  int expectHeight(void) { return m_expectH; }

  AVPixelFormat format(void) {
    return m_format;
  }

 protected:
  virtual void doRendering(IAVframe* frame) = 0;


  SSdlView* m_sdlView;
  AVPixelFormat m_format;
  VideoRenderMode m_renderMode;
  // 期望的yuv的长宽
  int m_expectW ;
  int m_expectH ;
  ITransVod* m_pTransVod;
 public:
  IUNKNOWN_BEGIN(IUnknown)
  IUNKNOWN_END()
};

class SDL2BuiltinRender : public CVideoBuiltinRender{
 public:
  SDL2BuiltinRender(ITransVod* pTranVod, SSdlView* h, 
                    VideoRenderMode mode);
  ~SDL2BuiltinRender();

  void UpdateRenderRect(const IAVframe* frame) override;
  void doRendering(IAVframe* frame) override;

  void rerender();
 private:
  void resetRenderRect(void);
  void resetRect(int w, int h);
  bool resetTexture(SDL_Renderer * sdlRenderer,int pixel_w, int pixel_h);
  void checkUpdateRect(const IAVframe* frame);

  SDL_Texture* m_sdl2Texture ;
  SComPtr<IAVframe> m_lastFrame;

  /**
   * 上一次初始化m_sdl2Texture时使用的像素宽高，
   * 用于检测Texture是否需要重新创建
   */
  int m_lastPixelW ;
  int m_lastPixelH ;

  // 当前屏幕的宽高
  int m_curScreenW ;
  int m_curScreenH ;

  // 图像渲染区域与区域左上角的平面坐标
  SDL_Rect m_renderRect;

  // 上一次渲染帧的宽高，用于过滤重复的计算
  int m_lastFrameW ;
  int m_lastFrameH ;
};

VOD_NS_END