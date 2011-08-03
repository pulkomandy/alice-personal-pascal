/*
 *DESC: A split up portion of the interpter, for less common nodes
 */

	 case Main(N_EXP_NIL):
		p_spush( (pointer)NULL );
		break;

	 case Main(N_EXP_PAREN):
		break;		/* value already on stack */
	 
	 case Main(N_EXP_FUNC):
		thesym = get_routine(eloc, &dummy);

		if( node_flag(eloc) & NF_PASSFUNC ) {
			p_spush( (pointer)dummy );
			p_spush( (pointer)thesym );
			break;
			}
		/* make room, MAKE ROOM! for return value */
		/* if a byte, we must set upper half to zero */
		temp1 = sym_size(thesym);
		if( !temp1 )
			temp1 = calc_size( gexp_type(eloc), TRUE );
		if( temp1 <= sizeof(rint) )
			int_spush( 0 );
		 else
			bump_stack( size_adjust(temp1) );
			
		do_undef( sp.st_generic, U_CLEAR, (int)temp1 );
		/* somebody should check this ? */
		start_call( eloc );

		break;
	
	 case Main(N_EXP_SLASH):
		fltemp = f_spop();
		if( fltemp == FLOATNUM(0.0) )
			run_error( ER(80,"divz`Floating point divide by zero") );
		f_stop /= fltemp;
		break;
	 
	 case Main(N_EXP_DIV):
		temp1 = int_spop();
		if (temp1 == 0)
			run_error(ER(81,"divzero`attempted to divide by zero"));
		int_stop /= temp1;
		break;
	
	 case Main(N_EXP_MOD):
		temp1 = int_spop();
		if (temp1 <= 0)
			run_error(ER(82,"negmod`second operand of mod must be strictly positive"));
		int_stop %= temp1;
		/*
		 * On both the ONYX and QNX C compilers, (-1) % 3 -> -1,
		 * so we must add the second operand to make the result of
		 * op1 mod op2 correct.  I fear this is non-portable.
		 */
		if (int_stop < 0)
			int_stop += temp1;
		break;
	
#ifdef TURBO
	 case Main(N_EXP_SHL):
		temp1 = int_spop();
		int_stop <<= temp1;
		break;
	
	 case Main(N_EXP_SHR):
		temp1 = int_spop();
		/* shift must be a logical shift */
		((unsigned)int_stop) >>= temp1;
		break;
#endif

	 case Main(N_EXP_AND):
		temp1 = int_spop();
		int_stop &= temp1;
		break;

	 case Main(N_EXP_EQ):
		switch( int_kid(2,eloc) ) {
			case CC_POINTER:
				tempptr = p_spop();
				temp1 = p_spop() == tempptr;
				int_spush(temp1);
				break;
			case CC_BYTE:
			case CC_INTEGER:
				temp1 = int_spop();
				int_stop = int_stop == temp1;
				break;
			case CC_SET:
				tempptr = (pointer)set_spop();
				temp1 = set_eq((pointer)set_spop(), tempptr);
				int_spush(temp1);
				break;
			case CC_REAL:
				fltemp = f_spop();
				temp1 = fltemp == f_spop();
				int_spush( temp1 );
				break;
			case CC_STRING:
				tempptr = str_spop();
				temp1 = !ustrcmp( tempptr, str_spop() );
				int_spush( temp1 );
				break;
			}
		break;

	 case Main(N_EXP_NE):
		switch( int_kid(2,eloc) ) {
			case CC_POINTER:
				tempptr = p_spop();
				temp1 = p_spop() != tempptr;
				int_spush(temp1);
				break;
			case CC_BYTE:
			case CC_INTEGER:
				temp1 = int_spop();
				int_stop = int_stop != temp1;
				break;
			case CC_SET:
				tempptr = (pointer)set_spop();
				temp1 = !set_eq((pointer)set_spop(), tempptr);
				int_spush(temp1);
				break;
			case CC_REAL:
				fltemp = f_spop();
				temp1 = fltemp != f_spop();
				int_spush( temp1 );
				break;
			case CC_STRING:
				tempptr = str_spop();
				temp1 = ustrcmp( tempptr, str_spop() ) != 0;
				int_spush( temp1 );
				break;
			}
		break;
	 case Main(N_STUB):
		{
		extern ClassNum Stub_Exec[];
		if( !strchr( Stub_Exec, int_kid(0,eloc) ) )
			run_error( ER(83,"execstub`Placeholders can't be executed") );
		}

