#include <SoftwareSerial.h>
#include <string.h>
#include <LiquidCrystal.h> 

//GSM
#define RX_PIN 9
#define TX_PIN 10

//Switches
#define Up   A0
#define Down A1
#define Ok   A2
#define Done A3

SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); 


char SMSRead[100];
char RFIDData[15];

/* Store RFID card details */
const char RFID_1[15] = "3C00AE61887B";   //Milk
const char RFID_2[15] = "3C00AE7B739A";   //Egg
const char RFID_3[15] = "3C00AF71799B";   //Oil
const char RFID_4[15] = "3C00AF352284";   //Soap

unsigned int CartDB[4] = {0, 0, 0, 0};

/* Price deatils */
unsigned int Price[4] = {50, 10, 200, 30};

char Buffer_data = "";
unsigned int  Count = 0;
unsigned long Total = 0;

// Define the keyword to search in SMS
const char keyword[] = "Register";

char RegisteredNo[14];
unsigned int ItemIndex = 0;
bool RegDone = 0;
  
void setup() {

  mySerial.begin(19200); // SIM900 module
  Serial.begin(9600);    //For RFID Reader

  lcd.begin(16, 2);  //lcd init with 16*2

/* Pin mode for Switches */
  pinMode(Up,   INPUT_PULLUP);
  pinMode(Down, INPUT_PULLUP);
  pinMode(Ok,   INPUT_PULLUP); 
  pinMode(Done, INPUT_PULLUP);    

/* welcome note */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO SMART");   //1st line
  lcd.setCursor(0, 1);
  lcd.print("BILLING SYSTEM");     //2nd line
   delay(3000);
}

void loop() 
{
  if(RegDone == 0)
  {
    WaitForRegistration();
    FindRegistedNumber();
  }
  else
  {
  WaitForRfidSwipe();
  IdentifyScannedItem();
  DisplyAndCount();
  }
}

void GSMSendSMS(void)
{
/* Calculate bill*/
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending Bill to");
  lcd.setCursor(0, 1);
  lcd.print(RegisteredNo);

  Total = ((CartDB[0]) * (Price[0])) + ((CartDB[1]) * (Price[1])) + ((CartDB[2]) * (Price[2])) + ((CartDB[3]) * (Price[3]));

  mySerial.print("AT\r\n");
  WaitforGSMRes();
  delay(2000);


  mySerial.print("AT+CMGF=1\r\n");
  WaitforGSMRes();
  delay(2000);

  mySerial.print("AT+CMGS=\"");
  mySerial.print(RegisteredNo);
  mySerial.print("\"");
  mySerial.print("\r\n");
  WaitforGSMRes();
  delay(2000);

  mySerial.print("Your Final Bill is \n");
  if(CartDB[0] > 0)
  {
    mySerial.print("Milk  x ");
    mySerial.print(CartDB[0]);
    mySerial.print(" = ");
    mySerial.print((CartDB[0]) * (Price[0]));
    mySerial.print("\n");
  }
  if(CartDB[1] > 0)
  {
    mySerial.print("Egg  x ");
    mySerial.print(CartDB[1]);
    mySerial.print(" = ");
    mySerial.print((CartDB[1]) * (Price[1]));
    mySerial.print("\n");
  }
  if(CartDB[2] > 0)
  {
    mySerial.print("Oil   x ");
    mySerial.print(CartDB[2]);
    mySerial.print(" = ");
    mySerial.print((CartDB[2]) * (Price[2]));
    mySerial.print("\n");
  }
  if(CartDB[3] > 0)
  {
    mySerial.print("Soap x ");
    mySerial.print(CartDB[3]);
    mySerial.print(" = ");
    mySerial.print((CartDB[3]) * (Price[3]));
    mySerial.print("\n");
  }
  mySerial.print("Total = ");
  mySerial.print(Total);

  if(Total>0)
  {
  //  mySerial.print("\n You can pay at https://www.phonepe.com/business-solutions/payment-gateway/");
  }
  mySerial.print("\n\n Thanks for visiting us...");
  mySerial.println((char)26);
  WaitforGSMRes();
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Done");
}

void DisplyAndCount(void)
{
  while(1)
  {
    bool switchUpState    = digitalRead(Up);
    bool switchDownState  = digitalRead(Down);
    bool switchOkState    = digitalRead(Ok);
    bool switchDonewState = digitalRead(Done);

    if(switchUpState == 0)
    {
      CartDB[ItemIndex]++;

     clearSecondLine();
      lcd.setCursor(0, 1);
      lcd.print("C=");
      lcd.print(CartDB[ItemIndex]);
      lcd.print(" T=");
      Total = ((CartDB[0]) * (Price[0])) + ((CartDB[1]) * (Price[1])) + ((CartDB[2]) * (Price[2])) + ((CartDB[3]) * (Price[3]));
      lcd.print(Total);      
    }
    else if(switchDownState == 0)
    {
      if(CartDB[ItemIndex] > 0)
      {
        CartDB[ItemIndex]--;
        clearSecondLine();
        lcd.setCursor(0, 1);
        lcd.print("C=");
        lcd.print(CartDB[ItemIndex]);
        lcd.print(" T=");
        Total = ((CartDB[0]) * (Price[0])) + ((CartDB[1]) * (Price[1])) + ((CartDB[2]) * (Price[2])) + ((CartDB[3]) * (Price[3]));
        lcd.print(Total);      
      }
    }
    else if(switchOkState == 0)
    {
      break;
    }
    else if(switchDonewState == 0)
    {
      GSMSendSMS();
    }
    delay(200);
  }
}

 
void clearSecondLine() 
{
  lcd.setCursor(0, 1);              // Set cursor to beginning of the second line
  for (int i = 0; i < 16; i++) 
  {
    lcd.print(" ");                 // Print spaces to overwrite the existing content
  }
}

