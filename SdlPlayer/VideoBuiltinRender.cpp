#include "stdafx.h"
#include "VideoBuiltinRender.h"

using namespace transvod;

VOD_NS_BEGIN

CVideoBuiltinRender::CVideoBuiltinRender(ITransVod* pTranVod, 
                                         SSdlView *h, VideoRenderMode mode, 
                                         AVPixelFormat format)
:m_pTransVod(pTranVod)
,m_sdlView(h)
,m_renderMode(mode)
,m_format(format)
,m_expectW(0)
,m_expectH(0)
{
}


void CVideoBuiltinRender::onRenderFrame(IAVframe* frame) {
    UpdateRenderRect(frame);

    IAVframe* dstFrame = NULL;
    AVPixelFormat targetFormat = format();

    if (expectWidth() != frame->width() || expectHeight() != frame->height()
        || frame->format() != targetFormat) {
        int res = m_pTransVod->scale(frame, expectWidth(), expectHeight(), targetFormat, &dstFrame);
        if (res != expectHeight()) {
            return;
        }
    }
    else {
        dstFrame = frame;
    }

    m_pTransVod->feedbackRenderState(frame, before_render);
    doRendering(dstFrame);
    m_pTransVod->feedbackRenderState(frame, after_render);
     
    if (dstFrame != frame) {
        dstFrame->Release();
    }

}

SDL2BuiltinRender::SDL2BuiltinRender(ITransVod* pTransVod, SSdlView* h, 
                                     VideoRenderMode mode)
: CVideoBuiltinRender(pTransVod,h, mode, AV_PIX_FMT_YUV420P) {
  memset(&m_renderRect, 0, sizeof(m_renderRect));

  /**
   * 上一次初始化m_sdl2Texture时使用的像素宽高，
   * 用于检测Texture是否需要重新创建
   */
  m_lastPixelW = 0;
  m_lastPixelH = 0;

  // 当前屏幕的宽高
  m_curScreenW = 0;
  m_curScreenH = 0;

  // 上一次渲染帧的宽高，用于过滤重复的计算
  m_lastFrameW = 0;
  m_lastFrameH = 0;

  m_sdl2Texture = NULL;
	h->StartSdlRender();
}

SDL2BuiltinRender::~SDL2BuiltinRender() {
  m_sdlView->EndSdlRender();
}


void SDL2BuiltinRender::UpdateRenderRect(const IAVframe* frame) {
	CRect rcWnd = m_sdlView->GetWindowRect();
  int cur_w = rcWnd.Width();
  int cur_h = rcWnd.Height();
  if (cur_w != m_curScreenW || cur_h != m_curScreenH) {
    SLOGFMTI(
        "[SDL2BuiltinRender] reset render screen, width: (from %d to %d), "
        "height: (from %d to %d).",
        m_curScreenW, cur_w, m_curScreenH, cur_h);
    resetRect(cur_w, cur_h);
  }
  checkUpdateRect(frame);
}

void SDL2BuiltinRender::doRendering(IAVframe* frame) {
	SDL_Renderer * m_sdl2Render = m_sdlView->GetSdlRenderer();
  if (!m_sdl2Render) {
	  return;
  }
  resetTexture(m_sdl2Render,frame->width(), frame->height());
  if (!m_sdl2Texture) {
	 m_sdlView->ReleaseSdlRenderer(m_sdl2Render);
     return;
  }

  int line1Offset = frame->lineSize()[0] * frame->height();
  int line2Offset = line1Offset + frame->lineSize()[1] * frame->height() / 2;

  SDL_UpdateYUVTexture(m_sdl2Texture, NULL, frame->getData(),
                       frame->lineSize()[0], frame->getData() + line1Offset,
                       frame->lineSize()[1], frame->getData() + line2Offset,
                       frame->lineSize()[2]);

  SDL_RenderClear(m_sdl2Render);
  SDL_RenderCopy(m_sdl2Render, m_sdl2Texture, NULL, &m_renderRect);

	m_sdlView->ReleaseSdlRenderer(m_sdl2Render);
	m_lastFrame = frame;
}

