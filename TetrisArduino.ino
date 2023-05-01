#include <LedControl.h>

bool frame = false;
long lastDelay;
float frameRate = 30;
//needs to be a power of two times 5

//board
long long board = 0;
//piece
long long piece = 0;
//piece data
byte rotation = 1;
byte pivotX = 0;
byte pivotY = 0;
String pieceType = "";

//inputs 
int antiCW;
int CW;
int left;
int right;
int down;

LedControl lc=LedControl(7,9,8,1);
LedControl scoreLc=LedControl(12,10,11,1);

//dictionary defining everything needed for each piece
typedef struct { 
  String piece;
  long long value;
  int minX;
  int maxX;
  int minY;
  int maxY;
} pieceDictonary;

const pieceDictonary pieceArray[] {
    //L
    {"11",259,1,7,2,8},
    {"12",769,1,7,2,8},
    {"13",770,1,7,2,8},
    {"14",515,1,7,2,8},
    
    //X
    {"21",513,1,7,2,8},
    {"22",258,1,7,2,8},
    {"23",513,1,7,2,8},
    {"24",258,1,7,2,8},

    //O
    {"31",771,1,7,2,8},
    {"32",771,1,7,2,8},
    {"33",771,1,7,2,8},
    {"34",771,1,7,2,8},

    //I
    {"41",257,1,8,2,8},
    {"42",768,1,7,1,9},
    {"43",514,0,7,2,8},
    {"44",3,1,7,1,8},

    //dots
    {"51",256,1,8,2,9},
    {"52",512,0,7,2,9},
    {"53",2,0,7,1,8},
    {"54",1,1,8,1,8},
};

byte pieceFinder(String p){
  for(int i=0;i<20; i++){
    if(pieceArray[i].piece==p){
      return i;
    }
  }
  return 0;
}

void setup() {
  Serial.begin(9600);
  pinMode(2,OUTPUT);
  pinMode(3,INPUT_PULLUP);
  pinMode(4,INPUT_PULLUP);
  pinMode(5,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);
  pinMode(13,INPUT_PULLUP);
  lc.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0,6);       // Set the brightness to maximum value
  lc.clearDisplay(0);          // and clear the display
  scoreLc.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
  scoreLc.setIntensity(0,6);       // Set the brightness to maximum value
  scoreLc.clearDisplay(0);          // and clear the display
  randomSeed(analogRead(0));

}

void loop() {
  if(millis()-lastDelay > (1050/frameRate)){
    frameRate--;
  }
  if(millis()-lastDelay > (1000/frameRate)){
    lastDelay=millis();
    frame = true;
  }
  if(frame==true){
    //this is what happens every frame

    //plays the song
    playSong();

    //saves all the inputs
    input();    

    movement();

    movePiece();

    destroyLines();

    generatePiece();
    //renders the board
    boardRenderer();
    frame=false;
  }
}

//song variables
int tempo=144; 
int buzzer = 2;
int melody[] = 
{
  659, 4,659, 4, 494,8, 523,8, 587,4, 523,8, 494,8, 440, 4, 440,8, 523,8, 659,4, 587,8, 523,8, 494, -4, 523,8, 587,4, 659,4, 523, 4, 440,4, 440,8, 440,4, 494,8, 523,8, 587, -4, 698,8, 880,4, 784,8, 698,8, 659, -4, 523,8, 659,4, 587,8, 523,8, 494, 4, 494,8, 523,8, 587,4, 659,4, 523, 4, 440,4, 440,4, 0, 4, 659, 4, 494,8, 523,8, 587,4, 523,8, 494,8, 440, 4, 440,8, 523,8, 659,4, 587,8, 523,8, 494, -4, 523,8, 587,4, 659,4, 523, 4, 440,4, 440,8, 440,4, 494,8, 523,8, 587, -4, 698,8, 880,4, 784,8, 698,8, 659, -4, 523,8, 659,4, 587,8, 523,8, 494, 4, 494,8, 523,8, 587,4, 659,4, 523, 4, 440,4, 440,4, 0, 4,
  659,2, 523,2, 587,2, 494,2, 523,2, 440,2, 415,2, 494,4, 0,8, 659,2, 523,2, 587,2, 494,2, 523,4, 659,4, 880,2, 831,2, 
};
int notes=sizeof(melody)/sizeof(melody[0])/2; 
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;
int noteSteps = 0;
int thisNote = 0;
int melodyCounter;

void playSong(){
  melodyCounter +=1;
  if(melodyCounter >= noteSteps){
    noTone(buzzer);
    thisNote += 2;
      // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
      noteSteps = frameRate/5*8 / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteSteps = frameRate/5*8 / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }
  
    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration*0.9);
  
    // Wait for the specief duration before playing the next note.
    // stop the waveform generation before the next note.
    if(thisNote>=notes*2){
      thisNote=0;
    }
    melodyCounter = 0;
  }
}