void IdentifyScannedItem(void)
{
    lcd.clear();

    if (strcmp(RFIDData, RFID_1) == 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("Item:Milk Rs-");
      lcd.print(Price[0]);

      lcd.setCursor(0, 1);
      lcd.print("C=");
      lcd.print(CartDB[0]);
      lcd.print(" T=");
      Total = ((CartDB[0]) * (Price[0])) + ((CartDB[1]) * (Price[1])) + ((CartDB[2]) * (Price[2])) + ((CartDB[3]) * (Price[3]));
      lcd.print(Total);
      ItemIndex = 0;
    }
    else if (strcmp(RFIDData, RFID_2) == 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("Item:Egg Rs-");
      lcd.print(Price[1]);

      lcd.setCursor(0, 1);
      lcd.print("C=");
      lcd.print(CartDB[1]);
      lcd.print(" T=");
      Total =((CartDB[0]) * (Price[0])) + ((CartDB[1]) * (Price[1])) + ((CartDB[2]) * (Price[2])) + ((CartDB[3]) * (Price[3]));
      lcd.print(Total);
      ItemIndex = 1;
    }
    else if (strcmp(RFIDData, RFID_3) == 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("Item:Oil Rs-");
      lcd.print(Price[2]);

      lcd.setCursor(0, 1);
      lcd.print("C=");
      lcd.print(CartDB[2]);
      lcd.print(" T=");
      Total = ((CartDB[0]) * (Price[0])) + ((CartDB[1]) * (Price[1])) + ((CartDB[2]) * (Price[2])) + ((CartDB[3]) * (Price[3]));
      lcd.print(Total);
      ItemIndex = 2;
    }
    else if (strcmp(RFIDData, RFID_4) == 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("Item:Soap Rs-");
      lcd.print(Price[3]);

      lcd.setCursor(0, 1);
      lcd.print("C=");
      lcd.print(CartDB[3]);
      lcd.print(" T=");
      Total = ((CartDB[0]) * (Price[0])) + ((CartDB[1]) * (Price[1])) + ((CartDB[2]) * (Price[2])) + ((CartDB[3]) * (Price[3]));
      lcd.print(Total);
      ItemIndex = 3;
    }   
    delay(200);   
}

void clearSerialBuffer() 
{
  while (Serial.available() > 0) 
  {
    char c = Serial.read(); 
  }
}

void WaitForRfidSwipe(void)
{
  unsigned int index = 0;
  
  /* Write data to LCD display */
  lcd.clear();
  clearSerialBuffer();
  lcd.setCursor(0, 0);
  lcd.print("Please Scan Item");


  for(index= 0; index < 12 ; index++)
  {
    while(!Serial.available())
    {

    }
    while(Serial.available()>0) 
    {
      RFIDData[index] = Serial.read();
    }
  }
  RFIDData[index] = '\0';
}



void WaitforGSMRes(void)
{
  while(!mySerial.available())
  {

  }
  while(mySerial.available()>0) 
  {
    Buffer_data = mySerial.read();
   // Serial.print(Buffer_data);
  }
}


void WaitForRegistration(void)
{
	unsigned int i;
	i = 0;                             //For the SMS buffer count
		
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Register phone");
  lcd.setCursor(0, 1);
  lcd.print("at 9895810901");

	mySerial.print("AT\r\n");          //Enter in to AT command mode
	WaitforGSMRes();
	delay(1000);
	
	mySerial.print("ATE0\r\n");         //Turn OFF Echo
	WaitforGSMRes();
	delay(1000);
	
	mySerial.print("AT+CMGF=1\r\n");   //Enter in to GSM Text mode, for reciving SMS
	WaitforGSMRes();
	delay(1000);
	
	mySerial.println("AT+CMGD=1,4");  // Delete all SMS messages from memory
	WaitforGSMRes();
	delay(1000);
	
	mySerial.print("AT+CMGL=1\r\n");  //Read recent SMS received
	delay(2000);
	mySerial.find("\n");
	WaitforGSMRes();
	
	mySerial.flush();
	//Serial.println("Waiting for SMS...........");


  /*Read the sms and store*/
	while(!mySerial.available())
	{
	
	}
	while(mySerial.available()>0) 
	{
		SMSRead[i] = mySerial.read();
		i++;
		delay(5);  
	}
	SMSRead[i] ='\0';    //null character
	//Serial.println(SMSRead);
}

void FindRegistedNumber(void)
{
    if (strstr(SMSRead, keyword) != NULL) 
    {
        //Serial.println("Registered contat found");

        //+CMT: +919895866252\0
        char *ptr = strstr(SMSRead, "+CMT: ");
  
        if (ptr != NULL) 
        {
          ptr = ptr + 7;                        // Move pointer 7 bytes forward to skip "+CMT: "

          // Fetch the next 13 bytes
          strncpy(RegisteredNo, ptr, 13);
          RegisteredNo[13] = '\0'; 
          RegDone = 1;

          // Print the fetched bytes
          //Serial.print("numer:");
          //Serial.println(RegisteredNo);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Registed No:");
          lcd.setCursor(0, 1);
          lcd.print(RegisteredNo);
          delay(2000);
        } 
        else
        {
          //Serial.println("Not found contact");
        }
    } 
    else 
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Reg Failed");
      delay(2000);
    }
}

