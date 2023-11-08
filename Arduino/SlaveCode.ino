
/****************************/

#include <LiquidCrystal.h>
const int rs = 18, en = 19, d4 = 20, d5 = 21, d6 = 22, d7 = 23;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#define dir 0

int ledPin[] = {10,11,17,16};
int swPin[] = {4,5,6,7};
int DipSwPin[] = {15,14,13,12};
int DipSwAddrPin[] = {24,25,26,27};
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

int readDipSwAddress(){
  int dat = 0;
  digitalRead(DipSwAddrPin[0])? dat |= 1: dat &= 0xfe;
  digitalRead(DipSwAddrPin[1])? dat |= 2: dat &= 0xfd;
  digitalRead(DipSwAddrPin[2])? dat |= 4: dat &= 0xfb;
  digitalRead(DipSwAddrPin[3])? dat |= 8: dat &= 0xf7;  
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

void showLedStatus(int dat){
  lcd.setCursor(0,0);
  lcd.print("    ");
  lcd.setCursor(0,0);
  (dat & 0x08)? lcd.print("1") : lcd.print("0");
  (dat & 0x04)? lcd.print("1") : lcd.print("0");
  (dat & 0x02)? lcd.print("1") : lcd.print("0");
  (dat & 0x01)? lcd.print("1") : lcd.print("0");
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

byte findLRCinFrame(){
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

  dat01 = AsciiToHex(rxBuf[startFrame+13]) * 0x10;
  dat01 += AsciiToHex(rxBuf[startFrame+14]);

  datF = AsciiToHex(rxBuf[startFrame+15]) * 0x10;
  datF += AsciiToHex(rxBuf[startFrame+16]);
  lrc = findLRC_0xF(addrS,fnc,addrIO,countIO,dat01,datF);
  return lrc;
}

void responseModusAscii1_2(byte addrS,byte fnc,byte byteCount,byte datIO){
  byte lrc;
  lrc = 0 - (addrS + fnc + byteCount + datIO);
  Serial.write(':');
  Serial.write(ascii[addrS/0x10]);
  Serial.write(ascii[addrS%0x10]);
  Serial.write(ascii[fnc/0x10]);
  Serial.write(ascii[fnc%0x10]);
  Serial.write(ascii[byteCount/0x10]);
  Serial.write(ascii[byteCount%0x10]);
  Serial.write(ascii[datIO/0x10]);
  Serial.write(ascii[datIO%0x10]);
  Serial.write(ascii[lrc/0x10]);
  Serial.write(ascii[lrc%0x10]);
  Serial.write('\r');
  Serial.write('\n');
}
void responseModusAscii_0x04(byte addrS, byte fnc, int addrIO, int countIO) {
  byte lrc, byteCount, datH, datL;
  int adcCh[3], countB, addrB;
  switch (countIO) {
    case 1: byteCount = 2; break;
    case 2: byteCount = 4; break;
    default: byteCount = 6; break;
  }
  countB = countIO;
  addrB = addrIO;
  for (;countIO > 0; countIO--) {
    adcCh[addrIO] = analogRead(addrIO);
    addrIO++;
  }
  lrc = addrS + fnc + byteCount;
  countIO = countB;
  addrIO = addrB;
 
  for (; countB > 0; countB--) {
    datH = adcCh[addrB] >> 8;
    datL = adcCh[addrB];
    lrc += (datH + datL);
    addrB++;    
  }
  lrc = 0 - lrc;
  Serial.write(':');
  Serial.write(ascii[addrS / 0x10]);
  Serial.write(ascii[addrS % 0x10]);
  Serial.write(ascii[fnc / 0x10]);
  Serial.write(ascii[fnc % 0x10]);
  Serial.write(ascii[byteCount / 0x10]);
  Serial.write(ascii[byteCount % 0x10]);
  for (; countIO > 0; countIO--) {
    datH = adcCh[addrIO] >> 8;
    datL = adcCh[addrIO];
    Serial.write(ascii[datH/0x10]);
    Serial.write(ascii[datH%0x10]);
    Serial.write(ascii[datL/0x10]);
    Serial.write(ascii[datL%0x10]);
    addrIO++;
  }
  Serial.write(ascii[lrc/0x10]);
  Serial.write(ascii[lrc%0x10]);  
  Serial.write('\r');
  Serial.write('\n');
    
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(adcCh[2], HEX);
      
  lcd.setCursor(5, 0);
  lcd.print(adcCh[1], HEX);
      
  lcd.setCursor(10, 0);
  lcd.print(adcCh[0], HEX);

  lcd.setCursor(0,1);
  lcd.write(':');
  lcd.write(ascii[addrS / 0x10]);
  lcd.write(ascii[addrS % 0x10]);
  lcd.write(ascii[fnc / 0x10]);
  lcd.write(ascii[fnc % 0x10]);
  lcd.write(ascii[byteCount / 0x10]);
  lcd.write(ascii[byteCount % 0x10]);
  lcd.write(ascii[datH/0x10]);
  lcd.write(ascii[datH%0x10]);
  lcd.write(ascii[datL/0x10]);
  lcd.write(ascii[datL%0x10]);
  lcd.write(ascii[lrc/0x10]);
  lcd.write(ascii[lrc%0x10]); 
  lcd.write('\r');
  lcd.write('\n');
}

void responseModusAscii_0x05(byte addrS, byte fnc,int addrIO, int countIO) {
  byte lrc;
  if (countIO == 0xff00){
    lrc = 1 - (addrS + fnc + addrIO + countIO);
  }else {
    lrc = 0 - (addrS + fnc + addrIO + countIO); 
  } 
  Serial.write(':'); 
  Serial.write(ascii[addrS/0x10]);
  Serial.write(ascii[addrS%0x10]);
  Serial.write(ascii[fnc/0x10]);
  Serial.write(ascii[fnc%0x10]);
  Serial.write(ascii[(addrIO >> 12) & 0x0F]);
  Serial.write(ascii[(addrIO >> 8) & 0x0F]);
  Serial.write(ascii[(addrIO >> 4) & 0x0F]);
  Serial.write(ascii[addrIO & 0x0F]);
  Serial.write(ascii[(countIO >> 12) & 0x0F]);
  Serial.write(ascii[(countIO >> 8) & 0x0F]);
  Serial.write(ascii[(countIO >> 4) & 0x0F]);
  Serial.write(ascii[countIO & 0x0F]);
  Serial.write(ascii[lrc/0x10]);
  Serial.write(ascii[lrc%0x10]);
  Serial.write('\r');
  Serial.write('\n');
  
//  lcd.clear();
  lcd.setCursor(0,0);
  lcd.write(':');
  lcd.write(ascii[addrS/0x10]);
  lcd.write(ascii[addrS%0x10]);
  lcd.write(ascii[fnc/0x10]);
  lcd.write(ascii[fnc%0x10]);
  lcd.write(ascii[(addrIO >> 12) & 0x0F]);
  lcd.write(ascii[(addrIO >> 8) & 0x0F]);
  lcd.write(ascii[(addrIO >> 4) & 0x0F]);
  lcd.write(ascii[addrIO & 0x0F]); // ส่งค่า addrIO
  lcd.write(ascii[(countIO >> 12) & 0x0F]);
  lcd.write(ascii[(countIO >> 8) & 0x0F]);
  lcd.write(ascii[(countIO >> 4) & 0x0F]);
  lcd.write(ascii[countIO & 0x0F]); // ส่งค่า countIO
  lcd.write(ascii[(lrc >> 4) & 0x0F]);
  lcd.write(ascii[lrc & 0x0F]); // ส่งค่า LRC
  lcd.write(0x0d);
  lcd.write(0x0a); 
}

void responseModusAscii_0x0F(byte addrS , byte fnc, int addrIO, int countIO /*byte datF*/) {
  byte lrc;
  lrc = 0 - (addrS + fnc + addrIO+ countIO);  /*+ datF*/
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
//  Serial.write(ascii[(datF%0x100)/0x10]);
//  Serial.write(ascii[datF%0x10]);
  Serial.write(ascii[lrc/0x10]);
  Serial.write(ascii[lrc%0x10]);
  Serial.write('\r');
  Serial.write('\n');
}

void setup() {
  byte buff[30],n;
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  for(int i = 0;i<4;i++){
    pinMode(swPin[i],INPUT_PULLUP);
    pinMode(DipSwPin[i],INPUT_PULLUP);
    pinMode(DipSwAddrPin[i],INPUT_PULLUP);
    pinMode(ledPin[i],OUTPUT); digitalWrite(ledPin[i],0);
  }
  pinMode(dir,OUTPUT); digitalWrite(dir,0);
  delay(500); 
  n = Serial.available();
  Serial.readBytes(buff,n);
  showLedStatus(readLedStatus());
}

void loop(){
  byte fnc,lrc,byteCount,datIO,addrS,addrHardware,buff[30],n,datF;
  static int stDipSw = 0;
  int dat = 0,startFrame,adcB0,adcB1,adcB2;
  uint16_t addrIO,countIO;
  if(digitalRead(swPin[0]) == 0){
    digitalWrite(ledPin[0],digitalRead(ledPin[0]) ^ 1);
    dat = readLedStatus();
    showLedStatus(dat);
    delay(350);
  }
  if(digitalRead(swPin[1]) == 0){
    digitalWrite(ledPin[1],!digitalRead(ledPin[1]));
    dat = readLedStatus();
    showLedStatus(dat);
    delay(350);
  }
  if(digitalRead(swPin[2]) == 0){
    digitalWrite(ledPin[2],!digitalRead(ledPin[2]));
    dat = readLedStatus();
    showLedStatus(dat);
    delay(350);
  }
  if(digitalRead(swPin[3]) == 0){
    digitalWrite(ledPin[3],!digitalRead(ledPin[3]));
    dat = readLedStatus();
    showLedStatus(dat);
    delay(350);
  }
  dat = readDipSwStatus();
  if(dat != stDipSw){
    stDipSw = dat;
    showDipSwStatus(dat);
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
      
      /**Function 1**/
      
      if((addrS == addrHardware)&&(fnc == 0x01)){
        lrc = AsciiToHex(rxBuf[startFrame+13]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+14]);  
        if(lrc == findLRCinFrame()){
          addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
          addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
          addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
          addrIO += AsciiToHex(rxBuf[startFrame+8]);
          countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
          countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
          countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
          countIO += AsciiToHex(rxBuf[startFrame+12]);
          if(countIO == 1){
            datIO = digitalRead(ledPin[addrIO]);
            byteCount = 1;
          }else{
            datIO = readLedStatus();
            byteCount = 1;
            lcd.setCursor(8,1); lcd.print(datIO,HEX);
          }
          digitalWrite(dir,1);
          delay(150);
          responseModusAscii1_2(addrS,fnc,byteCount,datIO);
          delay(250);
          digitalWrite(dir,0);
          delay(150);
          n = Serial.available();
          Serial.readBytes(buff,n);
        }
       /**Function 2**/
      }else if((addrS == addrHardware)&&(fnc == 0x02)){
        lrc = AsciiToHex(rxBuf[startFrame+13]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+14]);  
        if(lrc == findLRCinFrame()){
          addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
          addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
          addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
          addrIO += AsciiToHex(rxBuf[startFrame+8]);
          countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
          countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
          countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
          countIO += AsciiToHex(rxBuf[startFrame+12]);
          if(countIO == 1){
            datIO = digitalRead(DipSwPin[addrIO]);
            byteCount = 1;
          }else{
            datIO = readDipSwStatus();
            byteCount = 1;
          }
          digitalWrite(dir,1);
          delay(150);
          responseModusAscii1_2(addrS,fnc,byteCount,datIO);
          delay(250);
          digitalWrite(dir,0);
          delay(150);
          n = Serial.available();
          Serial.readBytes(buff,n);
        }
      }else if((addrS == addrHardware)&&(fnc == 0x04)){
        lrc = AsciiToHex(rxBuf[startFrame+13]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+14]);  
        if(lrc == findLRCinFrame()){
          addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
          addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
          addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
          addrIO += AsciiToHex(rxBuf[startFrame+8]);
          countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
          countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
          countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
          countIO += AsciiToHex(rxBuf[startFrame+12]);
          digitalWrite(dir,1);
          delay(150);
          responseModusAscii_0x04(addrS,fnc,addrIO,countIO);
          delay(250);
          digitalWrite(dir,0);
          delay(150);
          n = Serial.available();
          Serial.readBytes(buff,n);
        }
      }
      else if((addrS == addrHardware)&&(fnc == 0x05)){
        lrc = AsciiToHex(rxBuf[startFrame+13]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+14]);  
        if(lrc == findLRCinFrame()){
          addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
          addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
          addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
          addrIO += AsciiToHex(rxBuf[startFrame+8]);
          countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
          countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
          countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
          countIO += AsciiToHex(rxBuf[startFrame+12]);
          
          if(countIO == 0xff00){ 
            digitalWrite(ledPin[addrIO],1);
          }else { 
            digitalWrite(ledPin[addrIO],0);
          }
          
          lcd.setCursor(0,1);
          lcd.print("FX5:B:");
          lcd.setCursor(8,1);
          lcd.print(addrIO,BIN);
          lcd.setCursor(14,1);
          lcd.print(countIO,HEX);
          delay(150);
          digitalWrite(dir,1);
          delay(100);
          responseModusAscii_0x05(addrS,fnc,addrIO,countIO);
          delay(200);
          digitalWrite(dir,0);
          delay(100);
          n = Serial.available();
          Serial.readBytes(buff,n);
//          n = 0;
//          while(rxBuf[startFrame+n] != 0x0a){
//             Serial.write(rxBuf[startFrame+n]);
//             n++;
//          }
        }
      }
      else if((addrS == addrHardware)&&(fnc == 0x0F)){
        lrc = AsciiToHex(rxBuf[startFrame+17]) * 0x10;
        lrc += AsciiToHex(rxBuf[startFrame+18]);  
        lcd.clear();
        lcd.setCursor(0,1); lcd.print("HEllFNC0X0F .. "); 
        if(lrc == findLRCinFrame_0x0F()){
            lcd.clear();
            lcd.setCursor(0,1); lcd.print("HEllFNC0X0F .. ");  
            addrIO = AsciiToHex(rxBuf[startFrame+5]) * 0x1000;
            addrIO += (AsciiToHex(rxBuf[startFrame+6]) * 0x100);
            addrIO += (AsciiToHex(rxBuf[startFrame+7]) * 0x10);
            addrIO += AsciiToHex(rxBuf[startFrame+8]);
            countIO = AsciiToHex(rxBuf[startFrame+9]) * 0x1000;
            countIO += (AsciiToHex(rxBuf[startFrame+10]) * 0x100);
            countIO += (AsciiToHex(rxBuf[startFrame+11]) * 0x10);
            countIO += AsciiToHex(rxBuf[startFrame+12]);
            datF += AsciiToHex(rxBuf[startFrame+15]) * 0x10;
            datF += AsciiToHex(rxBuf[startFrame+16]);
            if (countIO == 4) {
              digitalWrite(ledPin[3], datF & 0x08);
              digitalWrite(ledPin[2], datF & 0x04);
              digitalWrite(ledPin[1], datF & 0x02);
              digitalWrite(ledPin[0], datF & 0x01);
            }
            lcd.setCursor(0,0); lcd.print(datF, BIN); 
            digitalWrite(dir,1);
            delay(150);
            responseModusAscii_0x0F(addrS, fnc, addrIO, countIO);
            delay(250);
            digitalWrite(dir,0);
            delay(150);
            n = Serial.available();
            Serial.readBytes(buff,n); 
        }
      }
      tail = 0;
    }
  }
}