// BaseUtils.cpp

#include "BaseUtils.h"

#include <openssl/pem.h>
#include <openssl/md5.h>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <random>

#include <string.h>
#include <boost/locale.hpp>

namespace NiuMa {
	const float BaseUtils::FLOAT_TOLERANCE = 1.0e-5f;
	const double BaseUtils::DOUBLE_TOLERANCE = 1.0e-8f;
	const std::string BaseUtils::EMPTY_STRING("");
	const int BaseUtils::PRIME_NUMBERS[168] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
		101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241,
		251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
		421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599,
		601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773,
		787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977,
		983, 991, 997 };

	int BaseUtils::_seedIndex = 0;

	BaseUtils::BaseUtils() {}

	BaseUtils::~BaseUtils() {}

	bool BaseUtils::endWith(const std::string& str, const std::string& end) {
		if (str.empty() || end.empty())
			return false;
		if (str.length() < end.length())
			return false;
		std::string sub = str.substr(str.length() - end.length());
		if (sub == end)
			return true;
		return false;
	}

	void BaseUtils::fileName(const std::string& path, std::string& fileName) {
		fileName.clear();
		if (path.empty())
			return;
		std::string::size_type pos = path.rfind("/");
		if (pos == std::string::npos)
			pos = path.rfind("\\");
		if (pos == std::string::npos)
			fileName = path;
		else
			fileName = path.substr(pos + 1);
	}

	bool BaseUtils::toUtf8(const std::string& text, std::string& utf8) {
#ifdef _MSC_VER
		utf8 = boost::locale::conv::to_utf<char>(text, std::string("gb2312"));
#else
		utf8 = text;
#endif
		return true;
	}

	bool BaseUtils::fromUtf8(const std::string& utf8, std::string& text) {
#ifdef _MSC_VER
		text = boost::locale::conv::from_utf<char>(utf8, std::string("gb2312"));
#else
		text = utf8;
#endif
		return true;
	}

	bool BaseUtils::encodeBase64(std::string& base64, const char* input, int len) {
		base64.clear();
		size_t base64_len = static_cast<size_t>((len + 2) / 3 * 4);
		if (base64_len == 0)
			return false;
		// 申请内存时多加一些字节，如果仅仅申请base64_len数量的字节，则释放内存的时候会报错，不知道为什么
		unsigned char* buf = new unsigned char[base64_len + 16];
		int ret = EVP_EncodeBlock(buf, reinterpret_cast<const unsigned char*>(input), len);
		if (ret > 0)
			base64.assign(reinterpret_cast<char*>(buf), static_cast<size_t>(ret));
		delete[] buf;
		return (ret > 0);
	}

	bool BaseUtils::decodeBase64(const std::string& input, std::string& output) {
		output.clear();
		size_t input_len = input.size();
		if (input_len % 4 != 0)
			return false;

		size_t output_Len = (input_len / 4) * 3;
		unsigned char* buf = new unsigned char[output_Len + 1];
		// 判断返回值
		int ret = EVP_DecodeBlock(buf, (const unsigned char*)input.data(), (int)input_len);
		if (ret == -1) {
			// base64 decode failed
			delete[] buf;
			return false;
		}
		// 需要注意的是，被编码的数据大小不是3字节的整数倍时，base64后将会有一个'='或两个'='跟在后面，
		// 这样的话需要再解码之后看一下有几个'='，再把解码过的数据进行删减
		int nums = 0;
		while (input.at(--input_len) == '=') {
			ret--;
			nums++;
			if (nums > 2) {
				// input maybe not base64 str;
				delete[] buf;
				return false;
			}
		}
		output_Len = static_cast<size_t>(ret);
		output.assign(reinterpret_cast<const char*>(buf), output_Len);
		delete[] buf;
		return true;
	}

	void BaseUtils::encodeMD5(const std::string& input, std::string& output, bool len16, bool capital) {
		MD5_CTX ctx;
		MD5_Init(&ctx);
		MD5_Update(&ctx, input.c_str(), input.length());

		unsigned char buf[MD5_DIGEST_LENGTH] = {0};
		MD5_Final(buf, &ctx);

		std::stringstream ss;
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			ss << std::hex;
			if (capital)
				ss << std::uppercase;
			ss << std::setw(2) << std::setfill('0') << (int)buf[i];
		}
		output = ss.str();
		if (len16)
			output = output.substr(8, 16);
	}

	char* BaseUtils::strdup(const char* str) {
#ifdef _MSC_VER
		return ::_strdup(str);
#else
		return ::strdup(str);
#endif
	}

	time_t BaseUtils::getCurrentMillisecond() {
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::duration<double> time_since_epoch = now.time_since_epoch();
		time_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
		return ms;
	}

	time_t BaseUtils::getCurrentSecond() {
		time_t s = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		return s;
	}

	int BaseUtils::randInt(int s, int e) {
		if (s >= (e - 1))
			return s;
		/*unsigned int seed = 0;
		time_t ms = getCurrentMillisecond();
		std::string str = std::to_string(ms);
		if (str.length() > 8) {
			// 只要低8位作为随机种子
			str = str.substr(str.length() - 8);
			seed = static_cast<unsigned int>(atoi(str.c_str()));
		}
		else
			seed = static_cast<unsigned int>(ms);
		int index = _seedIndex;
		if (index < 0 || index > 167)
			index = 0;
		int prime = PRIME_NUMBERS[index++];
		// 这里可能会有多个线程同时修改_seedIndex，导致_seedIndex的值错乱，但是这里不需要担心这个问题，
		// 因为这里用的是局部变量index，而index是已经被限制了范围的
		_seedIndex = index;
		ms += static_cast<unsigned int>(prime);*/
		// 随机数引擎的种子源
		std::random_device rd;
		// 以 rd() 播种的 mersenne_twister_engine
		std::default_random_engine engine(rd());
		std::uniform_int_distribution<int> dist(s, e - 1);
		return dist(engine);
	}

	bool BaseUtils::realEquals(float val1, float val2, float tolerance) {
		float t = abs(val1 - val2);
		if (t > tolerance)
			return false;
		return true;
	}

	bool BaseUtils::real64Equals(double val1, double val2, double tolerance) {
		double t = abs(val1 - val2);
		if (t > tolerance)
			return false;
		return true;
	}

	double BaseUtils::calcGeoDistance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2) {
		// 地球半径
		const double RADIUS = 6372797.0;
		const double PI = 3.1415927;
		double rlat1 = lat1 * PI / 180.0;
		double rlat2 = lat2 * PI / 180.0;
		double rlon1 = lon1 * PI / 180.0;
		double rlon2 = lon2 * PI / 180.0;
		double x1 = cos(rlat1) * sin(rlon1);
		double y1 = sin(rlat1);
		double z1 = cos(rlon1) * cos(rlat1);
		double x2 = cos(rlat2) * sin(rlon2);
		double y2 = sin(rlat2);
		double z2 = cos(rlon2) * cos(rlat2);
		double cosK = x1 * x2 + y1 * y2 + z1 * z2;
		double acosK = acos(cosK);
		double r = 0.0;
		if (alt2 > alt1)
			r = RADIUS + alt1;
		else
			r = RADIUS + alt2;
		r *= acosK;
		if (!real64Equals(alt1, 0.0f) && !real64Equals(alt2, 0.0f))
			r += abs(alt2 - alt1);
		return r;
	}
}