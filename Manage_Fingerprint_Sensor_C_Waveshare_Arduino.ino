#include<SoftwareSerial.h>
#include"finger.h"

uint8_t id;
uint8_t finger_RxBuf[9];      
uint8_t finger_TxBuf[9];  

uint8_t  Finger_SleepFlag = 0;
int user_id;

SoftwareSerial mySerial(4, 5, false); // RX, TX   

void setup() {

   Serial.begin(19200);
  
   Finger_SoftwareSerial_Init(); 
   delay(1000);
   
   Finger_Wait_Until_OK();
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Choix : ");
  Serial.println(id);

  Analysis_PC_Command();

}

/***************************************************************************
* @brief      initialize the SoftwareSerial to communicate with Fingerprint module
****************************************************************************/
void Finger_SoftwareSerial_Init(void)
{
  mySerial.begin(19200);
  Serial.println(" ");  
}

/***************************************************************************
* @brief      Send a byte of data to the serial port
* @param      temp : Data to send
****************************************************************************/
void  TxByte(uint8_t temp)
{
  mySerial.write(temp);    
}

/***************************************************************************
* @brief      send a command, and wait for the response of module
* @param      Scnt: The number of bytes to send
        Rcnt: expect the number of bytes response module
        Nms: wait timeout: Delay
* @return     ACK_SUCCESS: success
          other: see the macro definition
****************************************************************************/
uint8_t TxAndRxCmd(uint8_t Scnt, uint8_t Rcnt, uint16_t Nms)
{
  uint8_t  i, j, CheckSum;
  uint16_t   uart_RxCount = 0;
  unsigned long  time_before = 0;
  unsigned long  time_after = 0;
  uint8_t   overflow_Flag = 0;; 
  
   TxByte(CMD_HEAD);     
   CheckSum = 0;
   for (i = 0; i < Scnt; i++)
   {
    TxByte(finger_TxBuf[i]);     
    CheckSum ^= finger_TxBuf[i];
   }  
   TxByte(CheckSum);
   TxByte(CMD_TAIL);  
   
   memset(finger_RxBuf,0,sizeof(finger_RxBuf));   ////////
 
   mySerial.flush();  /////
   
   // Receive time out: Nms
  time_before = millis();  
   do
   {
    overflow_Flag = 0;
    if(mySerial.available())
    {
      finger_RxBuf[uart_RxCount++] = mySerial.read();
    }
    time_after = millis();  
    if(time_before > time_after)   //if overflow (go back to zero)
    {
      time_before = millis();   // get time_before again
      overflow_Flag = 1;
    }
    
   } while (((uart_RxCount < Rcnt) && (time_after - time_before < Nms)) || (overflow_Flag == 1));

   user_id=finger_RxBuf[2]*0 +finger_RxBuf[3];
   if (uart_RxCount != Rcnt)return ACK_TIMEOUT;
   if (finger_RxBuf[0] != CMD_HEAD) return ACK_FAIL;
   if (finger_RxBuf[Rcnt - 1] != CMD_TAIL) return ACK_FAIL;
   if (finger_RxBuf[1] != (finger_TxBuf[0])) return ACK_FAIL;  
   CheckSum = 0;
   for (j = 1; j < uart_RxCount - 1; j++) CheckSum ^= finger_RxBuf[j];
   if (CheckSum != 0) return ACK_FAIL;    
   return ACK_SUCCESS;
}  

/***************************************************************************
* @brief      Query the number of existing fingerprints
* @return     0xFF: error
          other: success, the value is the number of existing fingerprints
****************************************************************************/
uint8_t GetUserCount(void)
{
  uint8_t m;
  
  finger_TxBuf[0] = CMD_USER_CNT;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = 0;
  finger_TxBuf[3] = 0;
  finger_TxBuf[4] = 0;  
  
  m = TxAndRxCmd(5, 8, 200);
      
  if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
  {
      return finger_RxBuf[3];
  }
  else
  {
    return 0xFF;
  }
}

/***************************************************************************
* @brief      Get Compare Level
* @return     0xFF: error
          other: success, the value is compare level
****************************************************************************/
uint8_t GetcompareLevel(void)
{
  uint8_t m;
  
  finger_TxBuf[0] = CMD_COM_LEV;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = 0;
  finger_TxBuf[3] = 1;
  finger_TxBuf[4] = 0;  
  
  m = TxAndRxCmd(5, 8, 200);
    
  if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
  {
      return finger_RxBuf[3];
  }
  else
  {
    return 0xFF;
  }
}

