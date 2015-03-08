/*
 *  Arduino True Random Number Generator.
 *
 *  Copyright (C) 2010 Efstathios Chatzikyriakidis (contact@efxa.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// include related libraries for ethernet and webduino web server.
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>

// sound sensor analog input pin number.
const int sensorPin = 0;

// number of samples to get from sound sensor.
const int sensorSamples = 400;

// delay time between sound sensor samples.
const long sampleDelay = 25;

// no-cost printing stream operator.
template<class T>
inline Print & operator << (Print & obj, T arg) {
  obj.print(arg);
  return obj;
}

// mac address for the arduino ethernet shield.
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// ip address for the arduino ethernet shield.
static uint8_t ip[] = { 192, 168, 0, 1 };

// the webduino web server.
WebServer webserver("" , 80); // don't use PREFIX for root directory!

// the web root directory.
#define WWW_ROOT "/"

// web interface xhtml page changeable messages.
#define PAGE_HEADER "Arduino True Random Number Generator - V1.0"
#define PAGE_TAG "The randomness comes from the sound size of environment measured by a sound sensor."
#define PAGE_FOOTER "Copyright (C) 2010 Efstathios Chatzikyriakidis ( GNU General Public License Version 3 )"

// web interface xhtml page header content.
P(contentHeaderHtml) = "<!DOCTYPE html\n"
                       "   PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
                       "   \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                       "\n"
                       "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>\n"
                       " <head>\n"
                       "  <meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />\n"
                       "  <title>"PAGE_HEADER"</title>\n"
                       "  <style type='text/css'>\n"
                       "   body {\n"
                       "     margin: 0px;\n"
                       "     font: normal small Arial, Helvetica, sans-serif;\n"
                       "     background-color: #CFCFB0;\n"
                       "   }\n"
                       "\n"
                       "   .logo {\n"
                       "     text-transform: uppercase;\n"
                       "     font-size: 25px;\n"
                       "     color: #323844;\n"
                       "     font-weight: bold;\n"
                       "     text-decoration: none;\n"
                       "   }\n"
                       "\n"
                       "   .tag {\n"
                       "     margin: 0;\n"
                       "     font-size: 13px;\n"
                       "     font-weight: bold;\n"
                       "     color: #656558;\n"
                       "     letter-spacing: 2px;\n"
                       "     text-decoration: none;\n"
                       "   }\n"
                       "\n"
                       "   .content {\n"
                       "     text-align: justify;\n"
                       "     line-height: 120%;\n"
                       "     padding: 10px;\n"
                       "   }\n"
                       "\n"
                       "   .heading {\n"
                       "     text-transform: uppercase;\n"
                       "     font-size: 14px;\n"
                       "     font-weight: bold;\n"
                       "     color: #81816D;\n"
                       "   }\n"
                       "\n"
                       "   .border {\n"
                       "     border: 1px solid #8E8E79;\n"
                       "   }\n"
                       "\n"
                       "   .redbox {\n"
                       "     background-color: #FF5800;\n"
                       "     border-bottom: 1px solid #F1F1D8;\n"
                       "     border-right: 1px solid #F1F1D8;\n"
                       "     border-left: 1px solid #F1F1D8;\n"
                       "   }\n"
                       "  </style>\n"
                       " </head>\n"
                       " <body>\n"
                       "  <table width='780' border='0' align='center' cellpadding='0' cellspacing='0'>\n"
                       "   <tr>\n"
                       "    <td class='redbox'>&nbsp;</td>\n"
                       "   </tr>\n"
                       "   <tr><td>&nbsp;</td></tr>\n"
                       "   <tr>\n"
                       "    <td align='center'>\n"
                       "     <table border='0' cellspacing='0' cellpadding='0'>\n"
                       "      <tr>\n"
                       "       <td align='center'><a href='/' class='logo'>"PAGE_HEADER"</a></td>\n"
                       "      </tr>\n"
                       "      <tr><td height='3'></td></tr>\n"
                       "      <tr>\n"
                       "       <td align='center'><a href='/' class='tag'>"PAGE_TAG"</a></td>\n"
                       "      </tr>\n"
                       "     </table>\n"
                       "    </td>\n"
                       "   </tr>\n"
                       "   <tr><td>&nbsp;</td></tr>\n"
                       "   <tr>\n"
                       "    <td bgcolor='#F3F3F3' class='border'>\n"
                       "     <table border='0' cellspacing='0' cellpadding='0'>\n"
                       "      <tr>\n"
                       "       <td class='content'>\n";

// web interface xhtml page footer content.
P(contentFooterHtml) = "       </td>\n"
                       "      </tr>\n"
                       "     </table>\n"
                       "    </td>\n"
                       "   </tr>\n"
                       "   <tr><td>&nbsp;</td></tr>\n"
                       "   <tr>\n"
                       "    <td align='center' bgcolor='#E2E2D2' class='border' height='40'>\n"
                       "     "PAGE_FOOTER"\n"
                       "    </td>\n"
                       "   </tr>\n"
                       "   <tr><td>&nbsp;</td></tr>\n"
                       "  </table>\n"
                       " </body>\n"
                       "</html>";

// root index page function handler.
void
rootPageCmd(WebServer &server, WebServer::ConnectionType type, char *urlTail, bool tailComplete) {
  // sends the standard "we're all OK" headers back to the browser.
  server.httpSuccess();

  // GET/POST: send html page, HEAD: send nothing. */
  if (type != WebServer::HEAD) {
    // send html page to the browser.
    server.printP(contentHeaderHtml);

    // fetch true random numbers.
    for (int i = 0; i < sensorSamples; i++) {
      // read the sample from the sound sensor.
      int sample = analogRead(sensorPin);

      // write the value to the page.
      server << sample << " ";

      // wait some time before the next sample.
      delay(sampleDelay);
    }

    server.printCRLF();
    server.printP(contentFooterHtml);
  }
}

// failure page function handler.
void
failurePageCmd(WebServer &server, WebServer::ConnectionType type, char *urlTail, bool tailComplete) {
  // sends the standard "we're all OK" headers back to the browser.
  server.httpSuccess();

  // GET/POST: send html page, HEAD: send nothing. */
  if (type != WebServer::HEAD) {
    // store the core message of the page.
    P(failurePageMsg) = "Failure: Page Not Found!";

    // send html page to the browser.
    server.printP(contentHeaderHtml);
    server.printP(failurePageMsg);
    server.printCRLF();
    server.printP(contentFooterHtml);
  }
}

// startup point entry (runs once).
void
setup() {
  // initialize the ethernet device.
  Ethernet.begin(mac, ip);

  // set the default and failure pages of the web server.
  webserver.setDefaultCommand(&rootPageCmd);
  webserver.setFailureCommand(&failurePageCmd);

  // begin running the web server.
  webserver.begin();
}

// loop the main sketch.
void
loop() {
  // process any request from the web server.
  webserver.processConnection();
}
