#include "stdafx.h"
#include "xhook.h"

#include <d3d11.h>
//#include <D3DX11.h>
#include <d3d10.h>
//#include <d3dx10.h>
#include <d3d9.h>
#include "./include/sharedmem.h"
#include "./include/promutex.h"
#include <exception>


#include "3rd_party/include/kiero.h"
#include <gl\gl.h>
#include <gl\glu.h>
#include "Tool.h"
#include "query_api.h"
#include <wingdi.h>

HWND xhook::render_hwnd = NULL;
int xhook::render_type = 0;
wchar_t xhook::shared_res_name[256];
wchar_t xhook::mutex_name[256];
void* xhook::old_address;

static int is_capture;

using ATL::CComPtr;

//dx9 hooked EndScene function
HRESULT __stdcall dx9_hkEndScene(IDirect3DDevice9* thiz);
//dx10
HRESULT __stdcall dx10_hkPresent(IDXGISwapChain* thiz, UINT SyncInterval, UINT Flags);
//dx11
HRESULT __stdcall dx11_hkPresent(IDXGISwapChain* thiz, UINT SyncInterval, UINT Flags);
//opengl_std
void __stdcall gl_hkglBegin(GLenum mode);
//opengl dn,nox
void __stdcall gl_hkwglSwapBuffers(HDC hdc);
//egl
unsigned int __stdcall gl_hkeglSwapBuffers(void* dpy, void* surface);

int xhook::setup(HWND hwnd_, int render_type_) {
	xhook::render_hwnd = hwnd_;
	wsprintf(xhook::shared_res_name, SHARED_RES_NAME_FORMAT, hwnd_);
	wsprintf(xhook::mutex_name, MUTEX_NAME_FORMAT, hwnd_);

	int idx = 0;
	void* address = nullptr;
	if (render_type_ == RDT_DX_DEFAULT || render_type_ == RDT_DX_D3D9) {

		render_type = kiero::RenderType::D3D9;
		idx = 42; address = dx9_hkEndScene;
	}
	else if (render_type_ == RDT_DX_D3D10) {
		render_type = kiero::RenderType::D3D10;
		idx = 8; address = dx10_hkPresent;
	}
	else if (render_type_ == RDT_DX_D3D11)
	{
		render_type = kiero::RenderType::D3D11;
		idx = 8; address = dx11_hkPresent;
	}
	else if (render_type_ == RDT_GL_DEFAULT || render_type_ == RDT_GL_NOX) {
		render_type = kiero::RenderType::OpenGL;
		idx = 2; address = gl_hkwglSwapBuffers;
	}
	else if (render_type_ == RDT_GL_STD) {
		render_type = kiero::RenderType::OpenGL;
		idx = 0; address = gl_hkglBegin;
	}
	else if(render_type_==RDT_GL_ES){
		render_type = kiero::RenderType::OpenglES;
		idx = 0; address = gl_hkeglSwapBuffers;
	}
	else {
		render_type = kiero::RenderType::None;
	}
	if (kiero::init(render_type) != kiero::Status::Success)
		return 0;

	is_capture = kiero::bind(idx, &old_address, address);
	return is_capture;
}

int xhook::release() {
	is_capture = 0;
	int ret = kiero::unbind();
	return 1;
}

static void CopyImageData(char* dst_, const char* src_, int rows_, int cols_, int fmt_) {
	if (fmt_ == IBF_B8G8R8A8) {
		::memcpy(dst_, src_, rows_*cols_ * 4);
	}
	else if (fmt_ == IBF_R8G8B8A8) {
		//pixels count
		int n = rows_ * cols_;
		for (int i = 0; i < n; ++i) {
			dst_[0] = src_[2];
			dst_[1] = src_[1];
			dst_[2] = src_[0];
			dst_[3] = src_[3];
			dst_ += 4; src_ += 4;
		}
	}
	else {
		//pixels count
		int n = rows_ * cols_;
		for (int i = 0; i < n; ++i) {
			*dst_++ = *src_++;
			*dst_++ = *src_++;
			*dst_++ = *src_++;
			dst_++;//dst is 4 B
		}
	}
}