/***************************************************************************
* @brief      Set Compare Level
* @param      temp: Compare Level,the default value is 5, can be set to 0-9, the bigger, the stricter
* @return     0xFF: error
          other: success, the value is compare level
****************************************************************************/
uint8_t SetcompareLevel(uint8_t temp)
{
  uint8_t m;
  
  finger_TxBuf[0] = CMD_COM_LEV;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = temp;
  finger_TxBuf[3] = 0;
  finger_TxBuf[4] = 0;  
  
  m = TxAndRxCmd(5, 8, 200);

  if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
  {
      return finger_RxBuf[3];
  }
  else
  {
    return 0xFF;
  }
}

/***************************************************************************
* @brief      Get the time that fingerprint collection wait timeout 
* @return     0xFF: error
          other: success, the value is the time that fingerprint collection wait timeout 
****************************************************************************/
uint8_t GetTimeOut(void)
{
  uint8_t m;
  
  finger_TxBuf[0] = CMD_TIMEOUT;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = 0;
  finger_TxBuf[3] = 1;
  finger_TxBuf[4] = 0;  
  
  m = TxAndRxCmd(5, 8, 200);
    
  if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
  {
      return finger_RxBuf[3];
  }
  else
  {
    return 0xFF;
  }
}

/***************************************************************************
* @brief      Register fingerprint
* @return     ACK_SUCCESS: success
          other: see the macro definition
****************************************************************************/
uint8_t AddUser(void)
{
  uint8_t m;
  
  m = GetUserCount();
  if (m >= USER_MAX_CNT)
    return ACK_FULL;


  finger_TxBuf[0] = CMD_ADD_1;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = m +1;
  finger_TxBuf[3] = 3;
  finger_TxBuf[4] = 0;    
  m = TxAndRxCmd(5, 8, 6000); 
  if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
  {
    finger_TxBuf[0] = CMD_ADD_3;
    m = TxAndRxCmd(5, 8, 6000);
    if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
    {
      return ACK_SUCCESS;
    }
    else
    return ACK_FAIL;
  }
  else
    return ACK_FAIL;
}

/***************************************************************************
* @brief      Clear fingerprints
* @return     ACK_SUCCESS:  success
          ACK_FAIL:     error
****************************************************************************/
uint8_t ClearAllUser(void)
{
  uint8_t m;
  
  finger_TxBuf[0] = CMD_DEL_ALL;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = 0;
  finger_TxBuf[3] = 0;
  finger_TxBuf[4] = 0;
  
  m = TxAndRxCmd(5, 8, 500);
  
  if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
  {     
    return ACK_SUCCESS;
  }
  else
  {
    return ACK_FAIL;
  }
}

uint8_t ClearFingerprint(void)
{
  uint8_t m;

  finger_TxBuf[0] = CMD_DEL;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = 0;
  finger_TxBuf[3] = 0;
  finger_TxBuf[4] = 0;
  
  m = TxAndRxCmd(5, 8, 500);
  
  if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
  {     
    return ACK_SUCCESS;
  }
  else
  {
    return ACK_FAIL;
  }
}

/***************************************************************************
* @brief      Check if user ID is between 1 and 3
* @return     TRUE
          FALSE
****************************************************************************/
uint8_t IsMasterUser(uint8_t UserID)
{
    if ((UserID == 1) || (UserID == 2) || (UserID == 3)) return TRUE;
      else  return FALSE;
}  

/***************************************************************************
* @brief      Fingerprint matching
* @return     ACK_SUCCESS: success
          other: see the macro definition
****************************************************************************/
uint8_t VerifyUser(void)
{
  uint8_t m;
  
  finger_TxBuf[0] = CMD_MATCH;
  finger_TxBuf[1] = 0;
  finger_TxBuf[2] = 0;
  finger_TxBuf[3] = 0;
  finger_TxBuf[4] = 0;
  
  m = TxAndRxCmd(5, 8, 5000);
    
  if ((m == ACK_SUCCESS) && (IsMasterUser(finger_RxBuf[4]) == TRUE) && finger_RxBuf[3] != 0)
  { 
     return ACK_SUCCESS;
  }
  else if(finger_RxBuf[4] == ACK_NO_USER)
  {
    return ACK_NO_USER;
  }
  else if(finger_RxBuf[4] == ACK_TIMEOUT)
  {
    return ACK_TIMEOUT;
  }

}

