﻿// OpInterface.cpp: OpInterface 的实现

#include "stdafx.h"
#include "libop.h"
#include "Common.h"
#include "WinApi.h"
#include "BKbase.h"
#include "ImageProc.h"
#include "Cmder.h"
#include "Injecter.h"
#include "Tool.h"
#include "AStar.hpp"
#include <filesystem>
// OpInterface

op::op() {
	_winapi = new WinApi;
	_bkproc = new bkbase;
	_image_proc = new ImageProc;

	//初始化目录
	wchar_t buff[256];
	::GetCurrentDirectoryW(256, buff);
	_curr_path = buff;
	_image_proc->_curr_path = _curr_path;
	//初始化键码表
	_vkmap[L"back"] = VK_BACK; _vkmap[L"ctrl"] = VK_CONTROL;
	_vkmap[L"alt"] = LVKF_ALT; _vkmap[L"shift"] = VK_SHIFT;
	_vkmap[L"win"] = VK_LWIN;
	_vkmap[L"space"] = L' '; _vkmap[L"tab"] = VK_TAB;
	_vkmap[L"esc"] = VK_CANCEL;
	_vkmap[L"enter"] = L'\r'; _vkmap[L"up"] = VK_UP;
	_vkmap[L"down"] = VK_DOWN; _vkmap[L"left"] = VK_LEFT;
	_vkmap[L"right"] = VK_RIGHT;
	_vkmap[L"f1"] = VK_F1; _vkmap[L"f2"] = VK_F2;
	_vkmap[L"f3"] = VK_F3; _vkmap[L"f4"] = VK_F4;
	_vkmap[L"f5"] = VK_F5; _vkmap[L"f6"] = VK_F6;
	_vkmap[L"f7"] = VK_F7; _vkmap[L"f8"] = VK_F8;
	_vkmap[L"f9"] = VK_F9; _vkmap[L"f10"] = VK_F10;
	_vkmap[L"f11"] = VK_F11; _vkmap[L"f12"] = VK_F12;
	//初始化 op 路径 & name
	static bool is_init = false;
	if (!is_init) {
		g_op_path.resize(512);
		DWORD real_size = ::GetModuleFileNameW(gInstance, g_op_path.data(), 512);
		g_op_path.resize(real_size);

		g_op_name = g_op_path.substr(g_op_path.rfind(L"\\") + 1);
		g_op_path = g_op_path.substr(0, g_op_path.rfind(L"\\"));

		is_init = true;
	}
}

op::~op() {
	delete _winapi;
	delete _bkproc;
	delete _image_proc;
}

long  op::Ver(std::wstring& ret) {
	
	//Tool::setlog("address=%d,str=%s", ver, ver);

	return S_OK;
}

long  op::SetPath(const wchar_t* path, long* ret) {
	std::filesystem::path p = path;
	auto fullpath = std::filesystem::absolute(p);
	_curr_path = fullpath.generic_wstring();
	_image_proc->_curr_path = _curr_path;
	*ret = 1;
	return S_OK;
}

long  op::GetPath(std::wstring& path) {
	return S_OK;
}

long  op::GetBasePath(std::wstring& path){
	wchar_t basepath[256];
	::GetModuleFileName(gInstance, basepath, 256);
	return S_OK;
}

long  op::SetShowErrorMsg(long show_type, long* ret){
	gShowError = show_type;
	*ret = 1;
	return S_OK;
}



long  op::Sleep(long millseconds, long* ret) {
	::Sleep(millseconds);
	*ret = 1;
	return S_OK;
}

long  op::InjectDll(const wchar_t* process_name, const wchar_t* dll_name, long* ret) {
	//auto proc = _wsto_string(process_name);
	//auto dll = _wsto_string(dll_name);
	//Injecter::EnablePrivilege(TRUE);
	//auto h = Injecter::InjectDll(process_name, dll_name);
	*ret = 0;
	return S_OK;
}

long  op::EnablePicCache(long enable, long* ret) {
	_image_proc->_enable_cache = enable;
	*ret = 1;
	return S_OK;
}

