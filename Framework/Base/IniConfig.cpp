// IniConfig.cpp

#include "Log.h"
#include "IniConfig.h"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace NiuMa {
	class IniData {
	public:
		IniData() {}
		virtual ~IniData() {}

	public:
		void loadIni(const std::string& file) {
			boost::property_tree::ini_parser::read_ini(file, _ini);
		}

		template <typename T>
		bool get(const std::string& section, const std::string& key, T& value) {
			std::string target = section + "." + key;
			bool ret = true;
			try {
				value = _ini.get<T>(target);
			}
			catch (std::exception& ex) {
				ErrorS << "Get config value (target: " << target << ") error: " << ex.what();
				ret = false;
			}
			return ret;
		}
		
	private:
		boost::property_tree::ptree _ini;
	};

	template<> IniConfig* Singleton<IniConfig>::_inst = nullptr;

	IniConfig::IniConfig() {}

	IniConfig::~IniConfig() {}

	void IniConfig::loadIni(const std::string& file) {
		_data = std::make_shared<IniData>();
		_data->loadIni(file);
	}

	bool IniConfig::getString(const std::string& section, const std::string& key, std::string& value) const {
		value.clear();
		if (!_data)
			return false;
		return _data->get<std::string>(section, key, value);
	}

	bool IniConfig::getInt(const std::string& section, const std::string& key, int& value) const {
		value = 0;
		if (!_data)
			return false;
		return _data->get<int>(section, key, value);
	}

	bool IniConfig::getInt64(const std::string& section, const std::string& key, long long& value) const {
		value = 0LL;
		if (!_data)
			return false;
		return _data->get<long long>(section, key, value);
	}

	bool IniConfig::getDouble(const std::string& section, const std::string& key, double& value) {
		value = 0.0;
		if (!_data)
			return false;
		return _data->get<double>(section, key, value);
	}
}