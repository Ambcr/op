#include "stdafx.h"
#include "ImageLoc.h"
#include "Common.h"
#include "ocr.h"
#include "Tool.h"

using std::to_wstring;
//检查是否为透明图，返回透明像素个数
int check_transparent(cv::Mat* img) {
	if (img->rows < 2 || img->cols < 2)
		return 0;
	bool x = img->at<uint>(0, 0) == img->at<uint>(0, img->cols - 1) &&
		img->at<uint>(0, 0) - img->at<uint>(img->rows - 1, 0) &&
		img->at<uint>(0, 0) - img->at<uint>(img->rows - 1, img->cols - 1);
	if (!x)
		return 0;
	uint c0 = img->at<uint>(0, 0);
	int n = img->rows*img->cols;
	uint* ptr = (uint*)img->data;
	int ct = 0;
	for (int i = 1; i < n; ++i) {
		if (ptr[i] == c0)
			++ct;
	}

	return ct < n ? ct : 0;
}


ImageBase::ImageBase()
{
	_x1 = _y1 = 0;
	_dx = _dy = 0;
}


ImageBase::~ImageBase()
{
}


long ImageBase::input_image(byte* image_data, int width, int height, long x1, long y1, long x2, long y2, int type) {
	int i, j;
	_x1 = x1; _y1 = y1;
	int cw = x2 - x1 - 1, ch = y2 - y1 - 1;
	if (type == -1) {//倒过来读
		_src.create(ch, cw, CV_8UC4);
		uchar *p, *p2;
		for (i = 0; i < ch; ++i) {
			p = _src.ptr<uchar>(i);
			p2 = image_data + (height - i - 1 - y1) * width * 4 + x1 * 4;//偏移
			memcpy(p, p2, 4 * cw);
		}
	}
	else {
		_src.create(ch, cw, CV_8UC4);
		uchar *p, *p2;
		for (i = 0; i < ch; ++i) {
			p = _src.ptr<uchar>(i);
			p2 = image_data + (i + y1) * width * 4 + x1 * 4;
			memcpy(p, p2, 4 * cw);
		}
	}
	return 1;
}

void ImageBase::set_offset(int dx, int dy) {
	_dx = -dx;
	_dy = -dy;
}

int ImageBase::get_bk_color(const cv::Mat& input) {
	int y[256] = { 0 };
	auto ptr = input.data;
	int n = input.cols * input.rows;
	for (int i = 0; i < n; ++i)
		y[ptr[i]]++;
	//scan max
	int m = 0;
	for (int i = 0; i < 256; ++i) {
		if (y[i] > y[m])m = i;
	}
	return m;
}

void ImageBase::bgr2binary(vector<color_df_t>& colors) {
	if (_src.empty())
		return;
	int ncols = _src.cols, nrows = _src.rows;
	_binary.create(nrows, ncols, CV_8UC1);
	for (int i = 0; i < nrows; ++i) {
		uchar* p = _src.ptr<uchar>(i);
		uchar* p2 = _binary.ptr<uchar>(i);
		for (int j = 0; j < ncols; ++j) {
			*p2 = WORD_BKCOLOR;
			for (auto&it : colors) {//对每个颜色描述
				if ((*(color_t*)p - it.color) <= it.df) {
					*p2 = WORD_COLOR;
					break;
				}
			}
			++p2;
			p += 4;
		}
	}
	//test
	//cv::imwrite("src.png", _src);
	//cv::imwrite("binary.png", _binary);
}

//二值化
void ImageBase::auto2binary()
{
	//转为灰度图
	cv::cvtColor(_src, _target, CV_BGR2GRAY);
	//创建二值图
	_binary.create(_target.size(), CV_8UC1);
	//获取背景颜色
	int bkcolor = get_bk_color(_target);
	int n = _target.cols * _target.rows;
	auto ptr1 = _target.data;
	auto ptr2 = _binary.data;
	for (int i = 0; i < n; ++i) {
		ptr2[i] = (std::abs(ptr1[i] - bkcolor) < 20 ? WORD_BKCOLOR : WORD_COLOR);
	}
}