long  op::AStarFindPath(long mapWidth, long mapHeight, const wchar_t* disable_points, long beginX, long beginY, long endX, long endY, std::wstring& path) {
	AStar as;
	vector<Vector2i>walls;
	vector<wstring> vstr;
	Vector2i tp;
	split(disable_points, vstr, L"|");
	for (auto&it : vstr) {
		if (swscanf(it.c_str(), L"%d,%d", &tp[0], &tp[1]) != 2)
			break;
		walls.push_back(tp);
	}
	list<Vector2i> paths;
	
	as.set_map( mapWidth, mapHeight , walls);
	as.findpath(beginX,beginY , endX,endY , paths);
	wstring pathstr;
	wchar_t buf[20];
	for (auto it = paths.rbegin(); it != paths.rend(); ++it) {
		auto v = *it;
		wsprintf(buf, L"%d,%d", v[0], v[1]);
		pathstr += buf;
		pathstr.push_back(L'|');
	}
	if (!pathstr.empty())
		pathstr.pop_back();
	return S_OK;
}


long  op::EnumWindow(long parent, const wchar_t* title, const wchar_t* class_name, long filter, std::wstring& retstr)
{
	// TODO: 在此添加实现代码
	std::unique_ptr<wchar_t> retstring(new wchar_t[MAX_PATH * 200]);
	memset(retstring.get(), 0, sizeof(wchar_t)*MAX_PATH * 200);
	_winapi->EnumWindow((HWND)parent, title, class_name, filter, retstring.get());
	//*retstr=_bstr_t(retstring);
	retstr = retstring.get();
	return 0;
}

long  op::EnumWindowByProcess(const wchar_t* process_name, const wchar_t* title, const wchar_t* class_name, long filter, std::wstring& retstring)
{
	// TODO: 在此添加实现代码
	std::unique_ptr<wchar_t> retstr(new wchar_t[MAX_PATH * 200]);
	memset(retstr.get(), 0, sizeof(wchar_t) * MAX_PATH * 200);
	_winapi->EnumWindow((HWND)0, title, class_name, filter, retstr.get(), process_name);
	//*retstring=_bstr_t(retstr);
	
	retstring = retstr.get();
	return S_OK;
}

long  op::EnumProcess(const wchar_t* name, std::wstring& retstring)
{
	// TODO: 在此添加实现代码
	std::unique_ptr<wchar_t> retstr(new wchar_t[MAX_PATH * 200]);
	memset(retstr.get(), 0, sizeof(wchar_t) * MAX_PATH * 200);
	_winapi->EnumProcess(name, retstr.get());
	//*retstring=_bstr_t(retstr);
	retstring = retstr.get();
	return S_OK;
}

long  op::ClientToScreen(long ClientToScreen, long* x, long* y, long* bret)
{
	// TODO: 在此添加实现代码

	*bret = _winapi->ClientToScreen(ClientToScreen, *x, *y);
	return S_OK;
}

long  op::FindWindow(const wchar_t* class_name, const wchar_t* title, long* rethwnd)
{
	// TODO: 在此添加实现代码
	*rethwnd = _winapi->FindWindow(class_name, title);
	return S_OK;
}

long  op::FindWindowByProcess(const wchar_t* process_name, const wchar_t* class_name, const wchar_t* title, long* rethwnd)
{
	// TODO: 在此添加实现代码
	_winapi->FindWindowByProcess(class_name, title, *rethwnd, process_name);
	return S_OK;
}

long  op::FindWindowByProcessId(long process_id, const wchar_t* class_name, const wchar_t* title, long* rethwnd)
{
	// TODO: 在此添加实现代码
	_winapi->FindWindowByProcess(class_name, title, *rethwnd, NULL, process_id);
	return S_OK;
}

long  op::FindWindowEx(long parent, const wchar_t* class_name, const wchar_t* title, long* rethwnd)
{
	// TODO: 在此添加实现代码
	*rethwnd = _winapi->FindWindowEx(parent,class_name, title);
	return S_OK;
}

