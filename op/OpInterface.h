﻿// OpInterface.h: OpInterface 的声明

#pragma once
#include "resource.h"       // 主符号



#include "op_i.h"
#include "Common.h"
#include "WinApi.h"
#include "BKbase.h"
#include "ImageProc.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif

using namespace ATL;


// OpInterface

class ATL_NO_VTABLE OpInterface :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<OpInterface, &CLSID_OpInterface>,
	public IDispatchImpl<IOpInterface, &IID_IOpInterface, &LIBID_opLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	OpInterface();

DECLARE_REGISTRY_RESOURCEID(106)


BEGIN_COM_MAP(OpInterface)
	COM_INTERFACE_ENTRY(IOpInterface)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
private:
	//一些共用变量

	//1. Windows API
	WinApi _winapi;
	// background module
	bkbase _bkproc;
	// work path
	std::wstring _curr_path;
	//image process
	ImageProc _image_proc;
	std::map<wstring, long> _vkmap;
public:
	//---------------基本设置/属性-------------------

	//1.版本号Version
	STDMETHOD(Ver)(BSTR* ret);
	//设置目录
	STDMETHOD(SetPath)(BSTR path, LONG* ret);
	//获取目录
	STDMETHOD(GetPath)(BSTR*path);
	//获取插件目录
	STDMETHOD(GetBasePath)(BSTR* path);
	//设置是否弹出错误信息,默认是打开 0为关闭，1为显示为信息框，2为保存到文件
	STDMETHOD(SetShowErrorMsg)(LONG show_type, LONG* ret);
	
	//sleep
	STDMETHOD(Sleep)(LONG millseconds, LONG* ret);
	//Process
	//inject dll
	STDMETHOD(InjectDll)(BSTR process_name,BSTR dll_name, LONG* ret);
	//设置是否开启或者关闭插件内部的图片缓存机制
	STDMETHOD(EnablePicCache)(LONG enable, LONG* ret);
	//---------------------algorithm-------------------------------
	//A星算法
	STDMETHOD(AStarFindPath)(LONG mapWidth,LONG mapHeight,BSTR disable_points,LONG beginX,LONG beginY, LONG endX,LONG endY,BSTR* path);
	//--------------------windows api------------------------------
	//根据指定条件,枚举系统中符合条件的窗口
	STDMETHOD(EnumWindow)(LONG parent, BSTR title, BSTR class_name, LONG filter, BSTR* retstr);
	//根据指定进程以及其它条件,枚举系统中符合条件的窗口
	STDMETHOD(EnumWindowByProcess)(BSTR process_name, BSTR title, BSTR class_name, LONG filter, BSTR* retstring);
	//根据指定进程名,枚举系统中符合条件的进程PID
	STDMETHOD(EnumProcess)(BSTR name, BSTR* retstring);
	//把窗口坐标转换为屏幕坐标
	STDMETHOD(ClientToScreen)(LONG ClientToScreen, VARIANT* x, VARIANT* y, LONG* bret);
	//查找符合类名或者标题名的顶层可见窗口
	STDMETHOD(FindWindow)(BSTR class_name, BSTR title, LONG* rethwnd);
	//根据指定的进程名字，来查找可见窗口
	STDMETHOD(FindWindowByProcess)(BSTR process_name, BSTR class_name, BSTR title, LONG* rethwnd);
	//根据指定的进程Id，来查找可见窗口
	STDMETHOD(FindWindowByProcessId)(LONG process_id, BSTR class_name, BSTR title, LONG* rethwnd);
	//查找符合类名或者标题名的顶层可见窗口,如果指定了parent,则在parent的第一层子窗口中查找
	STDMETHOD(FindWindowEx)(LONG parent, BSTR class_name, BSTR title, LONG* rethwnd);
	//获取窗口客户区域在屏幕上的位置
	STDMETHOD(GetClientRect)(LONG hwnd, VARIANT* x1, VARIANT* y1, VARIANT* x2, VARIANT* y2, LONG* nret);
	//获取窗口客户区域的宽度和高度
	STDMETHOD(GetClientSize)(LONG hwnd, VARIANT* width, VARIANT* height, LONG* nret);
	//获取顶层活动窗口中具有输入焦点的窗口句柄
	STDMETHOD(GetForegroundFocus)(LONG* rethwnd);
	//获取顶层活动窗口,可以获取到按键自带插件无法获取到的句柄
	STDMETHOD(GetForegroundWindow)(LONG* rethwnd);
	//获取鼠标指向的可见窗口句柄
	STDMETHOD(GetMousePointWindow)(LONG* rethwnd);
	//获取给定坐标的可见窗口句柄
	STDMETHOD(GetPointWindow)(LONG x, LONG y, LONG* rethwnd);
	//根据指定的pid获取进程详细信息
	STDMETHOD(GetProcessInfo)(LONG pid, BSTR* retstring);
	//获取特殊窗口
	STDMETHOD(GetSpecialWindow)(LONG flag, LONG* rethwnd);
	//获取给定窗口相关的窗口句柄
	STDMETHOD(GetWindow)(LONG hwnd, LONG flag, LONG* nret);
	//获取窗口的类名
	STDMETHOD(GetWindowClass)(LONG hwnd, BSTR* retstring);
	//获取指定窗口所在的进程ID
	STDMETHOD(GetWindowProcessId)(LONG hwnd, LONG* nretpid);
	//获取指定窗口所在的进程的exe文件全路径
	STDMETHOD(GetWindowProcessPath)(LONG hwnd, BSTR* retstring);
	//获取窗口在屏幕上的位置
	STDMETHOD(GetWindowRect)(LONG hwnd, VARIANT* x1, VARIANT* y1, VARIANT* x2, VARIANT* y2, LONG* nret);
	//获取指定窗口的一些属性
	STDMETHOD(GetWindowState)(LONG hwnd, LONG flag, LONG* rethwnd);
	//获取窗口的标题
	STDMETHOD(GetWindowTitle)(LONG hwnd, BSTR* rettitle);
	//移动指定窗口到指定位置
	STDMETHOD(MoveWindow)(LONG hwnd, LONG x, LONG y, LONG* nret);
	//把屏幕坐标转换为窗口坐标
	STDMETHOD(ScreenToClient)(LONG hwnd, VARIANT* x, VARIANT* y, LONG* nret);
	//向指定窗口发送粘贴命令
	STDMETHOD(SendPaste)(LONG hwnd, LONG* nret);
	//设置窗口客户区域的宽度和高度
	STDMETHOD(SetClientSize)(LONG hwnd, LONG width, LONG hight, LONG* nret);
	//设置窗口的状态
	STDMETHOD(SetWindowState)(LONG hwnd, LONG flag, LONG* nret);
	//设置窗口的大小
	STDMETHOD(SetWindowSize)(LONG hwnd, LONG width, LONG height, LONG* nret);
	//设置窗口的标题
	STDMETHOD(SetWindowText)(LONG hwnd, BSTR title, LONG* nret);
	//设置窗口的透明度
	STDMETHOD(SetWindowTransparent)(LONG hwnd, LONG trans, LONG* nret);
	//向指定窗口发送文本数据
	STDMETHOD(SendString)(LONG hwnd, BSTR str, LONG* ret);
	//向指定窗口发送文本数据-输入法
	STDMETHOD(SendStringIme)(LONG hwnd, BSTR str, LONG* ret);
	//运行可执行文件,可指定模式
	STDMETHOD(RunApp)(BSTR cmdline, LONG mode, LONG* ret);
	//运行可执行文件，可指定显示模式
	STDMETHOD(WinExec)(BSTR cmdline, LONG cmdshow, LONG* ret);

	//运行命令行并返回结果
	STDMETHOD(GetCmdStr)(BSTR cmd,LONG millseconds, BSTR* retstr);

	//--------------------Background -----------------------
	//bind window and beign capture screen
	STDMETHOD(BindWindow)(LONG hwnd, BSTR display, BSTR mouse, BSTR keypad, LONG mode,LONG *ret);
	//
	STDMETHOD(UnBindWindow)(LONG* ret);
	//--------------------mouse & keyboard------------------
	//获取鼠标位置.
	STDMETHOD(GetCursorPos)(VARIANT* x, VARIANT* y, LONG* ret);
	//鼠标相对于上次的位置移动rx,ry.
	STDMETHOD(MoveR)(LONG x, LONG y, LONG* ret);
	//把鼠标移动到目的点(x,y)
	STDMETHOD(MoveTo)(LONG x, LONG y, LONG* ret);
	//把鼠标移动到目的范围内的任意一点
	STDMETHOD(MoveToEx)(LONG x, LONG y,LONG w,LONG h, LONG* ret);
	//按下鼠标左键
	STDMETHOD(LeftClick)(LONG* ret);
	//双击鼠标左键
	STDMETHOD(LeftDoubleClick)(LONG* ret);
	//按住鼠标左键
	STDMETHOD(LeftDown)(LONG* ret);
	//弹起鼠标左键
	STDMETHOD(LeftUp)(LONG* ret);
	//按下鼠标中键
	STDMETHOD(MiddleClick)(LONG* ret);
	//按住鼠标中键
	STDMETHOD(MiddleDown)(LONG* ret);
	//弹起鼠标中键
	STDMETHOD(MiddleUp)(LONG* ret);
	//按下鼠标右键
	STDMETHOD(RightClick)(LONG* ret);
	//按住鼠标右键
	STDMETHOD(RightDown)(LONG* ret);
	//弹起鼠标右键
	STDMETHOD(RightUp)(LONG* ret);
	//滚轮向下滚
	STDMETHOD(WheelDown)(LONG* ret);
	//滚轮向上滚
	STDMETHOD(WheelUp)(LONG* ret);
	//获取指定的按键状态.(前台信息,不是后台)
	STDMETHOD(GetKeyState)(LONG vk_code, LONG* ret);
	//按住指定的虚拟键码
	STDMETHOD(KeyDown)(LONG vk_code, LONG* ret);
	//按住指定的虚拟键码
	STDMETHOD(KeyDownChar)(BSTR vk_code, LONG* ret);
	//弹起来虚拟键vk_code
	STDMETHOD(KeyUp)(LONG vk_code, LONG* ret);
	//弹起来虚拟键vk_code
	STDMETHOD(KeyUpChar)(BSTR vk_code, LONG* ret);
	//等待指定的按键按下 (前台,不是后台)
	STDMETHOD(WaitKey)(LONG vk_code,LONG time_out, LONG* ret);
	//发送字符串
	//STDMETHOD(SendString)(LONG HWND)
	//弹起来虚拟键vk_code
	STDMETHOD(KeyPress)(LONG vk_code, LONG* ret);
	STDMETHOD(KeyPressChar)(BSTR vk_code, LONG* ret);

	//--------------------image and color-----------------------
	//抓取指定区域(x1, y1, x2, y2)的图像, 保存为file
	STDMETHOD(Capture)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR file_name, LONG* ret);
	//比较指定坐标点(x,y)的颜色
	STDMETHOD(CmpColor)(LONG x, LONG y,BSTR color,DOUBLE sim, LONG* ret);
	//查找指定区域内的颜色
	STDMETHOD(FindColor)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR color,DOUBLE sim,LONG dir, VARIANT* x, VARIANT* y, LONG* ret);
	//查找指定区域内的所有颜色
	STDMETHOD(FindColorEx)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR color, DOUBLE sim,LONG dir, BSTR* retstr);
	//根据指定的多点查找颜色坐标
	STDMETHOD(FindMultiColor)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR first_color, BSTR offset_color, DOUBLE sim, LONG dir, VARIANT* x, VARIANT* y, LONG* ret);
	//根据指定的多点查找所有颜色坐标
	STDMETHOD(FindMultiColorEx)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR first_color, BSTR offset_color, DOUBLE sim, LONG dir,BSTR* retstr);
	//查找指定区域内的图片
	STDMETHOD(FindPic)(LONG x1,LONG y1,LONG x2,LONG y2,BSTR files, BSTR delta_color,DOUBLE sim,LONG dir,VARIANT* x,VARIANT* y,LONG* ret);
	//查找多个图片
	STDMETHOD(FindPicEx)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR files, BSTR delta_color, DOUBLE sim, LONG dir,BSTR* retstr);
	//获取(x,y)的颜色
	STDMETHOD(GetColor)(LONG x, LONG y, BSTR* ret);

	//----------------------ocr-------------------------
	//设置字库文件
	STDMETHOD(SetDict)(LONG idx, BSTR file_name, LONG* ret);
	//使用哪个字库文件进行识别
	STDMETHOD(UseDict)(LONG idx,  LONG* ret);
	//识别屏幕范围(x1,y1,x2,y2)内符合color_format的字符串,并且相似度为sim,sim取值范围(0.1-1.0),
	STDMETHOD(Ocr)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR color, DOUBLE sim,BSTR* ret_str);
	//回识别到的字符串，以及每个字符的坐标.
	STDMETHOD(OcrEx)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR color, DOUBLE sim, BSTR* ret_str);
	//在屏幕范围(x1,y1,x2,y2)内,查找string(可以是任意个字符串的组合),并返回符合color_format的坐标位置
	STDMETHOD(FindStr)(LONG x1, LONG y1, LONG x2, LONG y2,BSTR strs, BSTR color, DOUBLE sim, VARIANT* retx,VARIANT* rety,LONG* ret);
	//返回符合color_format的所有坐标位置
	STDMETHOD(FindStrEx)(LONG x1, LONG y1, LONG x2, LONG y2, BSTR strs, BSTR color, DOUBLE sim,BSTR* retstr);
	//识别屏幕范围(x1,y1,x2,y2)内的字符串,自动二值化，而无需指定颜色
	STDMETHOD(OcrAuto)(LONG x1, LONG y1, LONG x2, LONG y2, DOUBLE sim, BSTR* ret_str);
	//从文件中识别图片
	STDMETHOD(OcrFromFile)(BSTR file_name,BSTR color_format, DOUBLE sim, BSTR* retstr);
	//从文件中识别图片,无需指定颜色
	STDMETHOD(OcrAutoFromFile)(BSTR file_name, DOUBLE sim, BSTR* retstr);
	

};

OBJECT_ENTRY_AUTO(__uuidof(OpInterface), OpInterface)

