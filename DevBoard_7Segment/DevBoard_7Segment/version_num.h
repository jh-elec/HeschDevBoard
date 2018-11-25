/* Generated by CodeDescriptor 1.5.0.0907 */
/*
* Project Name      -> Build Infos
* Version           -> 1.0.0.1231
* Author            -> Jan Homann @ Workstadion.: XXXJANIIIX3XXX
* Build Date        -> 31.12.2017 11:19:49
* Description       -> Building infos from Preprocessor
*
*
*
*/

#ifndef _VERSION_NUM_H_
#define _VERSION_NUM_H_


#define VERSION_MAJOR  		1
#define VERSION_MINOR		0



#if (VERSION_MAJOR >= '0' && VERSION_MAJOR <= '9')
	static char verMajor = VERSION_MAJOR;
#else
	#if(VERSION_MAJOR >= 'a' && VERSION_MAJOR <= 'z' || VERSION_MAJOR >= 'A' && VERSION_MAJOR <= 'Z')
		#error VERSION_MAJOR.: only character from (0-9)
	#else
		static char verMajor = VERSION_MAJOR + '0';
	#endif
#endif

#if (VERSION_MINOR >= '0' && VERSION_MINOR <= '9')
	static char verMinor = VERSION_MINOR;
#else
	#if(VERSION_MINOR >= 'a' && VERSION_MINOR <= 'z' || VERSION_MINOR >= 'A' && VERSION_MINOR <= 'Z')
		#error VERSION_MINOR.: only character from (0-9)
	#else
		static char verMinor = VERSION_MINOR + '0';
	#endif
#endif

#endif // _VERSION_NUM_H_