long  op::GetClientRect(long hwnd, long* x1, long* y1, long* x2, long* y2, long* nret)
{
	// TODO: 在此添加实现代码
	
	*nret = _winapi->GetClientRect(hwnd, *x1, *y1, *x2, *y2);
	return S_OK;
}


long  op::GetClientSize(long hwnd, long* width, long* height, long* nret)
{
	// TODO: 在此添加实现代码
	
	*nret = _winapi->GetClientSize(hwnd, *width, *height);
	return S_OK;
}

long  op::GetForegroundFocus(long* rethwnd)
{
	// TODO: 在此添加实现代码
	*rethwnd = (LONG)::GetFocus();
	return S_OK;
}

long  op::GetForegroundWindow(long* rethwnd)
{
	// TODO: 在此添加实现代码
	*rethwnd = (LONG)::GetForegroundWindow();
	return S_OK;
}

long  op::GetMousePointWindow(long* rethwnd)
{
	// TODO: 在此添加实现代码
	//::Sleep(2000);
	_winapi->GetMousePointWindow(*rethwnd);
	return S_OK;
}

long  op::GetPointWindow(long x, long y, long* rethwnd)
{
	// TODO: 在此添加实现代码
	_winapi->GetMousePointWindow(*rethwnd, x, y);
	return S_OK;
}

long  op::GetProcessInfo(long pid, std::wstring& retstring)
{
	// TODO: 在此添加实现代码

	wchar_t retstr[MAX_PATH] = { 0 };
	_winapi->GetProcessInfo(pid, retstr);
	//* retstring=_bstr_t(retstr);
	
	retstring = retstr;
	return S_OK;
}

long  op::GetSpecialWindow(long flag, long* rethwnd)
{
	// TODO: 在此添加实现代码
	*rethwnd = 0;
	if (flag == 0)
		*rethwnd = (LONG)GetDesktopWindow();
	else if (flag == 1)
	{
		*rethwnd = (LONG)::FindWindow(L"Shell_TrayWnd", NULL);
	}

	return S_OK;
}

long  op::GetWindow(long hwnd, long flag, long* nret)
{
	// TODO: 在此添加实现代码
	_winapi->TSGetWindow(hwnd, flag, *nret);
	return S_OK;
}

long  op::GetWindowClass(long hwnd, std::wstring& retstring)
{
	// TODO: 在此添加实现代码
	wchar_t classname[MAX_PATH] = { 0 };
	::GetClassName((HWND)hwnd, classname, MAX_PATH);
	//* retstring=_bstr_t(classname);
	
	retstring = classname;
	return S_OK;
}

long  op::GetWindowProcessId(long hwnd, long* nretpid)
{
	// TODO: 在此添加实现代码
	DWORD pid = 0;
	::GetWindowThreadProcessId((HWND)hwnd, &pid);
	*nretpid = pid;
	return S_OK;
}

long  op::GetWindowProcessPath(long hwnd, std::wstring& retstring)
{
	// TODO: 在此添加实现代码
	DWORD pid = 0;
	::GetWindowThreadProcessId((HWND)hwnd, &pid);
	wchar_t process_path[MAX_PATH] = { 0 };
	_winapi->GetProcesspath(pid, process_path);
	//* retstring=_bstr_t(process_path);
	
	retstring = process_path;
	return S_OK;
}

long  op::GetWindowRect(long hwnd, long* x1, long* y1, long* x2, long* y2, long* nret)
{
	// TODO: 在此添加实现代码

	RECT winrect;
	*nret = ::GetWindowRect((HWND)hwnd, &winrect);
	*x1 = winrect.left;
	*y1 = winrect.top;
	*x2 = winrect.right;
	*y2 = winrect.bottom;
	return S_OK;
}

long  op::GetWindowState(long hwnd, long flag, long* rethwnd)
{
	// TODO: 在此添加实现代码
	*rethwnd = _winapi->GetWindowState(hwnd, flag);
	return S_OK;
}

