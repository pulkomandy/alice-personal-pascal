
	/*
	 *
	 *DESC:   SPECIAL FILE OF CASES FOR INFREQUENT INTERPRETER STATES
	 *DESC:   for use with dumb compilers that can't handle really long
	 *DESC:   functions
	 *
	 */


	 case S_PROGEND:
		prep_stacks(FALSE);	/* do not reset ex_stack */
		break;

		/* end of less used states */

#ifndef ICRIPPLE
	 case Main(N_ST_WITH):
		/* we store the pointer to the record in the 3rd kid */
		tempptr = p_spop();
		EDEBUG( 4, "Calculating WITH pointer %x was%x\n", (int)tempptr, 
				(int)(pointer) kid3(eloc) );
		p_spush( (pointer) kid3( eloc ) );	/* save away old one */
		s_kid3( eloc, NCAST tempptr );
		new_loc = kid2(eloc);
		state_push( S_WITHDONE );
		break;

	 case S_WITHDONE:
		/* restore the pointer that was there */
		s_kid3( eloc, NCAST p_spop() );
		break;

	 case Main(N_ST_CASE):
		find_case( int_spop(), FLCAST kid2(eloc), TRUE );
		break;
	 case Main(N_ST_SPECIAL):
		new_loc = kid2(eloc);
		state_push( S_NOP );
		break;
#endif ICRIPPLE
	 case Main(N_ST_BLOCK):
		/* we just did the block! */
		break;
	 case Main(N_ST_COUT):
		break;
	 case Main(N_OEXP_2): /* handle kid2 then go to push a default */
		/*
		state_push( S_OEXP21 );
		new_loc = kid2(eloc);
		*/
		break;
	 case Main(N_OEXP_3):	/* first two pushed, now do #3 */
		state_push( S_NOP );
		new_loc = kid3(eloc);
		break;

	 case S_CALL3:
		/* a list of i/o parameters, execute with type and file */
		/* just got back.  call routine, check, go */

		nkid = ++estop(e_int);

#ifdef PROC_INTERP
		EDEBUG( 6, "Calling function number %d address %x\n", /* { */
			tparent(get_routine(eloc,&dummy))-}b_func, 
			bfptr( get_routine(eloc,&dummy) ) );
		fflush( etrace );
#endif

		/* call the read/write function, which pops the value */
		/*   off the top of the stack for us */
		tempnode = node_kid(kid2(eloc), nkid-1 );
		type_ptr = gexp_type(tempnode);
		if( nkid == 1 && ( ntype(type_ptr)==N_TYP_FILE ) ) {
			/* this is the file for read write */
			/* file is stored at estack[1] */
			esoff(1,e_pointer) = p_spop();
			}

		 else

			bfptr(get_routine(eloc, &dummy)) (1, type_ptr, tempnode,
					esoff(1,e_pointer));
		if( nkid < listcount(FLCAST kid2(eloc) ) ) {
			state_push(S_CALL3);
			new_loc = node_kid( kid2(eloc), nkid );
			}
		 else {
			bfptr(get_routine(eloc,&dummy))(0, NIL, NIL, esoff(1,e_pointer));
			espop(e_int);	/* eliminate count */
			espop(e_pointer);	/* eliminate file */
			}
		break;

	 
#ifndef ICRIPPLE
	 case Main(N_ST_LABEL):
	 case Main(N_NOTDONE):
		/* also fairly boring */
		break;


	 case Main(N_ST_GOTO):
		do_a_goto();
		break;

#endif ICRIPPLE
	
	 case Main(N_T_COMMENT):
		/* should not normally get here */
		run_error(ER(55,"Got to comment %x"), eloc );
		break;

	 case Main(N_PROGRAM):
		/* should init text variables and all that rot */
		/* no arguments for main */
		/* to fool out make frame, push an argument pointer */



		st_display = (stack_pointer *)0;
		espush( (pointer)sp.st_stackloc, e_pointer );

		make_frame( 1, act_size(eloc)/sizeof(stackloc), NIL, eloc,
				st_display, FALSE );
		new_loc = kid5(eloc);
		state_push( S_PROGEND );
		break;

	default :
		run_error( ER(56,"bug`Got funny state/node %d"), cur_state );
		break;

	 case Main(N_ST_CALL):
		/* Just call the function in runtime.c to start this up
		 */
		/* we can recurse here as there is no call within a call */
		start_call( eloc );

		break;

