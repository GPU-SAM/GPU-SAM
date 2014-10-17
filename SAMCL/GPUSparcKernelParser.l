%{
#include <stdio.h>
#include "GPUSparc.hpp"
%}

%%

"//"[^\n]*\n {
	fprintf(yyout,"\n");
}

"/*"([^*]|\*+[^*/])*\*+"/" {
	;
}

__kernel[\n\t ]+(__attribute)?*void[^(]*"("[^)]*")" {
	yytext[strlen(yytext)-1] = '\0';
	fprintf(yyout,"%s", yytext);
	fprintf(yyout,",unsigned int GPUSPARC_GLOBAL_SIZE0");
	fprintf(yyout,",unsigned int GPUSPARC_GLOBAL_SIZE1");
	fprintf(yyout,",unsigned int GPUSPARC_GLOBAL_SIZE2");
	fprintf(yyout,",unsigned int GPUSPARC_GROUP_OFFSET0");
	fprintf(yyout,",unsigned int GPUSPARC_GROUP_OFFSET1");
	fprintf(yyout,",unsigned int GPUSPARC_GROUP_OFFSET2");
	fprintf(yyout,",unsigned int GPUSPARC_NUM_GROUPS0");
	fprintf(yyout,",unsigned int GPUSPARC_NUM_GROUPS1");
	fprintf(yyout,",unsigned int GPUSPARC_NUM_GROUPS2)");
}


__kernel[\n \t]+.*[\n \t]*void[\n \t]+[^(]*"("[^)]*")" {
	yytext[strlen(yytext)-1] = '\0';
	fprintf(yyout,"%s", yytext);
	fprintf(yyout,",unsigned int GPUSPARC_GLOBAL_SIZE0");
	fprintf(yyout,",unsigned int GPUSPARC_GLOBAL_SIZE1");
	fprintf(yyout,",unsigned int GPUSPARC_GLOBAL_SIZE2");
	fprintf(yyout,",unsigned int GPUSPARC_GROUP_OFFSET0");
	fprintf(yyout,",unsigned int GPUSPARC_GROUP_OFFSET1");
	fprintf(yyout,",unsigned int GPUSPARC_GROUP_OFFSET2");
	fprintf(yyout,",unsigned int GPUSPARC_NUM_GROUPS0");
	fprintf(yyout,",unsigned int GPUSPARC_NUM_GROUPS1");
	fprintf(yyout,",unsigned int GPUSPARC_NUM_GROUPS2)");
}


get_global_id[ \t]*\([\t ]*[0-2][\t ]*\) {
	unsigned int i;
	char dim = 0;
	fprintf(yyout,"(");
	for(i = 0; i < strlen(yytext); i++)
	{
		if (yytext[i] != '\t' && yytext[i] != ' ')
			fprintf(yyout,"%c", yytext[i]);
		if (yytext[i] == '0' || yytext[i] == '1' || yytext[i] == '2')
			dim = yytext[i];
	}
	fprintf(yyout,"+get_local_size(%c)*GPUSPARC_GROUP_OFFSET%c)",dim, dim);
}


get_global_size[ \t]*\([\t ]*[0-2][\t ]*\) {
	unsigned int i;
	char dim = 0;
	for(i = 0; i < strlen(yytext); i++)
		if (yytext[i] == '0' || yytext[i] == '1' || yytext[i] == '2')
			dim = yytext[i];
	fprintf(yyout,"GPUSPARC_GLOBAL_SIZE%c", dim);
}

get_group_id[ \t]*\([\t ]*[0-2][\t ]*\) {
	unsigned int i;
	char dim = 0;
	fprintf(yyout,"(");
	for(i = 0; i < strlen(yytext); i++)
	{
		if (yytext[i] != '\t' && yytext[i] != ' ')
			fprintf(yyout,"%c", yytext[i]);
		if (yytext[i] == '0' || yytext[i] == '1' || yytext[i] == '2')
			dim = yytext[i];
	}
	fprintf(yyout,"+GPUSPARC_GROUP_OFFSET%c)", dim);
}


get_num_groups[ \t]*\([\t ]*[0-2][\t ]*\) {
	unsigned int i;
	char dim = 0;
	for(i = 0; i < strlen(yytext); i++)
		if (yytext[i] == '0' || yytext[i] == '1' || yytext[i] == '2')
			dim = yytext[i];
	fprintf(yyout,"GPUSPARC_NUM_GROUPS%c", dim);
}

.|\n {
	ECHO;
}

%%

FILE *kernelParser_GPUSparc(const char *str)
{
	yy_scan_string (str);
	FILE *fp = fopen ("./kernelsourcetemp", "w+");
	yyout = fp;
	yylex();
	fclose (fp);
	fp = fopen ("./kernelsourcetemp", "r");
	return fp;
}
