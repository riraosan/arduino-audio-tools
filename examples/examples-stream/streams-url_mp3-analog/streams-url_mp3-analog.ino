/**
 * @file streams-url_mp3-i2s.ino
 * @author Phil Schatzmann
 * @brief decode MP3 stream from url and output it on I2S
 * @version 0.1
 * @date 2021-96-25
 * 
 * @copyright Copyright (c) 2021
 */

// set this in AudioConfig.h or here after installing https://github.com/pschatzmann/arduino-libhelix.git
#define USE_HELIX 

#include "AudioTools.h"
#include "AudioCodecs/CodecMP3Helix.h"

using namespace audio_tools;  

URLStream url("ssid","password");
I2SStream i2s; // final output of decoded stream
EncodedAudioStream dec(&i2s, new MP3DecoderHelix()); // Decoding stream
StreamCopy copier(dec, url); // copy url to decoder


void setup(){
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);  

  // setup i2s
  auto config = i2s.defaultConfig(TX_MODE);
  i2s.begin(config);

  // setup I2S based on sampling rate provided by decoder
  dec.setNotifyAudioChange(i2s);
  dec.begin();

// mp3 radio
  url.begin("https://centralcharts.ice.infomaniak.ch/centralcharts-128.mp3","audio/mp3");

}

void loop(){
  copier.copy();
}
