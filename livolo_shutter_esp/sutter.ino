
 void openShutter(void) {
  actuateShutter(true);
}

void closeShutter(void) {
  actuateShutter(false);
}

void actuateShutter(bool openValve) {
#if DEBUG
  Serial.print(openShutter ? F("Open") : F("Close")); Serial.println(F(" the Shutter."));
#endif
  if (openValve){
	 gLivolo.sendButton(LIVOLO_REMOTE_ID, LIVOLO_OPEN_BUTTON);
  } else {
 	 gLivolo.sendButton(LIVOLO_REMOTE_ID, LIVOLO_CLOSE_BUTTON);

  }
}
