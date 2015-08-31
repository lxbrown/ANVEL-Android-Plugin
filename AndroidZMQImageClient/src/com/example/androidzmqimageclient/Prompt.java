package com.example.androidzmqimageclient;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

//Dialog box to set IP address upon program starting
public class Prompt {
	static String ip;
	MessageListenerHandler clientMessageHandler;
	MessageListenerHandler frameMessageHandler;
	static boolean confirmed;
	Context context;
	Button start, setIP;
	TextView waiting;
	
	Prompt(Context context_, TextView waiting_, Button start_, 
			Button setIP_, MessageListenerHandler clientMessageHandler_,
			MessageListenerHandler frameMessageHandler_) {
		start = start_;
		setIP = setIP_;
		waiting = waiting_;
		confirmed = false;
		clientMessageHandler = clientMessageHandler_;
		frameMessageHandler = frameMessageHandler_;
		context = context_;
		
		ip = "192.168.1.148";
		
        start.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
		        new Thread(new ZeroMQSend(frameMessageHandler, ip)).start();
		        new Thread(new ZeroMQReceive(clientMessageHandler, ip)).start();
		        start.setAlpha(0);
		        start.setClickable(false);
		        setIP.setAlpha(0);
		        setIP.setClickable(false);
		        waiting.setAlpha(255);		
			}
		});
        
		// get prompts.xml view
		LayoutInflater li = LayoutInflater.from(context);
		View promptsView = li.inflate(R.layout.prompts, null);
	
		AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
				context);
	
		// set prompts.xml to alertdialog builder
		alertDialogBuilder.setView(promptsView);
	
		final EditText userInput = (EditText) promptsView
				.findViewById(R.id.editTextDialogUserInput);
		userInput.setText(ip);
	
		// set dialog message
		alertDialogBuilder
			.setCancelable(false)
			.setPositiveButton("Enter",
			  new DialogInterface.OnClickListener() {
			    public void onClick(DialogInterface dialog,int id) {
					ip = userInput.getText().toString();
					start.setAlpha(255);
					start.setClickable(true);
			    }
			  });
	
		// create alert dialog
		AlertDialog alertDialog = alertDialogBuilder.create();
	
		// show it
		alertDialog.show();
	}
}
