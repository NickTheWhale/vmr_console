#pragma once


/************************************************/
/**                 Active routing             **/
/************************************************/

/*
	array of routing information from routing.txt in the form:
	"Strip[<index>].<parameter>:<control index>",
	"Strip[<index>].<parameter>:<control index>"
*/
char routing[128][64]{};


/*
	array of parameter names derived from routing[][] in the form:
	"Strip[<index>].<parameter>:",
	"Strip[<index>].<parameter>"
*/
char paramNames[128][48]{};


/*
	array of control index derived from routing[][] in the form:
	"<control index>",
	"<control index>"
*/
char control[128][8]{};


/*
	parameter values updated by isDirty()
*/
float paramStates[128]{};