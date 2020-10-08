/*  to compile in terminal, and run:
sudo apt-get install: libsdl2-dev, libsdl2-image-dev, libsdl2-ttf-dev

g++ FILENAME.cpp -w -lSDL2 -lSDL2_image -lSDL2_ttf -o FILENAME && ./FILENAME
g++ states2.cpp -w -lSDL2 -lSDL2_image -lSDL2_ttf -o states2 && ./states2

-lSDL2_mixer libsdl2-mixer-dev not needed here, but for the future
*/

#include <SDL2/SDL.h>
#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <time.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>  
using namespace std;



random_device rd;     // only used once to initialise (seed) engine
mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

void displayHelpText();
void randomColorFunction();

string pather="/var/www/html/states/";

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
const int cellSize=6;//changing this value will affect checkDensity function
//checkDensity values need to be made dependent upon screen_width/height values
//so can change cellSize without affecting large rectangles
const int SCREEN_WIDTH=1364-(1364%cellSize); //1280, 1364
const int SCREEN_HEIGHT=764-(764%cellSize); //720, 764
bool running=true;
int lineXStart=0,lineYStart=0,lineXEnd=SCREEN_WIDTH,lineYEnd=0;
const int numStates=17;
const int numDots=(SCREEN_WIDTH/cellSize)*(SCREEN_HEIGHT/cellSize);
const int lastHDot=SCREEN_WIDTH-cellSize;
const int lastVDot=SCREEN_HEIGHT-cellSize;
const int stateColor[numStates][3]={
{0,0,0},

{255,0,0},
{0,255,0},
{255,255,0},
{0,0,255},
{0,155,155},
{255,0,255},
{255,255,255},
{140,255,0},

{0,255,42},
{255,0,48},
{255,150,0},
{186,255,0},
{0,186,255},
{234,0,255},
{117,0,146},
{188,3,3}

};

bool useRandColor=false;
int randColor[numStates][3];
int delayer=50;
int shiftX=0;
int shiftY=0;
int drawState=1;
bool drawMode=false;
bool dragging=false;
int renderShiftX=0, renderShiftY=0, renderShiftXLast=0, renderShiftYLast=0;
int mouseX=0, mouseY=0, mouseClickX=0, mouseClickY=0;
int rendScale=1;
bool showGrid=1;
bool ruleSaved=0;
int ruleSavedTick=0;
int numRuleSaves=0;
int numRuleAborts=0;
int countLargeRectBools=0,itter=0,countSame=0;
int checkDensityTicker=0;
int countSameTicker=0;
int countLargeRectBoolsTicker=0;
bool checkDensity=0;
int numLargeRects=0;
vector<bool>largeRectBool;
vector<bool>largeRectBoolS;
vector<int>largeRectBoolCounter;
const int largeRectSize=cellSize*20;

bool stepMode=0;
bool stepOnce=0;
bool pauser=0;
int dense=1;
bool altKey=0;

void checkDensityFunction();

//variables for auto restarting, when in autoRestart mode (key L), when screen is either blank or too full too long
bool autoRestart=0;
int stepCount=0;
int totalDead=0;

int stateCounter[numStates]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


//random
int rander(int min, int max){ 
uniform_int_distribution<int> uni(min,max); // guaranteed unbiased
auto random = uni(rng);
return random;
}//end rander


//values required to save and load a rule
int birthCount[numStates][numStates];
int deathCount[numStates][numStates];
int newState[numStates][numStates];

int birthNeighsMax=16;
int deathNeighsMax=2;
int bCondsMatch=1;
int dCondsMatch=0;

vector<string>stateFn;
int stateFni=0;

vector<string>XYStateFn;
int XYStateFni=0;

int interest=0;
int interesting[]={1289,818,1356,313,1313,1887};
int startingRule=0; //set to 0 for starting with random rule
int ruleFni=startingRule;
int ruleFnis=ruleFni;
vector<string>ruleFn;
void saveState();
void saveXYState();
void saveRule();
void readRuleFilenames();
void readStateFilenames();
void readXYStateFilenames();
void loadRule(int ruleFni);
void loadState(int stateFni);
void loadXYState(int XYStateFni, int i, int shiftX, int shiftY);
void saveScreenshotPNG();
void deleteAllDupRuleFiles();
void deleteRule();
bool bRuleExists();

//for printing text over window, in lieu of using console; should be include
SDL_Color textColor = { 255, 255, 255 };
bool displayVars=1;
bool displayHelp=0;
//Texture wrapper class
class LTexture{
public:
LTexture();
~LTexture();
bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
void free();
void renderText( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );
int getWidth();
int getHeight();

private:
SDL_Texture* mTexture;
int mWidth;
int mHeight;
};//end texture wrapper class

TTF_Font *gFont = NULL;
LTexture gTextTexture;
LTexture::LTexture(){mTexture=NULL;mWidth=0;mHeight=0;}
LTexture::~LTexture(){free();}
void LTexture::free(){
if( mTexture != NULL ){
SDL_DestroyTexture( mTexture );
mTexture = NULL;
mWidth = 0;
mHeight = 0;
}}//end free

bool LTexture::loadFromRenderedText(string textureText, SDL_Color textColor ){
free();
SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
mWidth = textSurface->w;
mHeight = textSurface->h;
SDL_FreeSurface( textSurface );
return mTexture != NULL;
}//end loadFromRenderedText

void LTexture::renderText( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip ){
SDL_Rect renderQuad = { x, y, mWidth, mHeight };
if( clip != NULL ){
renderQuad.w = clip->w;
renderQuad.h = clip->h;
}

SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}//end render text

int LTexture::getWidth(){return mWidth;}
int LTexture::getHeight(){return mHeight;}