//long ImageBase::imageloc(images_t& images, double sim, long&x, long&y) {
//	x = y = -1;
//	if (_src.empty())return 0;
//	//cv::imwrite("input.png", _src);
//	for (auto&it : images) {
//		_target = cv::imread(_wsto_string(it));
//		if (_target.empty()) {
//			//setlog(L"ImageExtend::imageloc,file:%s read false.", it.c_str());
//			continue;
//		}
//		//cv::cvtColor(_src, _src_gray, CV_BGR2GRAY);
//		//cv::cvtColor(_target, _target_gray, CV_BGR2GRAY);
//		int result_cols = _src.cols - _target.cols + 1;
//		int result_rows = _src.rows - _target.rows + 1;
//		_result.create(result_rows, result_cols, CV_32FC1);
//		cv::matchTemplate(_src, _target, _result, CV_TM_SQDIFF);
//		//cv::normalize(_result, _result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
//
//
//		double minval, maxval, bestval;
//		cv::Point pt1, pt2;
//		cv::minMaxLoc(_result, &minval, &maxval, &pt1, &pt2);
//		minval /= 255.*255.*_target.rows*_target.cols;
//		bestval = 1. - minval;
//		//setlog(L"ImageExtend::imageloc(),file=%s, bestval=%lf,pos=[%d,%d]",it.c_str(), bestval, pt1.x, pt1.y);
//
//		if (bestval >= sim) {
//			x = pt1.x; y = pt1.y;
//			return 1;
//		}
//		else {
//			return 0;
//		}
//	}
//	return 0;
//
//
//}

long ImageBase::simple_match(long x, long y, cv::Mat* timg, color_t dfcolor, int max_error) {
	int err_ct = 0, k;
	for (int i = 0; i < timg->rows; ++i) {
		auto p1 = _src.ptr<uchar>(i + y) + x * 4;
		auto p2 = timg->ptr<uchar>(i);
		for (int j = 0; j < timg->cols; ++j) {
			if (*(color_t*)p1 - *(color_t*)p2 > dfcolor)
				++err_ct;
			if (err_ct > max_error)
				return 0;
			p1 += 4; p2 += 3;
		}
	}
	return 1;
}

long ImageBase::trans_match(long x, long y, cv::Mat* timg, color_t dfcolor, int max_error) {
	int err_ct = 0, k;
	//background color
	uint c0 = timg->at<uint>(0, 0);
	for (int i = 0; i < timg->rows; ++i) {
		auto p1 = _src.ptr<uchar>(i + y) + x * 4;
		auto p2 = timg->ptr<uchar>(i);
		for (int j = 0; j < timg->cols; ++j) {
			if (*(uint*)p2 != c0) {//ignore background color
				if (*(color_t*)p1 - *(color_t*)p2 > dfcolor)
					++err_ct;
				if (err_ct > max_error)
					return 0;
			}
			
			p1 += 4; p2 += 3;
		}
	}
	return 1;
}

//long ImageBase::ndiff_match(long x, long y, cv::Mat* timg, int max_error) {
//	int err_ct = 0, k;
//	//background color
//	uint c0 = timg->at<uint>(0, 0);
//	for (int i = 0; i < timg->rows; ++i) {
//		auto p1 = _src.ptr<uchar>(i + y) + x * 4;
//		auto p2 = timg->ptr<uchar>(i);
//		for (int j = 0; j < timg->cols; ++j) {
//			if (*(uint*)p2 != c0) {//ignore background color
//				if (*(uint*)p1 - *(uint*)p2)
//					++err_ct;
//				if (err_ct > max_error)
//					return 0;
//			}
//
//			p1 += 4; p2 += 3;
//		}
//	}
//	return 1;
//}

long ImageBase::GetPixel(long x, long y, color_t&cr) {
	if (!is_valid(x, y)) {
		setlog("Invalid pos:%d %d", x, y);
		return 0;
	}
		
	auto p = _src.ptr<uchar>(y) + 4 * x;
	static_assert(sizeof(color_t) == 4);
	cr.b = p[0]; cr.g = p[1]; cr.r = p[2];
	return 1;
}

long ImageBase::CmpColor(long x, long y, std::vector<color_df_t>&colors, double sim) {
	color_t cr;
	if (GetPixel(x, y, cr)) {
		for (auto&it : colors) {
			if (it.color - cr <= it.df)
				return 1;
		}
	}
	return 0;
}

long ImageBase::FindColor(vector<color_df_t>& colors, long&x, long&y) {
	for (int i = 0; i < _src.rows; ++i) {
		uchar* p = _src.ptr<uchar>(i);
		for (int j = 0; j < _src.cols; ++j) {
			for (auto&it : colors) {//对每个颜色描述
				if ((*(color_t*)p - it.color) <= it.df) {
					x = j+_x1+_dx; y = i+_y1+_dy;
					return 1;
				}
			}
			p += 4;
		}
	}
	x = y = -1;
	return 0;
}

