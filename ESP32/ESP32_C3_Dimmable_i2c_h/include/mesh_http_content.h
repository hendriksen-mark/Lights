#ifndef MESH_HTTP_CONTENT_H
#define MESH_HTTP_CONTENT_H

#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "custom_log.h"

// External references to variables from mesh.cpp
extern int subip;
extern IPAddress bridgeIp;
extern int bridgePort;
extern byte target;
extern byte target_ont;
extern byte current_ont;
extern int state_ont;
extern bool fout;
extern WebServer server_gordijn;

inline void handleRoot()
{
  String message = "<!DOCTYPE HTML>";
  message += "<html>";
  message += "<h1 align=center>Curtain control over ethernet+mesh</h1><br><br>";
  message += "set Target Pos  = ";
  message += target;
  message += "<br>";
  message += "rep Target Pos  = ";
  message += target_ont;
  message += "<br>";
  message += "rep Current Pos  = ";
  message += current_ont;
  message += "<br>";
  message += "rep State  = ";
  message += state_ont;
  message += "<br>";
  message += "error  = ";
  message += fout;
  message += "<br><br>";

  message += "<form action=\"/setTargetPosTest/\">";
  message += "SET Target";
  message += "<input type=\"range\" name=\"Pos\" min=\"0\" max=\"100\" value=\"" + (String)target + "\" step=\"1\" class=\"slider\">";
  // message += "<input type=\"text\"  name=\"Pos\" value=\"" + (String)target + "\">";
  message += "<input type=\"submit\" value=\"Submit\">";
  message += "</form>";
  message += "Current Target  = ";
  message += target;

  // message += "<a href=\"//setTargetPosTEST\"\"><button>SET Target Pos TEST</button></a>"; //aanpassen voor invullen

  message += "<br><br>";

  message += "<a href=\"/CurrentPosTest\"\"><button>GET Current Pos TEST</button></a>";
  message += "<a href=\"/getTargetPosTest\"\"><button>GET Target Pos TEST</button></a>";
  message += "<a href=\"/StateTest\"\"><button>GET State TEST</button></a>";

  message += "<a href=\"/Home\"\"><button>Home curtain</button></a>";

  message += "<br><br>";
  message += "Links voor Homebridge";
  message += "<br><br>";

  message += "<a href=\"/setTargetPos/?Pos=50\"\"><button>SET Target Pos 50%</button></a>";
  message += "<a href=\"/CurrentPos\"\"><button>GET Current Pos</button></a>";
  message += "<a href=\"/getTargetPos\"\"><button>GET Target Pos</button></a>";
  message += "<a href=\"/State\"\"><button>GET State</button></a>";

  message += "<h1 align=center>Set IP for mesh command</h1><br><br>";

  message += "<form action=\"/setIP/\">";
  message += "SET IP";
  message += "<input type=\"text\"  name=\"subip\" value=\"" + (String)subip + "\">";
  message += "<input type=\"submit\" value=\"Submit\">";
  message += "</form>";
  message += "Current IP = ";
  message += bridgeIp.toString();

  message += "<br><br>";
  message += "<form action=\"/setPORT/\">";
  message += "SEET PORT";
  message += "<input type=\"text\"  name=\"subport\" value=\"" + (String)bridgePort + "\">";
  message += "<input type=\"submit\" value=\"Submit\">";
  message += "</form>";
  message += "Current PORT = ";
  message += String(bridgePort);

  message += "<br><br>";
  message += "<form action=\"/discover/\">";
  message += "<input type=\"submit\" value=\"Discover DIYhue Bridge\">";
  message += "</form>";

  message += "<br><br>";
  message += "<a href=\"/info\"\"><button>Info</button></a>";
  message += "<a href=\"/ota\"\"><button>OTA Update</button></a>";
  message += "<a href=\"/\"\"><button>RELOAD PAGE</button></a><br/>";
  message += "<br><br>";
  message += "<a href=\"/reset\"\"><button>RESET</button></a><br/>";

  message += "</html>";
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/", "args:", server_gordijn.args());
  server_gordijn.send(200, "text/html", message);
}

#endif // MESH_HTTP_CONTENT_H
