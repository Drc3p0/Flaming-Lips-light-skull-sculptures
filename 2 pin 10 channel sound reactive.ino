#include <Adafruit_NeoPixel.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
 
const int numTracks = 5;
const char* tracks[] = {"TRACK1.wav", "TRACK2.wav", "TRACK3.wav","TRACK4.wav","TRACK5.wav"};
 
const int numChannels = 12;
const int switchPin = 16;
 
   bool trackIncremented = true;

AudioInputI2S          audioInput;         // audio shield: mic or line-in
AudioMixer4              mixer1;         //xy=298,510
AudioMixer4              mixer2;         //xy=298,510
AudioAnalyzeFFT1024    myFFT;
AudioOutputI2S         audioOutput;        // audio shield: headphones & line-out
AudioPlaySdWav           playWav1;       //xy=83,401
AudioConnection          patchCord1(playWav1, 0, mixer1, 3);
AudioConnection          patchCord2(playWav1, 1, mixer2, 3);
AudioConnection          patchCord5(audioInput, 0, mixer1,2);
AudioConnection          patchCord6(audioInput, 0, mixer2,2);
AudioConnection          patchCord10(mixer1, 0, audioOutput, 0);
AudioConnection          patchCord11(mixer2, 0, audioOutput, 1);
AudioControlSGTL5000 audioShield;
const int myInput = AUDIO_INPUT_MIC;
#define NEOPIXEL_PIN1    6
#define NEOPIXEL_PIN2    4
 
const int numPixels = numChannels / 3;
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(numPixels, NEOPIXEL_PIN1, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(numPixels, NEOPIXEL_PIN2, NEO_RGB + NEO_KHZ800);
int trackIndex = 0;
 
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

int bassSumHitTime = 0;
int bassSumPower = 0;
 
void setup() {
  Serial.begin(9600);
  AudioMemory(170);
  //button
  pinMode(switchPin, INPUT);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
      while (1) {
        Serial.println("Unable to access the SD card");
        delay(500);
      }
   }
  audioShield.enable();
  audioShield.volume(1.5);
  audioShield.inputSelect(myInput);
  //CORE_PIN14_CONFIG = 0;CORE 14 stops audio???
  pixels1.begin();
  pixels2.begin();
  for (int i = 0; i < numPixels; i++) {
    if(i < numPixels / 2){
      pixels1.setPixelColor(i, 0, 0, 0);      
    }
    if(i >= numPixels / 2){
      pixels2.setPixelColor(i, 0, 0, 0);
    }
  }
  myFFT.windowFunction(AudioWindowHanning1024);
}
 
elapsedMillis eM;
 
int lastMode = 0;  //used to be 0
int getMode() {
  int dr = digitalRead(switchPin);
//  int dr = 0;
  if(dr != lastMode){  
//    if(dr == lastMode){   //added in lieu of switchPin
   delay(100);
   Serial.println("Switching modes");
   Serial.println(dr);
   lastMode = dr;
  }
  return dr;
}
 
void outputToNeopixels(int bins[]){
  //make sure to prescale values
  for(int i = 0; i < numPixels; i++){
    unsigned int power[3];
    for(int x = 0; x < 3; x++){      
      power[x] = bins[x + (i * 3)];    
    }
    if(i < numPixels / 2){
      pixels1.setPixelColor(i, power[0], power[1], power[2]);      
    }
    if(i >= numPixels / 2){
      pixels2.setPixelColor(i, power[0], power[1], power[2]);
    }
  }
  for(int i=0;i < 12;i++){
   pixels1.setPixelColor(i, 0, 0, 0);
      pixels2.setPixelColor(i, 0, 0, 0);

  }
  pixels1.show();
  pixels2.show();
}
 