long ImageBase::FindColorEx(vector<color_df_t>& colors, std::wstring& retstr) {
	retstr.clear();
	int find_ct = 0;
	for (int i = 0; i < _src.rows; ++i) {
		uchar* p = _src.ptr<uchar>(i);
		for (int j = 0; j < _src.cols; ++j) {
			for (auto&it : colors) {//对每个颜色描述
				if ((*(color_t*)p - it.color) <= it.df) {
					retstr += std::to_wstring(j+_x1+_dx) + L"," + std::to_wstring(i+_y1+_dy);
					retstr += L"|";
					++find_ct;
					//return 1;
					if (find_ct > _max_return_obj_ct)
						goto _quick_break;
					break;
				}
			}
			p += 4;
		}
	}
_quick_break:
	if (!retstr.empty() && retstr.back() == L'|')
		retstr.pop_back();
	return find_ct;
}

long ImageBase::FindMultiColor(std::vector<color_df_t>&first_color, std::vector<pt_cr_df_t>& offset_color, double sim, long dir, long&x, long&y) {
	int max_err_ct = offset_color.size()*(1. - sim);
	int err_ct;
	for (int i = 0; i < _src.rows; ++i) {
		uchar* p = _src.ptr<uchar>(i);
		for (int j = 0; j < _src.cols; ++j) {
			//step 1. find first color
			for (auto&it : first_color) {//对每个颜色描述
				if ((*(color_t*)p - it.color) <= it.df) {
					//匹配其他坐标
					err_ct = 0;
					for (auto&off_cr : offset_color) {
						if (!CmpColor(j + off_cr.x, i + off_cr.y, off_cr.crdfs, sim))
							++err_ct;
						if (err_ct > max_err_ct)
							goto _quick_break;
					}
					//ok
					x = j+_x1+_dx, y = i + _y1 + _dy;
					return 1;
				}
			}
		_quick_break:
			p += 4;
		}
	}
	x = y = -1;
	return 0;
}

long ImageBase::FindMultiColorEx(std::vector<color_df_t>&first_color, std::vector<pt_cr_df_t>& offset_color, double sim, long dir, std::wstring& retstr) {
	int max_err_ct = offset_color.size()*(1. - sim);
	int err_ct;
	int find_ct = 0;
	for (int i = 0; i < _src.rows; ++i) {
		uchar* p = _src.ptr<uchar>(i);
		for (int j = 0; j < _src.cols; ++j) {
			//step 1. find first color
			for (auto&it : first_color) {//对每个颜色描述
				if ((*(color_t*)p - it.color) <= it.df) {
					//匹配其他坐标
					err_ct = 0;
					for (auto&off_cr : offset_color) {
						if (!CmpColor(j + off_cr.x, i + off_cr.y, off_cr.crdfs, sim))
							++err_ct;
						if (err_ct > max_err_ct)
							goto _quick_break;
					}
					//ok
					retstr += to_wstring(j + _x1 + _dx) + L"," + to_wstring(i + _y1 + _dy);
					retstr += L"|";
					++find_ct;
					if (find_ct > _max_return_obj_ct)
						goto _quick_return;
					else
						goto _quick_break;
				}
			}
		_quick_break:
			p += 4;
		}
	}
_quick_return:
	if (!retstr.empty() && retstr.back() == L'|')
		retstr.pop_back();
	return find_ct;
	//x = y = -1;
}

long ImageBase::FindPic(std::vector<cv::Mat*>&pics, color_t dfcolor, double sim, long&x, long&y) {
	vector<int> use_ts_match(pics.size());
	for (int i = 0; i < pics.size(); ++i)
		use_ts_match[i] = check_transparent(pics[i]);

	for (int i = 0; i < _src.rows; ++i) {
		for (int j = 0; j < _src.cols; ++j) {
			for (int pic_id = 0; pic_id < pics.size();++pic_id) {
				auto pic = pics[pic_id];
				//step 1. 边界检查
				if (i + pic->rows > _src.rows || j + pic->cols > _src.cols)
					continue;
				//step 2. 计算最大误差
				int max_err_ct = (pic->rows*pic->cols - use_ts_match[pic_id])*(1.0 - sim);
				//step 3. 开始匹配
				int match_ret = (use_ts_match[pic_id] ? trans_match(j, i, pic, dfcolor, max_err_ct) :
					simple_match(j, i, pic, dfcolor, max_err_ct));
				if (match_ret) {
					x = j + _x1 + _dx; y = i + _y1 + _dy;
					return pic_id;
				}
			}//end for pics

		}//end for j
	}//end for i
	x = y = -1;
	return -1;
}