SDL_Rect textBackRect={0,0,0,0};
SDL_Rect mouseRect={0, 0, 1, 1};

//dot class
class dots{
public:
int largeRectId;
int state,stateTemp;
int id;
int near8[9];
void drawDot(int x, int y);
void nearN();
SDL_Rect fillRect={0, 0, cellSize, cellSize};
} dot[numDots];

//draw dots
void dots::drawDot(int x, int y){
fillRect.x=x + (renderShiftX*cellSize); 
fillRect.y=y + (renderShiftY*cellSize);
if(useRandColor){
SDL_SetRenderDrawColor(gRenderer, 
randColor[state][0], randColor[state][1], randColor[state][2], 255);	
}else{
SDL_SetRenderDrawColor(gRenderer, 
stateColor[state][0], stateColor[state][1], stateColor[state][2], 255);	
}	
SDL_RenderFillRect(gRenderer, &fillRect);
}//end draw dots

//set dot id
void setId(){
for(int c=0;c<numDots;c++){dot[c].id=c;}

}//end set dot id


//set state of each dot
void setState(){
stepCount=0;
for(int c=0;c<numDots;c++){ 
int density=rander(0,dense);//high dense value =low density
if(density==0){
dot[c].state=rander(1,numStates-1);
}else{dot[c].state=0;}
dot[c].stateTemp=dot[c].state;
}
}//end set State




//set rule values; tweak random ranges for various effects
void setRuleValues(){
stepCount=0;
setState();

for(int i=0;i<numStates;i++){
for(int n=0;n<numStates;n++){
//birth count: number of neighbors of a particular state required for cell to be born into newState; min value must be 1, or else get siezure inducing flashing; max value must be higher than 8, or else cells grow too fast; if max is too high, cells do not live long enough for forms to emerge;
//setting the deathNeighsMax value above the value which could actually occur works to limit the number of cells whose birth rules evaluate to true
birthCount[i][n]=rander(1,birthNeighsMax);
//death count: number of neighbors of a particular state required for a cell to change to state 0 (die);
//setting the deathNeighsMax value above the value which could actually occur works to limit the number of cells whose death rules evaluate to true
deathCount[i][n]=rander(0,deathNeighsMax);

//new state is one of 16 values between 1 and 16 inclusive
newState[i][n]=rander(1,numStates-1); //new state


}//end n loop
}//end i loop

countLargeRectBoolsTicker=0;
countSameTicker=0;
}//end set rule values

//neighbor state
void neighborState(){
stepCount++;
totalDead=0;
for(int i=1;i<numStates;i++){stateCounter[i]=0;}

for(int c=0;c<numDots;c++){

if(checkDensity){
largeRectBool[dot[c].largeRectId]=0;
largeRectBoolCounter[dot[c].largeRectId]=0;
}

if(dot[c].state==0){totalDead++;}//count total dead cells for auto restart
int nScount[numStates]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
for(int n=0;n<8;n++){
nScount[dot[ dot[c].near8[n] ].state]++; //neighbor state count
}

//interacting state rules

//birth
//number of matches required for cell to switch from 0 to new state
int bCount=0;

if(dot[c].state==0){
for(int i=1;i<numStates;i++){ 
for(int n=1;n<numStates;n++){

if(nScount[i]==birthCount[i][n]){
bCount++;
if(bCount>bCondsMatch){ 
dot[c].stateTemp=newState[i][n]; //set cell to new state
stateCounter[newState[i][n]]++; //for counting # cells in each state
goto death; }}

}//end n loop
}//end i loop
}//end if state == 0

death:

/*//example 4 state data:

    1,2,3,4
i: [3,7,8,1]
i: [0,4,3,6]
i: [0,2,7,1]
i: [0,2,0,5]
 
if nScount == 3 or 0 or 0 or 0
*/

//death

if(dot[c].state>0){

//dot[c].stateTemp=0;}
//although we might init dCondsMatch and deathNeighsMax to values which result in non-zero cells always changing to zero in the next step, in order to ensure we see the expected results when loading saved rule files, along with values for the above variables, as well as density, we leave the loop checks in place and let the logic kill the cell

int dCount=0; 
for(int i=1;i<numStates;i++){
for(int n=1;n<numStates;n++){
//if(dot[c].state==n){ //for painting, because cells will persist in alive states
if(nScount[i]>=deathCount[i][n]){ 
dCount++;
//try different comparison operators, for different results, but saved rules will not show the same effects which caused you to save them; could add variables to the saved rule files called deathOperator and birthOperator, then use a conditional here to choose which operator to use for a particular rule, and display the operator in use in the important variables; my saved rules for now are using the >= than operator for death, and > for birth
if(dCount>=dCondsMatch){dot[c].stateTemp=0; goto nextDot;}
}//end nScount>=
//}//if state==n
}//end n loop
}//end i loop
}//end if state > 0

nextDot:;

//uncommenting this line results in all cells dying at each step, so only birth rules apply, which is good for clearing the space for the unimpeded evolution of moving 'forms'
//if(dot[c].state>0&&dot[c].stateTemp>0){cout<<dot[c].id; pauser=true;}


}//all dots loop

int countStates=0;
for(int i=1;i<numStates;i++){
if(stateCounter[i]>0){countStates++;}
}


if(autoRestart&&(
(stepCount>10&& //let it get started
totalDead<numDots*.75) //too dense
||totalDead>=numDots-50 //almost empty
||countStates<4)){ //not enough variety

//start again without saving
setRuleValues();

numRuleAborts++; 

}else if(autoRestart&&stepCount>230){
numRuleSaves++;
saveRule(); ruleFnis=ruleFni; readRuleFilenames(); ruleFni=ruleFnis;

//start again after saving
setRuleValues();

}//end if auto restart


//save state
for(int c=0;c<numDots;c++){dot[c].state=dot[c].stateTemp;}
}//end neighbor State

