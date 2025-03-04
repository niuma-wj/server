// BaseUtils.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.07.10

#ifndef _NIU_MA_BASE_UTILS_H_
#define _NIU_MA_BASE_UTILS_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <list>

namespace NiuMa {
	class BaseUtils {
	private:
		// 禁止构造
		BaseUtils();
		virtual ~BaseUtils();

	public:
		// 浮点数最小容差值
		static const float FLOAT_TOLERANCE;

		// 浮点数最小容差值
		static const double DOUBLE_TOLERANCE;

		// 空字符串
		static const std::string EMPTY_STRING;

		// 1000以内的质数表
		static const int PRIME_NUMBERS[168];

	private:
		static int _seedIndex;

	public:
		// 判定字符串以xxx结尾
		static bool endWith(const std::string& str, const std::string& end);

		// 从文件路径中获取文件名
		static void fileName(const std::string& path, std::string& fileName);

		// 从数组中查找指定值
		template <typename T>
		static bool contain(const std::vector<T>& arr, const T& val) {
			typename std::vector<T>::const_iterator it = arr.begin();
			while (it != arr.end()) {
				if (*it == val)
					return true;
				++it;
			}
			return false;
		}

		/**
		 * 顺序依次从Map映射表中取出值
		 * @param dataMap 数据映射表
		 * @param sequences 数据映射表关键字顺序表
		 * @param <T1> 数据映射表关键字泛型
		 * @param <T2> 数据映射表值泛型
		 * @return 数据映射表中的值
		 */
		template <typename T1, typename T2>
		static T2 getInSequence(const std::unordered_map<T1, T2>& dataMap, std::list<T1>& sequence) {
			typename std::unordered_map<T1, T2>::const_iterator it;
			if (sequence.empty()) {
				it = dataMap.begin();
				while (it != dataMap.end()) {
					sequence.push_back(it->first);
					++it;
				}
			}
			T2 ret;
			while (!sequence.empty()) {
				T1 key = sequence.front();
				sequence.pop_front();
				it = dataMap.find(key);
				if (it != dataMap.end()) {
					ret = it->second;
					break;
				}
			}
			return ret;
		}

		// 内存转base64字符串
		static bool encodeBase64(std::string& base64, const char* input, int len);

		// base64字符串转内存
		static bool decodeBase64(const std::string& input, std::string& output);

		/**
		 * 生成MD5编码
		 * @param input 输入文本
		 * @param output 输出md5编码
		 * @param len16 是否输出16位，默认输出32位
		 * @param capital 是否输出大写，默认输出小写
		 */
		static void encodeMD5(const std::string& input, std::string& output, bool len16 = false, bool capital = false);

		/**
		 * 字符串拷贝
		 * @param str 源字符串
		 * @return 调用c函数拷贝字符串，需要调用free函数释放
		 */
		static char* strdup(const char* str);

		/**
		 * 获取当前时间，单位毫秒
		 * 从1970-01-01 00:00:00 UTC到现在的毫秒数
		 * @return 当前时间
		 */
		static time_t getCurrentMillisecond();

		/**
		 * 获取当前时间，单位秒
		 * 从1970-01-01 00:00:00 UTC到现在的秒数
		 * @return 当前时间
		 */
		static time_t getCurrentSecond();

		/**
		 * 生成一个在[s, e)区间内的整形随机数
		 * @param s 区间起始值(包含)
		 * @param e 区间截止值(不包含)
		 * @return 随机数
		 */
		static int randInt(int s, int e);

		/**
		 * 判定浮点数相等
		 * @param val1 数值1
		 * @param val2 数值2
		 * @param tolerance 容差
		 * @return true-相等，false-不等
		 */
		static bool realEquals(float val1, float val2, float tolerance = FLOAT_TOLERANCE);

		/**
		 * 判定浮点数相等
		 * @param val1 数值1
		 * @param val2 数值2
		 * @param tolerance 容差
		 * @return true-相等，false-不等
		 */
		static bool real64Equals(double val1, double val2, double tolerance = DOUBLE_TOLERANCE);

		/**
		 * 计算地理定位距离
		 * @param lat1 纬度1
		 * @param lon1 经度1
		 * @param alt1 海拔1
		 * @param lat2 纬度2
		 * @param lon2 经度2
		 * @param alt2 海拔2
		 * @return 两点间距离
		 */
		static double calcGeoDistance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2);
	};
}

#endif // !_NIU_MA_BASE_UTILS_H_