#pragma once

#include <string>
#include <map>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// BASE TYPE DEFINITION
typedef bool			ZBool;
#define ZTrue			true
#define ZFalse			false

typedef int				ZInt32;
typedef unsigned int	ZUInt32;
typedef char			ZChar;
typedef std::string		ZString;

//////////////////////////////////////////////////////////////////////////
// USEFUL MACROS

#define Z_NO_COPYABLE(classname)	\
	private:						\
	classname (const classname&);	\
	classname& operator = (const classname&);

#define Z_SAFE_DELETE(X)		\
	if (X) {delete X;}

#define NS_ZTEST			ns_ztest
#define NS_ZTEST_START		namespace NS_ZTEST {
#define NS_ZTEST_END		}