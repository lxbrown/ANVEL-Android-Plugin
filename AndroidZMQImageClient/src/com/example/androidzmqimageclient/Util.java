package com.example.androidzmqimageclient;

import android.os.Handler;
import android.os.Bundle;
import android.os.Message;
 
public class Util {
	public static final String MESSAGE_PAYLOAD_KEY = "jeromq-service-payload";
	
	public static Message bundledMessage(Handler uiThreadHandler, byte[] msg) {
		Message m = uiThreadHandler.obtainMessage();
	    prepareMessage(m, msg);
	    return m;
	};
	
	public static void prepareMessage(Message m, byte[] msg){
	      Bundle b = new Bundle();
	      b.putByteArray(MESSAGE_PAYLOAD_KEY, msg);
	      m.setData(b);
	      return ;      
	}
}