long  op::GetWindowTitle(long hwnd, std::wstring& rettitle)
{
	// TODO: 在此添加实现代码
	wchar_t title[MAX_PATH] = { 0 };
	::GetWindowText((HWND)hwnd, title, MAX_PATH);
	//* rettitle=_bstr_t(title);
	
	rettitle = title;
	return S_OK;
}

long  op::MoveWindow(long hwnd, long x, long y, long* nret)
{
	// TODO: 在此添加实现代码
	RECT winrect;
	::GetWindowRect((HWND)hwnd, &winrect);
	int width = winrect.right - winrect.left;
	int hight = winrect.bottom - winrect.top;
	*nret = ::MoveWindow((HWND)hwnd, x, y, width, hight, false);
	return S_OK;
}

long  op::ScreenToClient(long hwnd, long* x, long* y, long* nret)
{
	// TODO: 在此添加实现代码
	
	POINT point;
	*nret = ::ScreenToClient((HWND)hwnd, &point);
	*x = point.x;
	*y = point.y;
	return S_OK;
}

long  op::SendPaste(long hwnd, long* nret)
{
	// TODO: 在此添加实现代码
	*nret = _winapi->SendPaste(hwnd);
	return S_OK;
}

long  op::SetClientSize(long hwnd, long width, long hight, long* nret)
{
	// TODO: 在此添加实现代码
	*nret = _winapi->SetWindowSize(hwnd, width, hight);
	return S_OK;
}

long  op::SetWindowState(long hwnd, long flag, long* nret)
{
	// TODO: 在此添加实现代码  
	*nret = _winapi->SetWindowState(hwnd, flag);
	return S_OK;
}

long  op::SetWindowSize(long hwnd, long width, long height, long* nret)
{
	// TODO: 在此添加实现代码
	*nret = _winapi->SetWindowSize(hwnd, width, height, 1);
	return S_OK;
}

long  op::SetWindowText(long hwnd, const wchar_t* title, long* nret)
{
	// TODO: 在此添加实现代码  
	//*nret=gWindowObj.TSSetWindowState(hwnd,flag);
	*nret = ::SetWindowText((HWND)hwnd, title);
	return S_OK;
}

long  op::SetWindowTransparent(long hwnd, long trans, long* nret)
{
	// TODO: 在此添加实现代码
	*nret = _winapi->SetWindowTransparent(hwnd, trans);
	return S_OK;
}

long  op::SendString(long hwnd, const wchar_t* str, long* ret) {
	*ret = _winapi->SendString((HWND)hwnd, str);
	return S_OK;
}

long  op::SendStringIme(long hwnd, const wchar_t* str, long* ret) {
	*ret = _winapi->SendStringIme((HWND)hwnd, str);
	return S_OK;
}

long  op::RunApp(const wchar_t* cmdline, long mode, long* ret) {
	*ret = _winapi->RunApp(cmdline, mode);
	return S_OK;
}

long  op::WinExec(const wchar_t* cmdline, long cmdshow, long* ret) {
	auto str = _ws2string(cmdline);
	*ret = ::WinExec(str.c_str(), cmdshow) > 31 ? 1 : 0;
	return S_OK;
}

long  op::GetCmdStr(const wchar_t* cmd, long millseconds, std::wstring& retstr) {
	auto strcmd = _ws2string(cmd);
	Cmder cd;
	auto str = cd.GetCmdStr(strcmd, millseconds <= 0 ? 5 : millseconds);
	return 0;
}



long  op::BindWindow(long hwnd, const wchar_t* display, const wchar_t* mouse, const wchar_t* keypad, long mode, long *ret) {
	if (_bkproc->IsBind())
		_bkproc->UnBindWindow();
	*ret = _bkproc->BindWindow(hwnd, display, mouse, keypad, mode);
	if (*ret == 1) {
		_image_proc->set_offset(_bkproc->_pbkdisplay->_client_x, _bkproc->_pbkdisplay->_client_y);
	}
	return S_OK;
}

