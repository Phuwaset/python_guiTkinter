from tkinter import *
from tkinter import messagebox
from tkinter import ttk
import serial
import serial.tools.list_ports
import sys

port = None
comport = None
st = False
slAddr = None
stAddrIO = None
QuantityIO = None
adcCh = [0, 0, 0]
bufRx = ""  # เพิ่มการนิยาม bufRx เป็นสตริงว่าง
coilSt = 0  # นิยามตัวแปร coilSt และกำหนดค่าเริ่มต้นเป็น 0


def SendUart(d):
    global st,comport
    if st == False:
        messagebox.showinfo("Status of Serial Port", "Not select Com Port & OpenPort")
    else :
        comport.write(d.encode('utf-8'))   #send data to byte
        
def ConnectUART():
    global port,st,comport
    if (st==False) and (port != None):
        comport = serial.Serial(port,baudrate=9600)
        btnConnect.config(text="Disconnect")
        st = True
    elif st == True:
        btnConnect.config(text="Connect")
        st = False
        comport.close()

def serial_ports():    
    return serial.tools.list_ports.comports()
    
def on_select(event=None):
    global port
    print("comboboxes: ", cb.get())
    x = cb.get().split()
    port = x[0]
    

def check_Rx():
    global st, comport, slAddr, stAddrIO, QuantityIO, bufRx
    if (st == True) and (comport.in_waiting > 0):
        n = comport.in_waiting
        rx = comport.read(n).decode("utf-8")
        bufRx = rx  # กำหนดค่า bufRx เป็นข้อมูลที่รับมาผ่าน Serial Port
        d = [x for x in rx]
        print(d)
        addrS = (AsciiToDec(rx[1]) * 0x10) + AsciiToDec(rx[2])
        fnc = (AsciiToDec(rx[3]) * 0x10) + AsciiToDec(rx[4])
        dat = (AsciiToDec(rx[7]) * 0x10) + AsciiToDec(rx[8])
        print("addrS" + str(addrS) + "fnc" + str(fnc) + "datIO" + str(dat) + "Quantity" + str(QuantityIO))
        if fnc == 0x01:
            if QuantityIO == 4:
                ShowStatusCoil(dat)
            else:
                ShowStatusOnOff(stAddrIO, dat)
        elif fnc == 0x02:
            ShowStatusInput(dat)
        elif fnc == 0x04:
            ShowAnalogInput(stAddrIO, QuantityIO, bufRx)  # เรียกใช้งาน ShowAnalogInput() สำหรับค่า ADC
        else:
            Show2StatusOnOff(stAddrIO, dat)
    form.after(1000, check_Rx)

def ShowAnalogInput(AddrIO, QuantityIO, bufRx):
    global adcCh, lbAN0_val, lbAN1_val, lbAN2_val
    if QuantityIO == 3:
        adcDat = bufRx[7:11]
        adcCh[0] = int(adcDat, 16)
        L = len(lbAN0_val.get())
        if L:
            lbAN0_val.delete(0, L)
        lbAN0_val.insert(0, adcDat)

        adcDat = bufRx[11:15]
        adcCh[1] = int(adcDat, 16)
        L = len(lbAN1_val.get())
        if L:
            lbAN1_val.delete(0, L)
        lbAN1_val.insert(0, adcDat)

        adcDat = bufRx[15:19]
        adcCh[2] = int(adcDat, 16)
        L = len(lbAN2_val.get())
        if L:
            lbAN2_val.delete(0, L)
        lbAN2_val.insert(0, adcDat)
        
    elif QuantityIO == 2 :
        adcDat = bufRx[7:11]
        adcCh[0] = int(adcDat, 16)
        L = len(lbAN0_val.get())
        if L:
            lbAN0_val.delete(0, L)
        lbAN0_val.insert(0, adcDat)

        adcDat = bufRx[11:15]
        adcCh[1] = int(adcDat, 16)
        L = len(lbAN1_val.get())
        if L:
            lbAN1_val.delete(0, L)
        lbAN1_val.insert(0, adcDat)
    
    elif QuantityIO == 1 :
        adcDat = bufRx[7:11]
        adcCh[0] = int(adcDat, 16)
        L = len(lbAN0_val.get())
        if L:
            lbAN0_val.delete(0, L)
        lbAN0_val.insert(0, adcDat)