long ImageBase::FindPicEx(std::vector<cv::Mat*>&pics, color_t dfcolor, double sim, wstring& retstr) {
	int obj_ct = 0;
	retstr.clear();

	vector<int> use_ts_match(pics.size());
	for (int i = 0; i < pics.size(); ++i)
		use_ts_match[i] = check_transparent(pics[i]);

	for (int i = 0; i < _src.rows; ++i) {
		uchar* p = _src.ptr<uchar>(i);
		for (int j = 0; j < _src.cols; ++j) {
			for (int pic_id = 0; pic_id < pics.size();++i) {
				auto pic = pics[pic_id];
				//step 1. 边界检查
				if (i + pic->rows > _src.rows || j + pic->cols > _src.cols)
					continue;
				//step 2. 计算最大误差
				int max_err_ct = (pic->rows*pic->cols - use_ts_match[pic_id])*(1.0 - sim);
				//step 3. 开始匹配
				int match_ret = (use_ts_match[pic_id] ? trans_match(j, i, pic, dfcolor, max_err_ct) :
					simple_match(j, i, pic, dfcolor, max_err_ct));
				if (match_ret) {
					retstr += std::to_wstring(j + _x1 + _dx) + L"," + std::to_wstring(i + _y1 + _dy);
					retstr += L"|";
					++obj_ct;
					if (obj_ct > _max_return_obj_ct)
						goto _quick_return;
					else
						break;
				}
			}//end for pics

		}//end for j
	}//end for i
_quick_return:
	return obj_ct;
}


long ImageBase::Ocr(Dict& dict, double sim, wstring& retstr) {
	retstr.clear();
	std::map<point_t, wstring> ps;
	bin_ocr(_binary, _target, dict, sim, ps);
	for (auto&it : ps) {
		retstr += it.second;
	}
	return 1;
}

long ImageBase::OcrEx(Dict& dict, double sim, std::wstring& retstr) {
	retstr.clear();
	std::map<point_t, wstring> ps;
	bin_ocr(_binary, _target, dict, sim, ps);
	//x1,y1,str....|x2,y2,str2...|...
	int find_ct = 0;
	for (auto&it : ps) {
		retstr += std::to_wstring(it.first.x + _x1 + _dx);
		retstr += L",";
		retstr += std::to_wstring(it.first.y + _y1 + _dy);
		retstr += L",";
		retstr += it.second;
		retstr += L"|";
		++find_ct;
		if (find_ct > _max_return_obj_ct)
			break;
	}
	if (!retstr.empty() && retstr.back() == L'|')
		retstr.pop_back();
	return find_ct;
}

long ImageBase::FindStr(Dict& dict, const vector<wstring>& vstr, double sim, long& retx, long& rety) {
	retx = rety = -1;
	std::map<point_t, wstring> ps;
	bin_ocr(_binary, _target, dict, sim, ps);
	for (auto&it : ps) {
		for (auto&s : vstr) {
			if (it.second == s) {
				retx = it.first.x + _x1 + _dx;
				rety = it.first.y + _y1 + _dy;
				return 1;
			}
		}
	}
	return 0;
}

long ImageBase::FindStrEx(Dict& dict, const vector<wstring>& vstr, double sim, std::wstring& retstr) {
	retstr.clear();
	std::map<point_t, wstring> ps;
	bin_ocr(_binary, _target, dict, sim, ps);
	int find_ct = 0;
	for (auto&it : ps) {
		for (auto&s : vstr) {
			if (it.second == s) {
				retstr += std::to_wstring(it.first.x + _x1 + _dx);
				retstr += L",";
				retstr += std::to_wstring(it.first.y + _y1 + _dy);
				retstr += L"|";
				++find_ct;
				if (find_ct > _max_return_obj_ct)
					goto _quick_return;
				else
					break;
			}
		}
	}
_quick_return:
	if (!retstr.empty() && retstr.back() == L'|')
		retstr.pop_back();
	return find_ct;
}
