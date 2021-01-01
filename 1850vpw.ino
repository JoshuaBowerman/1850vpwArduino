
const int dataInPin = 2;    //These are seperate because of the circuitry required to interact with the line
const int dataOutPin = 3;   //They both go to pin 2 of the ODB harness eventually

//Interrupt Variables
volatile bool fullFrame = false;//Marks whether or not the buffer contains an entire frame
volatile int buff[4096];         //256 byte buffer for incoming frames, Start Of Frame is represanted as 3
volatile int buffI = 0;         //Buffer Index
volatile unsigned int currentByte = 0;   //The byte being assembled
volatile int currentBit = 0;    //Bit index
volatile unsigned long lastChange = 0; // it is zero to force the program to ignore its previous state.
volatile int normBit =0;

//Timing Margins
const int margin = 25;       //The margin in uS for data pulses time, eg. if we are looking for 64uS we will take 54-74uS if the margin is 10uS
#define withinMargin(Duration,Target) (((Target - margin) <= Duration) && (Duration  <= (Target + margin))) // whether or not the duration meets the target margins.

void setup() {
  pinMode(dataOutPin, OUTPUT);
  pinMode(dataInPin, INPUT);
  
  Serial.begin(115200);
  Serial.println("BEGIN DATA");
  
  //setup our interrupt
  attachInterrupt(digitalPinToInterrupt(dataInPin), pin_ISR, CHANGE);

}
int Frame[4096];
int frameSize = 0;




void loop() {
  if(fullFrame){
    //let's expand this frame
    for(int i = 0; buff[i+1] != 3;i++){ //while we are in the first frame.
      Frame[i] = buff[i];
      frameSize = i;
    }

    //rearange the buffer
    noInterrupts();
      for(int i = frameSize + 1; i < 256; i++){
        buff[i - frameSize - 1] = buff[i];
      }
      buffI = frameSize;
    interrupts();

    //Send the frame
    Serial.print("FRAME:");
    Serial.print(Frame[0]);
    for(int i = 1; i < frameSize; i++){
      Serial.print(",");
      Serial.print(Frame[i]);
    }
    Serial.println("");
    
  }
}


//processes a bit and adds it to the buffer.
//Also marks the buffer as containing an entire frame if it receives the begining of a new frame.
void insertBit(unsigned int v) {
  if (v == 5) {
    normBit = 0;
    fullFrame = true;
    //End of Frame
    //Justify and write byte being read, unless it's empty
    if (currentBit > 0) {
      currentByte = currentByte << (8 - currentBit);
      buff[buffI++] = currentByte;
      currentBit = 0;
    }
    buff[buffI++] = -5;
  }
  else if(v > 1){
    if (currentBit > 0) {
      currentByte = currentByte << (8 - currentBit);
      buff[buffI++] = currentByte;
      currentBit = 0;
    }
    buff[buffI++] = -v;
  }else{
  currentByte = (currentByte << 1) + v;
  currentBit += 1;
  if (currentBit == 8) {
    buff[buffI++] = currentByte;
    currentByte = 0;
    currentBit = 0;
  }
  }



}

void pin_ISR() {
  unsigned long curTime = micros();
  unsigned long pulseDuration = curTime - lastChange;
  lastChange = curTime;

  bool state = !digitalRead(dataInPin); // We invert the state since we want the state of the pulse not the current state.

  if (state == HIGH) {
    if (withinMargin(pulseDuration, 64)) {
      //1
      //Discard Normilization Bit
      if(buff[buffI - 1] == -4){
           normBit = 1;
      }else{
           insertBit(1);
      }
    }
    if (withinMargin(pulseDuration, 128)) {
      //0
      //Discard Normilization Bit
      if(buff[buffI - 1] == -4){
           normBit = 1;
      }else{
           insertBit(0);
      }
    }
    if (withinMargin(pulseDuration, 200)) {
      //End of Data
      insertBit(4);
    }
    if (withinMargin(pulseDuration, 280)) {
      //End of Frame
      insertBit(5);
    }
  } else {
    if (withinMargin(pulseDuration, 64)) {
      //0
      
      if(
      insertBit(0);
    }
    if (withinMargin(pulseDuration, 128)) {
      //1
      //Discard Normilization Bit
      insertBit(1);
    }
    if (withinMargin(pulseDuration, 200)) {
      //Start of Frame
      insertBit(3);
    }
    
  }

}
