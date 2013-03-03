//NETWORK SETUP
static uint8_t mac[6] = { 
  0xDE, 0xCA, 0xFB, 0xAD, 0x00, 0x22 };
// CHANGE THIS TO MATCH YOUR HOST NETWORK
static uint8_t ip[4] = { 
//  192, 168, 1, 250 }; 
  169, 254, 184, 250 }; 
static uint8_t  gateway[4] = { 
//  192, 168, 1, 254 }; 
  169, 254, 184, 244 }; 


char* getOutletState() {
  if (outletState == HIGH) {
    return "On"; 
  } 
  else { 
    return "Off";
  }
}

/* commands are functions that get called by the webserver framework
 * they can read any posted data from client, and they output to the
 * server to send data back to the web browser. */
void statusCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  //The HTML Header   
  P(headerOpen) = 
    "<!DOCTYPE html><html><head>"
    "<title>6004 Sous Vide</title>"
    "<link href='http://jqueryui.com/latest/themes/base/ui.all.css' rel=stylesheet />"
    "<style>"
    "p,h2,.pad{padding: 0pt 0.7em;}"
    "#powerStatus{float:right;width:22px;height:20px;border:none;}"
    ".Off{background-color:crimson}"
    ".On{background-color:limegreen}"
    "</style>"
    "</head>"
    "<body style='font-size:62.5%;width:300px; margin-left:auto;margin-right:auto;'>"
    "<div class='ui-widget'>"
    "<div class='ui-widget-header'><div id='powerStatus' class='";

  P(headerClose) =
    "'><span class='ui-icon ui-icon-power'></span></div>"
    "<h2>Current Status</h2>"
    "</div>"
    "<div class='ui-widget-content'>"
    "<p>";
    
  P(setTempForm) = 
    "</p><form method='post'><p><label for='setTemp'>Set Temperature (c)</label>&nbsp;<input type='text' name='setTemp' size='8' value='";
  P(footer) = "'><input type='submit' value='set'></p></form></div></div></body></html>";

  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      /* readURLParam returns false when there are no more parameters
       * to read from the input.  We pass in buffers for it to store
       * the name and value strings along with the length of those
       * buffers. */
      repeat = server.readPOSTparam(name, 16, value, 16);

      /* this is a standard string comparison function.  It returns 0
       * when there's an exact match.  We're looking for a parameter
       * named "pwrToggle" here. */
      if (strcmp(name, "setTemp") == 0)
      {
        pidSetpoint = atof(value);
      }
    } 
    while (repeat);

    // after procesing the POST data, tell the web browser to reload
    // the page using a GET method. 
    server.httpSeeOther(PREFIX);
    return;
  }

  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();

  /* we don't output the body for a HEAD request */
  if (type == WebServer::GET )
  {
    /* store the HTML in program memory using the P macro */

    server.printP(headerOpen);
    server << getOutletState();
    server.printP(headerClose);
    server << "<strong>Setpoint C: </strong>" << dtostrf(pidSetpoint,7,2,currentTempString) << "<br>";
    server << "<strong>Setpoint F: </strong>" << dtostrf(DallasTemperature::toFahrenheit(pidSetpoint),7,2,currentTempString) << "<br><br>";
    server << "<strong>Power: </strong>" << getOutletState() << "<br>";
    server << "<strong>Temp C: </strong>" << dtostrf(currentTempC,7,2,currentTempString) << "<br>";
    server << "<strong>Temp F: </strong>" << dtostrf(DallasTemperature::toFahrenheit(currentTempC),7,2,currentTempString) << "<br>";
    server.printP(setTempForm);
    server << dtostrf(pidSetpoint,0,2,currentTempString);
    server.printP(footer);
  }
}

void webServerSetup(){
  
  /* initialize the Ethernet adapter */
  Ethernet.begin(mac, ip, gateway);

   /* setup our default command that will be run when the user accesses
   * the root page on the server */
  webserver.setDefaultCommand(&statusCmd);

  /* run the same command if you try to load /index.html, a common
   * default page name */
  webserver.addCommand("index.html", &statusCmd);

  /* start the webserver */
  webserver.begin(); 
}