#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

void SDL2BuiltinRender::resetRenderRect(void) {
  memset(&m_renderRect, 0, sizeof(m_renderRect));
  m_renderRect.w = m_curScreenW;
  m_renderRect.h = m_curScreenH;

  m_expectW = FFALIGN(m_curScreenW, 4);
  m_expectH = FFALIGN(m_curScreenH, 4);
}

void SDL2BuiltinRender::resetRect(int w, int h) {
  m_curScreenW = w;
  m_curScreenH = h;

  resetRenderRect();

  m_lastFrameW = 0;
  m_lastFrameH = 0;
}

bool SDL2BuiltinRender::resetTexture(SDL_Renderer * sdlRenderer,int pixel_w, int pixel_h) {
  if (pixel_w == m_lastPixelW && pixel_h == m_lastPixelH) {
    return true;
  }

  SLOGFMTI(
      "[SDL2BuiltinRender] resetTexture, old rect: [%d, %d], new rect: [%d, "
      "%d].",
      m_lastPixelW, m_lastPixelH, pixel_w, pixel_h);

  m_lastPixelW = pixel_w;
  m_lastPixelH = pixel_h;
  if (m_sdl2Texture) {
    SDL_DestroyTexture(m_sdl2Texture);
    m_sdl2Texture = NULL;
  }

  m_sdl2Texture =
      SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV,
                        SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);
  return m_sdl2Texture != NULL;
}

inline bool isDoubleEqual(double a, double b)
{
	const double kEpsilon = 10e-9;
	return (fabs(a - b) < kEpsilon);
}

