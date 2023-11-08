#include <LiquidCrystal.h>
const int rs = 18, en = 19, d4 = 20, d5 = 21, d6 = 22, d7 = 23;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#define dir 4

int ledPin[] = {12,13,14,15};
int ledSwPin[] = {16,17,10,11};
int swPin[] = {0,1,2,3,7};
int DipSwPin[] = {24,25,26,27};
int DipSwAddrPin[] = {28,29,30,31};
char ascii[] = "0123456789ABCDEF";
char rxBuf[30],tail=0;

int readLedStatus(){
  int dat = 0;
  digitalRead(ledPin[0])? dat |= 1: dat &= 0xfe;
  digitalRead(ledPin[1])? dat |= 2: dat &= 0xfd;
  digitalRead(ledPin[2])? dat |= 4: dat &= 0xfb;
  digitalRead(ledPin[3])? dat |= 8: dat &= 0xf7;  
  return dat;
}

int readDipSwStatus(){
  int dat = 0;
  digitalRead(DipSwPin[0])? dat |= 1: dat &= 0xfe;
  digitalRead(DipSwPin[1])? dat |= 2: dat &= 0xfd;
  digitalRead(DipSwPin[2])? dat |= 4: dat &= 0xfb;
  digitalRead(DipSwPin[3])? dat |= 8: dat &= 0xf7;  
  return dat;
}

int readDipSwAddress(){
  int dat = 0;
  digitalRead(DipSwAddrPin[0])? dat |= 1: dat &= 0xfe;
  digitalRead(DipSwAddrPin[1])? dat |= 2: dat &= 0xfd;
  digitalRead(DipSwAddrPin[2])? dat |= 4: dat &= 0xfb;
  digitalRead(DipSwAddrPin[3])? dat |= 8: dat &= 0xf7;  
  return dat;
}

void showLedStatusLCD(int dat){
  lcd.setCursor(0,0);
  lcd.print("    ");
  lcd.setCursor(0,0);
  (dat & 0x08)? lcd.print("1") : lcd.print("0");
  (dat & 0x04)? lcd.print("1") : lcd.print("0");
  (dat & 0x02)? lcd.print("1") : lcd.print("0");
  (dat & 0x01)? lcd.print("1") : lcd.print("0");
}

void showLedStatusHardware(int dat){
  (dat & 0x08)? digitalWrite(ledPin[3],1) : digitalWrite(ledPin[3],0);
  (dat & 0x04)? digitalWrite(ledPin[2],1) : digitalWrite(ledPin[2],0);
  (dat & 0x02)? digitalWrite(ledPin[1],1) : digitalWrite(ledPin[1],0);
  (dat & 0x01)? digitalWrite(ledPin[0],1) : digitalWrite(ledPin[0],0);
}

void showDipSwStatus(int dat){
  lcd.setCursor(8,0);
  lcd.print("    ");
  lcd.setCursor(8,0);
  (dat & 0x08)? lcd.print("1") : lcd.print("0");
  (dat & 0x04)? lcd.print("1") : lcd.print("0");
  (dat & 0x02)? lcd.print("1") : lcd.print("0");
  (dat & 0x01)? lcd.print("1") : lcd.print("0");
}
void showLedStatusSw(int dat){
  (dat & 0x08)? digitalWrite(ledSwPin[3],1) : digitalWrite(ledSwPin[3],0);
  (dat & 0x04)? digitalWrite(ledSwPin[2],1) : digitalWrite(ledSwPin[2],0);
  (dat & 0x02)? digitalWrite(ledSwPin[1],1) : digitalWrite(ledSwPin[1],0);
  (dat & 0x01)? digitalWrite(ledSwPin[0],1) : digitalWrite(ledSwPin[0],0);
}
void showADC(int ch,int dat){
  if(ch == 0){
    lcd.setCursor(0,1);
    lcd.print("    ");
    lcd.setCursor(0,1);
    lcd.print(dat);
  }else if(ch == 1){
    lcd.setCursor(5,1);
    lcd.print("    ");
    lcd.setCursor(5,1);
    lcd.print(dat);
  }else{
    lcd.setCursor(10,1);
    lcd.print("    ");
    lcd.setCursor(10,1);
    lcd.print(dat);
  }
}