int main(int argc, char* args[]){


srand( (unsigned int)time(0));//necessary for randomizer

readXYStateFilenames();
readStateFilenames();
readRuleFilenames();

SDL_Init(SDL_INIT_VIDEO);
SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ); //SDL_WINDOWPOS_CENTERED
gWindow = SDL_CreateWindow("interacting states", SDL_WINDOWPOS_CENTERED , 30, SCREEN_WIDTH, SCREEN_HEIGHT, 
//SDL_WINDOW_SHOWN );
SDL_WINDOW_FULLSCREEN_DESKTOP); 

gRenderer = SDL_CreateRenderer(gWindow, -1, 
//0 );
//SDL_RENDERER_PRESENTVSYNC | //not supported
SDL_RENDERER_SOFTWARE );
//SDL_RENDERER_ACCELERATED );
//SDL_RENDERER_SOFTWARE |SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
SDL_SetRenderDrawBlendMode(gRenderer,SDL_BLENDMODE_BLEND);
SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);


TTF_Init();
gFont=TTF_OpenFont("Consolas.ttf", 28);

SDL_Event e;

setId();
for(int c=0;c<numDots;c++){dot[c].nearN();}

setRuleValues();


//set initial values for vector of booleans for larger square regions overlaying cells 
bool firstRow=1;
int x=0,y=0,id=0,rx=0,firstId=id;
for(int c=0;c<numDots;c++){

if(rx<largeRectSize){ rx+=cellSize; 
}else if(rx==largeRectSize){ //only do this once per large rectangle
id++; rx=0; 

if(firstRow){
largeRectBoolCounter.push_back(0);
largeRectBool.push_back(false);
largeRectBoolS.push_back(false);
} //end if first row

}//reached right side of large rectangle

if(x<=lastHDot){x+=cellSize; //same row
}else{ x=0; y+=cellSize;  id=firstId; firstRow=0;} //start new row, id goes back to first row id: example: if cellSize==6, then lastId will be:  0, 10, 20...

if(y>largeRectSize){  y=0;
firstId+=floor(SCREEN_WIDTH/largeRectSize); //example: 1280/120=10.667= floor = 10
id=firstId; firstRow=1;
}

dot[c].largeRectId=id;//all dots in same large rectangle have same id
}//end num dots loop

numLargeRects=largeRectBool.size();


//end set large rect vector of booleans

/*
if(startingRule>0){
loadRule(startingRule);//non random 
}else{
ruleFni=rander(0, ruleFn.size()-1);
loadRule(ruleFni); 
}
*/