long long intersection;
int movePiceCounter=0;

void generatePiece(){

  if(piece==0){
    pieceType = String(random(1,6));
    piece = pieceArray[pieceFinder(pieceType+String(1))].value;
    piece = piece << 51;
    rotation = 1;
    pivotX = 4;
    pivotY = 2;
    movePiceCounter = 0;
    intersection = (unsigned long long)piece&(unsigned long long)board;
    if((unsigned long long)intersection!=(unsigned long long)0){
      digitalWrite(buzzer,LOW);
      piece = 0;
      board = 0B0111111011111111111111111001110010011100111101110111111000101010;
      boardRenderer();
      exit(0);
    }
  }
}

float difficulty=1;
float speed;

void movePiece(){

  movePiceCounter +=1;
  speed = frameRate - ((frameRate/12)*(difficulty-1));
  if(movePiceCounter>speed){
    //checks if the movement is legal, the movement is legal when there are no collision and you have not reached the maximum distance
    intersection = ((unsigned long long)piece >> (unsigned long long)8)&(unsigned long long)board;
    if((unsigned long long)intersection==(unsigned long long)0 && pivotY+1<=pieceArray[pieceFinder(pieceType+String(rotation))].maxY){
      piece  = (unsigned long long)piece >> 8;
      pivotY +=1;
    }else{
      board = board+piece;
      piece = 0;
    }
    movePiceCounter = 0;
  }
  
}

int lineCleared = 0; 
long long points;
int lineClearedFrame;
long long tempBoard;
long long difficultyDisplay = 1;
long long scoreBase = 0B0000011000000010000000110000000000000000000000010000000100000111;

void destroyLines(){
  lineClearedFrame=0;
  tempBoard=0;
  for(int n=0; n<2; n++){
    for(int i = 0; i<8; i++){
      intersection = ((unsigned long long)board>>((unsigned long long)8*i))&(unsigned long long)0B11111111;
      if((unsigned long long)intersection == (unsigned long long)0B11111111){
        int jj=0;
        for(int j = 0; j<8; j++){
          if(j!=i){
            tempBoard += (((unsigned long long)board>>(unsigned long long)(8*j)) & (unsigned long long)0B11111111)<<(unsigned long long)(8*jj);
            jj+=1;
          }
        }
        board = tempBoard;
        tempBoard = 0;
        lineClearedFrame +=1;
        break;
      }
    }
  }
  if(lineClearedFrame==2){
    lineClearedFrame = 4;
  }
  points += lineClearedFrame*difficulty;
  lineCleared += lineClearedFrame;
  if(difficulty<12 && lineCleared >=4+difficulty){
    lineCleared -= 4+difficulty;
    difficulty++;
    difficultyDisplay += 0B1 << int(difficulty-1);
  }
}

void input(){
  CW = digitalRead(3);
  antiCW = digitalRead(6);
  left = digitalRead(4);
  right = digitalRead(5);
  down = digitalRead(13);
}

bool movedL = false;
bool movedR = false;
bool movedCW = false;
bool movedACW = false;
bool movedDown = false;
long long tempPiece;

