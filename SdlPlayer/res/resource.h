//stamp:0761029cf44fa687
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#ifndef _UIRES_H_
#define _UIRES_H_
	struct _UIRES{
		struct _UIDEF{
			const TCHAR * XML_INIT;
			}UIDEF;
		struct _LAYOUT{
			const TCHAR * XML_MAINWND;
			}LAYOUT;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			}values;
		struct _ICON{
			const TCHAR * ICON_LOGO;
			}ICON;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("LAYOUT:XML_MAINWND"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
		},
		{
			_T("ICON:ICON_LOGO"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * btn_play;
		 const wchar_t * btn_stop;
		 const wchar_t * edit_url;
		 const wchar_t * scroll_subtitles;
		 const wchar_t * sdl_back;
		 const wchar_t * sdl_front;
		 const wchar_t * sdl_view;
		 const wchar_t * slider_prog;
		 const wchar_t * txt_duration;
	}name;
	struct _id{
		int btn_play;
		int btn_stop;
		int edit_url;
		int scroll_subtitles;
		int sdl_back;
		int sdl_front;
		int sdl_view;
		int slider_prog;
		int txt_duration;
	}id;
	struct _color{
		int blue;
		int gray;
		int green;
		int red;
		int white;
	}color;
	struct _string{
		int title;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_play",
		L"btn_stop",
		L"edit_url",
		L"scroll_subtitles",
		L"sdl_back",
		L"sdl_front",
		L"sdl_view",
		L"slider_prog",
		L"txt_duration"
	}
	,
	{
		65539,
		65544,
		65538,
		65541,
		65537,
		65540,
		65536,
		65542,
		65543
	}
	,
	{
		0,
		1,
		2,
		3,
		4
	}
	,
	{
		0,
		1
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
