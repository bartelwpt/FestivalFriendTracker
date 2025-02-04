#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ApplicationController.h>
#include <List.hpp>
#include <State.h>
#include <StateMachine.h>
#include <Renderer.h>

#define OLED_RESET      -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define JOYSTICK_MID    16
#define JOYSTICK_RIGHT  4
#define JOYSTICK_LEFT   0
#define JOYSTICK_DOWN   2
#define JOYSTICK_UP     15

/* TASK DECLARATION */
void AppTask( void *pvParameters );


/* Global Object Declaration */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
JoystickController jController;
StateMachine stateMachine;
ApplicationController appController(&display, &jController, &stateMachine);

List<State*> stateList;
State menuState(stateMachine.nextId(), "Menu", &menuTest);
State buttonTestState(stateMachine.nextId(), "Button Test", &buttonTest);
State networkTestState(stateMachine.nextId(), "Network Test", &networkTest);

MenuItem btnTestMenuItem("Button-Test", &btnTestMenuItemCallback);
MenuItem nwTestMenuItem("Network-Test", &nwTestMenuItemCallback);
List<MenuItem*> menuItemList;

Menu *menu;

/* CALLBACK FUNCTIONS FOR Joystick Interrutps */
void IRAM_ATTR onJoystickMid() {
  jController.setMid(true);
}
void IRAM_ATTR onJoystickLeft() {
  jController.setLeft(true);
}
void IRAM_ATTR onJoystickRight() {
  jController.setRight(true);
}
void IRAM_ATTR onJoystickUp() {
  jController.setUp(true);
}
void IRAM_ATTR onJoystickDown() {
  jController.setDown(true);
}


/* INIT FUNCTIONS FOR GLOBAL OBJECTS */
void initMenu() {
  Serial.println("init menu");
  menuItemList.add(&btnTestMenuItem);
  menuItemList.add(&nwTestMenuItem);
  menu = new Menu(&display, &jController, &menuItemList);
}

void initStateMachine() {
  Serial.println("init StateMachine");
  stateList.add(&buttonTestState);
  stateList.add(&menuState);
  stateList.add(&networkTestState);
  stateMachine.addStates(stateList);
  stateMachine.setCurrentState(menuState.id());
}

void initJoystick() {
  pinMode(JOYSTICK_DOWN, INPUT_PULLUP);
  pinMode(JOYSTICK_UP, INPUT_PULLUP);
  pinMode(JOYSTICK_LEFT, INPUT_PULLUP);
  pinMode(JOYSTICK_RIGHT, INPUT_PULLUP);
  pinMode(JOYSTICK_MID, INPUT_PULLUP);

  attachInterrupt(JOYSTICK_DOWN, onJoystickDown, FALLING);
  attachInterrupt(JOYSTICK_UP, onJoystickUp, FALLING);
  attachInterrupt(JOYSTICK_LEFT, onJoystickLeft, FALLING);
  attachInterrupt(JOYSTICK_RIGHT, onJoystickRight, FALLING);
  attachInterrupt(JOYSTICK_MID, onJoystickMid, FALLING);
}

/* CALLBACK FUNCTIONS FOR STATES */
void buttonTest() {
  if (jController.locked()) {
    jController.unlock();
    jController.setMid(false);
  }

  jController.unlock();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(2,32);
  display.clearDisplay();
  display.println("Press the button ...");
  display.display();
  display.clearDisplay();
  display.setCursor(50,32);

  if (jController.mid()) {
    jController.setMid(false);
    jController.lock();
    display.setCursor(2,32);
    display.println("switch back to menu");
    display.display();
    delay(2000);
    stateMachine.setCurrentState(menuState.id());
  }

  if (jController.down()) {
    display.println("DOWN");
    jController.setDown(false); 
    display.display();
    delay(500);
  }

  if (jController.up()) {
    display.println("UP");
    jController.setUp(false);
    display.display();
    delay(500);
  }

  if (jController.left()) {
    display.println("LEFT");
    jController.setLeft(false);
    display.display();
    delay(500);
  }

  if (jController.right()) {
    display.println("RIGHT");
    jController.setRight(false);
    display.display();
    delay(500);
  }
}

void menuTest() {
  if (jController.locked()) {
    jController.unlock();
  }
  menu->display();
}

void networkTest() {
  if (jController.locked()) {
    jController.unlock();
    jController.setMid(false);
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(32,32);
  display.clearDisplay();
  display.println("NETWORKTEST");
  display.display();


  if (jController.mid()) {
    jController.setMid(false);
    jController.lock();
    display.clearDisplay();
    display.setCursor(2,32);
    display.println("back to menu");
    display.display();
    delay(2000);
    stateMachine.setCurrentState(menuState.id());
  }
}

/* CALLBACK FUNCTIONS FOR MENU ITEMS */
void btnTestMenuItemCallback() {
  stateMachine.setCurrentState(buttonTestState.id());
}
void nwTestMenuItemCallback() {
  stateMachine.setCurrentState(networkTestState.id());
}


/* MAIN APPLICATION LOGIC */
void setup() {
  Serial.begin(9600);
  Serial.println("START");
  Serial.print("Setup running on core: "); Serial.println(xPortGetCoreID());
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  initJoystick();
  initMenu();
  initStateMachine();

  xTaskCreatePinnedToCore(
    AppTask,
    "App",
    8000,
    NULL,
    1,
    NULL,
    1
  );
}

void loop() {
  // appController.run(); // starte den appController task
}

void AppTask( void *pvParameters ) {
  Serial.print("AppTask running on core: "); Serial.println(xPortGetCoreID());
  (void) pvParameters;
  appController.run();
}
