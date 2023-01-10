// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "lock.h"

int serial_port;
int stop_bg = 0;
int bg_running = 0;

void setup(){
	serial_port = open("/dev/ttyS0", O_RDWR);

	// Check for errors
	if (serial_port < 0) {
		printf("Error %i from open: %s\n", errno, strerror(errno));
	}

	struct termios tty;
	
	if(tcgetattr(serial_port, &tty) != 0) {
    		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}

	tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag |= CS8; // 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 0;
	cfsetispeed(&tty, B57600);
	cfsetospeed(&tty, B57600);
	
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
 		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}

	return;	
}

void send_packet(unsigned char data[], int len){
	unsigned char data_to_send[len];
	for (int i = 0; i < len-2; i++) {
		data_to_send[i] = data[i];
    	}
	unsigned char checksum = 0x00;
	for (int i = 6; i < len - 2; i++){
		checksum = checksum + data_to_send[i];	
	}
	data_to_send[len-2] = 0x00;
	data_to_send[len-1] = checksum;
	write(serial_port, data_to_send, len);
	return;
}

unsigned char *get_packet(int size){
	unsigned char *returnArray = NULL;
	returnArray = calloc(size, sizeof(unsigned char));
	for (int i = 0; i < size; i++){
		unsigned char byte;
		int j = read(serial_port, &byte, 1);
		returnArray[i] = byte;
	}
	
	return returnArray;
}

void set_led(int color, int mode){
	unsigned char msg[16] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x07,0x35};
	msg[10] = mode;
	msg[11] = 0x80;
	msg[12] = color;
	msg[13] = 0x00;
	msg[14] = 0xCC;
	msg[15] = 0xCC;
	send_packet(msg,sizeof(msg));
	get_packet(12);
}

void check_sensor(){
	unsigned char msg[12] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x40,0xCC,0xCC};
	send_packet(msg,sizeof(msg));
	get_packet(12);
}

void ReadSysPar(){	
	unsigned char msg[12] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x0f,0xCC,0xCC};
	send_packet(msg,sizeof(msg));
	get_packet(28);
}

unsigned char GenImg(){
	unsigned char msg[12] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x01,0xCC,0xCC};
	send_packet(msg,sizeof(msg));
	unsigned char *data = get_packet(12);
	unsigned char returnVal = data[9];
	free(data);
	return returnVal;
}

unsigned char ImageToCharFile(int charFile){
	unsigned char msg[13] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x04,0x02,0x01,0xCC,0xCC};
	msg[10]=charFile;
	send_packet(msg,sizeof(msg));
	get_packet(12);
}

void GenerateTemplate(){
	unsigned char msg[12] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x05,0xCC,0xCC};
	send_packet(msg,sizeof(msg));
	get_packet(12);
}

void Search(){
	unsigned char msg[17] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x08,0x04,0x01,0x00,0x01,0x00,0x03,0xCC,0xCC};
	send_packet(msg, sizeof(msg));
	unsigned char *return_data = get_packet(16);
	if (return_data[9] == 0){
		set_led(2,3);
		int fd, ret;
		fd = open("//dev//lockdev", O_RDWR);
		if (fd < 0) {
			printf("Can't open device file: %s\n", DEVICE_NAME);
		}
		else{
			ret = ioctl(fd, LOCK_TOGGLE);
		}
		close(fd);
		printf("\nMatch found\n");
		sleep(2);
		set_led(1,4);
	}else{
		set_led(1,3);
		printf("\nNo Match found\n");
		sleep(2);
		set_led(1,4);
	}	
	free(return_data);
}

void ReadIndexTable(int page){
	unsigned char msg[13] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x04,0x1F,0x00,0xCC,0xCC};
	msg[10] = page;
	send_packet(msg,sizeof(msg));
	unsigned char *return_data = get_packet(44);
	printf("\nIndex Data:\n");
	for (int i = 10; i < 42; i++){
		printf("0x%x\n", return_data[i]);
	}
	printf("\n");
}

void LEDTest(){
	set_led(1, 4);
	sleep(2);
	set_led(1, 3);
	sleep(2);
	set_led(2, 3);
	sleep(2);
	set_led(3, 3);
	sleep(2);
	set_led(1, 4);
}

void check_finger(){
	set_led(3,3);
	int found = 0;
	while(found == 0 && stop_bg == 0){
		unsigned char imageRes = GenImg();
		if (imageRes == 0){
			found = 1;
		}
	}
	if (!stop_bg){
		ImageToCharFile(1);	
		GenerateTemplate();
		Search();
	}
}

int GetImageFromScanner(){
	set_led(3,3);
	printf("\nPlace Finger on Fingerprint scanner\n");
	int length = 15;
	int counter;
	for(counter = 0; counter < length; counter++){
		printf(".");
		fflush(stdout);
		unsigned char imageRes = GenImg();
		if (imageRes == 0){
			break;
		}else{
			sleep(1);
		}
	}
	if (counter >= length){
		printf("\nTimeout\n");
		return 0;
	}else{
		return 1;

	}
}

unsigned char StoreTemplate(int bufferID, int index){
	unsigned char msg[15] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x06,0x06,0xAA,0xBB,0xBB,0xCC,0xCC};
	msg[10] = bufferID;
	msg[11] = index >> 8;
	msg[12] = index & 0xFF;
	send_packet(msg, sizeof(msg));
	unsigned char *return_data = get_packet(12);
	unsigned char return_code = return_data[9];
	free(return_data);
	return return_code;
}

