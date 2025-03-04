// IniConfig.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.30

#ifndef _NIU_MA_CONFIG_H_
#define _NIU_MA_CONFIG_H_

#include "Singleton.h"

#include <memory>
#include <string>

namespace NiuMa {
	class IniData;
	class IniConfig : public Singleton<IniConfig> {
	private:
		IniConfig();

	public:
		virtual ~IniConfig();
		friend class Singleton<IniConfig>;

	public:
		/**
		 * 加载ini配置文件
		 * @param file int文件路径
		 */
		void loadIni(const std::string& file);

		/**
		 * 获取字符串配置数值
		 * @param section 段名
		 * @param key 键名
		 * @param value 返回字符串配置数值
		 * @return 是否成功获取
		 */
		bool getString(const std::string& section, const std::string& key, std::string& value) const;

		/**
		 * 获取整数配置数值
		 * @param section 段名
		 * @param key 键名
		 * @param value 返回整数配置数值
		 * @return 是否成功获取
		 */
		bool getInt(const std::string& section, const std::string& key, int& value) const;

		/**
		 * 获取64位整数配置数值
		 * @param section 段名
		 * @param key 键名
		 * @param value 返回64位整数配置数值
		 * @return 是否成功获取
		 */
		bool getInt64(const std::string& section, const std::string& key, long long& value) const;

		/**
		 * 获取浮点数配置数值
		 * @param section 段名
		 * @param key 键名
		 * @param value 返回浮点数配置数值
		 * @return 是否成功获取
		 */
		bool getDouble(const std::string& section, const std::string& key, double& value);

	private:
		// 配置数据
		std::shared_ptr<IniData> _data;
	};
}

#endif // !_NIU_MA_CONFIG_H_