void movement(){
  if(down == 1 && movedDown==false){
    //checks if the movement is legal, the movement is legal when there are no collision and you have not reached the maximum distance
    intersection = ((unsigned long long)piece >> (unsigned long long)8)&(unsigned long long)board;
    if((unsigned long long)intersection==(unsigned long long)0 && pivotY+1<=pieceArray[pieceFinder(pieceType+String(rotation))].maxY){
      piece  = (unsigned long long)piece >> 8;
      pivotY +=1;
    }else{
      board = board+piece;
      piece = 0;
    }
    movePiceCounter = 0;
    movedDown=true;
  }else if(down == 0 && movedDown == true){
    movedDown = false;
  }

  
  if(left == 0 && movedL==false){
    intersection = ((unsigned long long)piece >> (unsigned long long)1)&(unsigned long long)board;
    if(pieceArray[pieceFinder(pieceType+String(rotation))].minX<pivotX && (unsigned long long)intersection==(unsigned long long)0){
      piece = (unsigned long long)piece >> 1;
      pivotX -=1;
    }
    movedL=true;
  }else if(left == 1 && movedL == true){
    movedL = false;
  }

  if(right == 0 && movedR==false){
    intersection = ((unsigned long long)piece << (unsigned long long)1)&(unsigned long long)board;
    if(pieceArray[pieceFinder(pieceType+String(rotation))].maxX>pivotX && (unsigned long long)intersection==(unsigned long long)0){
      piece = (unsigned long long)piece << 1;
      pivotX +=1;
    }
    movedR=true;
  }else if(right == 1 && movedR == true){
    movedR = false;
  }

  if(CW == 0 && movedCW==false){
    if(rotation == 4){
      tempPiece = pieceArray[pieceFinder(pieceType+"1")].value;
      rotation = 1;
    }else{
      tempPiece = pieceArray[pieceFinder(pieceType+String(rotation+1))].value;
      rotation +=1;
    }
    if(pivotX<pieceArray[pieceFinder(pieceType+String(rotation))].minX){
      pivotX = pieceArray[pieceFinder(pieceType+String(rotation))].minX;
    }if(pivotX>pieceArray[pieceFinder(pieceType+String(rotation))].maxX){
      pivotX = pieceArray[pieceFinder(pieceType+String(rotation))].maxX;
    }if(pivotY<pieceArray[pieceFinder(pieceType+String(rotation))].minY){
      pivotY = pieceArray[pieceFinder(pieceType+String(rotation))].minY;
    }if(pivotY>pieceArray[pieceFinder(pieceType+String(rotation))].maxY){
      pivotY = pieceArray[pieceFinder(pieceType+String(rotation))].maxY;
    }
    tempPiece = (unsigned long long)tempPiece << 8*(8-pivotY);
    tempPiece = (unsigned long long)tempPiece << 1*(pivotX-1);
    intersection = (unsigned long long)tempPiece & (unsigned long long)board;
    if((unsigned long long)intersection==(unsigned long long)0){
      piece = tempPiece;
    }else{
      if(rotation==1){
        rotation = 4;
      }else{
        rotation -=1;
      }
    }
    movedCW=true;
    
  }else if(CW == 1 && movedCW == true){
    movedCW = false;
  }

  if(antiCW == 0 && movedACW==false){
    if(rotation == 1){
      tempPiece = pieceArray[pieceFinder(pieceType+"4")].value;
      rotation = 4;
    }else{
      tempPiece = pieceArray[pieceFinder(pieceType+String(rotation-1))].value;
      rotation -=1;
    }
    if(pivotX<pieceArray[pieceFinder(pieceType+String(rotation))].minX){
      pivotX = pieceArray[pieceFinder(pieceType+String(rotation))].minX;
    }if(pivotX>pieceArray[pieceFinder(pieceType+String(rotation))].maxX){
      pivotX = pieceArray[pieceFinder(pieceType+String(rotation))].maxX;
    }if(pivotY<pieceArray[pieceFinder(pieceType+String(rotation))].minY){
      pivotY = pieceArray[pieceFinder(pieceType+String(rotation))].minY;
    }if(pivotY>pieceArray[pieceFinder(pieceType+String(rotation))].maxY){
      pivotY = pieceArray[pieceFinder(pieceType+String(rotation))].maxY;
    }
    tempPiece = (unsigned long long)tempPiece << 8*(8-pivotY);
    tempPiece = (unsigned long long)tempPiece << 1*(pivotX-1);
    intersection = (unsigned long long)tempPiece & (unsigned long long)board;
    if((unsigned long long)intersection==(unsigned long long)0){
      piece = tempPiece;
    }else{
      if(rotation==4){
        rotation = 1;
      }else{
        rotation +=1;
      }
    }
    movedACW=true;
    
  }else if(antiCW == 1 && movedACW == true){
    movedACW = false;
  }
  
}

long long boardAndPiece;
long long scoreAndBack;
void boardRenderer(){

  boardAndPiece = board+piece;
  
  for(int i=0;i<64;i++){
    lc.setLed(0,i & 0B111,(i& 0B111000)>>3,(boardAndPiece>>i)&0B1);
  }

  scoreAndBack = scoreBase + ((difficultyDisplay & 0B1111)<<4) + (((difficultyDisplay>>4) & 0B1111)<<12) + (((difficultyDisplay>>8) & 0B1111)<<20)+ ((points & 0B1111)<<44) + (((points>>4) & 0B1111)<<52) + (((points>>8) & 0B1111)<<60);
  for(int i=0;i<64;i++){
    scoreLc.setLed(0,i & 0B111,(i& 0B111000)>>3,(scoreAndBack>>i)&0B1);
  }
}

void bigPrint(uint64_t n){
  //Print unsigned long long integers (uint_64t)
  //CC (BY-NC) 2020
  //M. Eric Carr / paleotechnologist.net
  unsigned char temp;
  String result=""; //Start with a blank string
  if(n==0){Serial.println(0);return;} //Catch the zero case
  while(n){
    temp = n % 10;
    result=String(temp)+result; //Add this digit to the left of the string
    n=(n-temp)/10;      
    }//while
  Serial.println(result);
  }
