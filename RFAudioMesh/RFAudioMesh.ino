#include <RF24.h>
#include <SPI.h>
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <EEPROM.h>
#include <RF24Audio.h>
#include "printf.h" // General includes for radio and audio lib

RF24 radio(7,8); // Set radio up using pins 7 (CE) 8 (CS)
RF24Audio rfAudio(radio,1); // Set up the audio using the radio, and set to radio number 0.
 // Setting the radio number is only important if one-to-one communication is desired
 // in a multi-node radio group. See the privateBroadcast() function.

RF24Network network(radio);
RF24Mesh mesh(radio, network);


/** 
 * User Configuration: 
 * nodeID - A unique identifier for each radio. Allows addressing to change dynamically
 * with physical changes to the mesh. (numbers 1-255 allowed)
 * 
 * otherNodeID - A unique identifier for the 'other' radio
 * 
 **/
#define nodeID 1
#define otherNodeID 2    


uint32_t millisTimer=0;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};


void setup() { 
 Serial.begin(115200); // Enable Arduino serial library
 
 printf_begin(); // Radio library uses printf to output debug info 
 radio.begin(); // Must start the radio here, only if we want to print debug info
 radio.printDetails(); // Print the info
 // Set the nodeID
  mesh.setNodeID(nodeID);  
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
 rfAudio.begin(); // Start up the radio and audio libararies

 rfAudio.transmit();
 rfAudio.receive();
 rfAudio.setVolume(7); // max vol
}

void loop() {
 
 // Audio playback and button handling is all managed internally.
 // In this example, the radio is controlled by external buttons, so there is nothing to do here

  
  mesh.update();
  if(network.available()){
        RF24NetworkHeader header;
        uint32_t mills;
        network.read(header,&mills,sizeof(mills));
        Serial.print("Rcv "); Serial.print(mills);
        Serial.print(" from nodeID ");
        int _ID = mesh.getNodeID(header.from_node);
        if( _ID > 0){
           Serial.println(_ID);
        }else{
           Serial.println("Mesh ID Lookup Failed"); 
        }
  }
  
  
  // Send to the other node every second
  if(millis() - millisTimer >= 1000){
    millisTimer = millis();
    
    // Send an 'M' type to other Node containing the current millis()
    if(!mesh.write(&millisTimer,'M',sizeof(millisTimer),otherNodeID)){
            if( ! mesh.checkConnection() ){
              Serial.println("Renewing Address");
              mesh.renewAddress(); 
            }else{
              Serial.println("Send fail, Test OK"); 
            }
    }
  }
}