//running loop
while(running){

if(dragging){
renderShiftX=renderShiftXLast+(mouseX-mouseClickX)/(cellSize*rendScale);
renderShiftY=renderShiftYLast+(mouseY-mouseClickY)/(cellSize*rendScale);
}


//SDL_WaitEvent(&e);
while(SDL_PollEvent(&e)){

if(e.type==SDL_QUIT){running=false;}//x click exit


if(e.type == SDL_MOUSEWHEEL){

if(e.wheel.y < 0){ //zoom out 
if(rendScale>1){

renderShiftX+=((mouseX-SCREEN_WIDTH/2)+(SCREEN_WIDTH/2))/(cellSize*(rendScale*(rendScale-1)));
renderShiftY+=((mouseY-SCREEN_HEIGHT/2)+(SCREEN_HEIGHT/2))/(cellSize*(rendScale*(rendScale-1)));
rendScale--;

}
}else if(e.wheel.y>0){ //zoom in
rendScale++;
renderShiftX-=((mouseX-SCREEN_WIDTH/2)+(SCREEN_WIDTH/2))/(cellSize*(rendScale*(rendScale-1)));
renderShiftY-=((mouseY-SCREEN_HEIGHT/2)+(SCREEN_HEIGHT/2))/(cellSize*(rendScale*(rendScale-1)));
}

renderShiftXLast=renderShiftX;
renderShiftYLast=renderShiftY;
SDL_RenderSetScale(gRenderer,rendScale,rendScale);

}//end wheel



if(e.type == SDL_MOUSEBUTTONDOWN){

if(e.button.button == SDL_BUTTON_RIGHT){
renderShiftX=renderShiftXLast=0;
renderShiftY=renderShiftYLast=0;
rendScale=1;
SDL_RenderSetScale(gRenderer,rendScale,rendScale);
}


if(e.button.button == SDL_BUTTON_LEFT){
if(drawMode){
mouseRect.x=mouseX;
mouseRect.y=mouseY;
for(int k=0;k<numDots;k++){
if(SDL_HasIntersection(&dot[k].fillRect, &mouseRect)){ dot[k].state=drawState;}
}

}else{
mouseClickX=mouseX;
mouseClickY=mouseY;
dragging=true;
}
}} //end button down

if(e.type == SDL_MOUSEBUTTONUP){
if(e.button.button == SDL_BUTTON_LEFT){
renderShiftXLast=renderShiftX;
renderShiftYLast=renderShiftY;
dragging=false;
}}//end button up

if(e.type == SDL_KEYUP){ 

switch(e.key.keysym.sym){
case SDLK_ESCAPE: running=0; break; //escape exit
case SDLK_z: altKey=0; break;

case SDLK_k:
if(altKey){ruleFni-=10;}else{ruleFni+=10;}
if((unsigned int)ruleFni>ruleFn.size()-1){ruleFni=0;}
else if((unsigned int)ruleFni<0){ruleFni=ruleFn.size()-1;}
break;


case SDLK_l: 
autoRestart=!autoRestart; 

numRuleAborts=0;
numRuleSaves=0;
totalDead=0;
stepCount=0; 
countLargeRectBoolsTicker=0;
countSameTicker=0;
break;

case SDLK_m: SDL_SetWindowFullscreen(gWindow,SDL_WINDOW_FULLSCREEN_DESKTOP); break;
case SDLK_n: SDL_SetWindowFullscreen(gWindow,SDL_WINDOW_SHOWN); break;
case SDLK_b: saveScreenshotPNG();break;
case SDLK_r: setRuleValues(); break;
case SDLK_s: setState(); break;
case SDLK_p: pauser=!pauser; if(!pauser){drawMode=false;}break;
case SDLK_c:
if(altKey){deleteRule();}//else{deleteAllDupRuleFiles();}
break;
case SDLK_t: 
setRuleValues(); 
stepCount=0; 
countLargeRectBoolsTicker=0;
countSameTicker=0;

break;
case SDLK_v: if(bRuleExists()){cout<<"rule exists";}else{cout<<"rule is new";}break;
case SDLK_e: displayHelp=!displayHelp; break;
case SDLK_w: 
if(altKey){ //set to starting Rule
ruleFni=startingRule=ruleFn.size()-1;
loadRule(startingRule);setState();
}else{
ruleFni=rander(0, ruleFn.size()-1);
loadRule(ruleFni);setState(); 
}

break;
case SDLK_x: break;
case SDLK_a: stepMode=!stepMode; pauser=stepMode; break;
case SDLK_q: if(stepMode){stepOnce=1;} break;
case SDLK_f: 
saveRule(); ruleFnis=ruleFni; readRuleFilenames(); ruleFni=ruleFnis;
 break;
case SDLK_g: 
ruleFni=ruleFn.size()-1;loadRule(ruleFni); setState(); 
break;
case SDLK_h: 
ruleFni++; if((unsigned int)ruleFni>ruleFn.size()-1){ruleFni=0;}
loadRule(ruleFni); setState(); 
break;
case SDLK_SPACE: renderShiftX=0; renderShiftY=0;break;
case SDLK_SEMICOLON: showGrid=!showGrid; break;
case SDLK_QUOTE:if(altKey){dense+=1;}else{dense-=1;}break;
case SDLK_RETURN:break;
case SDLK_COMMA:
loadRule(interesting[interest]); setState();
ruleFni=interesting[interest];
interest++; if(interest>sizeof(interesting)/sizeof(interesting[0])-1){interest=0;}
break;
case SDLK_PERIOD:randomColorFunction();useRandColor=true;break;
case SDLK_j: 
if((unsigned int)ruleFni>0){ruleFni--;}else{ruleFni=ruleFn.size()-1;}
loadRule(ruleFni); setState(); 
break;

case SDLK_y:if(altKey){
if(birthNeighsMax<2){birthNeighsMax=1;}else{birthNeighsMax--;}
}else{birthNeighsMax++;} 
break;

case SDLK_u:
if(altKey){
if(deathNeighsMax<1){deathNeighsMax=0;}else{deathNeighsMax--;}
}else{deathNeighsMax++;}
break;

case SDLK_i:
if(altKey){
if(bCondsMatch<1){bCondsMatch=0;}else{bCondsMatch--;}
}else{bCondsMatch++;}
break;

case SDLK_o:
if(altKey){
if(dCondsMatch<1){dCondsMatch=0;}else{dCondsMatch--;}
}else{dCondsMatch++;}
break;

case SDLK_d: displayVars=!displayVars;break;

case SDLK_0: XYStateFni--; if(XYStateFni<0){XYStateFni=XYStateFn.size()-1;}
break; 
case SDLK_1: XYStateFni++; if(XYStateFni>XYStateFn.size()-1){XYStateFni=0;}
break;
case SDLK_2: if(altKey){drawState--;}else{drawState++;}
if(drawState<0){drawState=numStates-1;}
if(drawState>=numStates){drawState=0;}
 break;
case SDLK_3: 
drawMode=!drawMode;
if(drawMode){pauser=true;}else{pauser=false;}
break;
case SDLK_4: //saveState(); readStateFilenames(); 
saveXYState(); readXYStateFilenames();
break;
case SDLK_5: 
stepCount=0;
for(int c=0;c<numDots;c++){
dot[c].state=0;
dot[c].stateTemp=dot[c].state;
}
loadState(stateFni); 
pauser=true;
break;
case SDLK_6: 

stepCount=0;
for(int c=0;c<numDots;c++){
dot[c].state=0;
dot[c].stateTemp=dot[c].state;
}

shiftX=-40;
shiftY=10;

for(int p=0;p<30;p++){
for(int c=0;c<numDots;c++){
loadXYState(XYStateFni, c, shiftX, shiftY);
}
shiftX+=8;
if(p==9){shiftX=-40; shiftY-=15;}
if(p==19){shiftX=-40; shiftY-=15;}
}//end num repeats loop

pauser=true;
break;
case SDLK_7: 
delayer--;
if(delayer<0){delayer=0;}
break;
case SDLK_8: 
delayer++;
break;
case SDLK_9: 
stepCount=0;
for(int c=0;c<numDots;c++){
dot[c].state=0;
dot[c].stateTemp=dot[c].state;
}
break;
}//end switch keys

}else if(e.type == SDL_KEYDOWN){ 
switch( e.key.keysym.sym ){
case SDLK_ESCAPE: running=0; break; //escape exit
case SDLK_z: altKey=1; break;
}//end switch keys

}//end key down

}//end poll events




//if want calculations to continue, move !pauser after neighborState
if(!pauser||stepOnce){ 

if(stepMode&&stepOnce){stepOnce=0;}


if(autoRestart){
if(checkDensityTicker<15){ checkDensityTicker++;
}else{checkDensity=1; checkDensityTicker=0; }
}

neighborState();

}//end pauser


//Clear screen
//SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
SDL_RenderClear(gRenderer);
SDL_GetMouseState(&mouseX, &mouseY);

//draw dots
int x=0,y=0, minDots=2;
for(int c=0;c<numDots;c++){
if(dot[c].state>0){ 
dot[c].drawDot(x,y);

if(checkDensity){ //count large rectangles which contain minDots of dots with any state>0, if cellSize=6, number of large rects will be 60

largeRectBoolCounter[dot[c].largeRectId]++;
if(largeRectBoolCounter[dot[c].largeRectId]>minDots){
largeRectBool[dot[c].largeRectId]=1;}

}//end check density

}
if(x<lastHDot){x+=cellSize;}else{x=0;y+=cellSize;}
}//end num dots loop




//Draw grid horizontals
SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
if(showGrid){
lineXStart=0,lineYStart=0,lineXEnd=SCREEN_WIDTH,lineYEnd=0;
for(int c=0;c<SCREEN_HEIGHT/cellSize;c++){
SDL_RenderDrawLine(gRenderer, lineXStart, lineYStart, lineXEnd, lineYEnd);
lineYStart+=cellSize;
lineYEnd+=cellSize;
}

//draw grid verticals
lineXStart=0,lineYStart=0,lineXEnd=0,lineYEnd=SCREEN_HEIGHT;
for(int c=0;c<SCREEN_WIDTH/cellSize;c++){
SDL_RenderDrawLine(gRenderer, lineXStart, lineYStart, lineXEnd, lineYEnd);
lineXStart+=cellSize;
lineXEnd+=cellSize;
}
}//end if show Grid




//render text over window for displaying important variables

if(displayVars){
//rectangle background to text
textBackRect.x=(SCREEN_WIDTH/2-250); 
textBackRect.y=(SCREEN_HEIGHT/2-300);
textBackRect.w=450; 
textBackRect.h=620;
SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 210);		
SDL_RenderFillRect(gRenderer, &textBackRect);

int textX=SCREEN_WIDTH/2-230;

string tempString="";
int it=textBackRect.y;

//white text itself
//gTextTexture.loadFromRenderedText( 
//"stepCount: "+to_string((long long)stepCount), textColor);
//gTextTexture.renderText( textX, it+20);

//gTextTexture.loadFromRenderedText( 
//"heighter: "+to_string((long long)heighter)+" "+to_string((long long)widther), textColor);
//gTextTexture.renderText( textX, it+20);

gTextTexture.loadFromRenderedText( 
"delayer: "+to_string((long long)delayer), textColor); //delayer
gTextTexture.renderText( textX, it+20);

gTextTexture.loadFromRenderedText( 
"birthNeighsMax:"+to_string((long long)birthNeighsMax), textColor);
gTextTexture.renderText( textX, it+50);


gTextTexture.loadFromRenderedText( 
"deathNeighsMax:"+to_string((long long)deathNeighsMax), textColor);
gTextTexture.renderText( textX, it+80);

gTextTexture.loadFromRenderedText( 
"bCondsMatch:"+to_string((long long)bCondsMatch), textColor);
gTextTexture.renderText( textX, it+110);

gTextTexture.loadFromRenderedText( 
"dCondsMatch:"+to_string((long long)dCondsMatch), textColor);
gTextTexture.renderText( textX, it+140);


gTextTexture.loadFromRenderedText( 
"dense:"+to_string((long long)dense), textColor);
gTextTexture.renderText( textX, it+170);

gTextTexture.loadFromRenderedText( 
"countLargeRectBoolsTicker:"+to_string((long long)countLargeRectBoolsTicker)+tempString, textColor);
gTextTexture.renderText( textX, it+210);

gTextTexture.loadFromRenderedText( 
"countSameTicker:"+to_string((long long)countSameTicker)+tempString, textColor);
gTextTexture.renderText( textX, it+240);

gTextTexture.loadFromRenderedText( 
"countLargeRectBools:"+to_string((long long)countLargeRectBools)+tempString, textColor);
gTextTexture.renderText( textX, it+270);

gTextTexture.loadFromRenderedText( 
"countSame:"+to_string((long long)countSame)+tempString, textColor);
gTextTexture.renderText( textX, it+300);

gTextTexture.loadFromRenderedText( 
"autoRestart:"+to_string((long long)autoRestart), textColor);
gTextTexture.renderText( textX, it+340);

gTextTexture.loadFromRenderedText( 
"filename:"+ruleFn[ruleFni], textColor);
gTextTexture.renderText( textX, it+370);

gTextTexture.loadFromRenderedText( 
"totalDead:"+to_string((long long)totalDead), textColor);
gTextTexture.renderText( textX, it+400);

gTextTexture.loadFromRenderedText( 
"largeRectBool.size():"+to_string((long long)largeRectBool.size()), textColor);
gTextTexture.renderText( textX, it+430);

gTextTexture.loadFromRenderedText( 
"numRuleSaves:"+to_string((long long)numRuleSaves), textColor);
gTextTexture.renderText( textX, it+460);

gTextTexture.loadFromRenderedText( 
"numRuleAborts:"+to_string((long long)numRuleAborts), textColor);
gTextTexture.renderText( textX, it+490);

gTextTexture.loadFromRenderedText( 
"ruleFn.size():"+to_string((long long)ruleFn.size()), textColor);
gTextTexture.renderText( textX, it+520);

gTextTexture.loadFromRenderedText( 
"ruleFni:"+to_string((long long)ruleFni), textColor);
gTextTexture.renderText( textX, it+550);

//gTextTexture.loadFromRenderedText( 
//"filename:"+ruleFn[ruleFn.size()-1], textColor);
//gTextTexture.renderText( textX, it+580);

gTextTexture.loadFromRenderedText( 
"stepCount:"+to_string((long long)stepCount), textColor);
gTextTexture.renderText( textX, it+580);


SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);

}//end if display vars

