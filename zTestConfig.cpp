#include "zTestConfig.h"
#include "zStrUtil.h"

#include "tinyxml/tinyxml.h"

NS_ZTEST_CONFIG_START

/************************************************************************/
/* 
For each config, we have some attributes:
1) A KEY-VALUE mapping, like:
  <key>value</key>
2) A cluster of config-s, like:
  <key> <key1>value1</key1><key2>value2</key2> </key>
3) A sub-config-file reference, like: 
  <key fileref='ref-file-path'/>
*/
/************************************************************************/

class zConfigItem;
class zRootConfigItem;	//public zConfigItem

typedef std::map<ZString, zConfigVector>	zSubConfgs;
typedef std::map<ZString, ZString>			zStrMap;
typedef std::map<ZString, zStrVector>		zChildStrMap;

class zConfigItem: public IConfig
{
public:
	virtual ~zConfigItem()
	{
		for (zSubConfgs::iterator it = m_childConfigs.begin(); it != m_childConfigs.end(); ++it)
		{
			for (zConfigVector::iterator itconfig = it->second.begin(); itconfig != it->second.end(); ++itconfig)
				delete *itconfig;
		}
		m_childConfigs.clear();
	};

	virtual ZBool QueryChild(const ZString& key, ZString& value);
	virtual ZBool QueryChild(const ZString& key, ZInt32& value);
	virtual ZBool QueryChild(const ZString& key, zStrVector& value);
	virtual ZBool QueryChild(const ZString& key, zIntVector& value);

	virtual ZBool QueryAttr(const ZString& key, ZString& value);
	virtual ZBool QueryAttr(const ZString& key, ZInt32& value);
		
	virtual ZBool QuerySubConfig(const ZString * path_base, ZUInt32 path_len, zConfigVector& configs);

private:
	void ParseChildrenAsConfigs(const ZString& child, zConfigVector& configs);

protected:
	zConfigItem():m_xml(NULL){};
	zConfigItem(TiXmlElement* xml):m_xml(xml){};

	zChildStrMap	m_children;
	zStrMap			m_attributes;
	zSubConfgs		m_childConfigs;
	TiXmlElement*	m_xml;

private:
	Z_NO_COPYABLE(zConfigItem);
};

//////////////////////////////////////////////////////////////////////////
//class zConfigItem
ZBool zConfigItem::QueryChild(const ZString& key, zStrVector& value)
{
	if (!m_xml) return ZFalse;
	if (key.empty()) return ZFalse;

	zChildStrMap::iterator it = m_children.find(key);
	if (m_children.end() != it)
	{
		value = it->second;
		return ZTrue;
	}

	value.clear();

	TiXmlElement* child = m_xml->FirstChildElement(key.c_str());
	while(child)
	{
		const char* v = child->Value();
		if (v)
			value.push_back(ZString(v));
		
		child = child->NextSiblingElement(key.c_str());
	}
	
	m_children.insert(std::make_pair(key,value));
	return ZTrue;
};

ZBool zConfigItem::QueryChild(const ZString& key, zIntVector& value)
{
	value.clear();
	zStrVector strvals;
	if (QueryChild(key,strvals))
	{
		for (zStrVector::iterator it = strvals.begin(); it != strvals.end(); ++it)
		{
			ZInt32 i32v = 0;
			if (NS_ZTEST::FromString(*it,i32v))
			{
				value.push_back(i32v);
			}
		}

		return ZTrue;
	}
	return ZFalse;
};

ZBool zConfigItem::QueryChild(const ZString& key, ZString& value)
{
	zStrVector strvals;
	if (QueryChild(key,strvals))
	{
		if (strvals.empty())
			return ZFalse;

		value = strvals[0];
		return ZTrue;
	}
	return ZFalse;
};

ZBool zConfigItem::QueryChild(const ZString& key, ZInt32& value)
{
	ZString strVal;
	if (QueryChild(key,strVal))
	{
		return NS_ZTEST::FromString(strVal,value);
	}
	return ZFalse;
};

ZBool zConfigItem::QueryAttr(const ZString& key, ZString& value)
{
	if (!m_xml || key.empty()) return ZFalse;

	zStrMap::iterator it = m_attributes.find(key);
	if (m_attributes.end() != it)
	{
		value = it->second;
		return ZTrue;
	}

	const char* v = m_xml->Attribute(key.c_str());
	
	if (v)
		value = v;
	else
		value = ZString();

	m_attributes.insert(std::make_pair(key,value));

	return ZTrue;
};

ZBool zConfigItem::QueryAttr(const ZString& key, ZInt32& value)
{
	ZString strVal;
	if (QueryAttr(key,strVal))
	{
		return NS_ZTEST::FromString(strVal,value);
	}
	return ZFalse;
};

void zConfigItem::ParseChildrenAsConfigs(const ZString& child_key, zConfigVector& configs)
{
	if(child_key.empty() || !m_xml) return;

	TiXmlElement* child = m_xml->FirstChildElement(child_key.c_str());
	while(child)
	{
		zConfigItem* item = new zConfigItem(child);
		if (item)
			configs.push_back(item);

		child = child->NextSiblingElement(child_key.c_str());
	}
	m_childConfigs.insert(std::make_pair(child_key, configs));
}

ZBool zConfigItem::QuerySubConfig(const ZString * path_base, ZUInt32 path_len, zConfigVector& configs)
{
	if (!path_base || !m_xml || path_len == 0) return ZFalse;

	ZString child_key = path_base[0];

	zSubConfgs::iterator it = m_childConfigs.find(child_key);
	if (m_childConfigs.end() != it)
	{
		if (it->second.empty()) 
			return ZFalse;
		else{
			if (1 == path_len)
			{
				configs = it->second;
				return ZTrue;
			}
			else
			{
				return it->second[0]->QuerySubConfig(++path_base, --path_len,configs);
			}
		}
	}else{
		zConfigVector tmp;
		ParseChildrenAsConfigs(child_key,tmp);

		if (tmp.empty())
			return ZFalse;
		else
		{
			if (1 == path_len)
			{
				configs = tmp;
				return ZTrue;
			}
			else
			{
				return tmp[0]->QuerySubConfig(++path_base, --path_len,configs);
			}
		}
	}
};

class zRootConfigItem: public zConfigItem
{
public:
	zRootConfigItem(const ZString& config_str)
		:m_doc(NULL)
	{
		if (!config_str.empty())
		{
			m_doc = new TiXmlDocument;
			if (m_doc)
			{
				//m_doc->LoadFile(configFile.c_str());
				m_doc->Parse(config_str.c_str());
				m_xml = m_doc->RootElement();
			}
		}		
	};

	~zRootConfigItem()
	{
		Z_SAFE_DELETE(m_doc);
	};

private:
	Z_NO_COPYABLE(zRootConfigItem);
	zRootConfigItem();
private:
	TiXmlDocument* m_doc;
};

IConfig* zConfigMgr::CreateFrom(const ZString& config_str)
{
	if (config_str.empty()) return NULL;
	return new zRootConfigItem(config_str);
};

ZBool zConfigMgr::Destory(IConfig* config)
{
	Z_SAFE_DELETE(config);
	return ZTrue;
}

NS_ZTEST_CONFIG_END