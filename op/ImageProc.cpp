#include "stdafx.h"
#include "ImageProc.h"
#include "Tool.h"
#include <fstream>
#include <bitset>
#include <algorithm>
ImageProc::ImageProc()
{
	_curr_idx = 0;
	_enable_cache = 1;
}


ImageProc::~ImageProc()
{
}

long ImageProc::Capture(const std::wstring& file) {
	wstring fpath = file;
	if (fpath.find(L'\\') == -1)
		return cv::imwrite(_ws2string(_curr_path + L"\\" + fpath), _src);
	else
		return cv::imwrite(_ws2string(fpath), _src);
}

long ImageProc::CmpColor(long x, long y, const std::wstring& scolor, double sim) {
	std::vector<color_df_t> vcolor;
	str2colordfs(scolor, vcolor);
	return ImageBase::CmpColor(x, y, vcolor, sim);

	
}

long ImageProc::FindColor(const wstring& color, double sim, long dir, long&x, long&y) {
	std::vector<color_df_t>colors;
	str2colordfs(color, colors);
	return ImageBase::FindColor(colors, x, y);
}

long ImageProc::FindColoEx(const wstring& color, double sim, long dir, wstring& retstr) {
	std::vector<color_df_t>colors;
	str2colordfs(color, colors);
	return ImageBase::FindColorEx(colors, retstr);
}

long ImageProc::FindMultiColor(const wstring& first_color, const wstring& offset_color, double sim, long dir, long&x, long&y) {
	std::vector<color_df_t>vfirst_color;
	str2colordfs(first_color, vfirst_color);
	std::vector<wstring> vseconds;
	split(offset_color, vseconds, L",");
	std::vector<pt_cr_df_t>voffset_cr;
	pt_cr_df_t tp;
	for (auto&it : vseconds) {
		size_t id1, id2;
		id1 = it.find(L'|');
		id2 = (id1 == wstring::npos ? wstring::npos : it.find(L'|', id1));
		if (id2 != wstring::npos) {
			swscanf(it.c_str(), L"%d|%d", &tp.x, &tp.y);
			if (id2 + 1 != it.length())
				str2colordfs(it.substr(id2 + 1), tp.crdfs);
			else
				break;
			voffset_cr.push_back(tp);
		}
	}
	return ImageBase::FindMultiColor(vfirst_color,voffset_cr,sim,dir,x,y);
}

long ImageProc::FindMultiColorEx(const wstring& first_color, const wstring& offset_color, double sim, long dir, wstring& retstr) {
	std::vector<color_df_t>vfirst_color;
	str2colordfs(first_color, vfirst_color);
	std::vector<wstring> vseconds;
	split(offset_color, vseconds, L",");
	std::vector<pt_cr_df_t>voffset_cr;
	pt_cr_df_t tp;
	for (auto&it : vseconds) {
		size_t id1, id2;
		id1 = it.find(L'|');
		id2 = (id1 == wstring::npos ? wstring::npos : it.find(L'|', id1));
		if (id2 != wstring::npos) {
			swscanf(it.c_str(), L"%d|%d", &tp.x, &tp.y);
			if (id2 + 1 != it.length())
				str2colordfs(it.substr(id2 + 1), tp.crdfs);
			else
				break;
			voffset_cr.push_back(tp);
		}
	}
	return ImageBase::FindMultiColorEx(vfirst_color, voffset_cr, sim, dir, retstr);
}
//图形定位
long ImageProc::FindPic(const std::wstring& files, const wstring& delta_colors, double sim, long dir, long& x, long &y) {
	vector<cv::Mat*>vpic;
	//vector<color_t> vcolor;
	color_t dfcolor;
	files2mats(files, vpic);
	dfcolor.str2color(delta_colors);
	//str2colors(delta_colors, vcolor);
	sim = 0.5 + sim / 2;
	long ret = ImageBase::FindPic(vpic, dfcolor,sim, x, y);
	//清理缓存
	if (!_enable_cache)
		_pic_cache.clear();
	return ret;
}
//
long ImageProc::FindPicEx(const std::wstring& files, const wstring& delta_colors, double sim, long dir, wstring& retstr) {
	vector<cv::Mat*>vpic;
	//vector<color_t> vcolor;
	color_t dfcolor;
	files2mats(files, vpic);
	dfcolor.str2color(delta_colors);
	sim = 0.5 + sim / 2;
	long ret = ImageBase::FindPicEx(vpic, dfcolor,sim, retstr);
	//清理缓存
	if (!_enable_cache)
		_pic_cache.clear();
	return ret;
}

long ImageProc::SetDict(int idx, const wstring& file_name) {
	if (idx < 0 || idx >= _max_dict)
		return 0;
	_dicts[idx].clear();
	wstring fullpath;
	if (Path2GlobalPath(file_name, _curr_path, fullpath)) {
		if (fullpath.substr(fullpath.length() - 4) == L".txt")
			_dicts[idx].read_dict_dm(_ws2string(fullpath));
		else
			_dicts[idx].read_dict(_ws2string(fullpath));
	}
	else {
		setlog(L"file '%s' does not exist", file_name.c_str());
	}
	return _dicts[idx].empty() ? 0 : 1;

}