static DXGI_FORMAT GetDxgiFormat(DXGI_FORMAT format) {
	if (format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) {
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	}
	if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	return format;
}


//------------------------dx9-------------------------------
//screen capture
HRESULT dx9_capture(LPDIRECT3DDEVICE9 pDevice) {
	//save bmp
	//setlog("dx9screen_capture");
	HRESULT hr = NULL;
	CComPtr<IDirect3DSurface9> pSurface;
	hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
	if (FAILED(hr)) return hr;

	D3DSURFACE_DESC surface_Desc;
	hr = pSurface->GetDesc(&surface_Desc);
	if (FAILED(hr)) return hr;

	CComPtr<IDirect3DTexture9> pTex;
	CComPtr<IDirect3DSurface9> pTexSurface;
	hr = pDevice->CreateTexture(surface_Desc.Width,
		surface_Desc.Height,
		1,
		0,
		surface_Desc.Format,
		D3DPOOL_SYSTEMMEM, //必须为这个
		&pTex, NULL);
	if (hr < 0) {
		return hr;
	}
	hr = pTex->GetSurfaceLevel(0, &pTexSurface);
	if (hr < 0)return hr;
	hr = pDevice->GetRenderTargetData(pSurface, pTexSurface);

	D3DLOCKED_RECT lockedRect;

	pTex->LockRect(0, &lockedRect, NULL, D3DLOCK_READONLY);
	/*取像素*/
	sharedmem mem;
	promutex mutex;
	if (mem.open(xhook::shared_res_name) && mutex.open(xhook::mutex_name)) {
		mutex.lock();
		memcpy(mem.data<byte>(), (byte*)lockedRect.pBits, lockedRect.Pitch*surface_Desc.Height);
		mutex.unlock();
	}
	pTex->UnlockRect(0);
	
	return hr;
}
//dx9 hooked EndScene function
HRESULT STDMETHODCALLTYPE dx9_hkEndScene(IDirect3DDevice9* thiz)
{
	typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
	auto ret = ((EndScene)xhook::old_address)(thiz);
	if (is_capture)
		dx9_capture(thiz);

	return ret;
}
//------------------------------------------------------------

