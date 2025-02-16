/*
  sysctrl.c

  MiSTeryNano system control interface
*/

#include "sysctrl.h"

unsigned char core_id = 0;

static const char *core_names[] = {
  "<unset>", "Atari ST", "C64"
};

static void sys_begin(spi_t *spi, unsigned char cmd) {
  spi_begin(spi);  
  spi_tx_u08(spi, SPI_TARGET_SYS);
  spi_tx_u08(spi, cmd);
}  

int sys_status_is_valid(spi_t *spi) {
  sys_begin(spi, SPI_SYS_STATUS);
  spi_tx_u08(spi, 0);
  unsigned char b0 = spi_tx_u08(spi, 0);
  unsigned char b1 = spi_tx_u08(spi, 0);
  core_id = spi_tx_u08(spi, 0);
  spi_end(spi);  

  if((b0 == 0x5c) && (b1 == 0x42)) {
    printf("Core ID: %02x\r\n", core_id);
    if(core_id < 3) printf("Core: %s\r\n", core_names[core_id]);
  }
  
  return((b0 == 0x5c) && (b1 == 0x42));
}

void sys_set_leds(spi_t *spi, char leds) {
  sys_begin(spi, SPI_SYS_LEDS);
  spi_tx_u08(spi, leds);
  spi_end(spi);  
}

void sys_set_rgb(spi_t *spi, unsigned long rgb) {
  sys_begin(spi, SPI_SYS_RGB);
  spi_tx_u08(spi, (rgb >> 16) & 0xff); // R
  spi_tx_u08(spi, (rgb >> 8) & 0xff);  // G
  spi_tx_u08(spi, rgb & 0xff);         // B
  spi_end(spi);    
}

unsigned char sys_get_buttons(spi_t *spi) {
  unsigned char btns = 0;
  
  sys_begin(spi, SPI_SYS_BUTTONS);
  spi_tx_u08(spi, 0x00);
  btns = spi_tx_u08(spi, 0);
  spi_end(spi);

  return btns;
}

void sys_set_val(spi_t *spi, char id, uint8_t value) {
  printf("SYS set value %c = %d\r\n", id, value);
  
  sys_begin(spi, SPI_SYS_SETVAL);   // send value command
  spi_tx_u08(spi, id);              // value id
  spi_tx_u08(spi, value);           // value itself
  spi_end(spi);  
}

unsigned char sys_irq_ctrl(spi_t *spi, unsigned char ack) {
  sys_begin(spi, SPI_SYS_IRQ_CTRL);
  spi_tx_u08(spi, ack);
  unsigned char ret = spi_tx_u08(spi, 0);
  spi_end(spi);  
  return ret;
}

void sys_handle_interrupts(unsigned char pending) {
  if(pending & 0x02) // irq 1 = HID
    hid_handle_event();
  
  if(pending & 0x08) // irq 3 = SDC
    sdc_handle_event();
}