long ImageProc::UseDict(int idx) {
	if (idx < 0 || idx >= _max_dict)
		return 0;
	_curr_idx = idx;
	return 1;
}

long ImageProc::OCR(const wstring& color, double sim, std::wstring& out_str) {
	out_str.clear();
	vector<color_df_t> colors;
	str2colordfs(color, colors);
	if (sim<0. || sim>1.)
		sim = 1.;

	long s;
	ImageBase::bgr2binary(colors);

	s = ImageBase::Ocr(_dicts[_curr_idx], sim, out_str);
	return s;

}

wstring ImageProc::GetColor(long x, long y) {
	color_t cr;
	if (ImageBase::GetPixel(x, y, cr)) {
		return _s2wstring(cr.tostr());
	}
	else {
		return L"";
	}
}

void ImageProc::str2colordfs(const wstring& color_str, std::vector<color_df_t>& colors) {
	std::vector<wstring>vstr, vstr2;
	color_df_t cr;
	colors.clear();
	split(color_str, vstr, L"|");
	for (auto&it : vstr) {
		split(it, vstr2, L"-");
		cr.color.str2color(vstr2[0]);
		cr.df.str2color(vstr2.size() == 2 ? vstr2[1] : L"000000");
		colors.push_back(cr);
	}
}

void ImageProc::str2colors(const wstring& color, std::vector<color_t>& vcolor) {
	std::vector<wstring>vstr, vstr2;
	color_t cr;
	vcolor.clear();
	split(color, vstr, L"|");
	for (auto&it : vstr) {
		cr.str2color(it);
		vcolor.push_back(cr);
	}
}

void ImageProc::files2mats(const wstring& files, std::vector<cv::Mat*>& vpic) {
	std::vector<wstring>vstr, vstr2;
	cv::Mat* pm;
	vpic.clear();
	split(files, vstr, L"|");
	wstring tp;
	for (auto&it : vstr) {
		//路径转化
		if (!Path2GlobalPath(it, _curr_path, tp))
			continue;
		//先在缓存中查找
		if (_pic_cache.count(tp)) {
			pm = &_pic_cache[tp];
		}
		else {
			_pic_cache[tp] = cv::imread(_ws2string(tp));
			pm = &_pic_cache[tp];
		}
		vpic.push_back(pm);
	}
}

long ImageProc::OcrEx(const wstring& color, double sim, std::wstring& out_str) {
	out_str.clear();
	vector<color_df_t> colors;
	str2colordfs(color, colors);
	if (sim<0. || sim>1.)
		sim = 1.;
	ImageBase::bgr2binary(colors);
	return ImageBase::OcrEx(_dicts[_curr_idx], sim, out_str);
}

long ImageProc::FindStr(const wstring& str, const wstring& color, double sim, long& retx, long& rety) {
	vector<wstring> vstr;
	vector<color_df_t> colors;
	split(str, vstr, L"|");
	str2colordfs(color, colors);
	if (sim<0. || sim>1.)
		sim = 1.;
	ImageBase::bgr2binary(colors);
	return ImageBase::FindStr(_dicts[_curr_idx], vstr, sim, retx, rety);
}

long ImageProc::FindStrEx(const wstring& str, const wstring& color, double sim, std::wstring& out_str) {
	out_str.clear();
	vector<wstring> vstr;
	vector<color_df_t> colors;
	split(str, vstr, L"|");
	str2colordfs(color, colors);
	if (sim<0. || sim>1.)
		sim = 1.;
	ImageBase::bgr2binary(colors);
	return ImageBase::FindStrEx(_dicts[_curr_idx], vstr, sim, out_str);
}

long ImageProc::OcrAuto(double sim, std::wstring& retstr) {
	retstr.clear();
	
	if (sim<0. || sim>1.)
		sim = 1.;
	ImageBase::auto2binary();
	return ImageBase::Ocr(_dicts[_curr_idx], sim,retstr);
}

long ImageProc::OcrFromFile(const wstring& files, const wstring& color, double sim, std::wstring& retstr) {
	retstr.clear();
	if (sim<0. || sim>1.)
		sim = 1.;
	wstring fullpath;
	vector<color_df_t> colors;
	str2colordfs(color, colors);
	if (Path2GlobalPath(files, _curr_path, fullpath)) {
		_src = cv::imread(_ws2string(fullpath));
		
		ImageBase::bgr2binary(colors);
		return ImageBase::Ocr(_dicts[_curr_idx], sim, retstr);
	}
	return 0;
}

long ImageProc::OcrAutoFromFile(const wstring& files, double sim, std::wstring& retstr) {
	retstr.clear();
	if (sim<0. || sim>1.)
		sim = 1.;
	wstring fullpath;

	if (Path2GlobalPath(files, _curr_path, fullpath)) {
		_src = cv::imread(_ws2string(fullpath));
		ImageBase::auto2binary();
		return ImageBase::Ocr(_dicts[_curr_idx], sim, retstr);
	}
	return 0;
}
