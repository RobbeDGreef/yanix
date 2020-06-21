#include <cpu/ic.h>
#include <cpu/apic.h>
#include <cpu/pic.h>

int apic = 0;

int init_ic()
{
	apic = 0;
	if (init_apic() == -1)
		init_pic();

	else
		apic = 1;

	return 0;
}

void ic_send_eio(int inter)
{
	if (apic)
		apic_send_eio(inter);
	else
		pic_send_eio(inter);
}