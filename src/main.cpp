//The calibration basic is make using map in raw value 
//https://github.com/Bodmer/TFT_eWidget
//Graph test

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>             // Display
#include <TFT_eWidget.h>          // Widgets
#include <XPT2046_Touchscreen.h>  // Touch screen

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);
//XPT2046_Touchscreen touch(XPT2046_CS, 255);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240,touchScreenMaximumY = 3800;

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

TFT_eSPI tft = TFT_eSPI();
//TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

// Crear un botón
TFT_eSPI_Button myButton;

//Crear grafico
GraphWidget gr = GraphWidget(&tft);    // Graph widget

// Traces are drawn on tft using graph instance
TraceWidget tr1 = TraceWidget(&gr);    // Graph trace 1
TraceWidget tr2 = TraceWidget(&gr);    // Graph trace 2


void updateButton(TFT_eSPI_Button &btn, TFT_eSPI &tft,
                  const char* labelConst, bool pressed,
                  int16_t x, int16_t y,
                  int16_t w, int16_t h,
                  uint16_t outline, uint16_t fill, uint16_t textColor,
                  uint8_t textSize = 2) {
  char* label = (char*)labelConst;
  btn.initButton(&tft, x, y, w, h, outline, fill, textColor, label, textSize);
  btn.drawButton(pressed);
}


bool state = false;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.invertDisplay(true); //becouse the driver is inverter in color
  tft.setRotation(1);
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(mySpi);
  touch.setRotation(1);

  tft.fillScreen(TFT_BLACK);
  int x = 320 / 2; // center of display
  int y = 100;
  int fontSize = 1;
  // Crear botón inicialmente con texto "ON"
  updateButton(myButton, tft, "ON", false,
               250, 210, 80, 40,     // x, y, width, height
               TFT_WHITE, TFT_BLUE, TFT_WHITE,
               1);


  //Crera y configurar grafico
   // Graph area is 250 pixels wide, 150 high, dark grey background
  gr.createGraph(260, 150, tft.color565(20, 20, 20));

  // x scale units is from 0 to 100, y scale units is -50 to 50
  gr.setGraphScale(0.0, 100.0, -50.0, 50.0);

  // X grid starts at 0 with lines every 10 x-scale units
  // Y grid starts at -50 with lines every 25 y-scale units
  // blue grid
  gr.setGraphGrid(0.0, 10.0, -50.0, 25.0, TFT_BLUE);

  // Draw empty graph, top left corner at 30,15 on TFT
  gr.drawGraph(30, 15);

  // Start a trace with using red and another with green
  tr1.startTrace(TFT_RED);
  tr2.startTrace(TFT_GREEN);

  // Add points on graph to trace 1 using graph scale factors
  tr1.addPoint(0.0, 0.0);
  tr1.addPoint(100.0, 0.0);

  // Add points on graph to trace 2 using graph scale factors
  // Points are off graph so the plotted line is clipped to graph area
  tr2.addPoint(0.0, -100.0);
  tr2.addPoint(100.0, 100.0);

  // Get x,y pixel coordinates of any scaled point on graph
  // and ring that point.
  tft.drawCircle(gr.getPointX(50.0), gr.getPointY(0.0), 5, TFT_MAGENTA);

  // Draw the x axis scale
  tft.setTextDatum(TC_DATUM); // Top centre text datum
  tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(-50.0) + 3);
  tft.drawNumber(50, gr.getPointX(50.0), gr.getPointY(-50.0) + 3);
  tft.drawNumber(100, gr.getPointX(100.0), gr.getPointY(-50.0) + 3);

  // Draw the y axis scale
  tft.setTextDatum(MR_DATUM); // Middle right text datum
  tft.drawNumber(-50, gr.getPointX(0.0), gr.getPointY(-50.0));
  tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(0.0));
  tft.drawNumber(50, gr.getPointX(0.0), gr.getPointY(50.0));

  // Restart traces with new colours
  tr1.startTrace(TFT_WHITE);
  tr2.startTrace(TFT_YELLOW);


}

void loop() {
   //For the graph
  static uint32_t plotTime = millis();
  static float gx = 0.0, gy = 0.0;
  static float delta = 7.0;


        // Sample periodically
  if (  (millis() - plotTime >= 10) && state) {
    plotTime = millis();

    // Add a new point on each trace
    tr1.addPoint(gx, gy);
    tr2.addPoint(gx, gy/3.0); // half y amplitude

    // Create next plot point
    gx += 1.0;
    gy += delta;
    if (gy >  50.0) { delta = -7.0; gy =  50.0; }
    if (gy < -50.0) { delta =  7.0; gy = -50.0; }

    // If the end of the graph is reached start 2 new traces
    if (gx > 100.0) {
      gx = 0.0;
      gy = 0.0;

      // Draw empty graph at 40,10 on display
      gr.drawGraph(30, 15);
      // Start new trace
      tr1.startTrace(TFT_YELLOW);
      tr2.startTrace(TFT_GREEN);
    }
  }
   
//Touch loop
  bool pressed = touch.touched();
  if (pressed) {

    TS_Point p = touch.getPoint();
    //bool pressed = tft.getTouch(&x, &y);  // o tu método de lectura táctil

    // Mapear coordenadas si es necesario, dependiendo de tu rotación
    //Basic calibration 
    if(p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
    if(p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
    if(p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
    if(p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;
    //Map this to the pixel position
    p.x = map(p.x,touchScreenMinimumX,touchScreenMaximumX,1,screenWidth); /* Touchscreen X calibration */
    p.y = map(p.y,touchScreenMinimumY,touchScreenMaximumY,1,screenHeight); /* Touchscreen Y calibration */

    int x = p.x;
    int y = p.y;
    Serial.printf("Touch at: %d, %d\n", x, y);

    myButton.press(pressed && myButton.contains(x, y));
    if (myButton.justPressed()) {
      state = !state;
      // Cambiar texto y color según estado
      const char* newText = state ? "OFF" : "ON";
      uint16_t fillColor = state ? TFT_RED : TFT_BLUE;
      // Actualizar botón
      updateButton(myButton, tft, newText, true,
                  250, 210, 80, 40,
                  TFT_WHITE, fillColor, TFT_WHITE,
                  1);
      delay(100);  // pequeño retardo visual
      myButton.drawButton(false);  // volver al estado no presionado
      Serial.println(newText);
      myButton.press(false); //esto es importante para evitar que el boton se quede activo despues del juspressed
    }
    delay(100); // evitar rebotes
  }
}
