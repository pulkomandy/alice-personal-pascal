
struct sent_data {
	unsigned editor_seg;
	int ed_level;
	nodep BT_text;
	nodep BT_integer;
	nodep BT_real;
	nodep BT_char;
	nodep BT_pointer;
	nodep BT_set;
	nodep SP_passtype;
	nodep BT_boolean;
	nodep SP_file;
	nodep SP_ordinal;
	nodep SP_number;
	nodep *cursor_adr;
	struct node_info **node_table;
	ClassNum **K_Classes;
	char **Class_Names;
	char **Node_Names;
	type_code **Class_Types;
	nodep SP_string;
	int max_stack_size;
	int loc_stack_size;
};

#ifdef PROC_INTERP
extern struct sent_data edit_data;
#define cursor (@edit_data.cursor_adr)
#endif