#ifndef ICRIPPLE
	 case S_CALL1:	/* simulate list for routine, handle value arrays */
		do_1call( eloc );
		break;

	 case S_RETURN: /* return from a procedure or function */
		/* restore the stack pointer */
		thescope = *(pointer)st_display;	/* current scope */
		/* arg pointer for the guy who just came back is our stack */
		sp.st_stackloc = st_display[-thescope].st_stackloc;
		call_depth--;
		/* check that return value is clean */

		EDEBUG(4, "Reset stack to -%d index of %x\n", thescope,
				(int)st_display );

		
#ifdef UDEBUG
		within_hide = (int)(get_fpointers(st_display)[2]);
		EDEBUG(4, "within hide set to %d\n", within_hide,0 );

#endif
		st_display = up_frame(st_display);
		/*loc_stack = (estackloc *)(get_fpointers(st_display)[3]);*/
		/* close local files openend in this function */
		{
		extern pointer free_top;
		closeall(FALSE,free_top,sp.st_generic);
		}
		/* if a function, we should check return value */
		if( ntype(eloc) == N_EXP_FUNC ) {
#ifdef TURBO
			nodep rettype;
			thesym = get_routine(eloc, &dummy);
			rettype = sym_type(thesym);
			if( ntype(rettype) == N_TYP_STRING ) {
				unsigned int retsize, retlen;
				int diff;
				/* must be careful of allignment problems */
				retsize = size_adjust(2+int_kid(1,rettype));
				retlen = sp.st_generic[0];
				do_undef( sp.st_generic+1, U_CHECK, retlen+1);
				EDEBUG(5,"str return len %d size+2 %d\n",retlen,retsize);
				retlen = word_allign(retlen);
				if( diff = retsize - 2 - retlen ) {
					blk_move( sp.st_generic + diff, 
						sp.st_generic, retlen+2 );
					unload_stack( diff );
					}
				p_spush( (pointer)0 ); /* for string pop */
				do_undef( sp.st_generic,U_SET,retlen+2+sizeof(pointer));

				}
#endif TURBO
			check_undef( sp.st_stackloc, sym_size(thesym) );
			}
		cur_state = 0;	/* allow real number conversion */
		break;
	
#endif ICRIPPLE
	 case S_CALL2: /* simulated list, builtin procedure */
		/* just got back.  increment, check, go */
		nkid = ++estop(e_int);
		thesym = get_routine(eloc, &dummy);
		if( nkid < listcount(FLCAST kid2(eloc)) ) {
			state_push(S_CALL2);
			new_loc = node_kid( kid2(eloc), nkid );
			/* first push the type onto list */
			if( sym_dtype(thesym) < T_BPROC )	
				p_spush( (pointer)gexp_type(new_loc) );
			}
		 else {
			pointer old_stack;

			espop(e_int);	/* eliminate count */
			old_stack = estop(e_pointer);
			/* get set to call it */
			EDEBUG(5,"Call builtin with %d args at %x\n",
				nkid, (int)old_stack );
			bfptr(thesym)(nkid, old_stack, thesym);
			/* pop off all the args we pushed */
			/* use value on stack so builtin can change it */
			sp.st_stackloc = espop(e_pointer);
			cur_state = 0;
			}
		break;
	
	 case Main(N_ST_ASSIGN):
		/* There should be type checking.
		 * The address we are to place in should be on the left,
		 * and the value on the right.  The actual form of assignment
		 * must depend on the type, as we could be talking bytes,
		 * words or even arrays.
		 * both values are currently on stack
		 */

		 /* bytes are stored in ints, but we don't know what order */

		temp1 = int_kid(2,eloc);
#ifdef LARGE
		EDEBUG( 8, "Assignment type %d at %lx\n", temp1, eloc );
#else
		EDEBUG( 8, "Assignment type %d at %x\n", temp1, eloc );
