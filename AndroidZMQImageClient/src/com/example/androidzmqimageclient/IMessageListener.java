package com.example.androidzmqimageclient;

public interface IMessageListener {
	void messageReceived(byte[] messageBody);
}