long  op::UnBindWindow(long* ret) {
	*ret = _bkproc->UnBindWindow();
	return S_OK;
}

long  op::GetCursorPos(long* x, long* y, long* ret) {
	
	*ret = _bkproc->_bkmouse.GetCursorPos(*x, *y);
	return S_OK;
}

long  op::MoveR(long x, long y, long* ret) {
	*ret = _bkproc->_bkmouse.MoveR(x, y);
	return S_OK;
}
//把鼠标移动到目的点(x,y)
long  op::MoveTo(long x, long y, long* ret) {
	*ret = _bkproc->_bkmouse.MoveTo(x, y);
	return S_OK;
}

long  op::MoveToEx(long x, long y, long w, long h, long* ret) {
	*ret = _bkproc->_bkmouse.MoveToEx(x, y, w, h);
	return S_OK;
}

long  op::LeftClick(long* ret) {
	*ret = _bkproc->_bkmouse.LeftClick();
	return S_OK;
}

long  op::LeftDoubleClick(long* ret) {
	*ret = _bkproc->_bkmouse.LeftDoubleClick();
	return S_OK;
}

long  op::LeftDown(long* ret) {
	*ret = _bkproc->_bkmouse.LeftDown();
	return S_OK;
}

long  op::LeftUp(long* ret) {
	*ret = _bkproc->_bkmouse.LeftUp();
	return S_OK;
}

long  op::MiddleClick(long* ret) {
	*ret = _bkproc->_bkmouse.MiddleClick();
	return S_OK;
}

long  op::MiddleDown(long* ret) {
	*ret = _bkproc->_bkmouse.MiddleDown();
	return S_OK;
}

long  op::MiddleUp(long* ret) {
	*ret = _bkproc->_bkmouse.MiddleUp();
	return S_OK;
}

long  op::RightClick(long* ret) {
	*ret = _bkproc->_bkmouse.RightClick();
	return S_OK;
}

long  op::RightDown(long* ret) {
	*ret = _bkproc->_bkmouse.RightDown();
	return S_OK;
}

long  op::RightUp(long* ret) {
	*ret = _bkproc->_bkmouse.RightUp();
	return S_OK;
}

long  op::WheelDown(long* ret) {
	*ret = _bkproc->_bkmouse.WheelDown();
	return S_OK;
}

long  op::WheelUp(long* ret) {
	*ret = _bkproc->_bkmouse.WheelUp();
	return S_OK;
}

long  op::GetKeyState(long vk_code, long* ret) {
	*ret = _bkproc->_keypad.GetKeyState(vk_code);
	return S_OK;
}

long  op::KeyDown(long vk_code, long* ret) {
	*ret = _bkproc->_keypad.KeyDown(vk_code);
	return S_OK;
}

long  op::KeyDownChar(const wchar_t* vk_code, long* ret) {
	auto nlen = wcslen(vk_code);
	*ret = 0;
	if (nlen > 0) {
		long vk = _vkmap.count(vk_code) ? _vkmap[vk_code] : vk_code[0];
		*ret = _bkproc->_keypad.KeyDown(vk);
	}
	
	return S_OK;
}

long  op::KeyUp(long vk_code, long* ret) {
	*ret = _bkproc->_keypad.KeyUp(vk_code);
	return S_OK;
}

long  op::KeyUpChar(const wchar_t* vk_code, long* ret) {
	auto nlen = wcslen(vk_code);
	*ret = 0;
	if (nlen > 0) {
		long vk = _vkmap.count(vk_code) ? _vkmap[vk_code] : vk_code[0];
		*ret = _bkproc->_keypad.KeyUp(vk);
	}
	return S_OK;
}

long  op::WaitKey(long vk_code, long time_out, long* ret) {
	if (time_out < 0)time_out = 0;
	*ret = _bkproc->_keypad.WaitKey(vk_code, time_out);
	return S_OK;
}

long  op::KeyPress(long vk_code, long* ret) {
	
		*ret = _bkproc->_keypad.KeyPress(vk_code);
	
	return S_OK;
}