//-----------------------dx10----------------------------------
//screen capture
void dx10_capture(IDXGISwapChain* pswapchain) {
	using Texture2D = ID3D10Texture2D * ;

	HRESULT hr;
	ID3D10Device *pdevices = nullptr;
	ID3D10Resource* backbuffer = nullptr;
	Texture2D textDst = nullptr;
	//LPD3D10BLOB pblob = nullptr;

	//setlog("before GetBuffer");
	hr = pswapchain->GetBuffer(0, __uuidof(ID3D10Resource), (void**)&backbuffer);
	if (hr < 0) {
		setlog("pswapchain->GetBuffer error code=%d", hr);
		is_capture = 0;
		return;
	}
	backbuffer->GetDevice(&pdevices);

	if (!pdevices) {
		//setlog(" pswapchain->GetDevice false");
		backbuffer->Release();
		is_capture = 0;
		return;
	}
	//auto p

	DXGI_SWAP_CHAIN_DESC desc;
	pswapchain->GetDesc(&desc);;
	//backbuffer->GetDesc(&desc);
	// If texture is multisampled, then we can use ResolveSubresource to copy it into a non-multisampled texture
	//Texture2D textureResolved = nullptr;


	D3D10_TEXTURE2D_DESC textDesc = {};
	textDesc.Format = GetDxgiFormat(desc.BufferDesc.Format);
	textDesc.Width = desc.BufferDesc.Width;
	textDesc.Height = desc.BufferDesc.Height;
	textDesc.MipLevels = 1;
	textDesc.ArraySize = 1;
	textDesc.SampleDesc.Count = 1;
	textDesc.Usage = D3D10_USAGE_STAGING;
	textDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
	hr = pdevices->CreateTexture2D(&textDesc, nullptr, &textDst);
	if (hr < 0) {
		pdevices->Release();
		backbuffer->Release();
		return;
	}

	pdevices->CopyResource(textDst, backbuffer);

	D3D10_MAPPED_TEXTURE2D mapText = { 0,0 };

	hr = textDst->Map(0, D3D10_MAP_READ, 0, &mapText);

	//hr = pD3DX10SaveTextureToMemory(textureDest, D3DX10_IMAGE_FILE_FORMAT::D3DX10_IFF_BMP, &pblob, 0);
	if (hr < 0) {
		setlog("textDst->Map false,hr=%d", hr);
		is_capture = 0;
		return;
	}

	int fmt = IBF_R8G8B8A8;
	if (textDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM ||
		textDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM ||
		textDesc.Format == DXGI_FORMAT_B8G8R8A8_TYPELESS ||
		textDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
		textDesc.Format == DXGI_FORMAT_B8G8R8X8_TYPELESS ||
		textDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)
	{
		fmt = IBF_B8G8R8A8;
	}

	sharedmem mem;
	promutex mutex;
	if (mem.open(xhook::shared_res_name) && mutex.open(xhook::mutex_name)) {

		mutex.lock();
		//memcpy(mem.data<char>(), mapText.pData, textDesc.Width*textDesc.Height * 4);
		CopyImageData(mem.data<char>(), (char*)mapText.pData, textDesc.Height, textDesc.Width, fmt);
		mutex.unlock();
	}
	else {
		setlog("mem.open(xhook::shared_res_name) && mutex.open(xhook::mutex_name)");
	}
	//release
	SAFE_RELEASE(textDst);
	SAFE_RELEASE(pdevices);
	SAFE_RELEASE(backbuffer);
	//pblob->Release();
	//setlog("pblob->Release()");
}
//dx10 hook Present
HRESULT STDMETHODCALLTYPE dx10_hkPresent(IDXGISwapChain* thiz, UINT SyncInterval, UINT Flags) {
	typedef long(__stdcall* Present_t)(IDXGISwapChain* pswapchain, UINT x1, UINT x2);
	if (is_capture)
		dx10_capture(thiz);
	return ((Present_t)xhook::old_address)(thiz, SyncInterval, Flags);
	//thiz.
}
//------------------------------------------------------------

