
#include "alice.h"

#include <osbind.h>
#include "jump.h"

char *
getfilename( str, flag )
char *str;
int flag;
{
	*str = 0;
	get_file( "AP", str );
	return str;
}

/*------------------------------*/
/*	get_file 		*/
/*------------------------------*/
int
get_file(extnt, got_file)
char	*extnt, *got_file;
{
	int	butn, ii;
	char	tmp_path[64], tmp_name[13];

	tmp_name[0] = '\0';
	tmp_path[0] = '\0';

	if (*got_file)
		parse_fname(got_file, tmp_path, tmp_name, extnt);
	if (!tmp_path[0])
		get_path(&tmp_path[0], extnt);

	fsel_input(tmp_path, tmp_name, &butn);

	draw_all();

	if (butn && tmp_name[0] != 0 ) {
		strcpy(got_file, tmp_path);
		for (ii = 0; got_file[ii] && got_file[ii] != '*'; ii++);
		got_file[ii - 1] = '\0';
		strcat (got_file, "\\");
		strcat(got_file, tmp_name);
		return (TRUE);
	}
	else
		err_return();

}

/*------------------------------*/
/*	parse_fname 		*/
/*------------------------------*/

parse_fname(full, path, name, extnt)
char	*full, *path, *name, *extnt;
{
	int	i, j;
	char	*s, *d;

	for (i = strlen(full); i--; )		/* scan for end of path */
		if (full[i] == '\\' || full[i] == ':')
			break;
	if (i == -1)
		strcpy(name, full);		/* "Naked" file name */
	else
	{
		strcpy(name, &full[i+1]);
		for (s = full, d = path, j = 0; j++ < i + 1;
		    *d++ = *s++);
		strcpy(&path[i+1], "*.");
		strcat(path, extnt);
	}
}

/*------------------------------*/
/*	get_path 		*/
/*------------------------------*/

get_path(tmp_path, spec)
char	*tmp_path, *spec;
{
	int	cur_drv;

	cur_drv = Dgetdrv();
	tmp_path[0] = cur_drv + 'A';
	tmp_path[1] = ':';
	Dgetpath(&tmp_path[2], 0);
	if (strlen(tmp_path) > 3)
		strcat(tmp_path, "\\");
	else
		tmp_path[2] = '\0';
	strcat(tmp_path, "*.");
	strcat(tmp_path, spec);
}
