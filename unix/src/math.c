/*
 * Various math functions
 */

#include <errno.h>
#include <math.h>

double	frexp();

static double pi	 =3.14159265358979323846264338e0;
static double sq2p1	 =2.414213562373095048802e0;
static double sq2m1	 = .414213562373095048802e0;
static double pio2	 =1.570796326794896619231e0;
static double pio4	 = .785398163397448309615e0;

double
_sin(arg)
register double	arg;
{
	register long	nPiFromZero;
	register double x;
	register double	x2;
	register double	xn;
	register double	result;

	/*
	 * put x in range -pi/2 <= x <= pi/2
	 */
	nPiFromZero = (long)(arg / pi + 0.5);
	x = arg - nPiFromZero * pi;
	x2 = x * x;

	result = ((((((1.605904384e-10	* x2
		       - 2.505210839e-8)* x2
		      + 2.755731922e-6)	* x2
		     - 1.984126984e-4)	* x2
		    + 8.333333333e-3)	* x2
		   - 1.666666667e-1)	* x2
		  + 1.0) * x;
	/* max. error < (pi/2)^15/15! < 6.6e-10 */

	if (nPiFromZero % 2 == 1)
		result *= -1.0;
	return result;
}

double
_cos(arg)
register double arg;
{
	return sin(arg + pio2);
}

/*
	floating-point arctangent

	atan returns the value of the arctangent of its
	argument in the range [-pi/2,pi/2].

	there are no error returns.

	coefficients are #5077 from Hart & Cheney. (19.56D)
*/
static double atan_p4	 = .161536412982230228262e2;
static double atan_p3	 = .26842548195503973794141e3;
static double atan_p2	 = .11530293515404850115428136e4;
static double atan_p1	 = .178040631643319697105464587e4;
static double atan_p0	 = .89678597403663861959987488e3;
static double atan_q4	 = .5895697050844462222791e2;
static double atan_q3	 = .536265374031215315104235e3;
static double atan_q2	 = .16667838148816337184521798e4;
static double atan_q1	 = .207933497444540981287275926e4;
static double atan_q0	 = .89678597403663861962481162e3;

/*
	atan makes its argument positive and
	calls the inner routine satan.
*/

double
_atan(arg)
double arg;
{
	double satan();

	if(arg>0)
		return(satan(arg));
	else
		return(-satan(-arg));
}
/*
	satan reduces its argument (known to be positive)
	to the range [0,0.414...] and calls xatan.
*/

static double
satan(arg)
double arg;
{
	double	xatan();

	if(arg < sq2m1)
		return(xatan(arg));
	else if(arg > sq2p1)
		return(pio2 - xatan(1.0/arg));
	else
		return(pio4 + xatan((arg-1.0)/(arg+1.0)));
}

/*
	xatan evaluates a series valid in the
	range [-0.414...,+0.414...].
*/

static double
xatan(arg)
double arg;
{
	double argsq;
	double value;

	argsq = arg*arg;
	value = ((((atan_p4*argsq + atan_p3)*argsq + atan_p2)*argsq + atan_p1)*argsq + atan_p0);
	value = value/(((((argsq + atan_q4)*argsq + atan_q3)*argsq + atan_q2)*argsq + atan_q1)*argsq + atan_q0);
	return(value*arg);
}

/*
	log returns the natural logarithm of its floating
	point argument.

	The coefficients are #2705 from Hart & Cheney. (19.38D)

	It calls frexp.
*/

static double	log2	= 0.693147180559945309e0;
static double	ln10	= 2.302585092994045684;
static double	sqrto2	= 0.707106781186547524e0;
static double	log_p0	= -.240139179559210510e2;
static double	log_p1	= 0.309572928215376501e2;
static double	log_p2	= -.963769093368686593e1;
static double	log_p3	= 0.421087371217979714e0;
static double	log_q0	= -.120069589779605255e2;
static double	log_q1	= 0.194809660700889731e2;
static double	log_q2	= -.891110902798312337e1;

