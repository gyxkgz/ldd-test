#include <bcm2835.h>
#include<unistd.h>

int main(int argc ,char* argv[])
{
	int n = atoi(argv[1]);
    bcm2835_init();
	bcm2835_gpio_fsel(21,BCM2835_GPIO_FSEL_OUTP);
	while(n--)
	{
		bcm2835_gpio_set(21);
		sleep(1);
		bcm2835_gpio_clr(21);
		sleep(1);
	}
	
	return 0;
	
}