/***************************************************************************
* @brief      Wait until the fingerprint module works properly
****************************************************************************/
void Finger_Wait_Until_OK(void)
{   
    //digitalWrite(Finger_RST_Pin , LOW);
  //delay(300); 
    //digitalWrite(Finger_RST_Pin , HIGH);
  //delay(300);  // Wait for module to start
    
   // ERROR: Please ensure that the module power supply is 3.3V or 5V, 
   // the serial line is correct, the baud rate defaults to 19200,
   // and finally the power is switched off, and then power on again !
  while(SetcompareLevel(5) != 5)
  {   
    Serial.println("*Please waiting for communicating normally, if it always keep waiting here, please check your connection!*");
  }

  Serial.println(" ");
  Serial.write("*************** WaveShare Capacitive Fingerprint Reader Test ***************\r\n");
  Serial.write("Compare Level:  5    (can be set to 0-9, the bigger, the stricter)\r\n"); 
  Serial.write("Fingerprint recorded in database:  ");  Serial.print(GetUserCount());
  Serial.write("\r\n Use the serial port to send the commands to operate the module:\r\n"); 
  Serial.write(" 1 : See the number of fingerprint recorded in database\r\n"); 
  Serial.write(" 2 : Add a fingerprint (Each entry needs to be read two times: \"beep\", "); Serial.write("put the finger on sensor\r\n"); 
  Serial.write(" 3 : Match fingerprints (Send the command, then put your finger on sensor. "); Serial.write("Each time you send a command, module waits and matches once)\r\n"); 
  Serial.write(" 199 : Erase the fingerprints database\r\n"); 
  Serial.write("*************** WaveShare Capacitive Fingerprint Reader Test ***************\r\n"); 
  Serial.println(" ");
}

/***************************************************************************
* @brief      Analysis of the serial port 2 command (computer serial assistant)
****************************************************************************/

void Analysis_PC_Command() {
    if(id == 1) {
      Serial.print("Number of fingerprints recorded in database: "); Serial.println(GetUserCount());
    }
    
    if(id == 2) {
      //Serial.println(" Add fingerprint (Each entry needs to be read two times: "); 
      Serial.println("Put your finger on the sensor"); 
            switch(AddUser())
            {
              case ACK_SUCCESS:
                Serial.println("Fingerprint recorded!");
                break;
              
              case ACK_FAIL:      
                Serial.println("Failed: Put your finger in the middle of sensor, or this fingerprint is already recorded!");
                break;
              
              case ACK_FULL:      
                Serial.println("Failed: the database is full!");
                break;    
            }
       Serial.print("Number of fingerprints recorded in database: "); Serial.println(GetUserCount());
    }

    if(id == 3) {
            Serial.println("Put your finger on the sensor");
            switch(VerifyUser())
            {
              case ACK_SUCCESS: 
                Serial.print("Fingerprin No.");
                Serial.print(user_id); 
                Serial.println(" matched !");
                break;
              case ACK_NO_USER:
                Serial.println("Failed: This fingerprint already exists in base !");
                break;
              case ACK_TIMEOUT: 
                Serial.println("Failed: time out!");
                break;  
            }
    }

    if(id == 199) {
            ClearAllUser();
            Serial.println("All fingerprints are deleted!");
    }
  }
  
/***************************************************************************
* @brief  
     If you enter the sleep mode, then open the Automatic wake-up function of the finger,
     begin to check if the finger is pressed, and then start the module and match
****************************************************************************/
void Auto_Verify_Finger(void)
{
      mySerial.begin(19200);
      Serial.println("Matching : Keep your finger on the sensor");
      delay(2000);   
      switch(VerifyUser())
      {       
        case ACK_SUCCESS: 
          Serial.println("Remove your finger...");  
          delay(1500);
          Serial.print("Fingerprint No.");
          Serial.print(user_id);
          Serial.println(" matched");
          break;
        case ACK_NO_USER:
          Serial.println("Remove your finger...");  
          Serial.println("Failed: this fingerprint doesn't exist in base!");
          break;
        case ACK_TIMEOUT: 
          Serial.println("Failed: Time out !");
          break;  
        default:
          Serial.println("Failed: error!");
          break;
     }
}
