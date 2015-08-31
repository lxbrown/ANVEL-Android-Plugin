package com.example.androidzmqimageclient;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
 
public class MainActivity extends Activity implements SensorEventListener {
    private ImageView image;
    private TextView waiting;
    private Button setIP;
    private Button forward;
    private Button back;
    private Button left;
    private Button right;
    private Button forleft;
    private Button forright;
    private Button backleft;
    private Button backright;
    private Button close;
    private Button start;
    private Button control;
    private SensorManager senSensorManager;
    private Sensor senAccelerometer;
    private long lastUpdate = 0;
    Context context;
    boolean firstUpdate;
    boolean accelerometer;
    
    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
    	if(accelerometer) {
	    	Sensor mySensor = sensorEvent.sensor;
	    	
	    	if(mySensor.getType() == Sensor.TYPE_ACCELEROMETER) {
	    		float x = sensorEvent.values[0];
	    		float y = sensorEvent.values[1];
	    		float z = sensorEvent.values[2];
	    		
	    		long curTime = System.currentTimeMillis();
	    		
	    		if((curTime - lastUpdate) > 100) {
	    			lastUpdate = curTime;
	    			
	    			if(x > 2 && y > 2) {
	    				ZeroMQSend.direction = "t";
	    			}
	    			else if(x > 2 && y < -2) {
	    				ZeroMQSend.direction = "e";
	    			}
	    			else if(x < -2 && y > 2) {
	    				ZeroMQSend.direction = "w";
	    			}
	    			else if(x < -2 && y < -2) {
	    				ZeroMQSend.direction = "q";
	    			}
	    			else if(x > 2) {
	    				ZeroMQSend.direction = "b";
	    			}
	    			else if(x < -2) {
	    				ZeroMQSend.direction = "f";
	    			}
	    			else if(y > 2) {
	    				ZeroMQSend.direction = "r";
	    			}
	    			else if(y < -2) {
	    				ZeroMQSend.direction = "l";
	    			}
	    			else {
	    				ZeroMQSend.direction = "s";
	    			}
	    		}
	    	}
    	}
    }
    
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    	
    }
 
    //Update the screen when a message is received
    private void clientMessageReceived(byte[] messageBody) {
    		updateScreen(messageBody);
    }
    
    //A close command was sent so the app needs to be restarted
    private void frameMessageReceived(byte[] messageBody) {
		Intent mStartActivity = new Intent(context, MainActivity.class);
		int mPendingIntentId = 123456;
		PendingIntent mPendingIntent = PendingIntent.getActivity(context, mPendingIntentId, 
				mStartActivity, PendingIntent.FLAG_CANCEL_CURRENT);
		AlarmManager mgr = (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);
		mgr.set(AlarmManager.RTC, System.currentTimeMillis() + 100, mPendingIntent);
		System.exit(0);	
    }
    
	//Update the ImageView to show the most recent camera feed image
    private void updateScreen(byte[] bytes) {
    	//Once a feed is achieved, make buttons visible and remove "waiting" text
    	if(firstUpdate) {
	    	getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	    	waiting.setVisibility(View.GONE);
	    	setIP.setClickable(false);
	    	setIP.setAlpha(0);
	    	control.setClickable(true);
	    	control.setAlpha(220);
	    	forward.setClickable(true);
	    	forward.setAlpha(220);
	    	back.setClickable(true);
	    	back.setAlpha(220);
	    	left.setClickable(true);
	    	left.setAlpha(220);
	    	right.setClickable(true);
	    	right.setAlpha(220);
	    	forleft.setClickable(true);
	    	forleft.setAlpha(220);
	    	forright.setClickable(true);
	    	forright.setAlpha(220);
	    	backleft.setClickable(true);
	    	backleft.setAlpha(220);
	    	backright.setClickable(true);
	    	backright.setAlpha(220);
	    	close.setClickable(true);
	    	close.setAlpha(220);
	    	firstUpdate = false;
    	}
    	
    	//Set incoming byte array to a bitmap and display it
    	Bitmap bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
    	image.setImageBitmap(bitmap);    	
    }
    
	private final MessageListenerHandler clientMessageHandler = new MessageListenerHandler(
            new IMessageListener() {
                @Override
                public void messageReceived(byte[] messageBody) {
                    clientMessageReceived(messageBody);
                }
            },
            Util.MESSAGE_PAYLOAD_KEY);
	
	private final MessageListenerHandler frameMessageHandler = new MessageListenerHandler(
            new IMessageListener() {
                @Override
                public void messageReceived(byte[] messageBody) {
                    frameMessageReceived(messageBody);
                }
            },
            Util.MESSAGE_PAYLOAD_KEY);
          
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        context = this;
        firstUpdate = true;
        accelerometer = false;
        
        image = (ImageView) findViewById(R.id.test_image);
        waiting = (TextView) findViewById(R.id.textView2);
        setIP = (Button) findViewById(R.id.button1);
        forward = (Button) findViewById(R.id.forward);
        back = (Button) findViewById(R.id.back);
        left = (Button) findViewById(R.id.left);
        right = (Button) findViewById(R.id.right);
        forleft = (Button) findViewById(R.id.fl);
        forright = (Button) findViewById(R.id.fr);
        backleft = (Button) findViewById(R.id.bl);
        backright = (Button) findViewById(R.id.br);
        close = (Button) findViewById(R.id.close);
        start = (Button) findViewById(R.id.start);
        control = (Button) findViewById(R.id.control);
        
        senSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        senAccelerometer = senSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        senSensorManager.registerListener(this,  senAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
        
        //Open dialog box
        setIP.setOnClickListener(new View.OnClickListener() {			
			@Override
			public void onClick(View v) {
				new Prompt(context, waiting, start, setIP, clientMessageHandler, frameMessageHandler);
			}
		});
        
        control.setOnClickListener(new View.OnClickListener() {
        	@Override
        	public void onClick(View v) {
        		if(!accelerometer) {
        			accelerometer = true;
        	    	forward.setClickable(false);
        	    	forward.setAlpha(0);
        	    	back.setClickable(false);
        	    	back.setAlpha(0);
        	    	left.setClickable(false);
        	    	left.setAlpha(0);
        	    	right.setClickable(false);
        	    	right.setAlpha(0);
        	    	forleft.setClickable(false);
        	    	forleft.setAlpha(0);
        	    	forright.setClickable(false);
        	    	forright.setAlpha(0);
        	    	backleft.setClickable(false);
        	    	backleft.setAlpha(0);
        	    	backright.setClickable(false);
        	    	backright.setAlpha(0);
        	    	control.setText("Button Control");
        		}
        		else {
        			accelerometer = false;
        	    	forward.setClickable(true);
        	    	forward.setAlpha(220);
        	    	back.setClickable(true);
        	    	back.setAlpha(220);
        	    	left.setClickable(true);
        	    	left.setAlpha(220);
        	    	right.setClickable(true);
        	    	right.setAlpha(220);
        	    	forleft.setClickable(true);
        	    	forleft.setAlpha(220);
        	    	forright.setClickable(true);
        	    	forright.setAlpha(220);
        	    	backleft.setClickable(true);
        	    	backleft.setAlpha(220);
        	    	backright.setClickable(true);
        	    	backright.setAlpha(220);
        	    	control.setText("Accelerometer Control");
        		}
        	}
        });
        
        //The following eight buttons set the direction to be sent
        //and will send it as long as the button is pressed
        forward.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "f";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        back.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "b";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        left.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "l";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        right.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "r";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        forleft.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "q";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        forright.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "w";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        backleft.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "e";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        backright.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					ZeroMQSend.direction = "t";
				break;
				case MotionEvent.ACTION_UP:
					ZeroMQSend.direction = "s";
				}
				return true;
			}
        });
        
        //Send a "c" which signifies the app is ready for the
        //program to close
        close.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				ZeroMQSend.direction = "c";			
			}

        });
      
        setIP.performClick();
    }
}