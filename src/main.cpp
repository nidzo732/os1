#include "core.h"

int main(int argc, char *argv[])
{
	asm cli;
	go(argc, argv);
	int toReturn=get_main_result();
	asm sti;
	return toReturn;
}