def AsciiToDec(ascii_char):
    if '0' <= ascii_char <= '9':
        return ord(ascii_char) - ord('0')
    elif 'A' <= ascii_char <= 'F':
        return ord(ascii_char) - ord('A') + 10
    elif 'a' <= ascii_char <= 'f':
        return ord(ascii_char) - ord('a') + 10
    else :
        raise ValueError("Invalid input")

def asciiToHex(c):
    if '0' <= c <= '9':
        c = int(c) & 0x0F
    else:
        c = (int(c) & 0x0F) + 9
    return c

def findLRC(s):
    asc = "0123456789ABCDEF"
    L = 1
    n = (len(s) - 1) // 2
    LRC = 0
    while n > 0:
        LRC_part1 = int(ascii_to_hex(s[L]), 16) * 0x10
        LRC_part2 = int(ascii_to_hex(s[L+1]), 16)
        LRC += LRC_part1 + LRC_part2
        L += 2
        n -= 1
    LRC = LRC % 256
    LRC = (~LRC) + 1
    LRC = 256 + LRC
    r = asc[LRC // 16] + asc[LRC % 16]
    print(r)
    return r 

def callback(event):
    global coilSt  # ระบุว่าคุณกำลังใช้งานตัวแปร coilSt จาก global scope
    if st == True:
        print("clicked at" , event.x,event.y)
        if (event.x >= 100) and (event.x <= 120) and (event.y >= 70) and(event.y <= 90):
            coilSt ^= 0x01
        elif (event.x >= 70) and (event.x <= 90) and (event.y >= 70) and(event.y <= 90):  
             coilSt ^= 0x02
        elif (event.x >= 40) and (event.x <= 60) and (event.y >= 70) and(event.y <= 90):  
             coilSt ^= 0x04
        elif (event.x >= 10) and (event.x <= 30) and (event.y >= 70) and(event.y <= 90):  
             coilSt ^= 0x08
        print(coilSt)
        ShowStatusCoil(coilSt)

        
def ShowStatusOnOff(startAddr, stCon):
    if stCon & 0x08 :
        create_circle(startAddr + 20, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 20, 80, 10, myCanvas, "white")
    if stCon & 0x04 :
        create_circle(startAddr + 49, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 49, 80, 10, myCanvas, "white")
    if stCon & 0x02 :
        create_circle(startAddr + 80, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 80, 80, 10, myCanvas, "white")
    if stCon & 0x01 :
        create_circle(startAddr + 110, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 110, 80, 10, myCanvas, "white")
        
def Show2StatusOnOff(startAddr, stCon):
    if stCon & 0x08 :
        create_circle(startAddr + 20, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 20, 80, 10, myCanvas, "white")
    if stCon & 0x04 :
        create_circle(startAddr + 49, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 49, 80, 10, myCanvas, "white")
    if stCon & 0x02 :
        create_circle(startAddr + 80, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 80, 80, 10, myCanvas, "white")
    if stCon & 0x01 :
        create_circle(startAddr + 110, 80, 10, myCanvas, "red")
    else:
        create_circle(startAddr + 110, 80, 10, myCanvas, "white")   


def ReadCoils():
    global slAddr,stAddrIO,QuantityIO
    if len(enSlave.get()) == 2:
        if len(enStartIO.get()) == 4:
            if len(enQuantityIO.get()) == 4:
                modbusASC = ":" + enSlave.get() + "01" + enStartIO.get() + enQuantityIO.get()
                modbusASC = modbusASC + findLRC(modbusASC) + "\r\n"
                print(modbusASC) 
                SendUart(modbusASC)
                slAddr = int(enSlave.get(), 16)
                stAddrIO = int(enStartIO.get())
                QuantityIO = int(enQuantityIO.get())
            else:
                messagebox.showinfo("Quantity IO Invaild", "ป้อนข้อมูล Quantity IO จำนวน 4 หลักเท่านั้น")
        else:
            messagebox.showinfo("Start Address to Invaild","ป้อนข้อมูล Start Addrss IO จำนวน 2 หลักเท่านั้น")
    else:
         messagebox.showinfo("Start Address to Invaild","ป้อนข้อมูล Slave Addrss IO จำนวน 2 หลักเท่านั้น")


def ReadInputs():
    global slAddr,stAddrIO,QuantityIO
    if len(enSlave.get()) == 2:
        if len(enStartIO.get()) == 4:
            if len(enQuantityIO.get()) == 4:
                modbusASC = ":" + enSlave.get() + "02" + enStartIO.get() + enQuantityIO.get()
                lrc = findLRC(modbusASC)
                modbusASC = modbusASC + lrc + "\r\n"
                print(modbusASC) 
                SendUart(modbusASC)
                slAddr = int(enSlave.get(), 16)
                stAddrIO = int(enStartIO.get())
                QuantityIO = int(enQuantityIO.get())
            else:
                messagebox.showinfo("Quantity IO Invaild", "ป้อนข้อมูล Quantity IO จำนวน 4 หลักเท่านั้น")
        else:
            messagebox.showinfo("Start Address to Invaild","ป้อนข้อมูล Start Addrss IO จำนวน 2 หลักเท่านั้น")
    else:
         messagebox.showinfo("Start Address to Invaild","ป้อนข้อมูล Slave Addrss IO จำนวน 2 หลักเท่านั้น")
         
         
def ReadInputRegisters():
    global slAddr, stAddrIO, QuantityIO, bufRx
    if len(enSlave.get()) == 2:
        if len(enStartIO.get()) == 4:
            if len(enQuantityIO.get()) == 4:
                modbusASC = ":" + enSlave.get() + "04" + enStartIO.get() + enQuantityIO.get()
                lrc = findLRC(modbusASC)
                modbusASC = modbusASC + lrc + "\r\n"
                print(modbusASC)
                SendUart(modbusASC)
                slAddr = int(enSlave.get(), 16)
                stAddrIO = int(enStartIO.get())
                QuantityIO = int(enQuantityIO.get())
                bufRx = None  # รีเซ็ตค่า bufRx เพื่อเตรียมรับข้อมูลใหม่
            else:
                messagebox.showinfo("Invalid Quantity IO", "Please enter a 4-digit Quantity IO.")
        else:
            messagebox.showinfo("Invalid Start Address IO", "Please enter a 4-digit Start Address IO.")
    else:
         messagebox.showinfo("Invalid Slave Address", "Please enter a 2-digit Slave Address.")

def WriteSingleCoil():
    global slAddr,stAddrIO,QuantityIO,coilSt
    asciiArr = "0123456789abcdef"
    if len(enSlave.get()) == 2:
        if len(enQuantityIO.get()) == 4:
            if len(enQuantityIO.get()) == 4:
                modbusASC = ":" + enSlave.get() + "05" + enStartIO.get() + enQuantityIO.get()
                lrc = findLRC(modbusASC)
                modbusASC = modbusASC + lrc + "\r\n"
                SendUart(modbusASC)
                print(modbusASC) 
                slAddr = int(enSlave.get(), 16)
                stAddrIO = int(enStartIO.get())
                QuantityIO = int(enQuantityIO.get(), 16)
            else:
                messagebox.showinfo("Invalid Quantity IO", "Please enter a 4-digit Quantity IO.")
        else:
            messagebox.showinfo("Invalid Start Address IO", "Please enter a 4-digit Start Address IO.")
    else:
         messagebox.showinfo("Invalid Slave Address", "Please enter a 2-digit Slave Address.")


def WriteMultiCoil():
    global slAddr,stAddrIO,QuantityIO,coilSt
    asciiArr = "0123456789abcdef"
    if len(enStartIO.get()) == 4:
        if len(enQuantityIO.get()) == 4:
            if len(enQuantityIO.get()) == 4:
                modbusASC = ":" + enSlave.get() + "0f" + enStartIO.get() + enQuantityIO.get() + "01" + "0" + asciiArr[coilSt]
                modbusASC = modbusASC + findLRC(modbusASC) + "\r\n"
                SendUart(modbusASC)
                print(modbusASC)
            else:
                messagebox.showinfo("Invalid Quantity IO", "Please enter a 4-digit Quantity IO.")
        else:
            messagebox.showinfo("Invalid Start Address IO", "Please enter a 4-digit Start Address IO.")
    else:
         messagebox.showinfo("Invalid Slave Address", "Please enter a 2-digit Slave Address.")

def Show2StatusOnOff(startAddr, stCon):
    global bufRx
    if bufRx is not None:
        if stCon & 0x08 :
            create_circle(startAddr + 20, 80, 10, myCanvas, "red")
        else:
            create_circle(startAddr + 20, 80, 10, myCanvas, "white")
        if stCon & 0x04 :
            create_circle(startAddr + 49, 80, 10, myCanvas, "red")
        else:
            create_circle(startAddr + 49, 80, 10, myCanvas, "white")
        if stCon & 0x02 :
            create_circle(startAddr + 80, 80, 10, myCanvas, "red")
        else:
            create_circle(startAddr + 80, 80, 10, myCanvas, "white")
        if stCon & 0x01 :
            create_circle(startAddr + 112, 80, 10, myCanvas, "red")
        else:
            create_circle(startAddr + 112, 80, 10, myCanvas, "white")          
            
def ascii_to_hex(c):
    if ('0' <= c <= '9'):
        c = int(c) & 0x0F
    else:
        c = (ord(c) & 0x0F) + 9
    return hex(c)
    
def on_exit():
    global st,comport
    if st==True:
        comport.close()
    form.destroy()
    sys.exit()

def create_circle(x, y, r, canvasName ,color): #center coordinates, radius
    x0 = x - r
    y0 = y - r
    x1 = x + r
    y1 = y + r
    return canvasName.create_oval(x0, y0, x1, y1,fill=color)

def create_rectangle(x0, y0, x1, y1 , canvasName ,color): 
    return canvasName.create_rectangle(x0, y0, x1, y1,fill=color)


def ShowStatusInput(stCon):
    if stCon & 0x08 :
        create_rectangle(150, 70, 170 , 90 , myCanvas , "red")
    else:
        create_rectangle(150, 70, 170 , 90 , myCanvas , "white")
    if stCon & 0x04 :
        create_rectangle(180, 70, 200 , 90 , myCanvas , "red")
    else:
        create_rectangle(180, 70, 200 , 90 , myCanvas , "white")
    if stCon & 0x02 :
        create_rectangle(210, 70, 230 , 90 , myCanvas , "red")
    else:
        create_rectangle(210, 70, 230 , 90 , myCanvas , "white")
    if stCon & 0x01 :
        create_rectangle(240, 70, 260 , 90 , myCanvas , "red")
    else:
        create_rectangle(240, 70, 260 , 90 , myCanvas , "white")

def ShowStatusCoil(stCoil):
    if stCoil & 8 :
        create_circle(20, 80, 10 , myCanvas , "red")
    else:
        create_circle(20, 80, 10 , myCanvas , "white")
    if stCoil & 4 :
        create_circle(50, 80, 10 , myCanvas , "red")
    else:
        create_circle(50, 80, 10 , myCanvas , "white")
    if stCoil & 2 :
        create_circle(80, 80, 10 , myCanvas , "red")
    else:
        create_circle(80, 80, 10 , myCanvas , "white")
    if stCoil & 1 :
        create_circle(110, 80, 10 , myCanvas , "red")
    else:
        create_circle(110, 80, 10 , myCanvas , "white")
        
# def update_gauge():
#     newvalue = random.randint(low_r,hi_r)
#     cnvs.itemconfig(id_text,text = str(newvalue) + " %")
#     # Rescale value to angle range (0%=120deg, 100%=30 deg)
#     angle = 120 * (hi_r - newvalue)/(hi_r - low_r) + 30
#     cnvs.itemconfig(id_needle,start = angle)
#     root.after(3000, update_gauge)
 
if _name_ == '_main_':
    form = Tk()
    form.title("Test Read Modbus ASCII Python GUI")
    form.geometry("500x500")
    myCanvas = Canvas(form,width=1000, height=600)
    myCanvas.bind("<Button-1>",callback)
    myCanvas.pack()
    
    #สร้างตัวแปรที่เชื่อมโยงกับ widget ต่างๆ
    lbSerial = Label(form, text = "Serial Port")
    cb = ttk.Combobox(form, values=serial_ports())
    cb.bind('<<ComboboxSelected>>', on_select)
    lbSlave = Label(form, text="Slave Address Device : ")
    lbStartIO = Label(form, text="Start Address IO:")
    lbQuantityIO = Label(form, text="Quantity IO:")
    enSlave = Entry(form, width=10)
    enStartIO = Entry(form, width=10)
    enQuantityIO = Entry(form, width=10)
    btnConnect = Button(form, text = "Connect", command = ConnectUART)
    lbControlLed = Label(form, text = "Button for Send Modbus ASCII Function 01-0F")
    btnFNC01 = Button(form, text = "FNC01", command = ReadCoils)
    btnFNC02 = Button(form, text = "FNC02", command = ReadInputs)
    btnFNC04 = Button(form, text="FNC04", command=ReadInputRegisters)
    btnFN05 = Button(form, text="FNC05", command=WriteSingleCoil)
    btnFN0F = Button(form, text="FNC0F", command=WriteMultiCoil)
    lbStCoil = Label(form, text = "Status of Coil")
    lbStInput = Label(form, text = "Status of Input")
    lbAN0 = Label(form, text="ADC 0:")
    lbAN0_val = Entry(form, width=10)
    lbAN1 = Label(form, text="ADC 1:")
    lbAN1_val = Entry(form, width=10)
    lbAN2 = Label(form, text="ADC 2:")
    lbAN2_val = Entry(form, width=10)
    lbRx = Label(form, text = "Call Back:")
    btnExit = Button(form, text = "ปิดโปรแกรม", command = on_exit)

    #สร้าง widget บน form gui
    lbSerial.place(x=10, y=5)
    cb.place(x=80, y=10)
    
    btnConnect.place(x=240, y=10)
    
    lbStCoil.place(x=10, y=40)
    lbStInput.place(x=150, y=40)
    
    lbControlLed.place(x=10, y=110)
    
    btnFNC01.place(x=10, y=135)
    btnFNC02.place(x=70, y=135)
    btnFNC04.place(x=130, y=135)
    btnFN05.place(x=190, y=135)
    btnFN0F.place(x=250, y=135)  # แทน xxx และ yyy ด้วยตำแหน่งที่คุณต้องการให้ปุ่มแสดงบน GUI
    lbSlave.place(x=10, y=170)
    enSlave.place(x=140, y=170)
    lbStartIO.place(x=210, y=170)
    enStartIO.place(x=310, y=170)
    lbQuantityIO.place(x=380, y=170)
    enQuantityIO.place(x=450, y=170)
    ShowStatusCoil(5)
    ShowStatusInput(12)
    lbAN0.place(x=10, y=220)
    lbAN0_val.place(x=60, y=220)
    lbAN1.place(x=150, y=220)
    lbAN1_val.place(x=200, y=220)
    lbAN2.place(x=290, y=220)
    lbAN2_val.place(x=340, y=220)
    lbRx.place(x=10, y=320)
    btnExit.place(x=10, y=350)
    
    #ประมวลผลใน loop main
    form.after(1000,check_Rx)
    form.protocol("WM_DELETE_WINDOW",on_exit)

    form.mainloop()