void getFFTArray(int bins[]){
  //highest piano key freq 4186.
  // 4186 / 22100 = .189
  // 3000 / 22100 =
   // 0.1357466063348416
  // .189 * 1028 = 194 // 139.55
  //139.55
 // log2(194) = 7.59;
 // log1.8(194) = 8.96
 // log1.5(194) = 13.02
 // 12.197
//   int binStep = topBin / numChannels;
//   float topBinExp = 12.19;
  //int topBinExp = 1;
  float generalPower = myFFT.read(3,17);
  Serial.println(generalPower);
  for(int i = 1;i <= numChannels; i++) {
 
    //Serial.println(i / numChannels);
  //  int binStart = pow(1.5, topBinExp * (float(i) / float(numChannels)))  - 1;
  //  int binEnd = pow(1.5, topBinExp * (float(i+1) / float(numChannels)));
    int binStart = 0;
    int binEnd = 0;
/*    if(i == 0){
      binStart = 3;
      binEnd = 4;
    }*/
    if(i == 1){
      binStart = 3;
      binEnd = 3;
    }
    if(i == 2){
      binStart = 4;
      binEnd = 4;
    }
   /* if(i == 3){
      binStart = 5;
      binEnd = 7;
    }*/
    if(i == 4){
      binStart = 5;
      binEnd = 5; 
    }
    if(i == 5){
      binStart = 6;
      binEnd = 7;
    }
    if(i == 6){
      binStart = 8;
      binEnd = 9;
    }
    if(i == 7){
      binStart = 10;
      binEnd = 11;
    }
    if(i == 8){
      binStart = 12;
      binEnd = 13;
    }

    /*if(i == 9){
      binStart = 5;
      binEnd = 7; 
    }*/
    if(i == 10){
      binStart = 14;
      binEnd = 15;
    }
    if(i == 11){
      binStart = 16;
      binEnd = 17;
    }
    if(i == 12){
      binStart = 18;
      binEnd = 20;
    }
    //delay(100);
    float fftScaled = myFFT.read(binStart, binEnd) * 2.5;
   /* if(i==5){
      fftScaled = fftScaled * 2.5;
    }*/
    if(fftScaled > 1){
      fftScaled = 1;
    }
/*    if(i==3){
      fftScaled = 1;
    }*/
//    fftScaled = 1;
    //bins[i] = fftScaled;
    /*Serial.println("Bins :");
    Serial.println(i);
    Serial.println(binStart);
    Serial.println(binEnd);
    Serial.println("POWER");*/
 
    //Serial.println(fftScaled);
    //Serial.println(fftScaled);
    bins[i] = fftScaled * 255;
    //bins[i] = centerFreq(fftScaled, 128);  
    //Serial.println(bins[i]);
  }
}
 
int centerFreq(float sinPower, int amp){
  int freqOutput = (sinPower * amp) + (128- (amp / 2));
  return freqOutput;
}
 
/*void playWavFile(){
  if(!playWav1.isPlaying()){
     playWav1.play(tracks[trackIndex]);
     trackIndex++;
     if(trackIndex >= numTracks){
       trackIndex = 0;
     }
   }*/
 
void playWavFile(){
  if(!playWav1.isPlaying() && trackIncremented){
     playWav1.play(tracks[trackIndex]);
     trackIncremented = false;
  }
  if(playWav1.isPlaying() && !trackIncremented ){
    trackIndex++;
    if(trackIndex >= numTracks){
     trackIndex = 0;
    }
    trackIncremented = true;
  }
}

 
void loop() {
  if(getMode() == 0){
    playWav1.stop();
    AudioConnection          patchCord4(audioInput, 0, myFFT, 0);
    //----initialize mode one---//
     //SOUND
    audioShield.micGain(30);
    //--loop of mode one--//
    while(getMode() == 0){
      doLights();
    }
  }
  else if (getMode() == 1){
    AudioConnection          patchCord3(playWav1, 0, myFFT, 0);
    audioShield.micGain(0);
    //--loop of mode two--//
    while(getMode() == 1) {
      //sound
      playWavFile();
      //lights
      doLights();
    }
  }
}

void doLights(){
    if (myFFT.available()) {
    int bins[numChannels];
    getFFTArray(bins);
    if(eM % 100 > 50){
      for(int i =0;i < 6;i++){
      /*  if(i > 5){
          if(bins[i] >= 130){
            bins[i] = 255;
          }
          if(bins[i] < 130){
            bins[i] = 0;
          }
        }*/
      }
    }
   // int binSum = bins[0] + bins[1] + bins[2] + bins[3] + bins[4] + bins[5];
   // if(binSum){
      
    //}
    outputToNeopixels(bins);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
/*      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    */}
  }
}
