/* Generated by CodeDescriptor 1.5.0.0907 */
/*
* Project Name      -> Software Status
* Version           -> 1.0.0.0216
* Author            -> Hm @ Workstadion.: QP-01-02
* Build Date        -> 16.02.2018 07:15:40
* Description       -> Aktuelle Statusmeldungen der Software
*
*
*
*/

typedef struct
{
	/*
	*	Fehlermeldungen!
	*/
	uint16_t crcErr			:1;
	uint16_t pswdErr		:1;
	uint16_t SerNrErr		:1;
	
	/*
	*	Meldungen. Keine Fehler!
	*/
	uint16_t newUserName	:1;
	uint16_t newDateSet		:1;
	uint16_t newTimeSet		:1;
	uint16_t newUartData	:1;
	uint16_t newRelaisSet	:1;
	
}state_t;