void SDL2BuiltinRender::checkUpdateRect(const IAVframe* frame) {
  /**
   * SDL2方案的实现细节
   *
   * 窗口宽度：screen_w
   * 窗口高度: screen_h
   * yuv的scale后的宽度：pixel_w
   * yuv的scale后的高度：pixel_h
   *
   * yuv大小等于窗口大小时，直接渲染，不做其他处理。不等时，根据不同的播放模式，
   * 有下面几种处理方式：
   * 1. 拉伸全屏模式：拉伸/缩小yuv到窗口大小
   * 2. 自适应全屏：计算窗口的宽高比(screen_ratio)与yuv的宽高比(pixel_ratio)，
   *    比较两者的大小关系。
   *    a. screen_ratio等于pixel_ratio，则直接拉伸/缩小yuv到窗口大小；
   *    b. screen_ratio大于pixel_ratio，则说明screen_w相对于yuv的宽度而言，
   *       是“偏长”的，则yuv应进行原比例伸缩，直到pixel_h等于screen_h，而此时的
   *       pixel_w小于screen_w，则SDL渲染时yuv的x轴应向右偏移
   *       (screen_w - pixel_w) / 2；
   *    c. screen_ratio小于pixel_ratio，类似情况b，yuv应该进行原比例伸缩，直到
   *       pixel_w等于screen_w，而此时pixel_h小于screen_h，则SDL渲染时yuv的y轴
   *       应向下方偏移(screen_h - pixel_h) / 2；
   * 3. 原始播放，分两种情况：
   *    a. 当yuv宽高都小于窗口宽高时，不对yuv进行伸缩，直接渲染
   *    b. 按照自适应全屏的模式进行处理。
   * 4. 裁剪全屏：计算窗口的宽高比(screen_ratio)与yuv的宽高比(pixel_ratio)，
   *    比较两者的大小关系。
   *    a. screen_ratio等于pixel_ratio，则直接拉伸/缩小yuv到窗口大小
   *    b. screen_ratio大于pixel_ratio，说明screen_w偏长，yuv进行原比例伸缩，直
   *       到pixel_w等于screen_w，而此时pixel_h已经大于screen_h，则SDL渲染时yuv的
   *       y轴应该向上偏移(screen_h - pixel_h) / 2，此时偏移量为负值，即会有部分
   *       图像画出窗口外。
   *    c.
   * screen_ratio小于pixel_ratio，说明screen_h偏长，yuv进行原比例伸缩，直到
   *       pixel_h等于screen_h，此时pixel_w大于screen_w，则SDL渲染时yuv的x轴应该
   *       左偏移向(screen_w - pixel_w) / 2，偏移量为负值。
   */
  if (frame->width() == m_lastFrameW && frame->height() == m_lastFrameH) {
    // 同样的帧尺寸已经处理过了，不需要再次进行计算
    return;
  }

  m_lastFrameW = frame->width();
  m_lastFrameH = frame->height();

  resetRenderRect();
  if ((frame->width() == m_curScreenW && frame->height() == m_curScreenH) ||
      VIDEO_RENDER_MODE_FILL == m_renderMode) {
    return;
  }

  if (VIDEO_RENDER_MODE_ORIGINAL == m_renderMode) {
    if (frame->width() < m_curScreenW && frame->height() < m_curScreenH) {
      m_renderRect.w = frame->width();
      m_renderRect.h = frame->height();
      m_expectW = FFALIGN(frame->width(), 4);
      m_expectH = FFALIGN(frame->height(), 4);

      m_renderRect.x = (m_curScreenW - m_expectW) / 2;
      m_renderRect.y = (m_curScreenH - m_expectH) / 2;
      return;
    }
  }

  if (m_curScreenW != frame->width() || m_curScreenH != frame->height()) {
    double screen_ratio = (double)m_curScreenW / m_curScreenH;
    double pixel_ratio = (double)frame->width() / frame->height();
    if (isDoubleEqual(screen_ratio, pixel_ratio)) {
      return;
    }

    if (m_renderMode == VIDEO_RENDER_MODE_ASPECT_FIT || m_renderMode == VIDEO_RENDER_MODE_ORIGINAL) {
      if (screen_ratio > pixel_ratio) {
        m_expectH = m_curScreenH;
        m_expectW =
            (int)(frame->width() * ((double)m_expectH / frame->height()));
        m_renderRect.x = (m_curScreenW - m_expectW) / 2;
      } else {
        m_expectW = m_curScreenW;
        m_expectH =
            (int)(frame->height() * ((double)m_expectW / frame->width()));
        m_renderRect.y = (m_curScreenH - m_expectH) / 2;
      }
    } else {
      if (!isDoubleEqual(screen_ratio, pixel_ratio)) {
        if (screen_ratio > pixel_ratio) {
          m_expectW = m_curScreenW;
          m_expectH =
              (int)(frame->height() * ((double)m_expectW / frame->width()));
          m_renderRect.y = (m_curScreenH - m_expectH) / 2;
        } else {
          m_expectH = m_curScreenH;
          m_expectW =
              (int)(frame->width() * ((double)m_expectH / frame->height()));
          m_renderRect.x = (m_curScreenW - m_expectW) / 2;
        }
      }
    }
    m_renderRect.w = m_expectW;
    m_renderRect.h = m_expectH;

    m_expectW = FFALIGN(m_expectW, 4);
    m_expectH = FFALIGN(m_expectH, 4);
  }
  return;
}

void SDL2BuiltinRender::rerender()
{
	if(m_lastFrame)
	{
		IAVframe *frame = m_lastFrame;
		UpdateRenderRect(frame);

		IAVframe* dstFrame = NULL;
		AVPixelFormat targetFormat = format();

		if (expectWidth() != frame->width() || expectHeight() != frame->height()
			|| frame->format() != targetFormat) {
				int res = m_pTransVod->scale(frame, expectWidth(), expectHeight(), targetFormat, &dstFrame);
				if (res != expectHeight()) {
					return;
				}
		}
		else {
			dstFrame = frame;
		}

		doRendering(dstFrame);
	}
}


VOD_NS_END