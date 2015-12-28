#pragma once

#include "zTestUtilBase.h"
#include <sstream>

NS_ZTEST_START

ZBool FromString(const ZString& str, ZInt32& ret)
{
	if (str.empty()) return ZFalse;
		
	ret = atoi(str.c_str());

	return ZTrue;
}

NS_ZTEST_END