//------------------------dx11----------------------------------
//screen capture
void dx11_capture(IDXGISwapChain* swapchain) {

	//setlog("d3d11 cap");
	using Texture2D = ID3D11Texture2D * ;
	HRESULT hr = 0;
	IDXGIResource* backbufferptr = nullptr;
	ID3D11Resource* backbuffer = nullptr;
	Texture2D textDst = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;

	hr = swapchain->GetBuffer(0, __uuidof(IDXGIResource), (void**)&backbufferptr);
	if (hr < 0) {
		setlog("pswapchain->GetBuffer,error code=%X", hr);
		is_capture = 0;
		return;
	}
	hr = backbufferptr->QueryInterface(__uuidof(ID3D11Resource), (void**)&backbuffer);
	if (hr < 0) {
		setlog("backbufferptr->QueryInterface,error code=%X", hr);
		is_capture = 0;
		return;
	}
	hr = swapchain->GetDevice(__uuidof(ID3D11Device), (void**)&device);
	if (hr < 0) {
		setlog("swapchain->GetDevice hr=%X", hr);
		is_capture = 0;
		return;
	}
	DXGI_SWAP_CHAIN_DESC desc;
	hr = swapchain->GetDesc(&desc);
	if (hr < 0) {
		setlog("swapchain->GetDesc hr=%X", hr);
		is_capture = 0;
		return;
	}

	D3D11_TEXTURE2D_DESC textDesc = { };
	textDesc.Format = GetDxgiFormat(desc.BufferDesc.Format);
	textDesc.Width = desc.BufferDesc.Width;
	textDesc.Height = desc.BufferDesc.Height;
	textDesc.MipLevels = 1;
	textDesc.ArraySize = 1;
	textDesc.SampleDesc.Count = 1;
	textDesc.Usage = D3D11_USAGE_STAGING;
	textDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr = device->CreateTexture2D(&textDesc, nullptr, &textDst);
	if (hr < 0) {
		setlog("device->CreateTexture2D,error code=%d", hr);
		is_capture = 0;
		return;
	}
	//DXGI_KEY
	device->GetImmediateContext(&context);
	if (!context) {
		setlog("!context");
		is_capture = 0;
		return;
	}

	context->CopyResource(textDst, backbuffer);

	D3D11_MAPPED_SUBRESOURCE mapSubres = { 0,0,0 };

	//hr = pD3DX11SaveTextureToMemory(context, textureDst, D3DX11_IMAGE_FILE_FORMAT::D3DX11_IFF_BMP, &pblob, 0);
	hr = context->Map(textDst, 0, D3D11_MAP_READ, 0, &mapSubres);
	if (hr < 0) {
		setlog("context->Map error code=%d", hr);
		is_capture = 0;
		return;
	}
	int fmt = IBF_R8G8B8A8;
	if (textDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM ||
		textDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM ||
		textDesc.Format == DXGI_FORMAT_B8G8R8A8_TYPELESS ||
		textDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
		textDesc.Format == DXGI_FORMAT_B8G8R8X8_TYPELESS ||
		textDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)
	{
		fmt = IBF_B8G8R8A8;
	}
	sharedmem mem;
	promutex mutex;
	if (mem.open(xhook::shared_res_name) && mutex.open(xhook::mutex_name)) {
		mutex.lock();
		//memcpy(mem.data<char>(), ptr + offset, bits);
		CopyImageData(mem.data<char>(), (char*)mapSubres.pData, textDesc.Height, textDesc.Width, fmt);
		mutex.unlock();
	}
	else {
		setlog("mem.open(xhook::shared_res_name) && mutex.open(xhook::mutex_name)");
	}
	context->Unmap(textDst, 0);
	SAFE_RELEASE(backbufferptr);
	SAFE_RELEASE(backbuffer);
	SAFE_RELEASE(device);
	SAFE_RELEASE(textDst);
	SAFE_RELEASE(context);
	//if (pblob)pblob->Release();
}
//hooked present
HRESULT __stdcall dx11_hkPresent(IDXGISwapChain* thiz, UINT SyncInterval, UINT Flags) {
	typedef long(__stdcall* Present_t)(IDXGISwapChain* pswapchain, UINT x1, UINT x2);
	if (is_capture)
		dx11_capture(thiz);
	return ((Present_t)xhook::old_address)(thiz, SyncInterval, Flags);
}
//------------------------------------------------------------

//-----------------------opengl-----------------------------
//screen capture
long gl_capture() {
	using glPixelStorei_t = decltype(glPixelStorei)*;
	using glReadBuffer_t = decltype(glReadBuffer)*;
	using glGetIntegerv_t = decltype(glGetIntegerv)*;
	using glReadPixels_t = decltype(glReadPixels)*;

	auto pglPixelStorei = (glPixelStorei_t)query_api("opengl32.dll", "glPixelStorei");
	auto pglReadBuffer = (glReadBuffer_t)query_api("opengl32.dll", "glReadBuffer");
	auto pglGetIntegerv = (glGetIntegerv_t)query_api("opengl32.dll", "glGetIntegerv");
	auto pglReadPixels = (glReadPixels_t)query_api("opengl32.dll", "glReadPixels");
	if (!pglPixelStorei || !pglReadBuffer || !pglGetIntegerv || !pglReadPixels) {
		is_capture = 0;
		setlog("error.!pglPixelStorei || !pglReadBuffer || !pglGetIntegerv || !pglReadPixels");
		return 0;
	}
	RECT rc;
	::GetClientRect(xhook::render_hwnd, &rc);
	int width = rc.right - rc.left, height = rc.bottom - rc.top;

	pglPixelStorei(GL_PACK_ALIGNMENT, 1);
	pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	pglReadBuffer(GL_FRONT);

	sharedmem mem;
	promutex mutex;
	if (mem.open(xhook::shared_res_name) && mutex.open(xhook::mutex_name)) {
		mutex.lock();
		pglReadPixels(0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, mem.data<byte>());
		mutex.unlock();
	}
	else {
		is_capture = 0;
		setlog("!mem.open(xhook::shared_res_name)&&mutex.open(xhook::mutex_name)");
	}
	//setlog("gl screen ok");
	return 0;
}
// hook glBegin or wglSwapLayerBuffers

