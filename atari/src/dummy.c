
#include <stdio.h>
#include <gemdefs.h>
#include <osbind.h>

GemInit()
{
}

GemTerm()
{
}

int
GemEvent()
{
	return gem_getc();
}

GemCommand()
{
}
