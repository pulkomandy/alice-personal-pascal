
extern int form_do();

int
form_ndo( tree, edit_obj )
long tree;
int *edit_obj;
{

	return form_do( tree, *edit_obj );
}
