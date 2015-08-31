package com.example.androidzmqimageclient;

import org.jeromq.ZMQ;

import android.os.Handler;

//Sends the android controller data to the program
public class ZeroMQSend implements Runnable {
    private final Handler uiThreadHandler;
    String ip;
    public static volatile String direction;
    

    public ZeroMQSend(Handler uiThreadHandler, String ip_) {
        this.uiThreadHandler = uiThreadHandler;
        ip = ip_;
        direction = "s";   
    }

    @Override
    public void run() {
    	//Set up socket and connect
        ZMQ.Context context = ZMQ.context(1);
        ZMQ.Socket socket = context.socket(ZMQ.PAIR);
        
    	socket.connect("tcp://" + ip + ":5555");

        while(!Thread.currentThread().isInterrupted()) {
        	//Force receive empty data to keep sockets synchronized
        	socket.recv(0);
        	
        	//Send the data
        	socket.send(direction.getBytes());
        	
        	//Inform MainActivity that it needs to restart the app
        	if(direction == "c")
        		uiThreadHandler.sendMessage(
            				Util.bundledMessage(uiThreadHandler, direction.getBytes()));
        }
    }
}