double
_log(arg)
double arg;
{
	double x,z, zsq, temp;
	int exp;

#ifdef notdef
	if(arg <= 0.) {
		errno = EDOM;
		return(-HUGE);
	}
#endif
	x = frexp(arg,&exp);
	while(x<0.5) {
		x = x*2;
		exp = exp-1;
	}
	if(x<sqrto2) {
		x = 2*x;
		exp = exp-1;
	}

	z = (x-1)/(x+1);
	zsq = z*z;

	temp = ((log_p3*zsq + log_p2)*zsq + log_p1)*zsq + log_p0;
	temp = temp/(((1.0*zsq + log_q2)*zsq + log_q1)*zsq + log_q0);
	temp = temp*z + exp*log2;
	return(temp);
}

#ifdef notdef
double
log10(arg)
double arg;
{

	return(log(arg)/ln10);
}
#endif

/*
	exp returns the exponential function of its
	floating-point argument.

	The coefficients are #1069 from Hart and Cheney. (22.35D)
*/

static double	exp_p0	= .2080384346694663001443843411e7;
static double	exp_p1	= .3028697169744036299076048876e5;
static double	exp_p2	= .6061485330061080841615584556e2;
static double	exp_q0	= .6002720360238832528230907598e7;
static double	exp_q1	= .3277251518082914423057964422e6;
static double	exp_q2	= .1749287689093076403844945335e4;
static double	log2e	= 1.4426950408889634073599247;
static double	sqrt2	= 1.4142135623730950488016887;
static double	maxf	= 10000;

double
exp(arg)
double arg;
{
	double fract;
	double temp1, temp2, xsq;
	int ent;

	if(arg == 0.)
		return(1.);
	if(arg < -maxf)
		return(0.);
#ifdef notdef
	if(arg > maxf) {
		errno = ERANGE;
		return(HUGE);
	}
#endif
	arg *= log2e;
	ent = floor(arg);
	fract = (arg-ent) - 0.5;
	xsq = fract*fract;
	temp1 = ((exp_p2*xsq+exp_p1)*xsq+exp_p0)*fract;
	temp2 = ((1.0*xsq+exp_q2)*xsq+exp_q1)*xsq + exp_q0;
	return(ldexp(sqrt2*(temp2+temp1)/(temp2-temp1), ent));
}

/*
	sqrt returns the square root of its floating
	point argument. Newton's method.

	calls frexp
*/

double
_sqrt(arg)
double arg;
{
	double x, temp;
	int exp;
	int i;

	if(arg <= 0.) {
#ifdef notdef
		if(arg < 0.)
			errno = EDOM;
#endif
		return(0.);
	}
	x = frexp(arg,&exp);
	while(x < 0.5) {
		x *= 2;
		exp--;
	}
	/*
	 * NOTE
	 * this wont work on 1's comp
	 */
	if(exp & 1) {
		x *= 2;
		exp--;
	}
	temp = 0.5*(1.0+x);

	while(exp > 60) {
		temp *= (1L<<30);
		exp -= 60;
	}
	while(exp < -60) {
		temp /= (1L<<30);
		exp += 60;
	}
	if(exp >= 0)
		temp *= 1L << (exp/2);
	else
		temp /= 1L << (-exp/2);
	for(i=0; i<=4; i++)
		temp = 0.5*(temp + arg/temp);
	return(temp);
}

main()
{
	double	d;

	for (;;) {
		scanf("%lf", &d);
		printf("%lg -- e.sin=%lg, e.cos=%lg, e.atan=%lg, e.log=%lg, e.sqrt=%lg\n",
		d,
		sin(d) - _sin(d),
		cos(d) - _cos(d),
		atan(d) - _atan(d),
		log(d) - _log(d),
		sqrt(d) - _sqrt(d));
	}
}