byte AsciiToHex(char c){
  if((c >= '0') && (c <= '9')){
    c = c & 0x0f;
  }else{
    c = (c & 0x0f) + 9;
  }
  return c;
}

byte findLRC(byte addrS,byte fnc,int addrIO,int countIO){
  byte lrc,addrH,addrL,countIO_h,countIO_l;
  addrH = addrIO >> 8;
  addrL = addrIO;
  countIO_h = countIO >> 8;
  countIO_l = countIO;
  lrc = 0 - (addrS + fnc + addrH + addrL + countIO_h + countIO_l);
  return lrc;
}

byte findLRC_0xF(byte addrS,byte fnc,int addrIO,int countIO,byte dat01,int datF){
  byte lrc,addrH,addrL,countIO_h,countIO_l,dat_01H,dat_01L,datF_H,datF_L;
  addrH = addrIO >> 8;
  addrL = addrIO;
  countIO_h = countIO >> 8;
  countIO_l = countIO;
  dat_01H = dat01 >>8;
  dat_01L = dat01;
  datF_H = datF >> 8;
  datF_L = datF ;
  lrc = 0 - (addrS + fnc + addrH + addrL + countIO_h + countIO_l + dat_01H + dat_01L + datF_H + datF_L);
  return lrc;
}

int findStartFrame(){
  int index=0;
  while(rxBuf[index] != ':') index++;
  return index;
}