#endif
		if( temp1 >= 0 )
		 switch( temp1 & 0x1f ) {
#ifdef Notdef
			case CC_CHRSTR:
				tempptr = str_spop();
				if( tempptr[0] != 1 )
					run_error( ER(293,"strcast`String of length %d assigned to char variable"), tempptr[0] );
				btemp1 = tempptr[STR_START];
				goto popadr;
#endif
			case CC_BYTE:
				temp2 = int_spop();
			 popadr:
				tempptr = p_spop();
				do_undef( tempptr, U_SET, sizeof(char) );
				*(bits8 *)tempptr = temp2;
				/* check subranges */
				if( temp1 & CC_SUBRANGE )
					sr_check(gexp_type(kid1(eloc)),(rint)temp2 );
#ifndef SAI
				if( step_flag & STEP_SINGLE  && !within_hide) {
					extern char *ordname();
					nonum_message( "Scalar := %s",
					 ordname(gexp_type(kid1(eloc)),temp2) );
					 }
#endif
				EDEBUG(3,"Assign byte %d\n", temp2, 0 );
				break;
#ifdef FULL
			case CC_PBYTE:
			case CC_PWORD:
				temp2 = int_spop();
				untemp = int_spop();
				outp( untemp, temp2 & 0xFF );
				if( temp1 == CC_PWORD )
					outp( untemp+1, (unsigned)temp2 >> 8 );
				break;
#endif

			case CC_STRPOINT:
				++p_stop;	/* increment string */
			case CC_POINTER:
				resptr = p_spop();
				tempptr = p_spop();
				do_undef( tempptr, U_SET, sizeof(pointer) );
				*(pointer *)tempptr = resptr;
				break;
			case CC_INTEGER:
				temp2 = int_spop();
				tempptr = p_spop();
				do_undef( tempptr, U_SET, sizeof(rint) );
				*(rint *)tempptr = temp2;
				/* check subranges */
				if( temp1 & CC_SUBRANGE )
					sr_check( gexp_type(kid1(eloc)),temp2 );
#ifndef SAI
				if( step_flag & STEP_SINGLE && !within_hide )
					nonum_message( "Integer := %d", temp2 );
#endif
				EDEBUG( 3,"Assign int/pointer %x\n", temp2, 0);
				break;
#ifndef ICRIPPLE
			case CC_REAL:
				fltemp = f_spop();
				tempptr = p_spop();
				do_undef( tempptr, U_SET, sizeof(rfloat) );
				*(rfloat *)tempptr = fltemp;
#ifndef SAI
				if( step_flag & STEP_SINGLE  && !within_hide)
					nonum_message( "Real := %g", fltemp );
#endif
				EDEBUG( 3, "Assign floating pt %g to %x\n",fltemp, (int)tempptr);
				break;
			case CC_SET:
				resptr = (pointer)set_spop();
				set_assign(p_spop(), resptr, eloc);
				break;
			case CC_STRING:
				resptr = str_spop();	/* ptr to string */
				tempptr = p_spop();	/* where it goes */
				str_assign( tempptr, resptr,
					get_stsize(gexp_type(kid1(eloc))) );
#ifndef SAI
				if( step_flag & STEP_SINGLE  && !within_hide)
					nonum_message( "String := \"%s\"",tempptr+1 );
#endif
				break;
#endif ICRIPPLE
			}
		 else {  /* block assign */
				resptr = p_spop();	/* address of struct */
				tempptr = p_spop();
				temp1 = -temp1;	

				EDEBUG(3,"Assign mem at %x to %x\n", (int)resptr, (int)tempptr );
				/* just copy over the bitmap, don't check it */
				copy_mask( tempptr, resptr, (int)temp1 );
				blk_move( tempptr, resptr, (int)temp1 );
				break;
			}
		break;

	 case Main(N_ST_IF):
		/* condition pushed */
		if( int_spop() ) {
			state_push( S_NOP );
			new_loc = kid2(eloc);
			}
		break;
	 case Main(N_ST_ELSE):
		/* condition pushed */
		state_push(S_NOP);
		new_loc = int_spop() ? kid2(eloc) : kid3( eloc );
		break;
	 case Main(N_ST_FOR):
	 case Main(N_ST_DOWNTO):
		/* variable and start pushed */
		state_push( S_FORTB );
		new_loc = kid3(eloc);
		break;
	 case S_FORTB:
		/* top of stack - upper bound, lower bound, var */
		temp1 = stloc(stackof(pointer),rint );
		sr_ccheck( kid1(eloc), temp1 );
		sr_ccheck( kid1(eloc), int_stop );
