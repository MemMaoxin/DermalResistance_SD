#include "boards.h"
#include "nrf_drv_gpiote.h"
 

void in_pin_handler(nrf_drv_gpiote_pin_t pin,nrf_gpiote_polarity_t action)
{
	if (pin == BUTTON_1)
		nrf_gpio_pin_toggle(7);
	
}
 
int main(void)
{
	ret_code_t err_code;

	bsp_board_init(BSP_INIT_LEDS);

	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);

	config.pull = NRF_GPIO_PIN_PULLUP;

	err_code = nrf_drv_gpiote_in_init(BUTTON_1,&config,in_pin_handler);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(BUTTON_1,true);
	while(true)
	{
	
	}
	return 0;
}