else if(displayHelp){

displayHelpText();


}//end if display help


if(ruleSaved){
//rectangle background to text

textBackRect.x=SCREEN_WIDTH-200; 
textBackRect.y=0; 
textBackRect.w=200; 
textBackRect.h=45;
SDL_SetRenderDrawColor(gRenderer, 100, 100, 100, 255);		
SDL_RenderFillRect(gRenderer, &textBackRect);


string sMess;
sMess="Rule Saved";
SDL_Color messText={255,255,255};
gTextTexture.loadFromRenderedText(sMess, messText);
gTextTexture.renderText(SCREEN_WIDTH-175, 10);

SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);	

ruleSavedTick++; if(ruleSavedTick>10){ruleSaved=0;ruleSavedTick=0;}
}//end if rule saved

SDL_RenderPresent(gRenderer);

if(checkDensity){checkDensityFunction();}
SDL_Delay(delayer);
}//end running

gTextTexture.free();
TTF_CloseFont(gFont);
gFont = NULL;
SDL_DestroyRenderer(gRenderer);
SDL_DestroyWindow(gWindow);
gWindow = NULL;
gRenderer = NULL;
TTF_Quit();
//IMG_Quit();
SDL_Quit();
largeRectBool.empty();
largeRectBoolCounter.empty();

