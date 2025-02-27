#include "stdafx.h"
#include "bkdisplay.h"
#include "Tool.h"

bkdisplay::bkdisplay()
{
	_hwnd = NULL;
	_shmem = nullptr;
	_pmutex = nullptr;
	_bind_state = 0;
	_width = _height = 0;
	_client_x = _client_y = 0;
}


bkdisplay::~bkdisplay()
{
}

long bkdisplay::bind_init() {
	wsprintf(_shared_res_name, SHARED_RES_NAME_FORMAT, _hwnd);
	wsprintf(_mutex_name, MUTEX_NAME_FORMAT, _hwnd);
	//bind_release();
	try {
		_shmem = new sharedmem();
		_shmem->open_create(_shared_res_name, SHARED_MEMORY_SIZE);
		_pmutex = new promutex();
		_pmutex->open_create(_mutex_name);
	}
	catch (std::exception&e) {
		setlog("bkdisplay::bind_init() %s exception:%s", _shared_res_name, e.what());
	}

	return 0;
}

long bkdisplay::bind_release() {
	SAFE_DELETE(_shmem);
	SAFE_DELETE(_pmutex);

	_hwnd = NULL;
	//_image_data = nullptr;
	return 0;
}

byte* bkdisplay::get_data() {
	return _shmem->data<byte>();
}

