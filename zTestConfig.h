#pragma once

#include "zTestUtilBase.h"

#define NS_ZTEST_CONFIG			ns_ztest_config
#define NS_ZTEST_CONFIG_START	namespace NS_ZTEST_CONFIG {
#define NS_ZTEST_CONFIG_END		}

NS_ZTEST_CONFIG_START

class IConfig;

typedef std::vector<ZString> zStrVector;
typedef std::vector<ZInt32>	zIntVector;
typedef std::vector<IConfig*> zConfigVector;

class IConfig
{
public:
	virtual	~IConfig() {}

	virtual ZBool QueryChild(const ZString& key, ZString& value) = 0;
	virtual ZBool QueryChild(const ZString& key, ZInt32& value) = 0;
	virtual ZBool QueryChild(const ZString& key, zStrVector& value) = 0;
	virtual ZBool QueryChild(const ZString& key, zIntVector& value) = 0;

	virtual ZBool QueryAttr(const ZString& key, ZString& value) = 0;
	virtual ZBool QueryAttr(const ZString& key, ZInt32& value) = 0;

	virtual ZBool QuerySubConfig(const ZString * path_base, ZUInt32 path_len, zConfigVector& configs) = 0;
};

class zConfigMgr
{
public:
	static IConfig* CreateFrom(const ZString& config_str);
	static ZBool Destory(IConfig*);
};

NS_ZTEST_CONFIG_END