return 0;
}//end main


//get nearest neighbor id's
void dots::nearN(){

near8[0]=id-SCREEN_WIDTH/cellSize-1;
near8[1]=id-SCREEN_WIDTH/cellSize;
near8[2]=id-SCREEN_WIDTH/cellSize+1;

near8[3]=id+1;

near8[4]=id+SCREEN_WIDTH/cellSize+1;
near8[5]=id+SCREEN_WIDTH/cellSize;
near8[6]=id+SCREEN_WIDTH/cellSize-1;

near8[7]=id-1;

//edge exception overrides

//left edge
if(id%(SCREEN_WIDTH/cellSize)==0){
near8[0]=id-1;
near8[7]=id+SCREEN_WIDTH/cellSize-1;
near8[6]=id+SCREEN_WIDTH/cellSize*2-1;
}

//right edge
if((id+1)%(SCREEN_WIDTH/cellSize)==0){
near8[2]=id-SCREEN_WIDTH/cellSize*2+1;
near8[3]=id-SCREEN_WIDTH/cellSize+1;
near8[4]=id+1;
}

//top edge
if(id<SCREEN_WIDTH/cellSize){
near8[0]=id+(numDots-SCREEN_WIDTH/cellSize)-1;
near8[1]=id+(numDots-SCREEN_WIDTH/cellSize);
near8[2]=id+(numDots-SCREEN_WIDTH/cellSize)+1;
}

//bottom edge
if(id>=numDots-SCREEN_WIDTH/cellSize){
near8[6]=id-(numDots-SCREEN_WIDTH/cellSize)-1;
near8[5]=id-(numDots-SCREEN_WIDTH/cellSize);
near8[4]=id-(numDots-SCREEN_WIDTH/cellSize)+1;
}

//corner exception overrides

//top left corner
if(id==0){
near8[0]=numDots-1;
}

//top right corner
if(id==SCREEN_WIDTH/cellSize-1){
near8[2]=numDots-SCREEN_WIDTH/cellSize;
}

//bottom left corner
if(id==numDots-SCREEN_WIDTH/cellSize){
near8[6]=SCREEN_WIDTH/cellSize-1;
}

//bottom right corner
if(id==numDots-1){
near8[4]=0;
}

}//end nearN

//save x, y, state to file with time as filename
void saveXYState(){
ofstream f;
string filen="/var/www/html/states/savedXYState/"+to_string(time(NULL));
string filex=".txt";
filen+=filex;
f.open(filen);
for(int i=0;i<numDots;i++){
if(dot[i].state>0){
f << dot[i].fillRect.x <<" "<< dot[i].fillRect.y <<" "<< dot[i].state <<"\n";
} 
}//end numDots loop
f.close();
}//end save x,y, state function

//save state to file with time as filename
void saveState(){
ofstream f;
string filen="/var/www/html/states/savedState/"+to_string(time(NULL));
string filex=".txt";
filen+=filex;
f.open(filen);
for(int i=0;i<numDots;i++){
if(dot[i].state>0){
f << dot[i].id <<" "<< dot[i].state <<"\n";
} 
}//end numDots loop
f.close();
}//end save state function


//save rule to file with time as filename
void saveRule(){
if(!bRuleExists()){

ofstream f;

string filen="/var/www/html/states/savedRules/"+to_string(time(NULL));
string filex=".txt";
filen+=filex;
f.open(filen);

for(int i=0;i<17;i++){for(int n=0;n<17;n++){
f << birthCount[i][n] <<" ";}} 

f << "\n";

for(int i=0;i<17;i++){for(int n=0;n<17;n++){
f << deathCount[i][n] <<" ";}} 

f << "\n";

for(int i=0;i<17;i++){for(int n=0;n<17;n++){
f << newState[i][n] <<" ";}}

f << "\n";

f << birthNeighsMax<< "\n";
f << deathNeighsMax<< "\n";
f << bCondsMatch<< "\n";
f << dCondsMatch<< "\n";

f << dense;

f.close();
ruleSaved=1;
//cout<<"Rule "<<ruleFn.size()-1<<" saved."<<endl;
}//end if ! rule exists
}//end save rule function