void register_finger(int index){
	int wasFingerOn = GetImageFromScanner();
	if (wasFingerOn == 1){
		ImageToCharFile(1);
		printf("\nRemove finger from Scanner\n");
		int empty = 0;
		while(empty == 0){
			unsigned char imageRes = GenImg();
			if (imageRes != 0){
				empty = 1;
			}
		}
		wasFingerOn = GetImageFromScanner();
		if (wasFingerOn == 1){
			ImageToCharFile(2);
			GenerateTemplate();
			unsigned char return_code = StoreTemplate(1, index);
			switch(return_code){
				case 0:
					printf("\nStored Finger success in ID %i\n", index);
					break;
				default:
					printf("\nError Storing Fingerprint (Error Code 0x%x)\n", return_code);
					break;	
			}
		}
	}
	set_led(1,4);
}

void delete_finger(int index){
	unsigned char msg[16] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x07,0x0c,0xAA,0xAA,0x00,0x01,0xCC,0xCC};
	msg[10] = index >> 8;
	msg[11] = index & 0xFF;
	send_packet(msg, sizeof(msg));
	unsigned char *return_data = get_packet(12);
	unsigned char return_code = return_data[9];
	free(return_data);
	switch(return_code){
		case 0:
			printf("\nSuccessfully deleted fingerprint id %i\n", index);
			break;
		default:
			printf("\nError deleting fingerprint id %i (Error Code 0x%x)\n", index, return_code);
		       	break;	
	}
	return;
}

void empty_fingerstore(){
	unsigned char msg[12] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x0d,0xCC,0xCC};
	send_packet(msg, sizeof(msg));
	unsigned char *return_data = get_packet(12);
	unsigned char return_code = return_data[9];
	free(return_data);
	switch(return_code){
		case 0:
			printf("\nSuccessfully wiped fingerprint store\n");
			break;
		default:
			printf("\nError deleting fingerprint store (Error Code 0x%x)\n", return_code);
		       	break;	
	}
	return;
	
}

void *bg_thread(){
	printf("\nStarting BG\n");
	while(stop_bg == 0){
		check_finger();	
	}
	stop_bg = 0;
	printf("\nStopping BG\n");
	bg_running = 0;
}

void main(int argc, char *argv[]) {
	setup();
	set_led(1,4);

	char *ip = "52.91.154.13";
  	int port = 50505;

  	int sock;
  	struct sockaddr_in addr;
  	socklen_t addr_size;
  	char buffer[1024];
  	int n;

 	sock = socket(AF_INET, SOCK_STREAM, 0);
  	if (sock < 0){
    		perror("[-]Socket error");
		close(serial_port);
    		exit(1);
 	}
  	printf("[+]TCP server socket created.\n");

  	memset(&addr, '\0', sizeof(addr));
  	addr.sin_family = AF_INET;
  	addr.sin_port = htons(port);
  	addr.sin_addr.s_addr = inet_addr(ip);

  	if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        	printf("connection with the server failed...\n");
		close(serial_port);
        	exit(1);
    	}
    	else{
        	printf("connected to the server..\n");
    	}

  	bzero(buffer, 1024);
	strcpy(buffer, "RPIClientConnect");
  	send(sock, buffer, strlen(buffer), 0);

  	while(1){
  		bzero(buffer, 1024);
  		recv(sock, buffer, sizeof(buffer), 0);
  		printf("Server: %s\n", buffer);

		int was_bg = 0;

		if (bg_running){
			stop_bg = 1;
			was_bg = 1;
			while(bg_running){
				sleep(1);
			}
		}

		if (strcmp(buffer, "exit\n") == 0){
            		close(sock);
            		break;
        	}else if(strcmp(buffer, "check\n") == 0){
			check_finger();
		}else if(strcmp(buffer, "reg\n") == 0){
			bzero(buffer, 1024);
			strcpy(buffer, "ack");
			send(sock, buffer, strlen(buffer), 0);
			bzero(buffer, 1024);
			recv(sock, buffer, sizeof(buffer), 0);
			int reg_index = atoi(buffer);
			register_finger(reg_index);
		}else if(strcmp(buffer, "index\n") == 0){
			ReadIndexTable(0);
		}else if(strcmp(buffer, "ledTest\n") == 0){
			LEDTest();
		}else if(strcmp(buffer, "delete\n") == 0){
			bzero(buffer, 1024);
			strcpy(buffer, "ack");
			send(sock, buffer, strlen(buffer), 0);
			bzero(buffer, 1024);
			recv(sock, buffer, sizeof(buffer), 0);
			int del_index = atoi(buffer);
			delete_finger(del_index);
		}else if(strcmp(buffer, "empty\n") == 0){
			empty_fingerstore();
		}else if(strcmp(buffer, "bg\n") == 0){
			pthread_t thread_id;
			bg_running = 1;
			pthread_create(&thread_id, NULL, bg_thread, NULL);

		}else{
			printf("\nError: Invalid mode\n");
		}

		if (was_bg){
			pthread_t thread_id1;
			bg_running = 1;
			pthread_create(&thread_id1, NULL, bg_thread, NULL);
		}

  	}

	set_led(1,4);
	close(serial_port);
	return;
}
