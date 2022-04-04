#pragma once
SNSBEGIN

class SScrollSubtitles : public SWindow, public ITimelineHandler
	{
		DEF_SOBJECT(SWindow,L"scrollSubtitles")
	public:
		SScrollSubtitles();
		~SScrollSubtitles();

		bool AddSubtitles(const SStringT & strSubtitle,int nType = 0);
	protected:// 通过 ITimelineHandler 继承
		void WINAPI OnNextFrame() override;

	protected:
		void WINAPI OnInitFinished(IXmlNode * xmlNode) override;

	protected:
		LRESULT OnAttrRows(const SStringW & strValue, BOOL bLoading);
		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"speed", m_nSpeed, FALSE)
			ATTR_CUSTOM(L"rows", OnAttrRows)
			ATTR_INT(L"xInterval", m_xInterval, TRUE)
			ATTR_INT(L"lineHeight", m_nLineHeight,TRUE)
			ATTR_INT(L"yOffset",m_yOffset,TRUE)
		SOUI_ATTRS_END()

	protected:
		void OnShowWindow(BOOL bShow, UINT nStatus);
		void OnPaint(IRenderTarget *pRT);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_SHOWWINDOW(OnShowWindow)
		SOUI_MSG_MAP_END()

	private:
		struct SubtitlesInfo
		{
			int  iRow;		//显示行
			long xOffset;   //显示偏移
			SIZE sz;		//占用空间
			SStringT strText; //显示内容
			int nType;		  //type
		};

		struct TypeFont {
			SAutoRefPtr<IFont> font;
			COLORREF		   color;
		};

		SList<SubtitlesInfo*> m_lstSubtitles;
		int					  m_nLineHeight;//行间距
		int					  m_xInterval;//水平间距
		int					  m_yOffset;
		int					  m_nSpeed;//每次步进的像素

		SArray<int>			  m_arrLineTail;//tail offset for lines.
		int					  m_iNextLine;  //next insert line

		SMap<int,TypeFont>      m_mapTypeFont;
		int					  m_nCount;
	};

SNSEND