//read rule filenames
void readRuleFilenames(){

ruleFn.clear();
//read all filenames from savedRule directory
DIR *dir;
struct dirent *ent;
dir = opendir ("/var/www/html/states/savedRules/");
int i=0;//for passing over dot, dot dot
while ((ent = readdir (dir)) != NULL){ i++; if(i>=3){
//cout<< ent->d_name <<endl;
ruleFn.push_back(ent->d_name);
sort( ruleFn.begin(), ruleFn.end() );
}}
closedir(dir);
ruleFni=ruleFn.size()-1;

//for(vector<string>::iterator it = ruleFn.begin(); it != ruleFn.end(); ++it) {
//cout << *it <<endl;
//cout<<ruleFn[ruleFn.size()-1]<<endl;

//cout<<ruleFn.size()<<endl;
}//end read rule filenames

//read x, y, state filenames
void readXYStateFilenames(){
XYStateFn.clear();
DIR *dir;
struct dirent *ent;
dir = opendir("/var/www/html/states/savedXYState/");
while ((ent = readdir (dir)) != NULL){ 
if(ent->d_name[0]!='.'&&ent->d_name[0]!='..'){
XYStateFn.push_back(ent->d_name);
sort( XYStateFn.begin(), XYStateFn.end() );
}}
closedir(dir);
XYStateFni=XYStateFn.size()-1;
//cout<<XYStateFni;
//cin.get();
}//end read x, y,  state  filenames


//read state filenames
void readStateFilenames(){
stateFn.clear();
DIR *dir;
struct dirent *ent;
dir = opendir("/var/www/html/states/savedState/");
while ((ent = readdir (dir)) != NULL){ 
if(ent->d_name[0]!='.'&&ent->d_name[0]!='..'){
stateFn.push_back(ent->d_name);
sort( stateFn.begin(), stateFn.end() );
}}
closedir(dir);
stateFni=stateFn.size()-1;
//cout<<stateFni;
//cin.get();
}//end read state filenames


//open a specific saved rule file using ruleFni as offset to vector and transfer values from text file to three arrays, birthCount, deathCount, and newState
void loadRule(int ruleFni){

ifstream infile("/var/www/html/states/savedRules/"+ruleFn[ruleFni]);

for(int i=0;i<numStates;i++){
for(int n=0;n<numStates;n++){
infile >> birthCount[i][n]; 
}}

for(int i=0;i<numStates;i++){
for(int n=0;n<numStates;n++){
infile >> deathCount[i][n]; 
}}

for(int i=0;i<numStates;i++){
for(int n=0;n<numStates;n++){
infile >> newState[i][n]; 
}}

infile >> birthNeighsMax;
infile >> deathNeighsMax;
infile >> bCondsMatch;
infile >> dCondsMatch;

infile >> dense;

}//end load rule


//load x,y, state function
void loadXYState(int stateFni, int i, int shiftX, int shiftY){
ifstream infile("/var/www/html/states/savedXYState/"+XYStateFn[XYStateFni]);
int x, y, s;
int shifterX=cellSize*shiftX;
int shifterY=cellSize*shiftY;
while(infile >> x >> y >> s){
if(dot[i].fillRect.x+shifterX==x&&dot[i].fillRect.y+shifterY==y){dot[i].state=s;}
}//end while
}//end load state


//load state function
void loadState(int stateFni){
ifstream infile("/var/www/html/states/savedState/"+stateFn[stateFni]);
int a, b;
while(infile >> a >> b){
dot[a].state=b;
}
}//end load state



//delete current rule file
void deleteRule(){
char fileNameString[100];
strcpy(fileNameString, "/var/www/html/states/savedRules/"); 
strcat(fileNameString, ruleFn[ruleFni].c_str());
remove(fileNameString); 
cout<<"Deleted rule "<<ruleFni<<" -- "<<fileNameString<<endl;
ruleFnis=ruleFni-1;
readRuleFilenames();
ruleFni=ruleFnis;
loadRule(ruleFnis);setState();
}//end delete rule


//check if current rule already exists before saving, so do not duplicate
bool bRuleExists(){

vector<vector<int>> data(ruleFn.size(), vector<int>(4)); 
bool bDup=true;

for(unsigned int n=0;n<ruleFn.size();n++){
ifstream infile("/var/www/html/states/savedRules/"+ruleFn[n]);
for(int k=0;k<4;k++){infile>>data[n][k];
//cout<<data[n][k]<<" ";
}
infile.close(); 
}

for(unsigned int n=0;n<ruleFn.size();n++){
bDup=true;
for(int i=0;i<4;i++){
if(data[n][i]!=birthCount[0][i]){bDup=false;}
}

if(bDup){
//cout<<"rule exists"<<endl;
return bDup;}
}//end n loop

//cout<<"rule saved"<<endl;
//cout<<"birthCount:";
//cout<<birthCount[0][0]<<" "<<birthCount[0][1]<<" "<<birthCount[0][2]<<" "<<birthCount[0][3]<<endl;
//cout<<"data:";
//cout<<data[ruleFn.size()-1][0]<<" "<<data[ruleFn.size()-1][1]<<" "<<data[ruleFn.size()-1][2]<<" "<<data[ruleFn.size()-1][3]<<endl;
return false;
}//end check if rule exists


//delete all duplicate rule files
void deleteAllDupRuleFiles(){

vector<vector<int>> data(ruleFn.size(), vector<int>(4)); 
bool bDelete;
int d=0;

for(unsigned int n=0;n<ruleFn.size()-1;n++){
ifstream infile("/var/www/html/states/savedRules/"+ruleFn[n]);
for(int k=0;k<4;k++){infile>>data[n][k];}
infile.close();
}

for(unsigned int n=0;n<ruleFn.size()-2;n++){
for(unsigned int j=n+1;j<ruleFn.size()-1;j++){

bDelete=true;
for(int i=0;i<4;i++){
if(data[n][i]!=data[j][i]){bDelete=false;}
}

if(bDelete){
char fileNameString[100];
strcpy(fileNameString, "/var/www/html/states/savedRules/"); 
strcat(fileNameString, ruleFn[j].c_str());
remove(fileNameString); 
cout<<"deleted rule "<<n<<" -- "<<fileNameString<<endl;
d++;
}//end if b delete
}//end j loop
}//end n loop

cout<<d<<" total deleted"<<endl;

readRuleFilenames(); //to update
}//end delete all dup rule files