#ifndef ICRIPPLE
	 case N_SET_SUBRANGE:	/* pushes two kids, nothing else */
		break;


	 case Main(N_EXP_SET):
		/* execute list, adding bits to set */
		eset_push();	/* start with empty set */
		if( listcount( FLCAST kid1(eloc) ) ) {
			espush(0, e_int);
			state_push(S_SET1);
			new_loc = kid1(kid1(eloc));
			}
		break;

#ifdef notdef
	 case Main(N_EXP_ESET):
		/* THIS IS TO GO AWAY */
		eset_push();
		break;
#endif


	 case N_EXP_IN:
		tempptr = (pointer)set_spop();
		temp1 = set_in(int_spop(), tempptr);
		int_spush(temp1);
		break;

#endif ICRIPPLE
	 case Main(N_EXP_LT):
		switch( int_kid(2,eloc) ) {
			case CC_BYTE:
			case CC_INTEGER:
				temp1 = int_spop();
				int_stop = int_stop < temp1;
				break;
			case CC_REAL:
				fltemp = f_spop();
				temp1 = f_spop() < fltemp;
				int_spush( temp1 );
				break;
			case CC_STRING:
				tempptr = str_spop();
				temp1 = ustrcmp( str_spop(), tempptr ) < 0;
				int_spush( temp1 );
				break;
			}
		break;

	 case Main(N_EXP_LE):
		switch( int_kid(2,eloc) ) {
			case CC_BYTE:
			case CC_INTEGER:
				temp1 = int_spop();
				int_stop = int_stop <= temp1;
				break;
			case CC_SET:
				tempptr = (pointer)set_spop();
				temp1 = set_le((pointer)set_spop(), tempptr);
				int_spush(temp1);
				break;
			case CC_REAL:
				fltemp = f_spop();
				temp1 = f_spop() <= fltemp;
				int_spush( temp1 );
				break;
			case CC_STRING:
				tempptr = str_spop();
				temp1 = ustrcmp( str_spop(), tempptr ) <= 0;
				int_spush( temp1 );
				break;
			default:
				run_error( ER(84,"Unset type in comparison") );
			}
		break;

	 case Main(N_EXP_GT):
		switch( int_kid(2,eloc) ) {
			case CC_BYTE:
			case CC_INTEGER:
				temp1 = int_spop();
				int_stop = int_stop > temp1;
				break;
			case CC_REAL:
				fltemp = f_spop();
				temp1 = f_spop() > fltemp;
				int_spush( temp1 );
				break;
			case CC_STRING:
				tempptr = str_spop();
				temp1 = ustrcmp( str_spop(), tempptr ) > 0;
				int_spush( temp1 );
				break;
			default:
				run_error( ER(84,"Unset type in comparison") );
				cursor = eloc;
			}
		break;

	 case Main(N_EXP_GE):
		switch( int_kid(2,eloc) ) {
			case CC_BYTE:
			case CC_INTEGER:
				temp1 = int_spop();
				int_stop = int_stop >= temp1;
				break;
			case CC_SET:
				tempptr = (pointer)set_spop();
				temp1 = set_ge((pointer)set_spop(), tempptr);
				int_spush(temp1);
				break;
			case CC_REAL:
				fltemp = f_spop();
				temp1 = f_spop() >= fltemp;
				int_spush( temp1 );
				break;
			case CC_STRING:
				tempptr = str_spop();
				temp1 = ustrcmp( str_spop(), tempptr ) >= 0;
				int_spush( temp1 );
				break;
			default:
				run_error( ER(84,"Unset type in comparison") );
			}
		break;


	 case Main(N_HIDE):
		if( !within_hide ) {
			within_hide = TRUE;
			EDEBUG(4, "within hide set to %d\n", within_hide,0 );
			state_push( S_UNHIDE );
			new_loc = kid2(eloc);
			break;
			}
	 case Main(N_REVEAL):
		/* just execute kid - special for single step */
		state_push(S_NOP);
		new_loc = kid2(eloc);
		break;

		
	 case Main(N_EXP_UPLUS):
		/* no action */
		break;
	 case Main(N_EXP_UMINUS):
		if( gexp_type( eloc ) == Basetype(BT_real) ) 
			f_stop = - f_stop;
		 else
			int_stop = - int_stop;
		break;

	
	 case Main(N_EXP_NOT):
		if( is_boolean( kid2(eloc) ) )
			int_stop = !int_stop;
		 else
			int_stop = ~int_stop;
		break;

#ifdef STUPID_COMPILER
	default :
		run_error( ER(85,"bug`Got funny state/node %d"), cur_state );
		break;
#endif
