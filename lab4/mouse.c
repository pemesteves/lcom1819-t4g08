#include <lcom/lcf.h>

#include <minix/syslib.h>
#include <minix/sysutil.h>
#include <stdint.h>

#include "8042.h"
#include "mouse.h"

int hook_id_mouse; //Global variable that will contain the value of hook_id used to subscribe and unsubscribe the interrupts

uint8_t mouseByte = 0; //Global variable that will contain a mouse byte

int mouse_subscribe_int(uint8_t *bit_no) {

  hook_id_mouse = (int) *bit_no;
  //Assigning the value of bit_no to the global variable hook_id
  //so that we can preserve bit_no when we call sys_irqsetpolicy

  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_mouse) != OK) //Subscribing a notification on every interrupt in the input TIMER0_IRQ
    return 1;                                                                          //Value of hook_id will be used later in timer_unsubscribe_int()

  *bit_no = (uint8_t) BIT(*bit_no); //Returning the bit with number bit_no setting to 1

  return 0;
}

int mouse_unsubscribe_int() {

  if (sys_irqrmpolicy(&hook_id_mouse) != OK) //Unsubscribing the subscription of the interrupt notification associated with the specified hook_id
    return 1;

  return 0;
}

void(mouse_ih)() {
  mouseByte = readMouseByte();
}

uint8_t readMouseByte() {
  uint32_t data;

  sys_inb(OUT_BUF, &data);

  return (uint8_t) data;
}

int enableDataReport() {
  uint32_t ack;
  do {
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_MOUSE) != OK)
      return 1;

    if (sys_outb(OUT_BUF, ENABLE_DATA_REPORT) != OK)
      return 1;

    if (sys_inb(OUT_BUF, &ack) != OK)
      return 1;
  } while (ack != ACK);

  return 0;
}

int disableDataReport() {
  /*sys_outb(KBC_CMD_REG, READ_COM_B); //Instruction to make possible the reading of the command byte
  uint32_t cmb;
  sys_inb(OUT_BUF, &cmb);                   //Reading the command byte
  sys_outb(KBC_CMD_REG, WRITE_COM_B);       //Instruction to make possible the writing of the command byte
  cmb |= ENABLE_INT_KBD | ENABLE_INT_MOUSE; //Enabling the keyboard interrupts
  sys_outb(OUT_BUF, cmb);                   //Writing the command byte to the output buffer
*/
  uint32_t ack;
  do {
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_MOUSE) != OK)
      return 1;

    if (sys_outb(OUT_BUF, DISABLE_DATA_REPORT) != OK)
      return 1;

    if (sys_inb(OUT_BUF, &ack) != OK)
      return 1;
  } while (ack != ACK);

  return 0;
}

int setRemoteMode() {
  uint32_t ack;

  do {
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_MOUSE) != OK)
      return 1;

    if (sys_outb(OUT_BUF, DISABLE_DATA_REPORT) != OK)
      return 1;

    if (sys_inb(OUT_BUF, &ack) != OK)
      return 1;

    if (ack == ERROR)
      return 1;
  } while (ack != ACK);

  do {
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_MOUSE) != OK)
      return 1;

    if (sys_outb(OUT_BUF, REMOTE_MODE) != OK)
      return 1;

    if (sys_inb(OUT_BUF, &ack) != OK)
      return 1;

    if (ack == ERROR)
      return 1;

  } while (ack != ACK);

  return 0;
}

int issueReadData() {
  uint32_t ack;
  do {
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_MOUSE) != OK)
      return 1;

    if (sys_outb(OUT_BUF, READ_DATA) != OK)
      return 1;

    if (sys_inb(OUT_BUF, &ack) != OK)
      return 1;

    if (ack == ERROR)
      return 1;
  } while (ack != ACK);
  return 0;
}

int setStreamMode() {
  uint32_t ack;
  do {
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_MOUSE) != OK)
      return 1;

    if (sys_outb(OUT_BUF, STREAM_MODE) != OK)
      return 1;

    if (sys_inb(OUT_BUF, &ack) != OK)
      return 1;

    if (ack == ERROR)
      return 1;
  } while (ack != ACK);
  
  if (disableDataReport())
    return 1;

  uint8_t cmb = minix_get_dflt_kbc_cmd_byte();
  if (sys_outb(KBC_CMD_REG, WRITE_COM_B) != OK) //Instruction to make possible the writing of the command byte
    return 1;
  if (sys_outb(OUT_BUF, (uint32_t) cmb) != OK) //Writing the command byte to the output buffer
    return 1;

  return 0;
}