//save png screenshot; png file is ~20kb vs 1.8MB for bmp
void saveScreenshotPNG(){

//count files in screenshot dir and use count as screenshot filename
vector<string>imNames;
DIR *dir;
struct dirent *ent;
dir = opendir ("/var/www/html/p/s/");
int i=0;//for passing over dot, dot dot
while ((ent = readdir (dir)) != NULL){ i++; if(i>=3){
imNames.push_back(ent->d_name);
}}
closedir(dir);
//cout<<endl <<imNames.size()<<endl;

//build image path and file name as const char*
string imName="/var/www/html/p/s/0";
imName+=to_string((long long)(imNames.size()));
imName+=".png";

//might need to use this if using hardware rendering
//SDL_Surface *sshot = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
//SDL_RenderReadPixels(gRenderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
//SDL_PIXELFORMAT_ARGB8888
//IMG_SavePNG(imName.c_str(), sshot);
//SDL_FreeSurface(sshot);

SDL_Surface *gScreen = SDL_GetWindowSurface(gWindow);
IMG_SavePNG(gScreen, imName.c_str());
SDL_FreeSurface(gScreen);
}//end save png screenshot


//for checking density
void checkDensityFunction(){
countLargeRectBools=0,itter=0,countSame=0;

for(vector<bool>::iterator it=largeRectBool.begin(); it!=largeRectBool.end(); ++it){
if(largeRectBool[itter]){countLargeRectBools++;}

//check if same large rectangles always contain activity over time
if(largeRectBoolS[itter]==largeRectBool[itter]){countSame++;}
largeRectBoolS[itter]=largeRectBool[itter];
itter++;}


if(autoRestart){

//values based on cellSize=6, numLargeRects=60, SCREEN_WIDTH=1280
//tweak these values for various cell sizes, num dots, screen resolutions, want to filter out cases of too many dots in every rectangle
int rectThresh=57;
int sameThresh=numLargeRects-1;

//if clump density is too low or too high
if(countLargeRectBools<2 //too many large rectangles with hardly any dots
||
countLargeRectBools>rectThresh){ //to many large rectangles with non-zero dots

countLargeRectBoolsTicker++;
if(countLargeRectBoolsTicker>3){

setRuleValues();

numRuleAborts++; }
}else{countLargeRectBoolsTicker=0;}

//if in a holding pattern, as of flashers, usually
if(countSame>sameThresh){ countSameTicker++;
if(countSameTicker>3){

setRuleValues();

numRuleAborts++; }
}else{countSameTicker=0;}

}//end if auto restart

checkDensity=0;
}//end check density function


string helpTextArray[27]={
"9: set random colors",
"W: select random rule from saved rules ",
"R: set new rule ",
"S: randomize states",
"F: save rule",
"G: load last saved rule from file",
"H: advance forward through all saved rules",
"J: go backward through all saved rules",
"K: increment by 10 ruleFni (offset to saved rule vector) without loading rule",
"Z+K: decrement ruleFni by 10, without loading rule",
"number keys to set initial randomized density, where higher number keys result in lower density",
"P: pause/resume",
"B: save screenshot as png to file",
"M: fullscreen",
"N: windowed mode",
"Y: increase birthNeighsMax; Z+Y to decrease",
"U: increase deathNeighsMax; Z+U to decrease",
"I: increase bCondsMatch; Z+I to decrease",
"O: increase dCondsMatch; Z+O to decrease",
"L: toggling auto restart mode for automatically setting new rule, conditionally",
"A: toggle stepMode",
"Q: step once",
//"Z+C: delete all duplicate rule files",
"Z+C: delete current rule file",
"D: toggle display of important variables",
"V: check if current rule is new or is saved already",
"E: display key shortcuts help",
"Left Mouse Button and Wheel for panning and zooming"
};

void displayHelpText(){
int lengther=sizeof(helpTextArray)/sizeof(*helpTextArray);

textBackRect.x=0; 
textBackRect.y=0;
textBackRect.w=SCREEN_WIDTH; 
textBackRect.h=SCREEN_HEIGHT;
SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 220);		
SDL_RenderFillRect(gRenderer, &textBackRect);

int ypos=0;
int it=textBackRect.y;
int textX=30;
for(int i=0;i<lengther;i++){
gTextTexture.loadFromRenderedText( helpTextArray[i], textColor);
gTextTexture.renderText( textX, it+ypos);
ypos+=28;
}

}//end display help text


//set random colors
void randomColorFunction(){

for(int i=0;i<numStates;i++){

randColor[i][0] = rander(0,255);  // red
randColor[i][1] = rander(0,255);  // green
randColor[i][2] = rander(0,255);  // blue

// find max and min indexes.
int max, min;

if(randColor[i][0] > randColor[i][1]){
max = (randColor[i][0] > randColor[i][2]) ? 0 : 2;
min = (randColor[i][1] < randColor[i][2]) ? 1 : 2;

}else{

max=(randColor[i][1] > randColor[i][2]) ? 1 : 2;
int notmax = 1 + max % 2;
min = (randColor[i][0] < randColor[i][notmax]) ? 0 : notmax;
}
randColor[i][max]=255;
randColor[i][min]=0;
}//end numstates loop
}//end random color function