byte findLRCinFrame_0x01(){
  
  int startFrame,addrIO,countIO;
  byte addrS,fnc,lrc,byteCount,datIO;
  
  startFrame = findStartFrame();
  addrS = AsciiToHex(rxBuf[startFrame+1]) * 0x10;
  addrS += AsciiToHex(rxBuf[startFrame+2]);
  
  fnc = AsciiToHex(rxBuf[startFrame+3]) * 0x10;
  fnc += AsciiToHex(rxBuf[startFrame+4]);
  
  byteCount = (AsciiToHex(rxBuf[startFrame+5]) * 0x10);
  byteCount += AsciiToHex(rxBuf[startFrame+6]);

  datIO = (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
  datIO += AsciiToHex(rxBuf[startFrame+8]);

  lrc = 0 - (addrS + fnc + byteCount + datIO);
  return lrc;
}

byte findLRCinFrame_0x02(){
  
  int startFrame,addrIO,countIO;
  byte addrS,fnc,lrc,byteCount,datIO;
  
  startFrame = findStartFrame();
  addrS = AsciiToHex(rxBuf[startFrame+1]) * 0x10;
  addrS += AsciiToHex(rxBuf[startFrame+2]);
  
  fnc = AsciiToHex(rxBuf[startFrame+3]) * 0x10;
  fnc += AsciiToHex(rxBuf[startFrame+4]);
  
  byteCount = (AsciiToHex(rxBuf[startFrame+5]) * 0x10);
  byteCount += AsciiToHex(rxBuf[startFrame+6]);

  datIO = (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
  datIO += AsciiToHex(rxBuf[startFrame+8]);

  lrc = 0 - (addrS + fnc + byteCount + datIO);
  return lrc;
}

byte findLRCinFrame_0x04(){
  
  int startFrame,addrIO,countIO;
  byte addrS,fnc,lrc,byteCount,datIO,datH, datL;
  
  startFrame = findStartFrame();
  addrS = AsciiToHex(rxBuf[startFrame+1]) * 0x10;
  addrS += AsciiToHex(rxBuf[startFrame+2]);
  
  fnc = AsciiToHex(rxBuf[startFrame+3]) * 0x10;
  fnc += AsciiToHex(rxBuf[startFrame+4]);
  
  byteCount = (AsciiToHex(rxBuf[startFrame+5]) * 0x10);
  byteCount += AsciiToHex(rxBuf[startFrame+6]);

  datH = (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
  datH += AsciiToHex(rxBuf[startFrame+8]);

  datL = (AsciiToHex(rxBuf[startFrame+9]) * 0x10);
  datL += AsciiToHex(rxBuf[startFrame+10]);
  lrc = 0 - (addrS + fnc + byteCount + datH + datL);
  return lrc;
}

byte findLRCinFrame_0x04x() {
  int startFrame,addrIO,countIO;
  byte addrS,fnc,lrc,byteCount,datIO,datH,datL,datH1,datL1,datH2,datL2;
  
  startFrame = findStartFrame();
  addrS = AsciiToHex(rxBuf[startFrame+1]) * 0x10;
  addrS += AsciiToHex(rxBuf[startFrame+2]);
  
  fnc = AsciiToHex(rxBuf[startFrame+3]) * 0x10;
  fnc += AsciiToHex(rxBuf[startFrame+4]);
  
  byteCount = (AsciiToHex(rxBuf[startFrame+5]) * 0x10);
  byteCount += AsciiToHex(rxBuf[startFrame+6]);

  datH = (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
  datH += AsciiToHex(rxBuf[startFrame+8]);

  datL = (AsciiToHex(rxBuf[startFrame+9]) * 0x10);
  datL += AsciiToHex(rxBuf[startFrame+10]);

  datH1 = (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
  datH1 += AsciiToHex(rxBuf[startFrame+12]);

  datL1 = (AsciiToHex(rxBuf[startFrame+13]) * 0x10);
  datL1 += AsciiToHex(rxBuf[startFrame+14]);
  
  datH2 = (AsciiToHex(rxBuf[startFrame+15]) * 0x10);
  datH2 += AsciiToHex(rxBuf[startFrame+16]);

  datL2 = (AsciiToHex(rxBuf[startFrame+17]) * 0x10);
  datL2 += AsciiToHex(rxBuf[startFrame+18]);
  
  lrc = 0 - (addrS + fnc + byteCount + datH + datL + datH1 + datL1 + datH2 + datL2);
  return lrc;
}

byte findLRCinFrame_0x05(){
  int startFrame,addrIO,countIO;
  byte addrS,fnc,lrc;
  startFrame = findStartFrame();
  addrS = AsciiToHex(rxBuf[startFrame+1]) * 0x10;
  addrS += AsciiToHex(rxBuf[startFrame+2]);
  fnc = AsciiToHex(rxBuf[startFrame+3]) * 0x10;
  fnc += AsciiToHex(rxBuf[startFrame+4]);
  addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
  addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
  addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
  addrIO += AsciiToHex(rxBuf[startFrame+8]);
  countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
  countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
  countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
  countIO += AsciiToHex(rxBuf[startFrame+12]);
  lrc = findLRC(addrS,fnc,addrIO,countIO);
  return lrc;
}

byte findLRCinFrame_0x0F(){
  int startFrame,addrIO,countIO;
  byte addrS,fnc,lrc,datF,dat01;
  
  startFrame = findStartFrame();
  
  addrS = AsciiToHex(rxBuf[startFrame+1]) * 0x10;
  addrS += AsciiToHex(rxBuf[startFrame+2]);
  
  fnc = AsciiToHex(rxBuf[startFrame+3]) * 0x10;
  fnc += AsciiToHex(rxBuf[startFrame+4]);
  
  addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
  addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
  addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
  addrIO += AsciiToHex(rxBuf[startFrame+8]);
  
  countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
  countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
  countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
  countIO += AsciiToHex(rxBuf[startFrame+12]);
//
//  dat01 = AsciiToHex(rxBuf[startFrame+13]) * 0x10;
//  dat01 += AsciiToHex(rxBuf[startFrame+14]);
//
//  datF = AsciiToHex(rxBuf[startFrame+15]) * 0x10;
//  datF += AsciiToHex(rxBuf[startFrame+16]);
  lrc = 0 - (addrS + fnc + addrIO + countIO);
  return lrc;
}

void sendModbusASCII(byte addrS,byte fnc,uint16_t addrIO,uint16_t countIO,byte lrc){
  Serial.write(':');
  Serial.write(ascii[addrS/0x10]);
  Serial.write(ascii[addrS%0x10]);
  Serial.write(ascii[fnc/0x10]);
  Serial.write(ascii[fnc%0x10]);
  Serial.write(ascii[addrIO/0x1000]);
  Serial.write(ascii[(addrIO%0x1000)/0x100]);
  Serial.write(ascii[(addrIO%0x100)/0x10]);
  Serial.write(ascii[addrIO%0x10]);
  Serial.write(ascii[countIO/0x1000]);
  Serial.write(ascii[(countIO%0x1000)/0x100]);
  Serial.write(ascii[(countIO%0x100)/0x10]);
  Serial.write(ascii[countIO%0x10]);
  Serial.write(ascii[lrc/0x10]);
  Serial.write(ascii[lrc%0x10]);
  Serial.write(0x0d);
  Serial.write(0x0a);
}

void sendModbusASCII_0x0F(byte addrS,byte fnc,uint16_t addrIO,uint16_t countIO,byte dat01,byte datF,byte lrc){
  Serial.write(':');
  Serial.write(ascii[addrS/0x10]);
  Serial.write(ascii[addrS%0x10]);
  Serial.write(ascii[fnc/0x10]);
  Serial.write(ascii[fnc%0x10]);
  Serial.write(ascii[addrIO/0x1000]);
  Serial.write(ascii[(addrIO%0x1000)/0x100]);
  Serial.write(ascii[(addrIO%0x100)/0x10]);
  Serial.write(ascii[addrIO%0x10]);
  Serial.write(ascii[countIO/0x1000]);
  Serial.write(ascii[(countIO%0x1000)/0x100]);
  Serial.write(ascii[(countIO%0x100)/0x10]);
  Serial.write(ascii[countIO%0x10]);
  Serial.write("01");
  Serial.write(ascii[(datF%0x100)/0x10]);
  Serial.write(ascii[datF%0x10]);
  Serial.write(ascii[lrc/0x10]);
  Serial.write(ascii[lrc%0x10]);
  Serial.write('\r');
  Serial.write('\n');
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
//  lcd.clear();
  for(int i = 0;i<4;i++){
    pinMode(DipSwPin[i],INPUT_PULLUP);
    pinMode(DipSwAddrPin[i],INPUT_PULLUP);
    pinMode(ledPin[i],OUTPUT); digitalWrite(ledPin[i],0);
    pinMode(ledSwPin[i],OUTPUT); digitalWrite(ledSwPin[i],0);
  }
  for(int j = 0; j<5; j++){
    pinMode(swPin[j],INPUT_PULLUP);
  }

  pinMode(dir,OUTPUT); digitalWrite(dir,HIGH);
  delay(500);
}

void loop(){
  byte fnc,lrc,byteCount,datIO,addrS,addrHardware,datF,dat01,datH,datL;
  int addrIO,countIO,dat = 0,startFrame,adc0;

  /******* function 01 SWPin[]*****/
  if(digitalRead(swPin[0]) == 0){
    addrS = readDipSwAddress();
    fnc = 0x01;
    dat = readDipSwStatus();
    if(dat <= 3){
      addrIO = dat;
      countIO = 1;
      //lcd.setCursor(10,1); lcd.print(countIO);
      
    }else{
      addrIO = 0;
      countIO = 4;
    }
    lrc = findLRC(addrS,fnc,addrIO,countIO);
    sendModbusASCII(addrS,fnc,addrIO,countIO,lrc);
    lcd.setCursor(0,0);
    lcd.print("Function~1");
    delay(100);
    digitalWrite(dir,0);
    delay(200); //20 byte
    digitalWrite(dir,1);
    delay(100);
    
    while(digitalRead(swPin[0]) == 0) delay(10);
  }

  // ******** Function 2 SWPin[] ********//
  
  if(digitalRead(swPin[1]) == 0){
    addrS = readDipSwAddress();
    fnc = 0x02;
    dat = readDipSwStatus();
    if(dat <= 3){
      addrIO = dat;
      countIO = 1;
      //lcd.setCursor(10,1); lcd.print(countIO);
      
    }else{
      addrIO = 0;
      countIO = 4;
    }
    lrc = findLRC(addrS,fnc,addrIO,countIO);
    sendModbusASCII(addrS,fnc,addrIO,countIO,lrc);
    lcd.setCursor(0,0);
    lcd.print("Function~2");
    delay(100);
    digitalWrite(dir,0);
    delay(200); 
    digitalWrite(dir,1);
    delay(100);
    while(digitalRead(swPin[1]) == 0) delay(100);
  }
  /************ Function 4 SWPin[] ************/
  if(digitalRead(swPin[2]) == 0){
    addrS = readDipSwAddress();
    fnc = 0x04;
    dat = readDipSwStatus();
//    if(dat == 0x01){
//      addrIO = 1;
//      countIO = 1;
//    }
    if(dat <= 2){
      addrIO = dat;
      countIO = 1;
    }else{
      addrIO = 0;
      countIO = 3;
    }
    lrc = findLRC(addrS,fnc,addrIO,countIO);
    sendModbusASCII(addrS,fnc,addrIO,countIO,lrc);
    lcd.setCursor(0,0);
    lcd.print("Function~4");
    delay(100);
    digitalWrite(dir,0);
    delay(200);
    digitalWrite(dir,1);
    delay(100);
    while(digitalRead(swPin[2]) == 0) delay(100);
  }
  if(digitalRead(swPin[3]) == 0){
    addrS = readDipSwAddress();
    fnc = 0x05;
    dat = readDipSwStatus();
    addrIO = dat & 0x03;
    dat = dat & 0x0c;
    
    if(dat == 0x0c){ 
      countIO = 0xff00;
      
    } else countIO = 0x0000; 
    
    lrc = findLRC(addrS,fnc,addrIO,countIO);
    sendModbusASCII(addrS,fnc,addrIO,countIO,lrc);
    lcd.setCursor(0,0);
    lcd.print("Function~5");
    delay(170);
    digitalWrite(dir,0);
    delay(270);
    digitalWrite(dir,1);
    delay(170);
    while(digitalRead(swPin[3]) == 0) delay(100);
}

/*************** function 0f ***************************/

 if (digitalRead(swPin[4]) == 0) {
  addrS = readDipSwAddress();
  fnc = 0x0f;
  datF = readDipSwStatus();
  addrIO = 0;
  countIO = 4;
  dat01 = 0x01;
  lrc = findLRC_0xF(addrS,fnc,addrIO,countIO,dat01,datF);
  sendModbusASCII_0x0F(addrS,fnc,addrIO,countIO,dat01,datF,lrc);
  lcd.setCursor(0, 0);
  lcd.print("Function~F");
  delay(150);
  digitalWrite(dir, 0);
  delay(250);
  digitalWrite(dir, 1);
  delay(150);
  while (digitalRead(swPin[4]) == 0) delay(100);
}


  if(Serial.available() > 0){
    rxBuf[tail++] = Serial.read();
    if(rxBuf[tail - 1] == 0x0a){
      startFrame = findStartFrame();
      addrHardware = readDipSwAddress();
      addrS = AsciiToHex(rxBuf[startFrame+1]) * 0x10;
      addrS += AsciiToHex(rxBuf[startFrame+2]);
      fnc = AsciiToHex(rxBuf[startFrame+3]) * 0x10;
      fnc += AsciiToHex(rxBuf[startFrame+4]);
      
      //******* Function 01 *******//

       if((addrS == addrHardware)&&(fnc == 0x01)){
//        lcd.clear();
        lrc = AsciiToHex(rxBuf[startFrame+9]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+10]); 
        
        if(lrc == findLRCinFrame_0x01()){
                      
           datIO = (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
           datIO += AsciiToHex(rxBuf[startFrame+8]);

           if(countIO == 1) {
            digitalWrite(ledPin[addrIO],datIO);
            lcd.setCursor(0,1); lcd.print("               ");
            lcd.setCursor(0,1); lcd.print("addrO : " + String(addrIO));
            lcd.print(" is " + String(datIO));
           } else {

            lcd.setCursor(0,1); lcd.print("               ");
            lcd.setCursor(0,1); lcd.print("addrO : " + String(addrIO));
            lcd.print(" OUT " + String(datIO,HEX));
            showLedStatusHardware(datIO);
           }
        }     
      }

      // ******************** Function 02 ************************** // 

        if((addrS == addrHardware)&&(fnc == 0x02)){
        lrc = AsciiToHex(rxBuf[startFrame+9]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+10]); 
        if(lrc == findLRCinFrame_0x02()){
                      
           datIO = (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
           datIO += AsciiToHex(rxBuf[startFrame+8]);

           if(countIO == 1) {
            digitalWrite(ledSwPin[addrIO],datIO);
            lcd.setCursor(0,1); lcd.print("               ");
            lcd.setCursor(0,1); lcd.print("addrI : " + String(addrIO));
            lcd.print(" is " + String(datIO));
           } else {

            lcd.setCursor(0,1); lcd.print("               ");
            lcd.setCursor(0,1); lcd.print("addrI : " + String(addrIO));
            lcd.print(" OUT " + String(datIO,HEX));
            showLedStatusSw(datIO);
           }
        }     
      }
      // ******************** Function 04 ************************** // 
//    
//      if((addrS == addrHardware) && (fnc == 0x04)) {
//        lrc = AsciiToHex(rxBuf[startFrame+11]) * 0x10;
//        lrc += AsciiToHex(rxBuf[startFrame+12]); 
//        lcd.setCursor(11,0); 
//        lcd.print("O4~" + String(lrc,HEX)); 
//        if(lrc == findLRCinFrame_0x04()){
//          
//           byteCount = (AsciiToHex(rxBuf[startFrame+5]) * 0x10);
//           byteCount += AsciiToHex(rxBuf[startFrame+6]);
//
//           datH = (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
//           datH += AsciiToHex(rxBuf[startFrame+8]);
//          
//           datL = (AsciiToHex(rxBuf[startFrame+9]) * 0x10);
//           datL += AsciiToHex(rxBuf[startFrame+10]);  
////           switch (byteCount == 2) {
////            case  
////           }      
//              lcd.setCursor(0, 1);
//              lcd.print("A1b:");
//              lcd.print(datH); // แสดงค่า datH
//              lcd.print(datL, HEX); // แสดงค่า datL ในรูปแบบเลขฐาน 16 (HEX)
//                 
//        }     
//      }

      if ((addrS == addrHardware) && (fnc == 0x04)) {
         byteCount = (AsciiToHex(rxBuf[startFrame+5]) * 0x10);
         byteCount += AsciiToHex(rxBuf[startFrame+6]);
         if(byteCount == 2) {
            lrc = AsciiToHex(rxBuf[startFrame + 11]) * 0x10;
            lrc += AsciiToHex(rxBuf[startFrame + 12]);
            lcd.setCursor(11, 0);
            lcd.print("04~" + String(lrc, HEX));
            if (lrc == findLRCinFrame_0x04()) {
              byte byteCount = (AsciiToHex(rxBuf[startFrame + 5]) * 0x10) + AsciiToHex(rxBuf[startFrame + 6]);
              byte datH = (AsciiToHex(rxBuf[startFrame + 7]) * 0x10) + AsciiToHex(rxBuf[startFrame + 8]);
              byte datL = (AsciiToHex(rxBuf[startFrame + 9]) * 0x10) + AsciiToHex(rxBuf[startFrame + 10]);
          
              int lcdX = 0;
              switch (byteCount) {
                case 2:
                  lcdX = 0;
                  break;
                case 4:
                  lcdX = 7;
                  break;
                case 6:
                  lcdX = 12;
                  break;
                // ** u can write ?
                default:
                  break;
              }
              lcd.setCursor(lcdX, 1);
              lcd.print("A1b:");
              lcd.print(datH,HEX); 
              char datLStr[3]; 
              sprintf(datLStr, "%02X", datL); 
              lcd.print(datLStr);
            }
        }else if (byteCount == 6){
          lrc = AsciiToHex(rxBuf[startFrame + 19]) * 0x10;
          lrc += AsciiToHex(rxBuf[startFrame + 20]);
          if (lrc == findLRCinFrame_0x04x()){
              byte byteCount = (AsciiToHex(rxBuf[startFrame + 5]) * 0x10) + AsciiToHex(rxBuf[startFrame + 6]);
              byte datH = (AsciiToHex(rxBuf[startFrame + 7]) * 0x10) + AsciiToHex(rxBuf[startFrame + 8]);
              byte datL = (AsciiToHex(rxBuf[startFrame + 9]) * 0x10) + AsciiToHex(rxBuf[startFrame + 10]);            
              byte datH1 = (AsciiToHex(rxBuf[startFrame + 11]) * 0x10) + AsciiToHex(rxBuf[startFrame + 12]);
              byte datL1 = (AsciiToHex(rxBuf[startFrame + 13]) * 0x10) + AsciiToHex(rxBuf[startFrame + 14]);            
              byte datH2 = (AsciiToHex(rxBuf[startFrame + 15]) * 0x10) + AsciiToHex(rxBuf[startFrame + 16]);
              byte datL2 = (AsciiToHex(rxBuf[startFrame + 17]) * 0x10) + AsciiToHex(rxBuf[startFrame + 18]);            
                              
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("HEll0x04X");  
              lcd.setCursor(11, 0);
              lcd.print("04~" + String(lrc, HEX));
              lcd.setCursor(12,1);
              lcd.print(datH,HEX);
              char datLStr[3]; 
              sprintf(datLStr, "%02X", datL); 
              lcd.print(datLStr);
              lcd.setCursor(6,1);
              lcd.print(datH1,HEX);
              sprintf(datLStr, "%02X", datL1); 
              lcd.print(datLStr);
              lcd.setCursor(1,1);
              lcd.print(datH2,HEX);
              sprintf(datLStr, "%02X", datL2); 
              lcd.print(datLStr);
          } 
       }
   }

      
      // ******************** Function 05 ************************** // 
      if((addrS == addrHardware) &&(fnc == 0x05)){
        lrc = AsciiToHex(rxBuf[startFrame+13]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+14]);
        lcd.setCursor(0,1);
        lcd.print("Hi:5x");
        if(lrc == findLRCinFrame_0x05()){
          lcd.setCursor(8,1);
          lcd.print(addrIO,HEX);
          lcd.setCursor(12,1);
          lcd.print(countIO,HEX);
//          addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
//          addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
//          addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
//          addrIO += AsciiToHex(rxBuf[startFrame+8]);
//          countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
//          countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
//          countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
//          countIO += AsciiToHex(rxBuf[startFrame+12]);
        } 
      }
      // ******************** Function 0F ************************** // 
      if ((addrS == addrHardware) && (fnc == 0x0F)) {
          lrc = AsciiToHex(rxBuf[startFrame + 13]) * 0x10;
          lrc += AsciiToHex(rxBuf[startFrame + 14]);
          if (lrc == findLRCinFrame_0x0F()) {
                  addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
                  addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
                  addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
                  addrIO += AsciiToHex(rxBuf[startFrame+8]);
                  
                  countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
                  countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
                  countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
                  countIO += AsciiToHex(rxBuf[startFrame+12]);
                  lcd.setCursor(12, 0);
                  lcd.print(lrc,HEX);
              if (countIO == 4) {
//                lcd.clear();
                  lcd.setCursor(0, 1);
                  lcd.print("OKAY..0XF");
                  lcd.setCursor(12,1);lcd.print(datF,BIN);
//                  showLedStatusLCD(datF >> 1);
//                  showLedStatusHardware(datF >> 1);
              }
          }
      }
      tail = 0; 
    }
  }
}