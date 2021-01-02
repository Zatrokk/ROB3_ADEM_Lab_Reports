#include <elapsedMillis.h>

uint8_t txBuffer[18];
uint8_t rxBuffer[18];

int id = 2; //write "254" for sending to every id
int torque = 0; //setting initial torque value to 0

// The protocol defines a header with fixed positions for the instruction and
// length fields:
#define DXL_LENGTH_POS 5
#define DXL_INSTRUCTION_POS 7

void put_int16t(int16_t value, uint8_t* buffer, size_t pos) //Little Endian, when sending
{
  buffer[pos] = (uint8_t)(value & 0x00ff);
  buffer[pos + 1] = (uint8_t)(value >> 8);
}

uint16_t get_uint16t(uint8_t* buffer, size_t pos) //Little Endian, when receiving
{
  uint16_t v = 0;
  v = buffer[pos + 1];
  v = v << 8;
  v = v | buffer[pos];
  return v;
}

//Function for obtaining the length of the package
inline size_t get_package_length(uint8_t* buffer) //inline is used to move the function inside the body of the calling function
{
  return get_uint16t(buffer, DXL_LENGTH_POS);
}

//CRC table
const uint16_t  crc_table[] PROGMEM =
{
  0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
  0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
  0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
  0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
  0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
  0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
  0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
  0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
  0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
  0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
  0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
  0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
  0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
  0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
  0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
  0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
  0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
  0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
  0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
  0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
  0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
  0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
  0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
  0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
  0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
  0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
  0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
  0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
  0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
  0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
  0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
  0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

//Computation of the CRC
uint16_t compute_crc(uint16_t crc_accum, uint8_t* data_blk_ptr, size_t data_blk_size)
{
  size_t i, j;

  for (j = 0; j < data_blk_size; j++)
  {
    i = ((uint16_t)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
    crc_accum = (crc_accum << 8) ^  pgm_read_word_near(crc_table + i);
  }
  return crc_accum;
}

//Function for the first 4 positions in txBuffer array, these do never change
void instruction_packet()
{
  txBuffer[0] = 0xff;
  txBuffer[1] = 0xff;
  txBuffer[2] = 0xfd;
  txBuffer[3] = 0x00;
}

//function for transfering the package
void transfer_package()
{
  size_t package_length = get_package_length(txBuffer) + 7;
  digitalWrite(3, HIGH);
  Serial1.write(txBuffer, package_length);
  Serial1.flush();
  digitalWrite(3, LOW);
}

//function for receiving data from the servos
bool receive_package(size_t timeout = 100) {
  elapsedMillis since_start = 0;
  size_t bytecount = 0;
  size_t remaining_read = 1;
  while (remaining_read > 0 && since_start < timeout) //while remaining_read is bigger than 0 and there has not passed 100 milliseconds
  {
    if (Serial1.available())
    {
      uint8_t incomming_byte = Serial1.read();//information from the serial1
      switch (bytecount)  //switch dependent on bytecount
      {
        case 0:
        case 1: if (incomming_byte == 0xFF) //checks if the incomming byte is 0
          {
            rxBuffer[bytecount] = incomming_byte; //if the position in the rxbuffer, depending on bytecount, is equal to the incomming byte, it will sum 1, and go to the next case
            ++bytecount;
          } else
          {
            bytecount = 0;
          }
          break;
        case 2: if (incomming_byte == 0xFD) //checks if the incomming byte is 0xfd
          {
            rxBuffer[bytecount] = incomming_byte; //the rxbuffer will be equal to the incomming byte
            ++bytecount;  //it will sum 1, and go to the next case
          } else
          {
            bytecount = 0;
          }
          break;
        case 3:
        case 4:
        case 5: rxBuffer[bytecount] = incomming_byte; //the byte in position 'bytecount' will be placed in the rxBuffer array
          ++bytecount;
          break;
        case 6: rxBuffer[bytecount] = incomming_byte;
          remaining_read = get_package_length(rxBuffer);
          ++bytecount;
          break;
        default: rxBuffer[bytecount] = incomming_byte;
          ++bytecount;
          --remaining_read;
          break;
      }
    }
    if (remaining_read == 0)
    {
      int message_length = rxBuffer[5] + 7;

      uint16_t received_crc = (rxBuffer[message_length - 1] << 8) | rxBuffer[message_length - 2];
      uint16_t computed_crc = compute_crc(0, rxBuffer, message_length - 2);
      if (received_crc == computed_crc)return true;
      else return false;
    }
  }
}

int16_t get_id()
{
  instruction_packet();
  txBuffer[4] = 254; //id set to 254, which will send to every possible id
  txBuffer[5] = 0x03; //length of the package (low order byte)
  txBuffer[6] = 0x00; //length of the package (high order byte)
  txBuffer[7] = 0x01; //instruction ping
  uint16_t crc = compute_crc(0, txBuffer, 8);
  txBuffer[8] = crc & 0x00FF;
  txBuffer[9] = (crc >> 8) & 0x00FF;
  transfer_package();

  receive_package();
  if (receive_package() == true)
  {
    int16_t id = rxBuffer[9] | (rxBuffer[10] << 8); //the id of the servos is placed in positions 9 and 10 of the status packet, 
    //little endian is applied for obtaining the number desired
    return id;
  }
  else return -1;
}

void send_reboot_command()
{
  instruction_packet();
  txBuffer[4] = id;
  txBuffer[5] = 0x03; //length of the package (low order byte)
  txBuffer[6] = 0x00; //length of the package (high order byte)
  txBuffer[7] = 0x08; //instruction reboot 
  uint16_t crc = compute_crc(0, txBuffer, 8);
  txBuffer[8] = crc & 0x00FF;
  txBuffer[9] = (crc >> 8) & 0x00FF;

  transfer_package();
  receive_package();
}

void enable_torque(int torque)
{
  instruction_packet();
  txBuffer[4] = id;
  txBuffer[5] = 0x06; //length of the package (low order byte)
  txBuffer[6] = 0x00; //length of the package (high order byte)
  txBuffer[7] = 0x03; //instruction write
  txBuffer[8] = 64; //adress of the torque_enable instruction (low order byte)
  txBuffer[9] = 0;  //adress of the torque_enable instruction (high order byte)
  txBuffer[10] = torque; //parameter for torque_enable
  uint16_t crc = compute_crc(0, txBuffer, 11);
  txBuffer[11] = crc & 0x00FF;
  txBuffer[12] = (crc >> 8) & 0x00FF;

  transfer_package();
  receive_package();
}

void enable_LED(int LED)
{
  instruction_packet();
  txBuffer[4] = id;
  txBuffer[5] = 0x06; //length of the package (low order byte)
  txBuffer[6] = 0x00; //length of the package (high order byte)
  txBuffer[7] = 0x03; //instruction write
  txBuffer[8] = 65; //adress of the torque_enable instruction (low order byte)
  txBuffer[9] = 0;  //adress of the torque_enable instruction (high order byte)
  txBuffer[10] = LED; //parameter for torque_enable
  uint16_t crc = compute_crc(0, txBuffer, 11);
  txBuffer[11] = crc & 0x00FF;
  txBuffer[12] = (crc >> 8) & 0x00FF;

  transfer_package();
  receive_package();
}

int16_t get_temperature()
{
  instruction_packet();
  txBuffer[4] = id;
  txBuffer[5] = 0x07; //length of the package (low order byte)
  txBuffer[6] = 0x00; //length of the package (high order byte)
  txBuffer[7] = 0x02; //instruction read
  txBuffer[8] = 146;  //adress of the present temperature instruction (low order byte)
  txBuffer[9] = 0x00; //adress of the present temperature instruction (high order byte)
  txBuffer[10] = 1; //low order byte from the data length
  txBuffer[11] = 0x00;  //high order byte from the data length
  uint16_t crc = compute_crc(0, txBuffer, 12);
  txBuffer[12] = crc & 0x00FF;
  txBuffer[13] = (crc >> 8) & 0x00FF;

  transfer_package();

  receive_package();

  if (receive_package() == true)
  {
    int8_t temperature = rxBuffer[9]; //the temperature is placed in position 9 of the status packet, no need for little endian
    //as the answer is 1 byte in length
    return temperature;
  }
  else return -1;
}

void set_P_gain(int16_t pgain)
{
  int8_t P_gain_L = pgain & 0x00FF; //low order byte of the P_gain value
  int8_t P_gain_H = (pgain >> 8) & 0x00FF;  //high order byte of the P_gain value

  instruction_packet();
  txBuffer[4] = id;
  txBuffer[5] = 0x07; //length of the package (low order byte)
  txBuffer[6] = 0x00; //length of the package (high order byte)
  txBuffer[7] = 0x03; //instruction write
  txBuffer[8] = 84; //adress of the P gain instruction (low order byte)
  txBuffer[9] = 0;  //adress of the P gain instruction (high order byte)
  txBuffer[10] = P_gain_L; 
  txBuffer[11] = P_gain_H;
  uint16_t crc = compute_crc(0, txBuffer, 12);
  txBuffer[12] = crc & 0x00FF;
  txBuffer[13] = (crc >> 8) & 0x00FF;

  transfer_package();

  receive_package();
}

int16_t get_pgain() //function for getting the pgain, for checking the actual pgain
{
  instruction_packet();
  txBuffer[4] = id;
  txBuffer[5] = 0x07; //length of the package (low order byte)
  txBuffer[6] = 0x00; //length of the package (high order byte)
  txBuffer[7] = 0x02; //instruction read
  txBuffer[8] = 84; //adress of the P gain instruction (low order byte)
  txBuffer[9] = 0x00; //adress of the P gain instruction (high order byte)
  txBuffer[10] = 2; //low order byte from the data length
  txBuffer[11] = 0x00;  //high order byte from the data length
  uint16_t crc = compute_crc(0, txBuffer, 12);
  txBuffer[12] = crc & 0x00FF;
  txBuffer[13] = (crc >> 8) & 0x00FF;

  transfer_package();

  receive_package();

  if (receive_package() == true)
  {
    int16_t P_gain = rxBuffer[9] | (rxBuffer[10] << 8); // //the p gain is placed in positions 9 and 10 of the status packet, little endian is applied
    //as the answer is 2 bytes in length
    return P_gain;
  }
  else return -1;
}

void setup()
{
  Serial.begin(9600);//the UART serial port with the computer is opened with a baud rate of 9600 bps.
  Serial1.begin(57600);// the UART serial port with the RS485 is opened with a baud rate of 57600 bps - (default of the dynamixel actuators):
  
  pinMode(3, OUTPUT); //setting digital pin 2 as outpu
}

void loop()
{
  float hertz = 1000 / 10;  //sampling frequency
  bool send_temperature = false;

  while (true)
  {
    long old_time = millis();
    if (Serial.available() > 0)
    {
      uint8_t action = Serial.read() - 48;
      switch (action)
      {
        case 0:
          enable_torque(0); //calls function enable_torque and gives variable torque the value 0
          break;
        case 1:
          enable_torque(1); //calls function enable_torque and gives variable torque the value 1
          break;
        case 2:
          enable_LED(0); //calls function enable_LED and gives variable LED the value 0
          break;
        case 3:
          enable_LED(1); //calls function enable_LED and gives variable LED the value 1
          break;
        case 4:
          send_temperature = true;  //sets function send_temperature to true
          break;
        case 5:
          send_temperature = false; //sets function send_temperature to false
          break;
        case 6:
          set_P_gain(400);  //calls function set_P_gain and gives variable pgain the value desired
          break;
        case 7:
          Serial.println(get_pgain()); //calls function get_pgain and prints the actual pgain
          break;
        case 8:
          Serial.println(get_id()); //calls function get_id and prints the id
          break;
      }
    }

    if (send_temperature == true) //when case 2 is "active"
    {
      int temperature = get_temperature();  //variable temperature is set to be what its returned from function get_temperature
      if (temperature < 80) //checking if the temperature is below the set max temperature
      {
        Serial.print("The temperature is: "); Serial.println(temperature);  //if the before mentioned statement is true, 
        //the temperature will be printed
      }
      else send_reboot_command; //if the before mentioned statement is not true, the send_reboot_command is called
    }
    while (millis() - old_time < hertz);
    //Serial.println(millis() - old_time);
  }
}
