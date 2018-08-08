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

#include <avr/io.h>

#include "version_num.h"
#include "build_info.h"


char *buildVer(void)
{
	static char build[] = "00000000000000000";
	
	/*
	*	major and minor version
	*/
	build[0] = verMajor;
	build[1] = '.';
	build[2] = ' ';
	build[3] = verMinor;
	build[4] = '.';
	build[5] = ' ';
		
	/*
	*	time.: hour|min
	*/
	build[6] = __TIME__[0];
	build[7] = __TIME__[1];
	build[8] = __TIME__[3];
	build[9] = __TIME__[4];
	
	build[10] = '.';
	build[11] = ' ';
	
	/*
	*	day
	*/
	build[12] = ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0');
	build[13] = (__DATE__[ 5]);
	
	/*
	*	month
	*/
	if(__DATE__[0] == 'J' && __DATE__[1] == 'a')
	build[14] = '1'; // Januar

	if(__DATE__[0] == 'F')
	build[14] = '2'; // Februar
	
	if(__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
	build[14] = '3'; // März
	
	if(__DATE__[0] == 'A' && __DATE__[1] == 'p')
	build[14] = '4'; // April
	
	if(__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
	build[14] = '5'; // Mai
	
	if(__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
	build[14] = '6'; // Juni
	
	if(__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
	build[14] = '7'; // Juli
	
	if(__DATE__[0] == 'A' && __DATE__[1] == 'u')
	build[14] = '8'; // August
	
	if(__DATE__[0] == 'S')
	build[14] = '9'; // September

	if(__DATE__[0] == 'O'){
		build[14] = '1';
		build[15] = '0'; // Oktober
	}

	if(__DATE__[0] == 'N'){
		build[14] = '1';
		build[15] = '1'; // November
	}
	
	if(__DATE__[0] == 'D'){
		build[14] = '1';
		build[15] = '2'; // Dezember
	}

	build[16] = '\0';
	
	return build;
}
