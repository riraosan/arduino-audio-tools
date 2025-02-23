/**
 * @file player-url-kit.ino
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-audiokit/player-url-audiokit/README.md
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

// set this in AudioConfig.h or here after installing https://github.com/pschatzmann/arduino-libhelix.git
#define USE_HELIX 

#include "AudioTools.h"
#include "AudioCodecs/CodecMP3Helix.h"
#include "AudioLibs/AudioKit.h"

using namespace audio_tools;  

const char *urls[] = {
  "http://centralcharts.ice.infomaniak.ch/",
  "http://centraljazz.ice.infomaniak.ch/",
  "http://centralrock.ice.infomaniak.ch/",
  "http://centralcountry.ice.infomaniak.ch/",
  "http://centralfunk.ice.infomaniak.ch/"
};
const char *wifi = "wifi";
const char *password = "password";

ICYStream urlStream(wifi, password);
AudioSourceURL source(urlStream, urls, "audio/mp3");
AudioKitStream kit;
MP3DecoderHelix decoder;
AudioPlayer player(source, kit, decoder);

void next(bool, int, void*) {
   player.next();
}

void previous(bool, int, void*) {
   player.previous();
}

void stopResume(bool, int, void*){
  if (player.isActive()){
    player.stop();
  } else{
    player.play();
  }
}

// Arduino setup
void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // setup output
  auto cfg = kit.defaultConfig(TX_MODE);
  kit.begin(cfg);

  // setup navigation
  kit.addAction(PIN_KEY4, next);
  kit.addAction(PIN_KEY3, previous);

  // setup player
  player.setVolume(0.7);
  player.begin();
}

// Arduino loop
void loop() {
  player.copy();
  kit.processActions();
}