void __stdcall gl_hkglBegin(GLenum mode) {
	static DWORD t = 0;
	using glBegin_t = decltype(glBegin)*;

	if (is_capture)
		gl_capture();
	((glBegin_t)xhook::old_address)(mode);
}

void __stdcall gl_hkwglSwapBuffers(HDC hdc) {
	using wglSwapBuffers_t = void(__stdcall*) (HDC hdc);
	if (is_capture)
		gl_capture();
	((wglSwapBuffers_t)xhook::old_address)(hdc);
}


//---------------------OPENGL ES------------------------------
//es 类似 opengl 截图，只是模块不同
long egl_capture() {
	using glPixelStorei_t = decltype(glPixelStorei)*;
	using glReadBuffer_t = decltype(glReadBuffer)*;
	using glGetIntegerv_t = decltype(glGetIntegerv)*;
	using glReadPixels_t = decltype(glReadPixels)*;

	auto pglPixelStorei = (glPixelStorei_t)query_api("libglesv2.dll", "glPixelStorei");
	auto pglReadBuffer = (glReadBuffer_t)query_api("libglesv2.dll", "glReadBuffer");
	auto pglGetIntegerv = (glGetIntegerv_t)query_api("libglesv2.dll", "glGetIntegerv");
	auto pglReadPixels = (glReadPixels_t)query_api("libglesv2.dll", "glReadPixels");
	if (!pglPixelStorei || !pglReadBuffer || !pglGetIntegerv || !pglReadPixels) {
		is_capture = 0;
		setlog("error.!pglPixelStorei || !pglReadBuffer || !pglGetIntegerv || !pglReadPixels");
		return 0;
	}
	RECT rc;
	::GetClientRect(xhook::render_hwnd, &rc);
	int width = rc.right - rc.left, height = rc.bottom - rc.top;

	pglPixelStorei(GL_PACK_ALIGNMENT, 1);
	pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	pglReadBuffer(GL_FRONT);

	sharedmem mem;
	promutex mutex;
	if (mem.open(xhook::shared_res_name) && mutex.open(xhook::mutex_name)) {
		mutex.lock();
		pglReadPixels(0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, mem.data<byte>());
		mutex.unlock();
	}
	else {
		is_capture = 0;
		setlog("!mem.open(xhook::shared_res_name)&&mutex.open(xhook::mutex_name)");
	}
	//setlog("gl screen ok");
	return 0;
}

unsigned int __stdcall gl_hkeglSwapBuffers(void* dpy, void* surface) {
	using eglSwapBuffers_t = decltype(gl_hkeglSwapBuffers)*;
	if (is_capture)
		egl_capture();
	return ((eglSwapBuffers_t)xhook::old_address)(dpy, surface);
	
}

//--------------export function--------------------------
long SetXHook(HWND hwnd_, int render_type_) {
	if (xhook::setup(hwnd_, render_type_) != 1)
		return 0;
	//setlog("in hook,hwnd=%d,bktype=%d", hwnd_, bktype_);
	return 1;
}

long UnXHook() {

	return xhook::release();
}