long  op::KeyPressChar(const wchar_t* vk_code, long* ret) {
	auto nlen = wcslen(vk_code);
	*ret = 0;
	if (nlen > 0) {
		long vk = _vkmap.count(vk_code) ? _vkmap[vk_code] : vk_code[0];
		*ret = _bkproc->_keypad.KeyPress(vk);
	}
	return S_OK;
}



//抓取指定区域(x1, y1, x2, y2)的图像, 保存为file
long  op::Capture(long x1, long y1, long x2, long y2, const wchar_t* file_name, long* ret) {
	
	*ret = 0;
	
	if (_bkproc->check_bind()&& _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		*ret = _image_proc->Capture(file_name);
	}
	return S_OK;
}
//比较指定坐标点(x,y)的颜色
long  op::CmpColor(long x, long y, const wchar_t* color, DOUBLE sim, long* ret) {
	//LONG rx = -1, ry = -1;
	*ret = 0;
	if (_bkproc->check_bind()) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			0, 0, _bkproc->get_widht(), _bkproc->get_height(), _bkproc->get_image_type());
		_bkproc->unlock_data();
		*ret = _image_proc->CmpColor(x, y, color, sim);
	}
	

	return S_OK;
}
//查找指定区域内的颜色
long  op::FindColor(long x1, long y1, long x2, long y2, const wchar_t* color, DOUBLE sim, long dir, long* x, long* y, long* ret) {
	
	*ret = 0;
	*x = *y = -1;
	
	if (_bkproc->check_bind() && _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		
		_bkproc->unlock_data();
		_image_proc->FindColor(color, sim, dir, *x, *y);
	}

	return S_OK;
}
//查找指定区域内的所有颜色
long  op::FindColorEx(long x1, long y1, long x2, long y2, const wchar_t* color, DOUBLE sim, long dir, std::wstring& retstr) {
	if (_bkproc->check_bind()&& _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		std::wstring str;
		_image_proc->FindColoEx(color, sim, dir, str);
	}
	return S_OK;
}
//根据指定的多点查找颜色坐标
long  op::FindMultiColor(long x1, long y1, long x2, long y2, const wchar_t* first_color, const wchar_t* offset_color, DOUBLE sim, long dir, long* x, long* y, long* ret) {
	
	*ret = 0;
	*x = *y = -1;
	
	if (_bkproc->check_bind()&& _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		*ret = _image_proc->FindMultiColor(first_color, offset_color, sim, dir, *x, *y);
		/*if (*ret) {
			rx += x1; ry += y1;
			rx -= _bkproc->_pbkdisplay->_client_x;
			ry -= _bkproc->_pbkdisplay->_client_y;
		}*/
	}
	
	return S_OK;
}
//根据指定的多点查找所有颜色坐标
long  op::FindMultiColorEx(long x1, long y1, long x2, long y2, const wchar_t* first_color, const wchar_t* offset_color, DOUBLE sim, long dir, std::wstring& retstr) {

	if (_bkproc->check_bind()&& _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		std::wstring str;
		_image_proc->FindMultiColorEx(first_color, offset_color, sim, dir, str);
	}
	return S_OK;
}
//查找指定区域内的图片
long  op::FindPic(long x1, long y1, long x2, long y2, const wchar_t* files, const wchar_t* delta_color, DOUBLE sim, long dir, long* x, long* y, long* ret) {
	
	*ret = 0;
	*x = *y = -1;
	
	if (_bkproc->check_bind()&& _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		*ret = _image_proc->FindPic(files, delta_color, sim, 0, *x, *y);
		/*if (*ret) {
			rx += x1; ry += y1;
			rx -= _bkproc->_pbkdisplay->_client_x;
			ry -= _bkproc->_pbkdisplay->_client_y;
		}*/
	}
	
	return S_OK;
}
//查找多个图片
long  op::FindPicEx(long x1, long y1, long x2, long y2, const wchar_t* files, const wchar_t* delta_color, DOUBLE sim, long dir, std::wstring& retstr) {
	
	if (_bkproc->check_bind() && _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		std::wstring str;
		_image_proc->FindPicEx(files, delta_color, sim, dir, str);
	}
	return 0;
}
//获取(x,y)的颜色
long  op::GetColor(long x, long y, std::wstring& ret) {
	color_t cr;
	if (_bkproc->check_bind()) {
		x += _bkproc->_pbkdisplay->_client_x;
		y += _bkproc->_pbkdisplay->_client_y;
	}
	if (x >= 0 && y >= 0 && x < _bkproc->get_widht() && y < _bkproc->get_height()) {
		auto p = _bkproc->GetScreenData() + (y*_bkproc->get_widht() * 4 + x * 4);
		cr = *(color_t*)p;
	}
	
	auto str = cr.tostr();

	return S_OK;
}