#ifdef BIGENDIAN
		/* if we have a brain damaged byte ordering and a byte
		 * size for loop, we must decrement the pointer so that
		 * the integer dealt with by this code cooresponds to the
		 * byte
		 */
		if( sym_size( kid_id(kid1(eloc))) == 1 )
			--stloc(2*stackof(pointer),pointer);
#endif
		intstar = (rint *)stloc(2*stackof(pointer),rint *);
		*intstar = temp1;
		do_undef( intstar, U_SET, sizeof(rint) );
		notfirst = FALSE;
		if( ntype(eloc) == N_ST_FOR )
			goto do_for1;
		 else
			goto do_down1;

	 case S_FOR2:	/* after loop is done */
		intstar = (rint *)stloc(2*stackof(pointer),rint *);
		if( *intstar == MAXINT )
			goto loop_done;
		++*intstar; /*increment variable, an int */
			/* fall through to first state */
	 case S_FOR1:
	 do_for1:
		/* now check variable and boundary */
		EDEBUG(4,"For top %d, variable is %d\n", int_stop, *intstar);
		if( int_stop < *intstar )  {
		  loop_done:
			int_spop();
			int_spop();
			/* mark the loop variable as a no-no */
			tempptr = p_spop();
			do_undef( tempptr, U_CLEAR, sizeof(rint) );
			break; /* finished */
			}
#ifdef Trace
		if( most_trlocs && !within_trace && notfirst )
			scan_trace( intstar, sizeof(rint) );
		notfirst = TRUE;
#endif
		
		state_push( S_FOR2 );
		new_loc = kid4(eloc);
		break;

#ifndef ICRIPPLE
	 case S_2DOWNTO:	/* after loop is done */
		intstar = (rint *)stloc(2*stackof(pointer), rint *); 
		if( *intstar == -MAXINT-1 )
			goto loop_done;
		(*intstar)--;
			/* fall through to first state */
	 case S_1DOWNTO:
	 do_down1:
		/* now check variable and boundary */
		if( int_stop > *intstar )  {
			goto loop_done;
			}
#ifdef Notdef
		if( most_trlocs && !within_trace && notfirst )
			scan_trace( intstar, sizeof(rint) );
		notfirst = TRUE;
#endif
		state_push( S_2DOWNTO );
		new_loc = kid4(eloc);
		break;

	 case S_SET1:
		nkid = ++estop(e_int);
		temp1 = int_spop();
#ifdef UDEBUG
		if( temp1 < MIN_SET || temp1 > MAX_SET )
			run_error(ER(57,"setsize`Sorry, sets element %d not allowed"),
						temp1 );
#endif UDEBUG
		arg_list = FLCAST kid1( eloc );
		if( ntype(node_kid( arg_list, nkid-1 )) == N_SET_SUBRANGE ) {
			/* do what must be done */
			temp2 = int_spop();	/* first element */
#ifdef UDEBUG
	/* PUT IN A CALL TO ORDNAME HERE */
		/* IT IS NOT AN ERROR TO HAVE RANGE IN WRONG ORDER, it is [] */
			if( temp2 < MIN_SET || temp2 > MAX_SET )
				run_error(ER(57,"setsize`Sorry, sets element %d not allowed"),
							temp2 );
#endif UDEBUG
			/* set all the bits the slow way */
			for( ;temp1 >= temp2; temp1-- )
				set_stop[ temp1 >> 3] |= 1 << (temp1 & 7);
				
			}
		 else {
			set_stop[ temp1 >> 3] |= 1 << (temp1 & 7);
			}

		if( nkid < listcount(arg_list) ) {
			state_push(S_SET1);
			new_loc = node_kid( arg_list, nkid );
			}
		 else {
			espop(e_int);	/* eliminate count */
			do_undef( (pointer)set_stop, U_SET, SET_BYTES );
			}
		break;
#endif ICRIPPLE

	 case S_UNHIDE:
		within_hide = FALSE;
		EDEBUG(4, "within hide set to %d\n", within_hide,0 );
		break;

#ifdef Trace
	 case Main(N_ST_TRACE):
		traceingon( p_spop(), calc_size( gexp_type(kid1(eloc)), TRUE ) );
		break;
	
	 case S_UNTRACE:
		{
		extern int within_trace;
		within_trace = FALSE;
		within_hide = int_spop();
		st_display = (stack_pointer *)p_spop();
		}
		break;
#else
	case Main(N_ST_TRACE):
		p_spop();
		break;
#endif
