// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//

#ifndef BASE_ARG_H_
#define BASE_ARG_H_

#include <vector>

class CArg  {
public:
	class ArgVal {
	public:
		ArgVal():charPtr(0){}
		ArgVal(const char* in) {
			charPtr = in;
		}
		bool null() {
			return charPtr==0;
		}
		operator const char* () const;
		operator int () const;
		const char* charPtr;
	};

	CArg(int argc, char** argv);
	virtual ~CArg();

	std::vector<ArgVal> find(const char* prefix);
	ArgVal find1(const char* prefix);

private:
	char** argv;
	int argc;
};

#endif // BASE_ARG_H_