//设置字库文件
long  op::SetDict(long idx, const wchar_t* file_name, long* ret) {
	*ret = _image_proc->SetDict(idx, file_name);
	return S_OK;
}
//使用哪个字库文件进行识别
long  op::UseDict(long idx, long* ret) {
	*ret = _image_proc->UseDict(idx);
	return S_OK;
}
//识别屏幕范围(x1,y1,x2,y2)内符合color_format的字符串,并且相似度为sim,sim取值范围(0.1-1.0),
long  op::Ocr(long x1, long y1, long x2, long y2, const wchar_t* color, DOUBLE sim, std::wstring& ret_str) {
	wstring str;
	if (_bkproc->check_bind() && _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		_image_proc->OCR(color, sim, str);
	}

	return S_OK;
}
//回识别到的字符串，以及每个字符的坐标.
long  op::OcrEx(long x1, long y1, long x2, long y2, const wchar_t* color, DOUBLE sim, std::wstring& ret_str) {
	wstring str;
	if (_bkproc->check_bind() && _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		_image_proc->OcrEx(color, sim, str);
	}

	return S_OK;
}
//在屏幕范围(x1,y1,x2,y2)内,查找string(可以是任意个字符串的组合),并返回符合color_format的坐标位置
long  op::FindStr(long x1, long y1, long x2, long y2, const wchar_t* strs, const wchar_t* color, DOUBLE sim, long* retx, long* rety,long* ret) {
	wstring str;
	*retx = *rety = -1;
	if (_bkproc->check_bind() && _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		*ret = _image_proc->FindStr(strs, color, sim, *retx, *rety);
	}

	return S_OK;
}
//返回符合color_format的所有坐标位置
long  op::FindStrEx(long x1, long y1, long x2, long y2, const wchar_t* strs, const wchar_t* color, DOUBLE sim, std::wstring& retstr) {
	wstring str;
	if (_bkproc->check_bind() && _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		_image_proc->FindStrEx(strs, color, sim, str);
	}

	return S_OK;
}

long  op::OcrAuto(long x1, long y1, long x2, long y2, DOUBLE sim, std::wstring& retstr) {
	wstring str;
	if (_bkproc->check_bind() && _bkproc->RectConvert(x1, y1, x2, y2)) {
		_bkproc->lock_data();
		_image_proc->input_image(_bkproc->GetScreenData(), _bkproc->get_widht(), _bkproc->get_height(),
			x1, y1, x2, y2, _bkproc->get_image_type());
		_bkproc->unlock_data();
		_image_proc->OcrAuto(sim, str);
	}

	return S_OK;
}

//从文件中识别图片
long  op::OcrFromFile(const wchar_t* file_name, const wchar_t* color_format, DOUBLE sim, std::wstring& retstr) {
	wstring str;
	_image_proc->OcrFromFile(file_name, color_format, sim, str);
	return S_OK;
}
//从文件中识别图片,无需指定颜色
long  op::OcrAutoFromFile(const wchar_t* file_name, DOUBLE sim, std::wstring& retstr){
	wstring str;
	_image_proc->OcrAutoFromFile(file_name, sim, str);
	return S_OK;
}

