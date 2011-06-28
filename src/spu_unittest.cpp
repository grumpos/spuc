#include "spu_unittest.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;


static vector<string> ErrorMessages;

void SPUTest_Expect( __m128 Result, __m128 Expected, const char* ErrorMsg )
{
	if ( 0 != memcmp( &Expected, &Result, sizeof(__m128) ) )
	{
		ErrorMessages.push_back(string(ErrorMsg) + "FAIL");
	}
	else
	{
		//ErrorMessages.push_back(string(ErrorMsg) + "PASS");
	}
}

void SPUTest_ReportErrors()
{
	copy( ErrorMessages.cbegin(), ErrorMessages.cend(), ostream_iterator<string>(cout, "\n"));
}