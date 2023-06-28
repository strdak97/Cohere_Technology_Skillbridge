unsigned char crc8(const unsigned char *pc_block, unsigned int len); /* this function is already implemented, use it */

int compose_packet(unsigned char *packet, unsigned int id_number, unsigned char state,
                   unsigned long long int scenario_mask, unsigned int volume, unsigned int status)
{
  if (id_number > 0xFFFF || state > 0x7F || scenario_mask > 0xFFFFFFFFFFFF ||volume > 999999 || status < 0x00FF || status > 0xFFFF) {
     return -1;
  }
  else {
  sprintf(&packet[1], "%04X%02X%012lX%06d%04X", id_number, state, scenario_mask, volume, status);
   
  packet[0] = 0x2;
  packet[29] = 0x3;
   
  packet[30] = crc8(&packet[1], 28);

  return 1;
  }
}